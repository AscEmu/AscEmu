/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "Raid_The_Dreamrift.hpp"

#include "Objects/Units/Creatures/Creature.h"
#include "Server/Script/CreatureAIScript.hpp"
#include "Server/Script/InstanceScript.hpp"
#include "Server/Script/ScriptMgr.hpp"
#include "Utilities/Random.hpp"

#include <cmath>
#include <cstddef>
#include <utility>
#include <vector>

enum DreamriftEvents
{
    // Stage One: Insatiable Hunger
    EVENT_CHIMAERUS_RENDING_TEAR = 1,
    EVENT_CHIMAERUS_CAUSTIC_PHLEGM,
    EVENT_CHIMAERUS_RIFT_EMERGENCE,
    EVENT_CHIMAERUS_CONSUMING_MIASMA,
    EVENT_CHIMAERUS_ALNDUST_UPHEAVAL,

    // Stage Two: To The Skies
    EVENT_CHIMAERUS_CORRUPTED_DEVASTATION,
    EVENT_CHIMAERUS_SKY_CAUSTIC_PHLEGM,
    EVENT_CHIMAERUS_SKY_CONSUMING_MIASMA,
    EVENT_CHIMAERUS_RAVENOUS_DIVE,

    EVENT_MANIFESTATION_DRAW_CHECK
};

enum DreamriftActions
{
    ACTION_CHIMAERUS_DEVOUR = 1
};

namespace
{
    float const DreamriftTwoPi = 6.28318531f;

    // kit spells of the manifestations use their own event range
    uint32_t const DreamriftKitEventBase = 1000;

    uint32_t const ChimaerusBerserkMs = 540000;

    // one energy per second, at full energy Chimaerus takes to the skies
    uint32_t const ChimaerusFullEnergy = 100;
    uint32_t const ChimaerusSkiesMs = 30000;

    // manifestations close to Chimaerus are devoured
    float const ChimaerusDevourReach = 6.0f;
    float const ChimaerusSearchRange = 150.0f;
    uint32_t const ManifestationStepMs = 1000;

    enum ChimaerusStage : uint8_t
    {
        CHIMAERUS_STAGE_NONE = 0,
        CHIMAERUS_STAGE_HUNGER,
        CHIMAERUS_STAGE_SKIES
    };

    uint32_t const ChimaerusEntries[2] = { BOSS_CHIMAERUS, BOSS_CHIMAERUS_ALT };
    uint32_t const CoalescedManifestations[3] = { NPC_COALESCED_SWARMING_SHADE, NPC_COALESCED_HAUNTING_ESSENCE, NPC_COALESCED_COLOSSAL_HORROR };

    enum DreamriftAim : uint8_t
    {
        AIM_VICTIM,
        AIM_PLAYER,
        AIM_SELF,
        AIM_AROUND
    };

    struct DreamriftKitSpell
    {
        uint32_t spellId;
        DreamriftAim aim;
        uint32_t firstMs;
        uint32_t repeatMs;
    };

    LocationVector dreamriftPointAround(LocationVector const& origin, float minRange, float maxRange)
    {
        float const angle = Util::getRandomFloat(DreamriftTwoPi);
        float const range = Util::getRandomFloat(minRange, maxRange);
        return LocationVector(origin.x + range * std::cos(angle), origin.y + range * std::sin(angle), origin.z, angle);
    }

    // the fighting Chimaerus closest to a manifestation
    Creature* findHungryChimaerus(CreatureAIScript* pSeeker, float range)
    {
        for (uint32_t entry : ChimaerusEntries)
        {
            Creature* pChimaerus = pSeeker->findNearestCreature(entry, range);
            if (pChimaerus != nullptr && pChimaerus->isAlive() && pChimaerus->isInCombat())
                return pChimaerus;
        }

        return nullptr;
    }
}

//////////////////////////////////////////////////////////////////////////////////////////
// Instance

class DreamriftInstanceScript : public InstanceScript
{
public:
    explicit DreamriftInstanceScript(WorldMap* pMapMgr) : InstanceScript(pMapMgr)
    {
        setBossNumber(DreamriftEncounterCount);
    }

    static InstanceScript* Create(WorldMap* pMapMgr) { return new DreamriftInstanceScript(pMapMgr); }

    std::vector<Creature*> livingCoalescedManifestations()
    {
        std::vector<Creature*> manifestations;
        for (uint32_t entry : CoalescedManifestations)
        {
            for (Creature* pManifestation : getCreatureSetForEntry(entry))
            {
                if (pManifestation->isAlive())
                    manifestations.push_back(pManifestation);
            }
        }

        return manifestations;
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Shared combat base: event clock, berserk and target helpers

class DreamriftCombatAI : public CreatureAIScript
{
public:
    explicit DreamriftCombatAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        mBerserkLeftMs = 0;
        onEngage();
    }

    void OnCombatStop(Unit* /*pTarget*/) override
    {
        scriptEvents.resetEvents();
        summons.despawnAll();
        mBerserkLeftMs = 0;
        onDisengage();
    }

    void onSummonedCreature(Creature* pSummon) override { summons.summon(pSummon); }

    void AIUpdate(unsigned long time_passed) override
    {
        if (!_isInCombat())
            return;

        uint32_t const diff = static_cast<uint32_t>(time_passed);
        scriptEvents.updateEvents(diff, 0);
        tickBerserk(diff);
        onCombatTick(diff);

        // due events wait while a cast runs
        while (!_isCasting())
        {
            uint32_t const eventId = scriptEvents.getFinishedEvent();
            if (eventId == 0)
                break;

            onEvent(eventId);
        }
    }

protected:
    virtual void onEngage() {}
    virtual void onDisengage() {}
    virtual void onCombatTick(uint32_t /*diff*/) {}
    virtual void onEvent(uint32_t /*eventId*/) {}

    DreamriftInstanceScript* getDreamrift()
    {
        if (getCreature()->GetMapId() != MAP_THE_DREAMRIFT)
            return nullptr;

        return static_cast<DreamriftInstanceScript*>(getInstanceScript());
    }

    // random player, the tank only when nobody else is left
    Unit* pickPlayer(bool skipTank = false)
    {
        if (skipTank)
        {
            FilterArgs args(TargetFilter_Player);
            args.addFilter(TargetFilter_NotCurrent);
            if (Unit* pTarget = selectUnitTarget(args))
                return pTarget;
        }

        return selectUnitTarget(FilterArgs(TargetFilter_Player));
    }

    Unit* castAtPlayer(uint32_t spellId, bool skipTank = false, bool triggered = false)
    {
        Unit* pTarget = pickPlayer(skipTank);
        if (pTarget != nullptr)
            castSpell(pTarget, spellId, triggered);

        return pTarget;
    }

    // summoned add that joins the fight at once
    Creature* callAdd(uint32_t entry, LocationVector const& spot)
    {
        Creature* pAdd = summonCreature(entry, spot, CORPSE_DESPAWN);
        if (pAdd != nullptr)
            setZoneWideCombat(pAdd);

        return pAdd;
    }

    LocationVector aroundSelf(float minRange, float maxRange)
    {
        return dreamriftPointAround(getCreature()->GetPosition(), minRange, maxRange);
    }

    void warn(char const* text) { sendChatMessage(CHAT_MSG_RAID_BOSS_EMOTE, 0, text); }

    void armBerserk(uint32_t delayMs) { mBerserkLeftMs = delayMs; }

private:
    void tickBerserk(uint32_t diff)
    {
        if (mBerserkLeftMs == 0)
            return;

        if (mBerserkLeftMs > diff)
        {
            mBerserkLeftMs -= diff;
            return;
        }

        mBerserkLeftMs = 0;
        castSpellOnSelf(SPELL_DREAMRIFT_BERSERK, true);
    }

    uint32_t mBerserkLeftMs = 0;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Chimaerus the Undreamt God

class DreamriftChimaerusAI : public DreamriftCombatAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new DreamriftChimaerusAI(c); }
    explicit DreamriftChimaerusAI(Creature* pCreature) : DreamriftCombatAI(pCreature) {}

    void OnCombatStart(Unit* pTarget) override
    {
        if (DreamriftInstanceScript* pDreamrift = getDreamrift())
        {
            pDreamrift->sendUnitEncounter(EncounterFrameEngage, getCreature());
            pDreamrift->setBossState(DATA_CHIMAERUS, EncounterStates::InProgress);
        }

        DreamriftCombatAI::OnCombatStart(pTarget);
    }

    void OnCombatStop(Unit* pTarget) override
    {
        if (getCreature()->isAlive())
        {
            if (DreamriftInstanceScript* pDreamrift = getDreamrift())
            {
                pDreamrift->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
                pDreamrift->setBossState(DATA_CHIMAERUS, EncounterStates::Failed);
            }
        }

        DreamriftCombatAI::OnCombatStop(pTarget);
    }

    void OnDied(Unit* /*pKiller*/) override
    {
        scriptEvents.resetEvents();
        summons.despawnAll();

        if (DreamriftInstanceScript* pDreamrift = getDreamrift())
        {
            pDreamrift->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
            pDreamrift->setBossState(DATA_CHIMAERUS, EncounterStates::Performed);
        }
    }

    // a manifestation reached the Insatiable god
    void DoAction(int32_t action) override
    {
        if (action != ACTION_CHIMAERUS_DEVOUR || !_isInCombat() || !isAlive())
            return;

        castSpellOnSelf(SPELL_CHIMAERUS_CONSUME, true);
        castSpellOnSelf(SPELL_CHIMAERUS_CANNIBALIZED_ESSENCE, true);
    }

    // out of reach while in the skies
    void DamageTaken(Unit* /*pAttacker*/, uint32_t* damage) override
    {
        if (damage != nullptr && mStage == CHIMAERUS_STAGE_SKIES)
            *damage = 0;
    }

protected:
    void onEngage() override
    {
        armBerserk(ChimaerusBerserkMs);
        castSpellOnSelf(SPELL_CHIMAERUS_INSATIABLE, true);
        enterHunger();
    }

    void onDisengage() override
    {
        mStage = CHIMAERUS_STAGE_NONE;
        setRooted(false);
        _setMeleeDisabled(false);
        _removeAura(SPELL_CHIMAERUS_INSATIABLE);
        _removeAura(SPELL_CHIMAERUS_RIFT_SHROUD);
    }

    void onCombatTick(uint32_t diff) override
    {
        if (mStage != CHIMAERUS_STAGE_HUNGER)
            return;

        mEnergyClockMs += diff;
        while (mEnergyClockMs >= 1000)
        {
            mEnergyClockMs -= 1000;
            ++mEnergy;
        }

        if (mEnergy >= ChimaerusFullEnergy && !_isCasting())
            takeToTheSkies();
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_CHIMAERUS_RENDING_TEAR:
                castSpellOnVictim(SPELL_CHIMAERUS_RENDING_TEAR);
                scriptEvents.addEvent(EVENT_CHIMAERUS_RENDING_TEAR, 14000);
                break;
            case EVENT_CHIMAERUS_CAUSTIC_PHLEGM:
                castAtPlayer(SPELL_CHIMAERUS_CAUSTIC_PHLEGM, true);
                scriptEvents.addEvent(EVENT_CHIMAERUS_CAUSTIC_PHLEGM, 18000);
                break;
            case EVENT_CHIMAERUS_RIFT_EMERGENCE:
                warn("Manifestations emerge from the Rift and are drawn towards Chimaerus!");
                castSpellAOE(SPELL_CHIMAERUS_RIFT_EMERGENCE);
                coalesce(1, 1, isHeroic() ? 4 : 3);
                scriptEvents.addEvent(EVENT_CHIMAERUS_RIFT_EMERGENCE, 40000);
                break;
            case EVENT_CHIMAERUS_CONSUMING_MIASMA:
                castAtPlayer(SPELL_CHIMAERUS_CONSUMING_MIASMA, true);
                scriptEvents.addEvent(EVENT_CHIMAERUS_CONSUMING_MIASMA, 30000);
                break;
            case EVENT_CHIMAERUS_ALNDUST_UPHEAVAL:
                warn("Chimaerus wrenches its foes into the Rift with an Alndust Upheaval!");
                castSpellAOE(SPELL_CHIMAERUS_ALNDUST_UPHEAVAL);
                scriptEvents.addEvent(EVENT_CHIMAERUS_ALNDUST_UPHEAVAL, 45000);
                break;
            case EVENT_CHIMAERUS_CORRUPTED_DEVASTATION:
                castAtPlayer(SPELL_CHIMAERUS_CORRUPTED_DEVASTATION, false, true);
                scriptEvents.addEvent(EVENT_CHIMAERUS_CORRUPTED_DEVASTATION, 10000);
                break;
            case EVENT_CHIMAERUS_SKY_CAUSTIC_PHLEGM:
                castAtPlayer(SPELL_CHIMAERUS_SKY_CAUSTIC_PHLEGM, false, true);
                scriptEvents.addEvent(EVENT_CHIMAERUS_SKY_CAUSTIC_PHLEGM, 8000);
                break;
            case EVENT_CHIMAERUS_SKY_CONSUMING_MIASMA:
                castAtPlayer(SPELL_CHIMAERUS_SKY_CONSUMING_MIASMA, false, true);
                scriptEvents.addEvent(EVENT_CHIMAERUS_SKY_CONSUMING_MIASMA, 15000);
                break;
            case EVENT_CHIMAERUS_RAVENOUS_DIVE:
                ravenousDive();
                break;
            default:
                break;
        }
    }

private:
    void enterHunger()
    {
        mStage = CHIMAERUS_STAGE_HUNGER;
        mEnergy = 0;
        mEnergyClockMs = 0;

        scriptEvents.addEvent(EVENT_CHIMAERUS_RENDING_TEAR, 8000);
        scriptEvents.addEvent(EVENT_CHIMAERUS_CAUSTIC_PHLEGM, 14000);
        scriptEvents.addEvent(EVENT_CHIMAERUS_RIFT_EMERGENCE, 18000);
        scriptEvents.addEvent(EVENT_CHIMAERUS_CONSUMING_MIASMA, 26000);
        scriptEvents.addEvent(EVENT_CHIMAERUS_ALNDUST_UPHEAVAL, 35000);
    }

    void takeToTheSkies()
    {
        mStage = CHIMAERUS_STAGE_SKIES;
        scriptEvents.resetEvents();
        getCreature()->interruptSpell();
        warn("Chimaerus takes to the skies with Corrupted Devastation!");

        setRooted(true);
        _setMeleeDisabled(true);
        castSpellOnSelf(SPELL_CHIMAERUS_RIFT_SHROUD, true);
        coalesce(1, 2, isHeroic() ? 5 : 4);

        scriptEvents.addEvent(EVENT_CHIMAERUS_CORRUPTED_DEVASTATION, 2000);
        scriptEvents.addEvent(EVENT_CHIMAERUS_SKY_CAUSTIC_PHLEGM, 6000);
        scriptEvents.addEvent(EVENT_CHIMAERUS_SKY_CONSUMING_MIASMA, 12000);
        scriptEvents.addEvent(EVENT_CHIMAERUS_RAVENOUS_DIVE, ChimaerusSkiesMs);
    }

    // dives back down, normal difficulty devours what is left of the manifestations
    void ravenousDive()
    {
        scriptEvents.resetEvents();
        warn("Chimaerus plunges down in a Ravenous Dive!");
        _removeAura(SPELL_CHIMAERUS_RIFT_SHROUD);
        setRooted(false);
        _setMeleeDisabled(false);
        castAtPlayer(SPELL_CHIMAERUS_RAVENOUS_DIVE, false, true);

        if (!isHeroic())
        {
            if (DreamriftInstanceScript* pDreamrift = getDreamrift())
            {
                for (Creature* pManifestation : pDreamrift->livingCoalescedManifestations())
                {
                    pManifestation->Despawn(200, 0);
                    castSpellOnSelf(SPELL_CHIMAERUS_CANNIBALIZED_ESSENCE, true);
                }
            }
        }

        enterHunger();
    }

    // horrors and essences fight, shades crawl to the god
    void coalesce(uint32_t horrors, uint32_t essences, uint32_t shades)
    {
        for (uint32_t i = 0; i < horrors; ++i)
            callAdd(NPC_COALESCED_COLOSSAL_HORROR, aroundSelf(20.0f, 30.0f));

        for (uint32_t i = 0; i < essences; ++i)
            callAdd(NPC_COALESCED_HAUNTING_ESSENCE, aroundSelf(20.0f, 30.0f));

        for (uint32_t i = 0; i < shades; ++i)
            summonCreature(NPC_COALESCED_SWARMING_SHADE, aroundSelf(25.0f, 35.0f), CORPSE_DESPAWN);
    }

    uint8_t mStage = CHIMAERUS_STAGE_NONE;
    uint32_t mEnergy = 0;
    uint32_t mEnergyClockMs = 0;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Coalesced Manifestations

// fights with a list of timed spells, coalesced ones are devoured near Chimaerus
class DreamriftManifestationAI : public DreamriftCombatAI
{
public:
    DreamriftManifestationAI(Creature* pCreature, std::vector<DreamriftKitSpell> kit, uint32_t arrivalSpellId)
        : DreamriftCombatAI(pCreature), mKit(std::move(kit)), mArrivalSpellId(arrivalSpellId) {}

    void OnSummon(Unit* /*pSummoner*/) override { mCoalesced = true; }

protected:
    void onEngage() override
    {
        for (size_t i = 0; i < mKit.size(); ++i)
            scriptEvents.addEvent(DreamriftKitEventBase + static_cast<uint32_t>(i), mKit[i].firstMs);

        if (mArrivalSpellId != 0)
            castSpellAOE(mArrivalSpellId, true);

        scriptEvents.addEvent(EVENT_MANIFESTATION_DRAW_CHECK, ManifestationStepMs);
    }

    void onEvent(uint32_t eventId) override
    {
        if (eventId == EVENT_MANIFESTATION_DRAW_CHECK)
        {
            if (mCoalesced && devouredByChimaerus())
                return;

            scriptEvents.addEvent(EVENT_MANIFESTATION_DRAW_CHECK, ManifestationStepMs);
            return;
        }

        if (eventId < DreamriftKitEventBase || eventId >= DreamriftKitEventBase + mKit.size())
            return;

        DreamriftKitSpell const& entry = mKit[eventId - DreamriftKitEventBase];
        switch (entry.aim)
        {
            case AIM_VICTIM:
                castSpellOnVictim(entry.spellId);
                break;
            case AIM_PLAYER:
                castAtPlayer(entry.spellId);
                break;
            case AIM_SELF:
                castSpellOnSelf(entry.spellId);
                break;
            default:
                castSpellAOE(entry.spellId);
                break;
        }

        scriptEvents.addEvent(eventId, entry.repeatMs);
    }

private:
    bool devouredByChimaerus()
    {
        Creature* pChimaerus = findHungryChimaerus(this, ChimaerusDevourReach);
        if (pChimaerus == nullptr)
            return false;

        if (pChimaerus->GetScript() != nullptr)
            pChimaerus->GetScript()->DoAction(ACTION_CHIMAERUS_DEVOUR);

        scriptEvents.resetEvents();
        despawn(200, 0);
        return true;
    }

    std::vector<DreamriftKitSpell> mKit;
    uint32_t mArrivalSpellId;
    bool mCoalesced = false;
};

class DreamriftColossalHorrorAI : public DreamriftManifestationAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new DreamriftColossalHorrorAI(c); }
    explicit DreamriftColossalHorrorAI(Creature* pCreature)
        : DreamriftManifestationAI(pCreature, {
            { SPELL_HORROR_COLOSSAL_STRIKES, AIM_VICTIM, 6000, 15000 },
            { SPELL_HORROR_DISCORDANT_ROAR, AIM_AROUND, 12000, 20000 } }, SPELL_MANIFESTATION_RIFT_SICKNESS) {}
};

class DreamriftHauntingEssenceAI : public DreamriftManifestationAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new DreamriftHauntingEssenceAI(c); }
    explicit DreamriftHauntingEssenceAI(Creature* pCreature)
        : DreamriftManifestationAI(pCreature, {
            { SPELL_ESSENCE_ESSENCE_BOLT, AIM_PLAYER, 3000, 6000 },
            { SPELL_ESSENCE_FEARSOME_CRY, AIM_AROUND, 10000, 22000 } }, 0) {}
};

// crawls to Chimaerus and feeds him unless killed on the way
class DreamriftSwarmingShadeAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new DreamriftSwarmingShadeAI(c); }
    explicit DreamriftSwarmingShadeAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnSummon(Unit* /*pSummoner*/) override
    {
        mDrawn = true;
        setReactState(REACT_PASSIVE);
        _setMeleeDisabled(true);
    }

    void AIUpdate(unsigned long time_passed) override
    {
        if (!mDrawn || mSpent || !isAlive())
            return;

        mStepMs += static_cast<uint32_t>(time_passed);
        if (mStepMs < ManifestationStepMs)
            return;

        mStepMs = 0;

        Creature* pChimaerus = findHungryChimaerus(this, ChimaerusSearchRange);
        if (pChimaerus == nullptr)
        {
            mSpent = true;
            despawn(500, 0);
            return;
        }

        if (getCreature()->getDistance(pChimaerus) <= ChimaerusDevourReach)
        {
            mSpent = true;
            if (pChimaerus->GetScript() != nullptr)
                pChimaerus->GetScript()->DoAction(ACTION_CHIMAERUS_DEVOUR);

            despawn(200, 0);
            return;
        }

        LocationVector const destination = pChimaerus->GetPosition();
        moveTo(destination.x, destination.y, destination.z, false);
    }

private:
    uint32_t mStepMs = 0;
    bool mDrawn = false;
    bool mSpent = false;
};

void SetupTheDreamrift(ScriptMgr* mgr)
{
    mgr->register_instance_script(MAP_THE_DREAMRIFT, &DreamriftInstanceScript::Create);

    mgr->register_creature_script(BOSS_CHIMAERUS, &DreamriftChimaerusAI::Create);
    mgr->register_creature_script(BOSS_CHIMAERUS_ALT, &DreamriftChimaerusAI::Create);

    mgr->register_creature_script(NPC_RIFT_COLOSSAL_HORROR, &DreamriftColossalHorrorAI::Create);
    mgr->register_creature_script(NPC_RIFT_HAUNTING_ESSENCE, &DreamriftHauntingEssenceAI::Create);
    mgr->register_creature_script(NPC_RIFT_SWARMING_SHADE, &DreamriftSwarmingShadeAI::Create);

    mgr->register_creature_script(NPC_COALESCED_COLOSSAL_HORROR, &DreamriftColossalHorrorAI::Create);
    mgr->register_creature_script(NPC_COALESCED_HAUNTING_ESSENCE, &DreamriftHauntingEssenceAI::Create);
    mgr->register_creature_script(NPC_COALESCED_SWARMING_SHADE, &DreamriftSwarmingShadeAI::Create);
}
