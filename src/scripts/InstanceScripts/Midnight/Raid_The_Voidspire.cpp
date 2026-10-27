/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "Raid_The_Voidspire.hpp"

#include "Objects/Units/Creatures/Creature.h"
#include "Server/Script/CreatureAIScript.hpp"
#include "Server/Script/InstanceScript.hpp"
#include "Server/Script/ScriptMgr.hpp"
#include "Utilities/Random.hpp"

#include <cmath>
#include <cstddef>
#include <utility>
#include <vector>

enum VoidspireEvents
{
    // Imperator Averzian
    EVENT_AVERZIAN_SHADOWS_ADVANCE = 1,
    EVENT_AVERZIAN_MARCH_OF_THE_ENDLESS,
    EVENT_AVERZIAN_UMBRAL_COLLAPSE,
    EVENT_AVERZIAN_DARK_UPHEAVAL,
    EVENT_AVERZIAN_BLACKENING_WOUNDS,
    EVENT_AVERZIAN_OBLIVIONS_WRATH,
    EVENT_AVERZIAN_VOID_MARKED,
    EVENT_AVERZIAN_GLORY_CHECK,
    EVENT_VOIDSHAPER_GATHERING_DARKNESS,
    EVENT_VOIDSHAPER_TRANSFORM,
    EVENT_SOLDIER_PITCH_BULWARK,

    // Vorasius
    EVENT_VORASIUS_SHADOWCLAW_SLAM,
    EVENT_VORASIUS_PARASITE_EXPULSION,
    EVENT_VORASIUS_VOID_BREATH,
    EVENT_VORASIUS_PRIMORDIAL_ROAR,
    EVENT_VORASIUS_PULSE_CHECK,

    // Vaelgor & Ezzorak
    EVENT_TWINS_MIDNIGHT_FLAMES,
    EVENT_TWINS_SWAP,
    EVENT_TWINS_TWILIGHT_FURY,
    EVENT_TWINS_GRAPPLING_MAW,
    EVENT_VAELGOR_NULLBEAM,
    EVENT_VAELGOR_VAELWING,
    EVENT_VAELGOR_DREAD_BREATH,
    EVENT_EZZORAK_GLOOM,
    EVENT_EZZORAK_RAKFANG,
    EVENT_EZZORAK_VOID_HOWL,

    // Fallen-King Salhadaar
    EVENT_SALHADAAR_VOID_CONVERGENCE,
    EVENT_SALHADAAR_SHATTERING_TWILIGHT,
    EVENT_SALHADAAR_FRACTURED_PROJECTION,
    EVENT_SALHADAAR_DESPOTIC_COMMAND,
    EVENT_SALHADAAR_TWISTING_OBSCURITY,

    // Lightblinded Vanguard
    EVENT_VENEL_JUDGMENT,
    EVENT_VENEL_FINAL_VERDICT,
    EVENT_VENEL_DIVINE_STORM,
    EVENT_VENEL_SACRED_TOLL,
    EVENT_BELLAMY_AVENGERS_SHIELD,
    EVENT_BELLAMY_JUDGMENT,
    EVENT_BELLAMY_SHIELD_OF_THE_RIGHTEOUS,
    EVENT_SENN_EXORCISM,
    EVENT_SENN_SEARING_RADIANCE,
    EVENT_SENN_SACRED_SHIELD,
    EVENT_SENN_ELEKK_CHARGE,

    // Crown of the Cosmos
    EVENT_ALLERIA_SILVERSTRIKE_ARROW,
    EVENT_ALLERIA_GRASP_OF_EMPTINESS,
    EVENT_ALLERIA_VOID_EXPULSION,
    EVENT_ALLERIA_NULL_CORONA,
    EVENT_ALLERIA_VOID_DROPLETS,
    EVENT_ALLERIA_SILVERSTRIKE_BARRAGE,
    EVENT_ALLERIA_STELLAR_EMISSION,
    EVENT_ALLERIA_SINGULARITY_ERUPTION,
    EVENT_ALLERIA_ORBITING_MATTER,
    EVENT_ALLERIA_NEXT_STAGE,
    EVENT_ALLERIA_VOLATILE_FISSURE,
    EVENT_ALLERIA_RANGER_CAPTAINS_MARK,
    EVENT_ALLERIA_VOIDSTALKER_STING,
    EVENT_ALLERIA_CALL_OF_THE_VOID,
    EVENT_ALLERIA_DEVOURING_COSMOS,
    EVENT_ALLERIA_ASPECT_OF_THE_END,
    EVENT_ALLERIA_DARK_RUSH,
    EVENT_SENTINEL_MELEE_CHECK,
    EVENT_SENTINEL_LEASH
};

enum VoidspireActions
{
    ACTION_TWINS_SWAP = 1,
    ACTION_TWINS_FINAL_LANDING,
    ACTION_SALHADAAR_VOID_INFUSION,
    ACTION_SALHADAAR_ORB_DESTROYED,
    ACTION_VANGUARD_RETRIBUTION,
    ACTION_ALLERIA_SENTINEL_FALLEN,
    ACTION_ALLERIA_SIMULACRUM_SHATTERED
};

namespace
{
    float const VoidspireTwoPi = 6.28318531f;

    // kit spells of the adds use their own event range
    uint32_t const VoidspireKitEventBase = 1000;

    uint32_t const AverzianBerserkMs = 600000;
    uint32_t const VorasiusBerserkMs = 480000;
    uint32_t const TwinsBerserkMs = 540000;
    uint32_t const SalhadaarBerserkMs = 540000;
    uint32_t const VanguardBerserkMs = 480000;
    uint32_t const CrownBerserkMs = 600000;

    // three claims in a row unleash the March of the Endless
    uint32_t const AverzianMarchClaims = 3;
    float const AverzianGloryRange = 10.0f;

    uint32_t const AverzianSoldiers[] =
    {
        NPC_AVERZIAN_ABYSSAL_VOIDSHAPER,
        NPC_AVERZIAN_VOIDMAW,
        NPC_AVERZIAN_SHADOWGUARD_STALWART,
        NPC_AVERZIAN_VOIDBOUND_ANNIHILATOR,
        NPC_AVERZIAN_OBSCURION_ENDWALKER
    };

    // soldiers called by each Shadow's Advance, in turn
    uint32_t const AverzianWaveCount = 3;
    uint32_t const AverzianWaves[3][3] =
    {
        { NPC_AVERZIAN_ABYSSAL_VOIDSHAPER, NPC_AVERZIAN_SHADOWGUARD_STALWART, NPC_AVERZIAN_SHADOWGUARD_STALWART },
        { NPC_AVERZIAN_VOIDMAW, NPC_AVERZIAN_VOIDMAW, NPC_AVERZIAN_SHADOWGUARD_STALWART },
        { NPC_AVERZIAN_VOIDBOUND_ANNIHILATOR, NPC_AVERZIAN_ABYSSAL_VOIDSHAPER, 0 }
    };

    int32_t const VoidmawResiliencePct = 35;
    float const SoldierBulwarkRange = 40.0f;

    float const VorasiusPulseReach = 12.0f;

    int32_t const TwinsLandingPct = 30;

    // one energy per second, orbs that reach the king add more
    uint32_t const SalhadaarFullEnergy = 100;
    uint32_t const SalhadaarInfusionEnergy = 15;
    uint32_t const SalhadaarOrbStepMs = 1000;
    float const SalhadaarOrbReach = 4.0f;
    float const SalhadaarSearchRange = 150.0f;
    uint32_t const FracturedImageLifetimeMs = 40000;

    uint32_t const VanguardFullEnergy = 100;
    uint32_t const VanguardEnergyPerSecond = 2;
    int32_t const VanguardGuardPct = 50;

    uint32_t const CrownSentinelCount = 3;
    float const CrownSentinelRange = 18.0f;
    float const SentinelLeashRange = 30.0f;
    float const SentinelMeleeReach = 6.0f;

    enum CrownStage : uint8_t
    {
        CROWN_STAGE_NONE = 0,
        CROWN_STAGE_SENTINELS,
        CROWN_STAGE_CRUSHING,
        CROWN_STAGE_SIMULACRUM,
        CROWN_STAGE_SHATTERING,
        CROWN_STAGE_FINAL
    };

    uint32_t const CrownSentinels[3] = { NPC_CROWN_MORIUM, NPC_CROWN_DEMIAR, NPC_CROWN_VORELUS };
    uint32_t const AlleriaEntries[2] = { BOSS_ALLERIA_WINDRUNNER, BOSS_ALLERIA_WINDRUNNER_ALT };

    // members of the encounters fought as one
    struct VoidspireGroupSlot
    {
        uint32_t dataId;
        uint32_t entry;
    };

    VoidspireGroupSlot const VoidspireGroupSlots[] =
    {
        { DATA_VAELGOR_AND_EZZORAK,     BOSS_VAELGOR },
        { DATA_VAELGOR_AND_EZZORAK,     BOSS_EZZORAK },
        { DATA_LIGHTBLINDED_VANGUARD,   BOSS_COMMANDER_VENEL_LIGHTBLOOD },
        { DATA_LIGHTBLINDED_VANGUARD,   BOSS_GENERAL_AMIAS_BELLAMY },
        { DATA_LIGHTBLINDED_VANGUARD,   BOSS_WAR_CHAPLAIN_SENN }
    };

    enum VoidspireAim : uint8_t
    {
        AIM_VICTIM,
        AIM_PLAYER,
        AIM_SELF,
        AIM_AROUND
    };

    struct VoidspireKitSpell
    {
        uint32_t spellId;
        VoidspireAim aim;
        uint32_t firstMs;
        uint32_t repeatMs;
    };

    LocationVector voidspirePointAround(LocationVector const& origin, float minRange, float maxRange)
    {
        float const angle = Util::getRandomFloat(VoidspireTwoPi);
        float const range = Util::getRandomFloat(minRange, maxRange);
        return LocationVector(origin.x + range * std::cos(angle), origin.y + range * std::sin(angle), origin.z, angle);
    }

    // point on a circle, facing its centre
    LocationVector voidspirePointOnCircle(LocationVector const& origin, float range, float angle)
    {
        float const facing = std::fmod(angle + VoidspireTwoPi / 2.0f, VoidspireTwoPi);
        return LocationVector(origin.x + range * std::cos(angle), origin.y + range * std::sin(angle), origin.z, facing);
    }
}

//////////////////////////////////////////////////////////////////////////////////////////
// Instance

class VoidspireInstanceScript : public InstanceScript
{
public:
    explicit VoidspireInstanceScript(WorldMap* pMapMgr) : InstanceScript(pMapMgr)
    {
        setBossNumber(VoidspireEncounterCount);
    }

    static InstanceScript* Create(WorldMap* pMapMgr) { return new VoidspireInstanceScript(pMapMgr); }

    // a fallen dragon or paladin must not end the encounter while another stands
    bool setBossState(uint32_t id, EncounterStates state) override
    {
        if (state == EncounterStates::Performed && getBossState(id) == EncounterStates::InProgress && !isEncounterDown(id, nullptr))
            return false;

        return InstanceScript::setBossState(id, state);
    }

    void OnEncounterStateChange(uint32_t id, uint32_t state) override
    {
        if (state != EncounterStates::Failed)
            return;

        // a wiped group fights again in full
        for (Creature* pMember : getGroup(id))
        {
            if (!pMember->isAlive())
                pMember->despawn(1000, 5000);
        }
    }

    std::vector<Creature*> getGroup(uint32_t dataId)
    {
        std::vector<Creature*> group;
        for (VoidspireGroupSlot const& slot : VoidspireGroupSlots)
        {
            if (slot.dataId != dataId)
                continue;

            for (Creature* pMember : getCreatureSetForEntry(slot.entry))
                group.push_back(pMember);
        }

        return group;
    }

    // single boss encounters are down with their boss
    bool isEncounterDown(uint32_t dataId, Creature* pExcept)
    {
        for (Creature* pMember : getGroup(dataId))
        {
            if (pMember != pExcept && pMember->isAlive())
                return false;
        }

        return true;
    }

    // pulling one member pulls the rest
    void rallyGroup(uint32_t dataId, CreatureAIScript* pPuller)
    {
        if (mRallying)
            return;

        mRallying = true;
        for (Creature* pMember : getGroup(dataId))
        {
            if (pMember == pPuller->getCreature() || !pMember->isAlive() || pMember->isInCombat())
                continue;

            pPuller->setZoneWideCombat(pMember);
        }
        mRallying = false;
    }

    void signalGroup(uint32_t dataId, int32_t action)
    {
        for (Creature* pMember : getGroup(dataId))
        {
            if (pMember->isAlive() && pMember->GetScript() != nullptr)
                pMember->GetScript()->DoAction(action);
        }
    }

    Creature* livingCreature(uint32_t entry)
    {
        for (Creature* pCreature : getCreatureSetForEntry(entry))
        {
            if (pCreature->isAlive())
                return pCreature;
        }

        return nullptr;
    }

    void signalAlleria(int32_t action)
    {
        for (uint32_t entry : AlleriaEntries)
        {
            Creature* pAlleria = livingCreature(entry);
            if (pAlleria != nullptr && pAlleria->GetScript() != nullptr)
                pAlleria->GetScript()->DoAction(action);
        }
    }

private:
    bool mRallying = false;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Shared combat base: event clock, berserk and target helpers

class VoidspireCombatAI : public CreatureAIScript
{
public:
    explicit VoidspireCombatAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

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

    VoidspireInstanceScript* getVoidspire()
    {
        if (getCreature()->GetMapId() != MAP_THE_VOIDSPIRE)
            return nullptr;

        return static_cast<VoidspireInstanceScript*>(getInstanceScript());
    }

    Unit* currentVictim() { return getCreature()->getAIInterface()->getCurrentTarget(); }

    // distinct random players, the tank only fills up
    std::vector<Unit*> pickPlayers(size_t count, bool skipTank = false)
    {
        Unit* pVictim = currentVictim();
        FilterArgs const args(TargetFilter_Player);

        std::vector<Unit*> pool;
        std::vector<Unit*> reserve;
        for (Object* pObject : getCreature()->getInRangePlayersSet())
        {
            if (pObject == nullptr || !isValidUnitTarget(pObject, TargetFilter_Player, args))
                continue;

            Unit* pPlayer = static_cast<Unit*>(pObject);
            if (skipTank && pPlayer == pVictim)
                reserve.push_back(pPlayer);
            else
                pool.push_back(pPlayer);
        }

        std::vector<Unit*> picked;
        while (picked.size() < count && !pool.empty())
        {
            size_t const index = Util::getRandomUInt(static_cast<uint32_t>(pool.size() - 1));
            picked.push_back(pool[index]);
            pool[index] = pool.back();
            pool.pop_back();
        }

        for (Unit* pPlayer : reserve)
        {
            if (picked.size() < count)
                picked.push_back(pPlayer);
        }

        return picked;
    }

    Unit* pickPlayer(bool skipTank = false)
    {
        std::vector<Unit*> const picked = pickPlayers(1, skipTank);
        return picked.empty() ? nullptr : picked.front();
    }

    Unit* castAtPlayer(uint32_t spellId, bool skipTank = false, bool triggered = false)
    {
        Unit* pTarget = pickPlayer(skipTank);
        if (pTarget != nullptr)
            castSpell(pTarget, spellId, triggered);

        return pTarget;
    }

    void castAtPlayers(uint32_t spellId, size_t count, bool skipTank = false)
    {
        for (Unit* pTarget : pickPlayers(count, skipTank))
            castSpell(pTarget, spellId, true);
    }

    // ground spell under a random player
    void castUnderPlayer(uint32_t spellId, bool skipTank = false)
    {
        if (Unit* pTarget = pickPlayer(skipTank))
            getCreature()->castSpellLoc(pTarget->GetPosition(), spellId, false);
    }

    bool anyPlayerWithin(float range)
    {
        for (Object* pObject : getCreature()->getInRangePlayersSet())
        {
            if (pObject == nullptr)
                continue;

            Unit* pPlayer = static_cast<Unit*>(pObject);
            if (pPlayer->isAlive() && getCreature()->getDistance(pPlayer) <= range)
                return true;
        }

        return false;
    }

    // summoned add that joins the fight at once
    Creature* callAdd(uint32_t entry, LocationVector const& spot, CreatureSummonDespawnType despawnType = CORPSE_DESPAWN, uint32_t durationMs = 0)
    {
        Creature* pAdd = summonCreature(entry, spot, despawnType, durationMs);
        if (pAdd != nullptr)
            setZoneWideCombat(pAdd);

        return pAdd;
    }

    LocationVector aroundSelf(float minRange, float maxRange)
    {
        return voidspirePointAround(getCreature()->GetPosition(), minRange, maxRange);
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
        castSpellOnSelf(SPELL_VOIDSPIRE_BERSERK, true);
    }

    uint32_t mBerserkLeftMs = 0;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Encounter base: frame, boss state and group pull

class VoidspireBossAI : public VoidspireCombatAI
{
public:
    VoidspireBossAI(Creature* pCreature, uint32_t dataId) : VoidspireCombatAI(pCreature), mDataId(dataId) {}

    void OnCombatStart(Unit* pTarget) override
    {
        if (VoidspireInstanceScript* pVoidspire = getVoidspire())
        {
            pVoidspire->sendUnitEncounter(EncounterFrameEngage, getCreature());
            pVoidspire->setBossState(mDataId, EncounterStates::InProgress);
            pVoidspire->rallyGroup(mDataId, this);
        }

        VoidspireCombatAI::OnCombatStart(pTarget);
    }

    void OnCombatStop(Unit* pTarget) override
    {
        if (getCreature()->isAlive())
        {
            if (VoidspireInstanceScript* pVoidspire = getVoidspire())
            {
                pVoidspire->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
                pVoidspire->setBossState(mDataId, EncounterStates::Failed);
            }
        }

        VoidspireCombatAI::OnCombatStop(pTarget);
    }

    void OnDied(Unit* /*pKiller*/) override
    {
        scriptEvents.resetEvents();
        summons.despawnAll();

        if (VoidspireInstanceScript* pVoidspire = getVoidspire())
        {
            pVoidspire->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
            if (pVoidspire->isEncounterDown(mDataId, getCreature()))
                pVoidspire->setBossState(mDataId, EncounterStates::Performed);
        }

        onDefeat();
    }

protected:
    virtual void onDefeat() {}

    uint32_t mDataId;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Adds driven by a list of timed spells

class VoidspireKitAI : public VoidspireCombatAI
{
public:
    VoidspireKitAI(Creature* pCreature, std::vector<VoidspireKitSpell> kit) : VoidspireCombatAI(pCreature), mKit(std::move(kit)) {}

protected:
    void onEngage() override
    {
        for (size_t i = 0; i < mKit.size(); ++i)
            scriptEvents.addEvent(VoidspireKitEventBase + static_cast<uint32_t>(i), mKit[i].firstMs);

        onKitEngage();
    }

    void onEvent(uint32_t eventId) override
    {
        if (eventId >= VoidspireKitEventBase && eventId < VoidspireKitEventBase + mKit.size())
        {
            VoidspireKitSpell const& entry = mKit[eventId - VoidspireKitEventBase];
            useKitSpell(entry);
            scriptEvents.addEvent(eventId, entry.repeatMs);
            return;
        }

        onExtraEvent(eventId);
    }

    virtual void onKitEngage() {}
    virtual void onExtraEvent(uint32_t /*eventId*/) {}

private:
    void useKitSpell(VoidspireKitSpell const& entry)
    {
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
    }

    std::vector<VoidspireKitSpell> mKit;
};

// bursts with a last spell instead of dying
class VoidspireBursterAI : public VoidspireKitAI
{
public:
    VoidspireBursterAI(Creature* pCreature, std::vector<VoidspireKitSpell> kit, uint32_t burstSpellId)
        : VoidspireKitAI(pCreature, std::move(kit)), mBurstSpellId(burstSpellId) {}

    void DamageTaken(Unit* /*pAttacker*/, uint32_t* damage) override
    {
        if (damage == nullptr)
            return;

        if (mBurst)
        {
            *damage = 0;
            return;
        }

        uint32_t const health = getCreature()->getHealth();
        if (*damage < health)
            return;

        *damage = health > 1 ? health - 1 : 0;
        mBurst = true;
        scriptEvents.resetEvents();
        getCreature()->interruptSpell();
        castSpellAOE(mBurstSpellId, true);
        despawn(500, 0);
    }

private:
    uint32_t mBurstSpellId;
    bool mBurst = false;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Imperator Averzian

class VoidspireAverzianAI : public VoidspireBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new VoidspireAverzianAI(c); }
    explicit VoidspireAverzianAI(Creature* pCreature) : VoidspireBossAI(pCreature, DATA_IMPERATOR_AVERZIAN) {}

protected:
    void onEngage() override
    {
        mClaimsInRow = 0;
        mNextWave = 0;
        armBerserk(AverzianBerserkMs);

        scriptEvents.addEvent(EVENT_AVERZIAN_BLACKENING_WOUNDS, 8000);
        scriptEvents.addEvent(EVENT_AVERZIAN_SHADOWS_ADVANCE, 12000);
        scriptEvents.addEvent(EVENT_AVERZIAN_DARK_UPHEAVAL, 20000);
        scriptEvents.addEvent(EVENT_AVERZIAN_VOID_MARKED, 26000);
        scriptEvents.addEvent(EVENT_AVERZIAN_UMBRAL_COLLAPSE, 32000);
        scriptEvents.addEvent(EVENT_AVERZIAN_OBLIVIONS_WRATH, 38000);

        if (isHeroic())
            scriptEvents.addEvent(EVENT_AVERZIAN_GLORY_CHECK, 3000);
    }

    void onDisengage() override { _removeAura(SPELL_AVERZIAN_IMPERATORS_GLORY); }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_AVERZIAN_SHADOWS_ADVANCE:
                advanceShadows();
                scriptEvents.addEvent(EVENT_AVERZIAN_SHADOWS_ADVANCE, 45000);
                break;
            case EVENT_AVERZIAN_MARCH_OF_THE_ENDLESS:
                warn("Imperator Averzian holds three territories and unleashes the March of the Endless!");
                castSpellOnSelf(SPELL_AVERZIAN_MARCH_OF_THE_ENDLESS);
                break;
            case EVENT_AVERZIAN_UMBRAL_COLLAPSE:
                warn("Imperator Averzian calls down an Umbral Collapse! Share the impact!");
                castUnderPlayer(SPELL_AVERZIAN_UMBRAL_COLLAPSE, true);
                scriptEvents.addEvent(EVENT_AVERZIAN_UMBRAL_COLLAPSE, 40000);
                break;
            case EVENT_AVERZIAN_DARK_UPHEAVAL:
                castSpellAOE(SPELL_AVERZIAN_DARK_UPHEAVAL);
                scriptEvents.addEvent(EVENT_AVERZIAN_DARK_UPHEAVAL, 30000);
                break;
            case EVENT_AVERZIAN_BLACKENING_WOUNDS:
                castSpellOnVictim(SPELL_AVERZIAN_BLACKENING_WOUNDS);
                scriptEvents.addEvent(EVENT_AVERZIAN_BLACKENING_WOUNDS, 14000);
                break;
            case EVENT_AVERZIAN_OBLIVIONS_WRATH:
                castAtPlayer(SPELL_AVERZIAN_OBLIVIONS_WRATH, true);
                scriptEvents.addEvent(EVENT_AVERZIAN_OBLIVIONS_WRATH, 28000);
                break;
            case EVENT_AVERZIAN_VOID_MARKED:
                castAtPlayers(SPELL_AVERZIAN_VOID_MARKED, isHeroic() ? 3 : 2, true);
                scriptEvents.addEvent(EVENT_AVERZIAN_VOID_MARKED, 32000);
                break;
            case EVENT_AVERZIAN_GLORY_CHECK:
                updateGlory();
                scriptEvents.addEvent(EVENT_AVERZIAN_GLORY_CHECK, 3000);
                break;
            default:
                break;
        }
    }

private:
    void advanceShadows()
    {
        warn("Imperator Averzian's Shadow's Advance claims new territory!");
        castSpellOnSelf(SPELL_AVERZIAN_SHADOWS_ADVANCE);
        callSoldiers();

        if (++mClaimsInRow >= AverzianMarchClaims)
        {
            mClaimsInRow = 0;
            scriptEvents.addEvent(EVENT_AVERZIAN_MARCH_OF_THE_ENDLESS, 6000);
        }
    }

    void callSoldiers()
    {
        for (uint32_t entry : AverzianWaves[mNextWave])
        {
            if (entry != 0)
                callAdd(entry, aroundSelf(12.0f, 20.0f));
        }

        mNextWave = (mNextWave + 1) % AverzianWaveCount;
    }

    // heroic: soldiers within reach empower him
    void updateGlory()
    {
        bool const empowered = soldierNearby();
        bool const hasGlory = hasAura(SPELL_AVERZIAN_IMPERATORS_GLORY);
        if (empowered && !hasGlory)
            castSpellOnSelf(SPELL_AVERZIAN_IMPERATORS_GLORY, true);
        else if (!empowered && hasGlory)
            _removeAura(SPELL_AVERZIAN_IMPERATORS_GLORY);
    }

    bool soldierNearby()
    {
        for (uint32_t entry : AverzianSoldiers)
        {
            Creature* pSoldier = findNearestCreature(entry, AverzianGloryRange);
            if (pSoldier != nullptr && pSoldier->isAlive())
                return true;
        }

        return false;
    }

    uint32_t mClaimsInRow = 0;
    uint32_t mNextWave = 0;
};

// shields an ally with Pitch Bulwark, the Imperator first
class VoidspireAverzianSoldierAI : public VoidspireKitAI
{
public:
    VoidspireAverzianSoldierAI(Creature* pCreature, std::vector<VoidspireKitSpell> kit, uint32_t bulwarkFirstMs)
        : VoidspireKitAI(pCreature, std::move(kit)), mBulwarkFirstMs(bulwarkFirstMs) {}

protected:
    void onKitEngage() override
    {
        if (mBulwarkFirstMs != 0)
            scriptEvents.addEvent(EVENT_SOLDIER_PITCH_BULWARK, mBulwarkFirstMs);
    }

    void onExtraEvent(uint32_t eventId) override
    {
        if (eventId != EVENT_SOLDIER_PITCH_BULWARK)
            return;

        Creature* pImperator = findNearestCreature(BOSS_IMPERATOR_AVERZIAN, SoldierBulwarkRange);
        if (pImperator != nullptr && pImperator->isAlive())
            castSpell(pImperator, SPELL_SOLDIER_PITCH_BULWARK);
        else
            castSpellOnSelf(SPELL_SOLDIER_PITCH_BULWARK);

        scriptEvents.addEvent(EVENT_SOLDIER_PITCH_BULWARK, 20000);
    }

private:
    uint32_t mBulwarkFirstMs;
};

class VoidspireShadowguardStalwartAI : public VoidspireAverzianSoldierAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new VoidspireShadowguardStalwartAI(c); }
    explicit VoidspireShadowguardStalwartAI(Creature* pCreature) : VoidspireAverzianSoldierAI(pCreature, {}, 8000) {}
};

class VoidspireVoidboundAnnihilatorAI : public VoidspireAverzianSoldierAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new VoidspireVoidboundAnnihilatorAI(c); }
    explicit VoidspireVoidboundAnnihilatorAI(Creature* pCreature)
        : VoidspireAverzianSoldierAI(pCreature, { { SPELL_ANNIHILATOR_DARK_BARRAGE, AIM_PLAYER, 5000, 12000 } }, 14000) {}
};

// tries to reach full power with Gathering Darkness
class VoidspireAbyssalVoidshaperAI : public VoidspireKitAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new VoidspireAbyssalVoidshaperAI(c); }
    explicit VoidspireAbyssalVoidshaperAI(Creature* pCreature)
        : VoidspireKitAI(pCreature, {
            { SPELL_VOIDSHAPER_UMBRAL_BARRIER, AIM_SELF, 2000, 30000 },
            { SPELL_VOIDSHAPER_VOID_RUPTURE, AIM_PLAYER, 6000, 12000 } }) {}

protected:
    void onKitEngage() override { scriptEvents.addEvent(EVENT_VOIDSHAPER_GATHERING_DARKNESS, 30000); }

    void onExtraEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_VOIDSHAPER_GATHERING_DARKNESS:
                warn("An Abyssal Voidshaper reaches full power and begins Gathering Darkness!");
                castSpellAOE(SPELL_VOIDSHAPER_GATHERING_DARKNESS);
                if (isHeroic())
                    scriptEvents.addEvent(EVENT_VOIDSHAPER_TRANSFORM, 4000);
                else
                    scriptEvents.addEvent(EVENT_VOIDSHAPER_GATHERING_DARKNESS, 30000);
                break;
            case EVENT_VOIDSHAPER_TRANSFORM:
                becomeEndwalker();
                break;
            default:
                break;
        }
    }

private:
    // the Imperator owns the Endwalker so it leaves with his fight
    void becomeEndwalker()
    {
        scriptEvents.resetEvents();

        VoidspireInstanceScript* pVoidspire = getVoidspire();
        Creature* pImperator = pVoidspire != nullptr ? pVoidspire->livingCreature(BOSS_IMPERATOR_AVERZIAN) : nullptr;
        if (pImperator != nullptr)
        {
            Creature* pEndwalker = pImperator->summonCreature(NPC_AVERZIAN_OBSCURION_ENDWALKER, getCreature()->GetPosition(), CORPSE_DESPAWN);
            if (pEndwalker != nullptr)
                setZoneWideCombat(pEndwalker);
        }

        despawn(200, 0);
    }
};

class VoidspireVoidmawAI : public VoidspireKitAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new VoidspireVoidmawAI(c); }
    explicit VoidspireVoidmawAI(Creature* pCreature)
        : VoidspireKitAI(pCreature, { { SPELL_VOIDMAW_GNASHING_VOID, AIM_VICTIM, 5000, 10000 } }) {}

protected:
    void onKitEngage() override { mRestored = false; }

    // heroic: restores itself once at low health
    void onCombatTick(uint32_t /*diff*/) override
    {
        if (mRestored || !isHeroic() || _getHealthPercent() > VoidmawResiliencePct)
            return;

        mRestored = true;
        castSpellOnSelf(SPELL_VOIDMAW_DARK_RESILIENCE, true);
    }

private:
    bool mRestored = false;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Vorasius

class VoidspireVorasiusAI : public VoidspireBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new VoidspireVorasiusAI(c); }
    explicit VoidspireVorasiusAI(Creature* pCreature) : VoidspireBossAI(pCreature, DATA_VORASIUS) {}

protected:
    void onEngage() override
    {
        armBerserk(VorasiusBerserkMs);

        scriptEvents.addEvent(EVENT_VORASIUS_PULSE_CHECK, 5000);
        scriptEvents.addEvent(EVENT_VORASIUS_SHADOWCLAW_SLAM, 12000);
        scriptEvents.addEvent(EVENT_VORASIUS_PARASITE_EXPULSION, 20000);
        scriptEvents.addEvent(EVENT_VORASIUS_VOID_BREATH, 38000);
        scriptEvents.addEvent(EVENT_VORASIUS_PRIMORDIAL_ROAR, 50000);
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_VORASIUS_SHADOWCLAW_SLAM:
                castSpellOnVictim(SPELL_VORASIUS_SHADOWCLAW_SLAM);
                scriptEvents.addEvent(EVENT_VORASIUS_SHADOWCLAW_SLAM, 30000);
                break;
            case EVENT_VORASIUS_PARASITE_EXPULSION:
                expelParasites();
                scriptEvents.addEvent(EVENT_VORASIUS_PARASITE_EXPULSION, 40000);
                break;
            case EVENT_VORASIUS_VOID_BREATH:
                warn("Vorasius sweeps the battlefield with Void Breath!");
                castAtPlayer(SPELL_VORASIUS_VOID_BREATH, true);
                scriptEvents.addEvent(EVENT_VORASIUS_VOID_BREATH, 45000);
                break;
            case EVENT_VORASIUS_PRIMORDIAL_ROAR:
                // every roar adds to his power
                castSpellOnSelf(SPELL_VORASIUS_PRIMORDIAL_POWER, true);
                castSpellAOE(SPELL_VORASIUS_PRIMORDIAL_ROAR);
                scriptEvents.addEvent(EVENT_VORASIUS_PRIMORDIAL_ROAR, 50000);
                break;
            case EVENT_VORASIUS_PULSE_CHECK:
                if (!anyPlayerWithin(VorasiusPulseReach))
                    castSpellAOE(SPELL_VORASIUS_OVERPOWERING_PULSE, true);
                scriptEvents.addEvent(EVENT_VORASIUS_PULSE_CHECK, 3000);
                break;
            default:
                break;
        }
    }

private:
    void expelParasites()
    {
        warn("Vorasius shakes off a swarm of Blistercreeps!");

        uint32_t const count = isHeroic() ? 4 : 3;
        for (uint32_t i = 0; i < count; ++i)
            callAdd(NPC_VORASIUS_BLISTERCREEP, aroundSelf(6.0f, 12.0f));
    }
};

// fixates on one player and bursts in a Blisterburst
class VoidspireBlistercreepAI : public VoidspireBursterAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new VoidspireBlistercreepAI(c); }
    explicit VoidspireBlistercreepAI(Creature* pCreature)
        : VoidspireBursterAI(pCreature, { { SPELL_BLISTERCREEP_CREEP_SPIT, AIM_VICTIM, 4000, 7000 } }, SPELL_BLISTERCREEP_BLISTERBURST) {}

protected:
    void onKitEngage() override
    {
        if (Unit* pPrey = pickPlayer(true))
            addThreat(pPrey, 1000000.0f);
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Vaelgor & Ezzorak, one in the air while the other fights on the ground

class VoidspireTwinDragonAI : public VoidspireBossAI
{
public:
    VoidspireTwinDragonAI(Creature* pCreature, bool startsAirborne) : VoidspireBossAI(pCreature, DATA_VAELGOR_AND_EZZORAK), mStartsAirborne(startsAirborne) {}

    void DoAction(int32_t action) override
    {
        if (!_isInCombat() || !isAlive() || mLanded)
            return;

        if (action == ACTION_TWINS_SWAP)
            takeRole(!mAirborne);
        else if (action == ACTION_TWINS_FINAL_LANDING)
            landForGood();
    }

protected:
    void onEngage() override
    {
        mLanded = false;
        armBerserk(TwinsBerserkMs);
        takeRole(mStartsAirborne);

        if (leadsSwaps())
            scriptEvents.addEvent(EVENT_TWINS_MIDNIGHT_FLAMES, 55000);
    }

    void onDisengage() override
    {
        mAirborne = false;
        mLanded = false;
        applyAirborne(false);
    }

    void onCombatTick(uint32_t /*diff*/) override
    {
        if (mLanded || _getHealthPercent() > TwinsLandingPct)
            return;

        warn("Vaelgor and Ezzorak land together for a final stand!");
        if (VoidspireInstanceScript* pVoidspire = getVoidspire())
            pVoidspire->signalGroup(mDataId, ACTION_TWINS_FINAL_LANDING);
        else
            landForGood();
    }

    // the survivor lands at once
    void onDefeat() override
    {
        if (VoidspireInstanceScript* pVoidspire = getVoidspire())
            pVoidspire->signalGroup(mDataId, ACTION_TWINS_FINAL_LANDING);
    }

    // events both dragons share, true when handled
    bool onTwinEvent(uint32_t eventId)
    {
        switch (eventId)
        {
            case EVENT_TWINS_MIDNIGHT_FLAMES:
                warn("Midnight Flames reach full power! The dragons trade places!");
                castSpellAOE(SPELL_TWINS_MIDNIGHT_FLAMES);
                scriptEvents.addEvent(EVENT_TWINS_SWAP, 6000);
                scriptEvents.addEvent(EVENT_TWINS_MIDNIGHT_FLAMES, 60000);
                return true;
            case EVENT_TWINS_SWAP:
                if (VoidspireInstanceScript* pVoidspire = getVoidspire())
                    pVoidspire->signalGroup(mDataId, ACTION_TWINS_SWAP);
                else
                    takeRole(!mAirborne);
                return true;
            case EVENT_TWINS_GRAPPLING_MAW:
                castSpellOnVictim(SPELL_TWINS_GRAPPLING_MAW);
                scriptEvents.addEvent(EVENT_TWINS_GRAPPLING_MAW, 20000);
                return true;
            case EVENT_TWINS_TWILIGHT_FURY:
                castSpellAOE(SPELL_TWINS_TWILIGHT_FURY);
                scriptEvents.addEvent(EVENT_TWINS_TWILIGHT_FURY, 40000);
                return true;
            default:
                return false;
        }
    }

    virtual bool leadsSwaps() const { return false; }
    virtual void scheduleGroundKit() {}
    virtual void scheduleAirKit() {}
    virtual void clearKits() {}

    bool mAirborne = false;

private:
    void takeRole(bool airborne)
    {
        mAirborne = airborne;
        applyAirborne(airborne);
        clearKits();

        if (airborne)
        {
            scheduleAirKit();
            scriptEvents.addEvent(EVENT_TWINS_GRAPPLING_MAW, 12000);
        }
        else
        {
            scriptEvents.removeEvent(EVENT_TWINS_GRAPPLING_MAW);
            scheduleGroundKit();
        }
    }

    void landForGood()
    {
        mLanded = true;
        scriptEvents.removeEvent(EVENT_TWINS_MIDNIGHT_FLAMES);
        scriptEvents.removeEvent(EVENT_TWINS_SWAP);
        takeRole(false);

        castSpellOnSelf(SPELL_TWINS_TWILIGHT_BOND, true);
        scriptEvents.addEvent(EVENT_TWINS_TWILIGHT_FURY, 15000);
    }

    // a flying dragon keeps out of melee and holds its spot
    void applyAirborne(bool airborne)
    {
        _setMeleeDisabled(airborne);
        setRooted(airborne);
    }

    bool mStartsAirborne;
    bool mLanded = false;
};

class VoidspireVaelgorAI : public VoidspireTwinDragonAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new VoidspireVaelgorAI(c); }
    explicit VoidspireVaelgorAI(Creature* pCreature) : VoidspireTwinDragonAI(pCreature, false) {}

protected:
    bool leadsSwaps() const override { return true; }

    void scheduleGroundKit() override
    {
        scriptEvents.addEvent(EVENT_VAELGOR_VAELWING, 6000);
        scriptEvents.addEvent(EVENT_VAELGOR_NULLBEAM, 10000);
    }

    void scheduleAirKit() override { scriptEvents.addEvent(EVENT_VAELGOR_DREAD_BREATH, 8000); }

    void clearKits() override
    {
        scriptEvents.removeEvent(EVENT_VAELGOR_VAELWING);
        scriptEvents.removeEvent(EVENT_VAELGOR_NULLBEAM);
        scriptEvents.removeEvent(EVENT_VAELGOR_DREAD_BREATH);
    }

    void onEvent(uint32_t eventId) override
    {
        if (onTwinEvent(eventId))
            return;

        switch (eventId)
        {
            case EVENT_VAELGOR_NULLBEAM:
                castSpellOnVictim(SPELL_VAELGOR_NULLBEAM);
                scriptEvents.addEvent(EVENT_VAELGOR_NULLBEAM, 22000);
                break;
            case EVENT_VAELGOR_VAELWING:
                castSpellOnVictim(SPELL_VAELGOR_VAELWING);
                scriptEvents.addEvent(EVENT_VAELGOR_VAELWING, 16000);
                break;
            case EVENT_VAELGOR_DREAD_BREATH:
                warn("Vaelgor takes a deep breath from above!");
                castAtPlayer(SPELL_VAELGOR_DREAD_BREATH, true);
                scriptEvents.addEvent(EVENT_VAELGOR_DREAD_BREATH, 25000);
                break;
            default:
                break;
        }
    }
};

class VoidspireEzzorakAI : public VoidspireTwinDragonAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new VoidspireEzzorakAI(c); }
    explicit VoidspireEzzorakAI(Creature* pCreature) : VoidspireTwinDragonAI(pCreature, true) {}

protected:
    void scheduleGroundKit() override
    {
        scriptEvents.addEvent(EVENT_EZZORAK_RAKFANG, 7000);
        scriptEvents.addEvent(EVENT_EZZORAK_GLOOM, 12000);
    }

    void scheduleAirKit() override { scriptEvents.addEvent(EVENT_EZZORAK_VOID_HOWL, 10000); }

    void clearKits() override
    {
        scriptEvents.removeEvent(EVENT_EZZORAK_RAKFANG);
        scriptEvents.removeEvent(EVENT_EZZORAK_GLOOM);
        scriptEvents.removeEvent(EVENT_EZZORAK_VOID_HOWL);
    }

    void onEvent(uint32_t eventId) override
    {
        if (onTwinEvent(eventId))
            return;

        switch (eventId)
        {
            case EVENT_EZZORAK_GLOOM:
                castSpellOnVictim(SPELL_EZZORAK_GLOOM);
                scriptEvents.addEvent(EVENT_EZZORAK_GLOOM, 24000);
                break;
            case EVENT_EZZORAK_RAKFANG:
                castSpellOnVictim(SPELL_EZZORAK_RAKFANG);
                scriptEvents.addEvent(EVENT_EZZORAK_RAKFANG, 16000);
                break;
            case EVENT_EZZORAK_VOID_HOWL:
                howl();
                scriptEvents.addEvent(EVENT_EZZORAK_VOID_HOWL, 32000);
                break;
            default:
                break;
        }
    }

private:
    // Voidorbs gather around the players below
    void howl()
    {
        warn("Ezzorak's Void Howl calls forth Voidorbs!");
        castSpellAOE(SPELL_EZZORAK_VOID_HOWL);

        for (Unit* pTarget : pickPlayers(2, true))
            callAdd(NPC_EZZORAK_VOIDORB, voidspirePointAround(pTarget->GetPosition(), 4.0f, 8.0f), TIMED_DESPAWN, 30000);
    }
};

class VoidspireVoidorbAI : public VoidspireKitAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new VoidspireVoidorbAI(c); }
    explicit VoidspireVoidorbAI(Creature* pCreature)
        : VoidspireKitAI(pCreature, { { SPELL_VOIDORB_VOIDBOLT, AIM_PLAYER, 2000, 5000 } }) {}

protected:
    void onKitEngage() override
    {
        setRooted(true);
        _setMeleeDisabled(true);
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Fallen-King Salhadaar

class VoidspireSalhadaarAI : public VoidspireBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new VoidspireSalhadaarAI(c); }
    explicit VoidspireSalhadaarAI(Creature* pCreature) : VoidspireBossAI(pCreature, DATA_FALLEN_KING_SALHADAAR) {}

    void DoAction(int32_t action) override
    {
        if (!_isInCombat() || !isAlive())
            return;

        if (action == ACTION_SALHADAAR_VOID_INFUSION)
        {
            castSpellOnSelf(SPELL_SALHADAAR_VOID_INFUSION, true);
            mEnergy += SalhadaarInfusionEnergy;
        }
        else if (action == ACTION_SALHADAAR_ORB_DESTROYED && isHeroic())
        {
            castSpellAOE(SPELL_SALHADAAR_DARK_RADIATION, true);
        }
    }

protected:
    void onEngage() override
    {
        mEnergy = 0;
        mEnergyClockMs = 0;
        armBerserk(SalhadaarBerserkMs);
        castSpellOnSelf(SPELL_SALHADAAR_DESTABILIZING_STRIKES, true);

        scriptEvents.addEvent(EVENT_SALHADAAR_SHATTERING_TWILIGHT, 10000);
        scriptEvents.addEvent(EVENT_SALHADAAR_DESPOTIC_COMMAND, 14000);
        scriptEvents.addEvent(EVENT_SALHADAAR_VOID_CONVERGENCE, 18000);
        scriptEvents.addEvent(EVENT_SALHADAAR_TWISTING_OBSCURITY, 22000);
        scriptEvents.addEvent(EVENT_SALHADAAR_FRACTURED_PROJECTION, 30000);
    }

    void onDisengage() override { _removeAura(SPELL_SALHADAAR_DESTABILIZING_STRIKES); }

    void onCombatTick(uint32_t diff) override
    {
        mEnergyClockMs += diff;
        while (mEnergyClockMs >= 1000)
        {
            mEnergyClockMs -= 1000;
            ++mEnergy;
        }

        if (mEnergy < SalhadaarFullEnergy || _isCasting())
            return;

        mEnergy = 0;
        warn("Fallen-King Salhadaar is overwhelmed by the Void and begins his Entropic Unraveling!");
        castSpellOnSelf(SPELL_SALHADAAR_ENTROPIC_UNRAVELING);
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_SALHADAAR_VOID_CONVERGENCE:
                convergeVoid();
                scriptEvents.addEvent(EVENT_SALHADAAR_VOID_CONVERGENCE, 40000);
                break;
            case EVENT_SALHADAAR_SHATTERING_TWILIGHT:
                castSpellOnVictim(SPELL_SALHADAAR_SHATTERING_TWILIGHT);
                scriptEvents.addEvent(EVENT_SALHADAAR_SHATTERING_TWILIGHT, 28000);
                break;
            case EVENT_SALHADAAR_FRACTURED_PROJECTION:
                projectImages();
                scriptEvents.addEvent(EVENT_SALHADAAR_FRACTURED_PROJECTION, 45000);
                break;
            case EVENT_SALHADAAR_DESPOTIC_COMMAND:
                castAtPlayer(SPELL_SALHADAAR_DESPOTIC_COMMAND, true);
                scriptEvents.addEvent(EVENT_SALHADAAR_DESPOTIC_COMMAND, 30000);
                break;
            case EVENT_SALHADAAR_TWISTING_OBSCURITY:
                castSpellAOE(SPELL_SALHADAAR_TWISTING_OBSCURITY);
                scriptEvents.addEvent(EVENT_SALHADAAR_TWISTING_OBSCURITY, 35000);
                break;
            default:
                break;
        }
    }

private:
    // the Riftlabs release orbs that drift towards the king
    void convergeVoid()
    {
        warn("The Riftlabs release orbs of Concentrated Void!");
        castSpellAOE(SPELL_SALHADAAR_VOID_CONVERGENCE);

        uint32_t const count = isHeroic() ? 3 : 2;
        for (uint32_t i = 0; i < count; ++i)
            summonCreature(NPC_SALHADAAR_CONCENTRATED_VOID, aroundSelf(30.0f, 35.0f), CORPSE_DESPAWN);
    }

    void projectImages()
    {
        castSpellAOE(SPELL_SALHADAAR_FRACTURED_PROJECTION);

        uint32_t const count = isHeroic() ? 3 : 2;
        for (uint32_t i = 0; i < count; ++i)
            callAdd(NPC_SALHADAAR_FRACTURED_IMAGE, aroundSelf(10.0f, 18.0f), TIMED_DESPAWN, FracturedImageLifetimeMs);
    }

    uint32_t mEnergy = 0;
    uint32_t mEnergyClockMs = 0;
};

// drifts to Salhadaar and infuses him on arrival
class VoidspireConcentratedVoidAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new VoidspireConcentratedVoidAI(c); }
    explicit VoidspireConcentratedVoidAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnLoad() override
    {
        setReactState(REACT_PASSIVE);
        _setMeleeDisabled(true);
    }

    void AIUpdate(unsigned long time_passed) override
    {
        if (mSpent || !isAlive())
            return;

        mStepMs += static_cast<uint32_t>(time_passed);
        if (mStepMs < SalhadaarOrbStepMs)
            return;

        mStepMs = 0;

        Creature* pKing = findNearestCreature(BOSS_FALLEN_KING_SALHADAAR, SalhadaarSearchRange);
        if (pKing == nullptr || !pKing->isAlive() || !pKing->isInCombat())
        {
            mSpent = true;
            despawn(500, 0);
            return;
        }

        if (getCreature()->getDistance(pKing) <= SalhadaarOrbReach)
        {
            mSpent = true;
            if (pKing->GetScript() != nullptr)
                pKing->GetScript()->DoAction(ACTION_SALHADAAR_VOID_INFUSION);

            despawn(200, 0);
            return;
        }

        LocationVector const destination = pKing->GetPosition();
        moveTo(destination.x, destination.y, destination.z, false);
    }

    void OnDied(Unit* /*pKiller*/) override
    {
        if (mSpent)
            return;

        mSpent = true;
        Creature* pKing = findNearestCreature(BOSS_FALLEN_KING_SALHADAAR, SalhadaarSearchRange);
        if (pKing != nullptr && pKing->isAlive() && pKing->GetScript() != nullptr)
            pKing->GetScript()->DoAction(ACTION_SALHADAAR_ORB_DESTROYED);
    }

private:
    uint32_t mStepMs = 0;
    bool mSpent = false;
};

class VoidspireFracturedImageAI : public VoidspireKitAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new VoidspireFracturedImageAI(c); }
    explicit VoidspireFracturedImageAI(Creature* pCreature)
        : VoidspireKitAI(pCreature, { { SPELL_FRACTURED_IMAGE_SHADOW_FRACTURE, AIM_PLAYER, 2000, 8000 } }) {}

protected:
    void onKitEngage() override
    {
        setRooted(true);
        _setMeleeDisabled(true);
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Lightblinded Vanguard, an Aura and a finisher at full energy

class VoidspireVanguardAI : public VoidspireBossAI
{
public:
    VoidspireVanguardAI(Creature* pCreature, uint32_t startEnergy, uint32_t auraSpellId, uint32_t guardSpellId, char const* auraText)
        : VoidspireBossAI(pCreature, DATA_LIGHTBLINDED_VANGUARD), mStartEnergy(startEnergy), mAuraSpellId(auraSpellId), mGuardSpellId(guardSpellId), mAuraText(auraText) {}

    void DoAction(int32_t action) override
    {
        if (action == ACTION_VANGUARD_RETRIBUTION && _isInCombat() && isAlive())
            castSpellOnSelf(SPELL_VANGUARD_RETRIBUTION, true);
    }

protected:
    void onEngage() override
    {
        mEnergy = mStartEnergy;
        mEnergyClockMs = 0;
        mGuarded = false;
        armBerserk(VanguardBerserkMs);
        scheduleKit();
    }

    void onCombatTick(uint32_t diff) override
    {
        if (!mGuarded && _getHealthPercent() <= VanguardGuardPct)
        {
            mGuarded = true;
            castSpellOnSelf(mGuardSpellId, true);
        }

        mEnergyClockMs += diff;
        while (mEnergyClockMs >= 1000)
        {
            mEnergyClockMs -= 1000;
            mEnergy += VanguardEnergyPerSecond;
        }

        if (mEnergy < VanguardFullEnergy || _isCasting())
            return;

        mEnergy = 0;
        warn(mAuraText);
        castSpellOnSelf(mAuraSpellId, true);
        unleashFinisher();
    }

    // the survivors avenge the fallen
    void onDefeat() override
    {
        if (VoidspireInstanceScript* pVoidspire = getVoidspire())
            pVoidspire->signalGroup(mDataId, ACTION_VANGUARD_RETRIBUTION);
    }

    virtual void scheduleKit() {}
    virtual void unleashFinisher() {}

private:
    uint32_t mStartEnergy;
    uint32_t mAuraSpellId;
    uint32_t mGuardSpellId;
    char const* mAuraText;

    uint32_t mEnergy = 0;
    uint32_t mEnergyClockMs = 0;
    bool mGuarded = false;
};

class VoidspireVenelLightbloodAI : public VoidspireVanguardAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new VoidspireVenelLightbloodAI(c); }
    explicit VoidspireVenelLightbloodAI(Creature* pCreature)
        : VoidspireVanguardAI(pCreature, 0, SPELL_VENEL_AURA_OF_WRATH, SPELL_VENEL_AVENGING_WRATH, "Commander Venel Lightblood ignites his Aura of Wrath and passes an Execution Sentence!") {}

protected:
    void scheduleKit() override
    {
        scriptEvents.addEvent(EVENT_VENEL_JUDGMENT, 8000);
        scriptEvents.addEvent(EVENT_VENEL_DIVINE_STORM, 16000);
        scriptEvents.addEvent(EVENT_VENEL_SACRED_TOLL, 24000);
    }

    // the whole raid shares the sentence
    void unleashFinisher() override { castAtPlayer(SPELL_VENEL_EXECUTION_SENTENCE, true); }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_VENEL_JUDGMENT:
                castSpellOnVictim(SPELL_VENEL_JUDGMENT);
                scriptEvents.addEvent(EVENT_VENEL_FINAL_VERDICT, 3000);
                scriptEvents.addEvent(EVENT_VENEL_JUDGMENT, 18000);
                break;
            case EVENT_VENEL_FINAL_VERDICT:
                castSpellOnVictim(SPELL_VENEL_FINAL_VERDICT);
                break;
            case EVENT_VENEL_DIVINE_STORM:
                castSpellAOE(SPELL_VENEL_DIVINE_STORM);
                scriptEvents.addEvent(EVENT_VENEL_DIVINE_STORM, 25000);
                break;
            case EVENT_VENEL_SACRED_TOLL:
                castSpellAOE(SPELL_VENEL_SACRED_TOLL);
                scriptEvents.addEvent(EVENT_VENEL_SACRED_TOLL, 30000);
                break;
            default:
                break;
        }
    }
};

class VoidspireAmiasBellamyAI : public VoidspireVanguardAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new VoidspireAmiasBellamyAI(c); }
    explicit VoidspireAmiasBellamyAI(Creature* pCreature)
        : VoidspireVanguardAI(pCreature, 33, SPELL_BELLAMY_AURA_OF_DEVOTION, SPELL_VANGUARD_DIVINE_SHIELD, "General Amias Bellamy raises his Aura of Devotion and rings the Divine Toll!") {}

protected:
    void scheduleKit() override
    {
        scriptEvents.addEvent(EVENT_BELLAMY_JUDGMENT, 6000);
        scriptEvents.addEvent(EVENT_BELLAMY_AVENGERS_SHIELD, 10000);
    }

    void unleashFinisher() override { castSpellAOE(SPELL_BELLAMY_DIVINE_TOLL); }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_BELLAMY_JUDGMENT:
                castSpellOnVictim(SPELL_BELLAMY_JUDGMENT);
                scriptEvents.addEvent(EVENT_BELLAMY_SHIELD_OF_THE_RIGHTEOUS, 2500);
                scriptEvents.addEvent(EVENT_BELLAMY_JUDGMENT, 16000);
                break;
            case EVENT_BELLAMY_SHIELD_OF_THE_RIGHTEOUS:
                castSpellOnVictim(SPELL_BELLAMY_SHIELD_OF_THE_RIGHTEOUS);
                break;
            case EVENT_BELLAMY_AVENGERS_SHIELD:
                castAtPlayer(SPELL_BELLAMY_AVENGERS_SHIELD, true);
                scriptEvents.addEvent(EVENT_BELLAMY_AVENGERS_SHIELD, 20000);
                break;
            default:
                break;
        }
    }
};

class VoidspireChaplainSennAI : public VoidspireVanguardAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new VoidspireChaplainSennAI(c); }
    explicit VoidspireChaplainSennAI(Creature* pCreature)
        : VoidspireVanguardAI(pCreature, 66, SPELL_SENN_AURA_OF_PEACE, SPELL_VANGUARD_DIVINE_SHIELD, "War Chaplain Senn spreads her Aura of Peace and calls down Tyr's Wrath!") {}

protected:
    void scheduleKit() override
    {
        scriptEvents.addEvent(EVENT_SENN_EXORCISM, 5000);
        scriptEvents.addEvent(EVENT_SENN_SEARING_RADIANCE, 14000);
        scriptEvents.addEvent(EVENT_SENN_SACRED_SHIELD, 20000);
        scriptEvents.addEvent(EVENT_SENN_ELEKK_CHARGE, 26000);
    }

    void unleashFinisher() override { castAtPlayers(SPELL_SENN_TYRS_WRATH, 3); }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_SENN_EXORCISM:
                castAtPlayer(SPELL_SENN_EXORCISM);
                scriptEvents.addEvent(EVENT_SENN_EXORCISM, 12000);
                break;
            case EVENT_SENN_SEARING_RADIANCE:
                castSpellAOE(SPELL_SENN_SEARING_RADIANCE);
                scriptEvents.addEvent(EVENT_SENN_SEARING_RADIANCE, 24000);
                break;
            case EVENT_SENN_SACRED_SHIELD:
                castSpellOnSelf(SPELL_SENN_SACRED_SHIELD);
                scriptEvents.addEvent(EVENT_SENN_SACRED_SHIELD, 35000);
                break;
            case EVENT_SENN_ELEKK_CHARGE:
                castAtPlayer(SPELL_SENN_ELEKK_CHARGE, true);
                scriptEvents.addEvent(EVENT_SENN_ELEKK_CHARGE, 30000);
                break;
            default:
                break;
        }
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Crown of the Cosmos

// held back by the Undying Sentinels and the Rift Simulacrum, breaks free at the end
class VoidspireAlleriaAI : public VoidspireBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new VoidspireAlleriaAI(c); }
    explicit VoidspireAlleriaAI(Creature* pCreature) : VoidspireBossAI(pCreature, DATA_CROWN_OF_THE_COSMOS) {}

    void DoAction(int32_t action) override
    {
        if (!_isInCombat() || mFreed)
            return;

        if (action == ACTION_ALLERIA_SENTINEL_FALLEN && mStage == CROWN_STAGE_SENTINELS)
        {
            if (++mSentinelsFallen >= CrownSentinelCount)
                beginCrushingSingularity();
        }
        else if (action == ACTION_ALLERIA_SIMULACRUM_SHATTERED && mStage == CROWN_STAGE_SIMULACRUM)
        {
            beginShatteringSingularity();
        }
    }

    void DamageTaken(Unit* /*pAttacker*/, uint32_t* damage) override
    {
        if (damage == nullptr)
            return;

        // untouchable until the simulacrum is shattered
        if (mStage != CROWN_STAGE_FINAL || mFreed)
        {
            *damage = 0;
            return;
        }

        uint32_t const health = getCreature()->getHealth();
        if (*damage < health)
            return;

        *damage = health > 1 ? health - 1 : 0;
        breakFree();
    }

protected:
    void onEngage() override
    {
        mFreed = false;
        mSentinelsFallen = 0;
        mStage = CROWN_STAGE_SENTINELS;
        armBerserk(CrownBerserkMs);
        holdBack(true);

        scriptEvents.addEvent(EVENT_ALLERIA_SILVERSTRIKE_ARROW, 8000);
        scriptEvents.addEvent(EVENT_ALLERIA_GRASP_OF_EMPTINESS, 14000);
        scriptEvents.addEvent(EVENT_ALLERIA_VOID_EXPULSION, 20000);
        scriptEvents.addEvent(EVENT_ALLERIA_NULL_CORONA, 26000);
        scriptEvents.addEvent(EVENT_ALLERIA_VOID_DROPLETS, 30000);

        raiseSentinels();
    }

    void onDisengage() override
    {
        mStage = CROWN_STAGE_NONE;
        if (!mFreed)
            holdBack(false);
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_ALLERIA_SILVERSTRIKE_ARROW:
                castAtPlayer(SPELL_ALLERIA_SILVERSTRIKE_ARROW, true);
                scriptEvents.addEvent(EVENT_ALLERIA_SILVERSTRIKE_ARROW, 16000);
                break;
            case EVENT_ALLERIA_GRASP_OF_EMPTINESS:
                castAtPlayer(mStage == CROWN_STAGE_FINAL ? SPELL_ALLERIA_GRASP_OF_EMPTINESS_FINAL : SPELL_ALLERIA_GRASP_OF_EMPTINESS);
                scriptEvents.addEvent(EVENT_ALLERIA_GRASP_OF_EMPTINESS, 22000);
                break;
            case EVENT_ALLERIA_VOID_EXPULSION:
                if (isHeroic())
                    castAtPlayers(SPELL_ALLERIA_VOID_EXPULSION, 3);
                else
                    castAtPlayer(SPELL_ALLERIA_VOID_EXPULSION);
                scriptEvents.addEvent(EVENT_ALLERIA_VOID_EXPULSION, 30000);
                break;
            case EVENT_ALLERIA_NULL_CORONA:
                castAtPlayer(SPELL_ALLERIA_NULL_CORONA, false, true);
                scriptEvents.addEvent(EVENT_ALLERIA_NULL_CORONA, 32000);
                break;
            case EVENT_ALLERIA_VOID_DROPLETS:
                for (Unit* pTarget : pickPlayers(2, true))
                    callAdd(NPC_CROWN_VOID_DROPLET, voidspirePointAround(pTarget->GetPosition(), 3.0f, 6.0f));
                scriptEvents.addEvent(EVENT_ALLERIA_VOID_DROPLETS, 35000);
                break;
            case EVENT_ALLERIA_SILVERSTRIKE_BARRAGE:
                castSpellAOE(SPELL_ALLERIA_SILVERSTRIKE_BARRAGE);
                break;
            case EVENT_ALLERIA_STELLAR_EMISSION:
                castSpellAOE(SPELL_ALLERIA_STELLAR_EMISSION);
                break;
            case EVENT_ALLERIA_SINGULARITY_ERUPTION:
                warn("The singularity erupts!");
                castSpellAOE(SPELL_ALLERIA_SINGULARITY_ERUPTION);
                break;
            case EVENT_ALLERIA_ORBITING_MATTER:
                castSpellAOE(SPELL_ALLERIA_ORBITING_MATTER);
                break;
            case EVENT_ALLERIA_NEXT_STAGE:
                if (mStage == CROWN_STAGE_CRUSHING)
                    openSeveredRift();
                else if (mStage == CROWN_STAGE_SHATTERING)
                    releaseFullPower();
                break;
            case EVENT_ALLERIA_VOLATILE_FISSURE:
                castAtPlayer(SPELL_ALLERIA_VOLATILE_FISSURE);
                scriptEvents.addEvent(EVENT_ALLERIA_VOLATILE_FISSURE, 20000);
                break;
            case EVENT_ALLERIA_RANGER_CAPTAINS_MARK:
                castAtPlayer(SPELL_ALLERIA_RANGER_CAPTAINS_MARK, true);
                scriptEvents.addEvent(EVENT_ALLERIA_RANGER_CAPTAINS_MARK, 30000);
                break;
            case EVENT_ALLERIA_VOIDSTALKER_STING:
                castAtPlayer(SPELL_ALLERIA_VOIDSTALKER_STING);
                scriptEvents.addEvent(EVENT_ALLERIA_VOIDSTALKER_STING, 18000);
                break;
            case EVENT_ALLERIA_CALL_OF_THE_VOID:
                callOfTheVoid();
                scriptEvents.addEvent(EVENT_ALLERIA_CALL_OF_THE_VOID, 45000);
                break;
            case EVENT_ALLERIA_DEVOURING_COSMOS:
                warn("Alleria Windrunner unleashes the Devouring Cosmos!");
                castSpellAOE(SPELL_ALLERIA_DEVOURING_COSMOS);
                scriptEvents.addEvent(EVENT_ALLERIA_DEVOURING_COSMOS, 40000);
                break;
            case EVENT_ALLERIA_ASPECT_OF_THE_END:
                castSpellOnVictim(SPELL_ALLERIA_ASPECT_OF_THE_END);
                scriptEvents.addEvent(EVENT_ALLERIA_ASPECT_OF_THE_END, 25000);
                break;
            case EVENT_ALLERIA_DARK_RUSH:
                castAtPlayer(SPELL_ALLERIA_DARK_RUSH, true);
                scriptEvents.addEvent(EVENT_ALLERIA_DARK_RUSH, 35000);
                break;
            default:
                break;
        }
    }

private:
    void raiseSentinels()
    {
        LocationVector const origin = getCreature()->GetPosition();

        uint32_t raised = 0;
        for (uint32_t i = 0; i < CrownSentinelCount; ++i)
        {
            float const angle = origin.o + VoidspireTwoPi * static_cast<float>(i) / static_cast<float>(CrownSentinelCount);
            if (callAdd(CrownSentinels[i], voidspirePointOnCircle(origin, CrownSentinelRange, angle)) != nullptr)
                ++raised;
        }

        // missing guards count as fallen
        mSentinelsFallen = CrownSentinelCount - raised;
        if (mSentinelsFallen >= CrownSentinelCount)
            beginCrushingSingularity();
    }

    void beginCrushingSingularity()
    {
        mStage = CROWN_STAGE_CRUSHING;
        scriptEvents.resetEvents();
        warn("With the Undying Sentinels destroyed, Alleria Windrunner crashes into the platform!");
        castSpellAOE(SPELL_ALLERIA_SILVERSTRIKE_BARRAGE);

        scriptEvents.addEvent(EVENT_ALLERIA_STELLAR_EMISSION, 4000);
        scriptEvents.addEvent(EVENT_ALLERIA_SINGULARITY_ERUPTION, 9000);
        scriptEvents.addEvent(EVENT_ALLERIA_NEXT_STAGE, 15000);
    }

    void openSeveredRift()
    {
        mStage = CROWN_STAGE_SIMULACRUM;
        warn("Alleria Windrunner tears open a Spatial Rift and summons a Rift Simulacrum!");

        // without a simulacrum the stage ends at once
        if (callAdd(NPC_CROWN_RIFT_SIMULACRUM, aroundSelf(6.0f, 10.0f)) == nullptr)
        {
            beginShatteringSingularity();
            return;
        }

        scriptEvents.addEvent(EVENT_ALLERIA_VOIDSTALKER_STING, 6000);
        scriptEvents.addEvent(EVENT_ALLERIA_VOLATILE_FISSURE, 10000);
        scriptEvents.addEvent(EVENT_ALLERIA_CALL_OF_THE_VOID, 14000);
        scriptEvents.addEvent(EVENT_ALLERIA_RANGER_CAPTAINS_MARK, 18000);
        scriptEvents.addEvent(EVENT_ALLERIA_VOID_EXPULSION, 24000);
    }

    void beginShatteringSingularity()
    {
        mStage = CROWN_STAGE_SHATTERING;
        scriptEvents.resetEvents();
        warn("The Rift Simulacrum shatters! The singularity tears apart!");
        castSpellAOE(SPELL_ALLERIA_STELLAR_EMISSION);

        scriptEvents.addEvent(EVENT_ALLERIA_SILVERSTRIKE_BARRAGE, 5000);
        scriptEvents.addEvent(EVENT_ALLERIA_ORBITING_MATTER, 10000);
        scriptEvents.addEvent(EVENT_ALLERIA_NEXT_STAGE, 16000);
    }

    void releaseFullPower()
    {
        mStage = CROWN_STAGE_FINAL;
        holdBack(false);
        warn("Alleria Windrunner releases her full Void power!");

        scriptEvents.addEvent(EVENT_ALLERIA_VOIDSTALKER_STING, 6000);
        scriptEvents.addEvent(EVENT_ALLERIA_ASPECT_OF_THE_END, 8000);
        scriptEvents.addEvent(EVENT_ALLERIA_GRASP_OF_EMPTINESS, 12000);
        scriptEvents.addEvent(EVENT_ALLERIA_NULL_CORONA, 16000);
        scriptEvents.addEvent(EVENT_ALLERIA_DEVOURING_COSMOS, 20000);
        scriptEvents.addEvent(EVENT_ALLERIA_DARK_RUSH, 26000);
    }

    void callOfTheVoid()
    {
        castSpellAOE(SPELL_ALLERIA_CALL_OF_THE_VOID);

        uint32_t const count = isHeroic() ? 3 : 2;
        for (uint32_t i = 0; i < count; ++i)
            callAdd(NPC_CROWN_UNDYING_VOIDSPAWN, aroundSelf(10.0f, 18.0f));
    }

    // she is freed from the Void instead of dying
    void breakFree()
    {
        mFreed = true;
        armBerserk(0);
        scriptEvents.resetEvents();
        summons.despawnAll();
        getCreature()->interruptSpell();
        warn("Alleria Windrunner breaks free of the Void's grasp!");

        holdBack(true);
        addUnitFlags(UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);

        if (VoidspireInstanceScript* pVoidspire = getVoidspire())
        {
            pVoidspire->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
            pVoidspire->setBossState(mDataId, EncounterStates::Performed);
        }

        despawn(10000, 0);
    }

    void holdBack(bool held)
    {
        setRooted(held);
        _setMeleeDisabled(held);
    }

    uint8_t mStage = CROWN_STAGE_NONE;
    uint32_t mSentinelsFallen = 0;
    bool mFreed = false;
};

// bound to its portal, Echoing Darkness when nobody is in melee
class VoidspireUndyingSentinelAI : public VoidspireKitAI
{
public:
    VoidspireUndyingSentinelAI(Creature* pCreature, VoidspireKitSpell signature) : VoidspireKitAI(pCreature, { signature }) {}

    void OnDied(Unit* /*pKiller*/) override
    {
        if (VoidspireInstanceScript* pVoidspire = getVoidspire())
            pVoidspire->signalAlleria(ACTION_ALLERIA_SENTINEL_FALLEN);
    }

protected:
    void onKitEngage() override
    {
        if (!mHasPortal)
        {
            mPortal = getCreature()->GetPosition();
            mHasPortal = true;
        }

        castSpellOnSelf(SPELL_SENTINEL_UMBRAL_TETHER, true);
        scriptEvents.addEvent(EVENT_SENTINEL_MELEE_CHECK, 4000);
        scriptEvents.addEvent(EVENT_SENTINEL_LEASH, 2000);
    }

    void onExtraEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_SENTINEL_MELEE_CHECK:
                if (!anyPlayerWithin(SentinelMeleeReach))
                    castSpellAOE(SPELL_SENTINEL_ECHOING_DARKNESS);
                scriptEvents.addEvent(EVENT_SENTINEL_MELEE_CHECK, 4000);
                break;
            case EVENT_SENTINEL_LEASH:
                if (getCreature()->getDistance(mPortal) > SentinelLeashRange)
                    MoveTeleport(mPortal);
                scriptEvents.addEvent(EVENT_SENTINEL_LEASH, 2000);
                break;
            default:
                break;
        }
    }

private:
    LocationVector mPortal;
    bool mHasPortal = false;
};

class VoidspireMoriumAI : public VoidspireUndyingSentinelAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new VoidspireMoriumAI(c); }
    explicit VoidspireMoriumAI(Creature* pCreature) : VoidspireUndyingSentinelAI(pCreature, { SPELL_MORIUM_DARK_HAND, AIM_VICTIM, 6000, 15000 }) {}
};

class VoidspireDemiarAI : public VoidspireUndyingSentinelAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new VoidspireDemiarAI(c); }
    explicit VoidspireDemiarAI(Creature* pCreature) : VoidspireUndyingSentinelAI(pCreature, { SPELL_DEMIAR_INTERRUPTING_TREMOR, AIM_AROUND, 9000, 20000 }) {}
};

class VoidspireVorelusAI : public VoidspireUndyingSentinelAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new VoidspireVorelusAI(c); }
    explicit VoidspireVorelusAI(Creature* pCreature) : VoidspireUndyingSentinelAI(pCreature, { SPELL_VORELUS_RAVENOUS_ABYSS, AIM_PLAYER, 12000, 25000 }) {}
};

class VoidspireRiftSimulacrumAI : public VoidspireKitAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new VoidspireRiftSimulacrumAI(c); }
    explicit VoidspireRiftSimulacrumAI(Creature* pCreature)
        : VoidspireKitAI(pCreature, {
            { SPELL_SIMULACRUM_RIFT_SLASH, AIM_VICTIM, 5000, 9000 },
            { SPELL_ALLERIA_NULL_CORONA, AIM_PLAYER, 12000, 25000 },
            { SPELL_SIMULACRUM_EMPOWERING_DARKNESS, AIM_AROUND, 18000, 30000 },
            { SPELL_SIMULACRUM_COSMIC_BARRIER, AIM_SELF, 25000, 40000 } }) {}

    void OnDied(Unit* /*pKiller*/) override
    {
        if (VoidspireInstanceScript* pVoidspire = getVoidspire())
            pVoidspire->signalAlleria(ACTION_ALLERIA_SIMULACRUM_SHATTERED);
    }
};

class VoidspireUndyingVoidspawnAI : public VoidspireKitAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new VoidspireUndyingVoidspawnAI(c); }
    explicit VoidspireUndyingVoidspawnAI(Creature* pCreature)
        : VoidspireKitAI(pCreature, { { SPELL_VOIDSPAWN_VOID_BARRAGE, AIM_PLAYER, 4000, 10000 } }) {}

protected:
    void onKitEngage() override { castSpellOnSelf(SPELL_VOIDSPAWN_UMBRAL_TETHER, true); }
};

// splashes Corrupting Essence when it dies
class VoidspireVoidDropletAI : public VoidspireBursterAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new VoidspireVoidDropletAI(c); }
    explicit VoidspireVoidDropletAI(Creature* pCreature) : VoidspireBursterAI(pCreature, {}, SPELL_DROPLET_CORRUPTING_ESSENCE) {}
};

void SetupTheVoidspire(ScriptMgr* mgr)
{
    mgr->register_instance_script(MAP_THE_VOIDSPIRE, &VoidspireInstanceScript::Create);

    // Imperator Averzian
    mgr->register_creature_script(BOSS_IMPERATOR_AVERZIAN, &VoidspireAverzianAI::Create);
    mgr->register_creature_script(NPC_AVERZIAN_ABYSSAL_VOIDSHAPER, &VoidspireAbyssalVoidshaperAI::Create);
    mgr->register_creature_script(NPC_AVERZIAN_VOIDMAW, &VoidspireVoidmawAI::Create);
    mgr->register_creature_script(NPC_AVERZIAN_SHADOWGUARD_STALWART, &VoidspireShadowguardStalwartAI::Create);
    mgr->register_creature_script(NPC_AVERZIAN_VOIDBOUND_ANNIHILATOR, &VoidspireVoidboundAnnihilatorAI::Create);

    // Vorasius
    mgr->register_creature_script(BOSS_VORASIUS, &VoidspireVorasiusAI::Create);
    mgr->register_creature_script(NPC_VORASIUS_BLISTERCREEP, &VoidspireBlistercreepAI::Create);
    mgr->register_creature_script(NPC_VORASIUS_BLISTERCREEP_ALT, &VoidspireBlistercreepAI::Create);

    // Vaelgor & Ezzorak
    mgr->register_creature_script(BOSS_VAELGOR, &VoidspireVaelgorAI::Create);
    mgr->register_creature_script(BOSS_EZZORAK, &VoidspireEzzorakAI::Create);
    mgr->register_creature_script(NPC_EZZORAK_VOIDORB, &VoidspireVoidorbAI::Create);

    // Fallen-King Salhadaar
    mgr->register_creature_script(BOSS_FALLEN_KING_SALHADAAR, &VoidspireSalhadaarAI::Create);
    mgr->register_creature_script(NPC_SALHADAAR_CONCENTRATED_VOID, &VoidspireConcentratedVoidAI::Create);
    mgr->register_creature_script(NPC_SALHADAAR_FRACTURED_IMAGE, &VoidspireFracturedImageAI::Create);

    // Lightblinded Vanguard
    mgr->register_creature_script(BOSS_COMMANDER_VENEL_LIGHTBLOOD, &VoidspireVenelLightbloodAI::Create);
    mgr->register_creature_script(BOSS_GENERAL_AMIAS_BELLAMY, &VoidspireAmiasBellamyAI::Create);
    mgr->register_creature_script(BOSS_WAR_CHAPLAIN_SENN, &VoidspireChaplainSennAI::Create);

    // Crown of the Cosmos
    mgr->register_creature_script(BOSS_ALLERIA_WINDRUNNER, &VoidspireAlleriaAI::Create);
    mgr->register_creature_script(BOSS_ALLERIA_WINDRUNNER_ALT, &VoidspireAlleriaAI::Create);
    mgr->register_creature_script(NPC_CROWN_MORIUM, &VoidspireMoriumAI::Create);
    mgr->register_creature_script(NPC_CROWN_DEMIAR, &VoidspireDemiarAI::Create);
    mgr->register_creature_script(NPC_CROWN_VORELUS, &VoidspireVorelusAI::Create);
    mgr->register_creature_script(NPC_CROWN_RIFT_SIMULACRUM, &VoidspireRiftSimulacrumAI::Create);
    mgr->register_creature_script(NPC_CROWN_UNDYING_VOIDSPAWN, &VoidspireUndyingVoidspawnAI::Create);
    mgr->register_creature_script(NPC_CROWN_VOID_DROPLET, &VoidspireVoidDropletAI::Create);
}
