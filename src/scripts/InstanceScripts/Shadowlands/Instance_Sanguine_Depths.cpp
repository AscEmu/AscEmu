/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "Instance_Sanguine_Depths.hpp"

#include "Objects/Units/Creatures/Creature.h"
#include "Server/Script/CreatureAIScript.hpp"
#include "Server/Script/InstanceScript.hpp"
#include "Server/Script/ScriptMgr.hpp"
#include "Utilities/Random.hpp"

#include <algorithm>
#include <cmath>

enum SanguineDepthsEvents
{
    EVENT_KRYXIS_VICIOUS_HEADBUTT = 1,
    EVENT_KRYXIS_HUNGERING_DRAIN,
    EVENT_KRYXIS_JUGGERNAUT_RUSH,
    EVENT_KRYXIS_SEVERING_SMASH,

    EVENT_TARVOLD_CASTIGATE,
    EVENT_TARVOLD_COALESCE_MANIFESTATION,

    EVENT_BERYLLIA_IRON_SPIKES,
    EVENT_BERYLLIA_AGONIZE,
    EVENT_BERYLLIA_ENDLESS_TORMENT,
    EVENT_BERYLLIA_RITE_OF_SUPREMACY,

    EVENT_KAAL_WICKED_RUSH,
    EVENT_KAAL_WICKED_GASH,
    EVENT_KAAL_PIERCING_BLUR,
    EVENT_KAAL_GLOOM_SQUALL
};

namespace
{
    float const SanguineTwoPi = 6.28318531f;

    uint32_t const ManifestationFullEnergyMs = 30000;

    float const ZraliReach = 100.0f;

    uint32_t const KaalRetreatPct = 50;
    uint32_t const KaalVanishMs = 5000;

    LocationVector sanguineRing(LocationVector const& origin, float minRange, float maxRange)
    {
        float const angle = Util::getRandomFloat(SanguineTwoPi);
        float const range = Util::getRandomFloat(minRange, maxRange);
        return LocationVector(origin.x + range * std::cos(angle), origin.y + range * std::sin(angle), origin.z, angle);
    }

    // true once when the countdown runs out
    bool sanguineCountdown(uint32_t& leftMs, uint32_t diff)
    {
        if (leftMs == 0)
            return false;

        if (leftMs > diff)
        {
            leftMs -= diff;
            return false;
        }

        leftMs = 0;
        return true;
    }
}

//////////////////////////////////////////////////////////////////////////////////////////
// Instance

class SanguineDepthsInstanceScript : public InstanceScript
{
public:
    explicit SanguineDepthsInstanceScript(WorldMap* pMapMgr) : InstanceScript(pMapMgr)
    {
        setBossNumber(SanguineDepthsEncounterCount);
    }

    static InstanceScript* Create(WorldMap* pMapMgr) { return new SanguineDepthsInstanceScript(pMapMgr); }

    // a retreated Kaal does not come back
    void OnCreaturePushToWorld(Creature* pCreature) override
    {
        uint32_t const entry = pCreature->getEntry();
        if ((entry == BOSS_GENERAL_KAAL || entry == BOSS_GENERAL_KAAL_ALT) && getBossState(DATA_GENERAL_KAAL) == EncounterStates::Performed)
            pCreature->Despawn(1000, 0);
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Shared combat base: event clock and target helpers

class SanguineDepthsCombatAI : public CreatureAIScript
{
public:
    explicit SanguineDepthsCombatAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override { onEngage(); }

    void OnCombatStop(Unit* /*pTarget*/) override
    {
        scriptEvents.resetEvents();
        summons.despawnAll();
        onDisengage();
    }

    void onSummonedCreature(Creature* pSummon) override { summons.summon(pSummon); }

    void AIUpdate(unsigned long time_passed) override
    {
        if (!_isInCombat())
            return;

        uint32_t const diff = static_cast<uint32_t>(time_passed);
        scriptEvents.updateEvents(diff, 0);
        onCombatTick(diff);

        if (_isCasting())
            return;

        if (uint32_t const eventId = scriptEvents.getFinishedEvent())
            onEvent(eventId);
    }

protected:
    virtual void onEngage() {}
    virtual void onDisengage() {}
    virtual void onCombatTick(uint32_t /*diff*/) {}
    virtual void onEvent(uint32_t /*eventId*/) {}

    SanguineDepthsInstanceScript* getDepths()
    {
        if (getCreature()->GetMapId() != MAP_SANGUINE_DEPTHS)
            return nullptr;

        return static_cast<SanguineDepthsInstanceScript*>(getInstanceScript());
    }

    Unit* randomPlayer() { return selectUnitTarget(FilterArgs(TargetFilter_Player)); }

    // anyone but the tank, the tank when alone
    Unit* randomNonTank()
    {
        FilterArgs args(TargetFilter_Player);
        args.addFilter(TargetFilter_NotCurrent);
        if (Unit* pTarget = selectUnitTarget(args))
            return pTarget;

        return randomPlayer();
    }

    Unit* castAtRandom(uint32_t spellId, bool skipTank = false)
    {
        Unit* pTarget = skipTank ? randomNonTank() : randomPlayer();
        if (pTarget != nullptr)
            castSpell(pTarget, spellId);

        return pTarget;
    }

    // ground spell under a random player
    void castUnderRandom(uint32_t spellId)
    {
        if (Unit* pTarget = randomPlayer())
            getCreature()->castSpellLoc(pTarget->GetPosition(), spellId);
    }

    // summoned add on the side of its summoner that joins the fight at once
    Creature* callAdd(uint32_t entry, LocationVector const& spot)
    {
        Creature* pAdd = summonCreature(entry, spot, CORPSE_DESPAWN);
        if (pAdd == nullptr)
            return nullptr;

        pAdd->setFaction(getCreature()->getFactionTemplate());
        setZoneWideCombat(pAdd);
        return pAdd;
    }

    LocationVector aroundSelf(float minRange, float maxRange) { return sanguineRing(getCreature()->GetPosition(), minRange, maxRange); }

    Creature* zrali()
    {
        Creature* pZrali = findNearestCreature(NPC_ZRALI, ZraliReach);
        return pZrali != nullptr && pZrali->isAlive() ? pZrali : nullptr;
    }

    void warn(char const* text) { sendChatMessage(CHAT_MSG_RAID_BOSS_EMOTE, 0, text); }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Encounter base: frame and boss state

class SanguineDepthsBossAI : public SanguineDepthsCombatAI
{
public:
    SanguineDepthsBossAI(Creature* pCreature, uint32_t dataId) : SanguineDepthsCombatAI(pCreature), mDataId(dataId) {}

    void OnCombatStart(Unit* pTarget) override
    {
        if (SanguineDepthsInstanceScript* pDepths = getDepths())
        {
            pDepths->sendUnitEncounter(EncounterFrameEngage, getCreature());
            pDepths->setBossState(mDataId, EncounterStates::InProgress);
        }

        SanguineDepthsCombatAI::OnCombatStart(pTarget);
    }

    void OnCombatStop(Unit* pTarget) override
    {
        if (isAlive())
        {
            if (SanguineDepthsInstanceScript* pDepths = getDepths())
            {
                pDepths->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
                pDepths->setBossState(mDataId, EncounterStates::Failed);
            }
        }

        SanguineDepthsCombatAI::OnCombatStop(pTarget);
    }

    void OnDied(Unit* /*pKiller*/) override
    {
        completeEncounter();
        scriptEvents.resetEvents();
        summons.despawnAll();
    }

protected:
    void completeEncounter()
    {
        if (SanguineDepthsInstanceScript* pDepths = getDepths())
        {
            pDepths->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
            pDepths->setBossState(mDataId, EncounterStates::Performed);
        }
    }

    uint32_t mDataId;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Kryxis the Voracious

class SanguineDepthsKryxisAI : public SanguineDepthsBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new SanguineDepthsKryxisAI(c); }
    explicit SanguineDepthsKryxisAI(Creature* pCreature) : SanguineDepthsBossAI(pCreature, DATA_KRYXIS_THE_VORACIOUS) {}

protected:
    void onEngage() override
    {
        scriptEvents.addEvent(EVENT_KRYXIS_VICIOUS_HEADBUTT, 5000);
        scriptEvents.addEvent(EVENT_KRYXIS_HUNGERING_DRAIN, 10000);
        scriptEvents.addEvent(EVENT_KRYXIS_JUGGERNAUT_RUSH, 16000);
        // a full energy bar ends in a Severing Smash
        scriptEvents.addEvent(EVENT_KRYXIS_SEVERING_SMASH, 40000);
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_KRYXIS_VICIOUS_HEADBUTT:
                castSpellOnVictim(SPELL_KRYXIS_VICIOUS_HEADBUTT);
                scriptEvents.addEvent(EVENT_KRYXIS_VICIOUS_HEADBUTT, 12000);
                break;
            case EVENT_KRYXIS_HUNGERING_DRAIN:
                castSpellOnSelf(SPELL_KRYXIS_HUNGERING_DRAIN);
                scriptEvents.addEvent(EVENT_KRYXIS_HUNGERING_DRAIN, 19000);
                break;
            case EVENT_KRYXIS_JUGGERNAUT_RUSH:
                if (castAtRandom(SPELL_KRYXIS_JUGGERNAUT_RUSH, true) != nullptr)
                    warn("Kryxis the Voracious charges with a Juggernaut Rush!");
                scriptEvents.addEvent(EVENT_KRYXIS_JUGGERNAUT_RUSH, 22000);
                break;
            case EVENT_KRYXIS_SEVERING_SMASH:
                warn("Kryxis the Voracious unleashes a Severing Smash!");
                castSpellAOE(SPELL_KRYXIS_SEVERING_SMASH);
                scriptEvents.addEvent(EVENT_KRYXIS_SEVERING_SMASH, 45000);
                break;
            default:
                break;
        }
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Executor Tarvold

class SanguineDepthsExecutorTarvoldAI : public SanguineDepthsBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new SanguineDepthsExecutorTarvoldAI(c); }
    explicit SanguineDepthsExecutorTarvoldAI(Creature* pCreature) : SanguineDepthsBossAI(pCreature, DATA_EXECUTOR_TARVOLD) {}

protected:
    void onEngage() override
    {
        scriptEvents.addEvent(EVENT_TARVOLD_CASTIGATE, 6000);
        scriptEvents.addEvent(EVENT_TARVOLD_COALESCE_MANIFESTATION, 12000);
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_TARVOLD_CASTIGATE:
                castAtRandom(SPELL_TARVOLD_CASTIGATE, true);
                scriptEvents.addEvent(EVENT_TARVOLD_CASTIGATE, 15000);
                break;
            case EVENT_TARVOLD_COALESCE_MANIFESTATION:
                warn("Executor Tarvold releases a Fleeting Manifestation!");
                callAdd(NPC_TARVOLD_FLEETING_MANIFESTATION, aroundSelf(8.0f, 14.0f));
                scriptEvents.addEvent(EVENT_TARVOLD_COALESCE_MANIFESTATION, 25000);
                break;
            default:
                break;
        }
    }
};

// feeds on anima until it bursts, leaves Residue when slain as well
class SanguineDepthsFleetingManifestationAI : public SanguineDepthsCombatAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new SanguineDepthsFleetingManifestationAI(c); }
    explicit SanguineDepthsFleetingManifestationAI(Creature* pCreature) : SanguineDepthsCombatAI(pCreature) {}

    void DamageTaken(Unit* /*pAttacker*/, uint32_t* damage) override
    {
        if (damage == nullptr || mErupted || *damage < getCreature()->getHealth())
            return;

        mErupted = true;
        castSpellAOE(SPELL_MANIFESTATION_RESIDUE, true);
    }

protected:
    void onEngage() override
    {
        mErupted = false;
        mEnergyLeftMs = ManifestationFullEnergyMs;
        castSpellOnSelf(SPELL_MANIFESTATION_GROWING_PRIDE, true);
    }

    void onCombatTick(uint32_t diff) override
    {
        if (mErupted || !sanguineCountdown(mEnergyLeftMs, diff))
            return;

        mErupted = true;
        castSpellAOE(SPELL_MANIFESTATION_RESIDUE, true);
        despawn(500, 0);
    }

private:
    bool mErupted = false;
    uint32_t mEnergyLeftMs = 0;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Grand Proctor Beryllia

class SanguineDepthsGrandProctorBerylliaAI : public SanguineDepthsBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new SanguineDepthsGrandProctorBerylliaAI(c); }
    explicit SanguineDepthsGrandProctorBerylliaAI(Creature* pCreature) : SanguineDepthsBossAI(pCreature, DATA_GRAND_PROCTOR_BERYLLIA) {}

protected:
    void onEngage() override
    {
        scriptEvents.addEvent(EVENT_BERYLLIA_IRON_SPIKES, 5000);
        scriptEvents.addEvent(EVENT_BERYLLIA_AGONIZE, 9000);
        scriptEvents.addEvent(EVENT_BERYLLIA_ENDLESS_TORMENT, 18000);
        // a full energy bar ends in the Rite of Supremacy
        scriptEvents.addEvent(EVENT_BERYLLIA_RITE_OF_SUPREMACY, 40000);
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_BERYLLIA_IRON_SPIKES:
                castSpellOnVictim(SPELL_BERYLLIA_IRON_SPIKES);
                scriptEvents.addEvent(EVENT_BERYLLIA_IRON_SPIKES, 12000);
                break;
            case EVENT_BERYLLIA_AGONIZE:
                castAtRandom(SPELL_BERYLLIA_AGONIZE, true);
                scriptEvents.addEvent(EVENT_BERYLLIA_AGONIZE, 15000);
                break;
            case EVENT_BERYLLIA_ENDLESS_TORMENT:
                // the torment is drawn through the captive naaru
                if (Creature* pZrali = zrali())
                    castSpell(pZrali, SPELL_BERYLLIA_ENDLESS_TORMENT);
                else
                    castSpellAOE(SPELL_BERYLLIA_ANGUISHED_CRIES);
                scriptEvents.addEvent(EVENT_BERYLLIA_ENDLESS_TORMENT, 25000);
                break;
            case EVENT_BERYLLIA_RITE_OF_SUPREMACY:
                warn("Grand Proctor Beryllia prepares the Rite of Supremacy! Seek Z'rali's Shining Radiance!");
                if (Creature* pZrali = zrali())
                    pZrali->castSpellLoc(aroundSelf(8.0f, 15.0f), SPELL_ZRALI_SHINING_RADIANCE_RITE, true);
                castSpellAOE(SPELL_BERYLLIA_RITE_OF_SUPREMACY);
                scriptEvents.addEvent(EVENT_BERYLLIA_RITE_OF_SUPREMACY, 40000);
                break;
            default:
                break;
        }
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// General Kaal, retreats at half health

class SanguineDepthsGeneralKaalAI : public SanguineDepthsBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new SanguineDepthsGeneralKaalAI(c); }
    explicit SanguineDepthsGeneralKaalAI(Creature* pCreature) : SanguineDepthsBossAI(pCreature, DATA_GENERAL_KAAL) {}

    void OnCombatStop(Unit* pTarget) override
    {
        // leaving after the retreat is no wipe
        if (mRetreated)
        {
            SanguineDepthsCombatAI::OnCombatStop(pTarget);
            return;
        }

        SanguineDepthsBossAI::OnCombatStop(pTarget);
    }

    void DamageTaken(Unit* /*pAttacker*/, uint32_t* damage) override
    {
        if (damage == nullptr)
            return;

        if (mRetreated)
        {
            *damage = 0;
            return;
        }

        uint32_t const floor = std::max<uint32_t>(getCreature()->getMaxHealth() / 100 * KaalRetreatPct, 1);
        uint32_t const health = getCreature()->getHealth();
        if (health > *damage + floor)
            return;

        *damage = health > floor ? health - floor : 0;
        mRetreatPending = true;
    }

protected:
    void onEngage() override
    {
        mRetreatPending = false;
        scriptEvents.addEvent(EVENT_KAAL_WICKED_RUSH, 8000);
        scriptEvents.addEvent(EVENT_KAAL_PIERCING_BLUR, 13000);
        scriptEvents.addEvent(EVENT_KAAL_GLOOM_SQUALL, 25000);
    }

    void onCombatTick(uint32_t /*diff*/) override
    {
        if (mRetreatPending && !mRetreated)
            retreat();
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_KAAL_WICKED_RUSH:
                castSpellAOE(SPELL_KAAL_WICKED_RUSH);
                scriptEvents.addEvent(EVENT_KAAL_WICKED_GASH, 2000);
                scriptEvents.addEvent(EVENT_KAAL_WICKED_RUSH, 18000);
                break;
            case EVENT_KAAL_WICKED_GASH:
                castSpellAOE(SPELL_KAAL_WICKED_GASH, true);
                break;
            case EVENT_KAAL_PIERCING_BLUR:
                castUnderRandom(SPELL_KAAL_PIERCING_BLUR);
                scriptEvents.addEvent(EVENT_KAAL_PIERCING_BLUR, 15000);
                break;
            case EVENT_KAAL_GLOOM_SQUALL:
                warn("General Kaal gathers a Gloom Squall! Seek Z'rali's Shining Radiance!");
                if (Creature* pZrali = zrali())
                    pZrali->castSpell(pZrali, SPELL_ZRALI_SHINING_RADIANCE_SQUALL, true);
                castSpellAOE(SPELL_KAAL_GLOOM_SQUALL);
                scriptEvents.addEvent(EVENT_KAAL_GLOOM_SQUALL, 30000);
                break;
            default:
                break;
        }
    }

private:
    // the encounter ends here, she leaves the battlefield
    void retreat()
    {
        mRetreatPending = false;
        mRetreated = true;
        warn("General Kaal breaks off the fight and retreats into the depths!");

        getCreature()->interruptSpell();
        scriptEvents.resetEvents();
        summons.despawnAll();
        completeEncounter();

        setReactState(REACT_PASSIVE);
        attackStop();
        addUnitFlags(UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
        _wipeHateList();
        despawn(KaalVanishMs, 0);
    }

    bool mRetreatPending = false;
    bool mRetreated = false;
};

void SetupSanguineDepths(ScriptMgr* mgr)
{
    mgr->register_instance_script(MAP_SANGUINE_DEPTHS, &SanguineDepthsInstanceScript::Create);

    // Kryxis the Voracious
    mgr->register_creature_script(BOSS_KRYXIS_THE_VORACIOUS, &SanguineDepthsKryxisAI::Create);

    // Executor Tarvold
    mgr->register_creature_script(BOSS_EXECUTOR_TARVOLD, &SanguineDepthsExecutorTarvoldAI::Create);
    mgr->register_creature_script(NPC_TARVOLD_FLEETING_MANIFESTATION, &SanguineDepthsFleetingManifestationAI::Create);
    mgr->register_creature_script(NPC_TARVOLD_FLEETING_MANIFESTATION_ALT, &SanguineDepthsFleetingManifestationAI::Create);

    // Grand Proctor Beryllia
    mgr->register_creature_script(BOSS_GRAND_PROCTOR_BERYLLIA, &SanguineDepthsGrandProctorBerylliaAI::Create);

    // General Kaal
    mgr->register_creature_script(BOSS_GENERAL_KAAL, &SanguineDepthsGeneralKaalAI::Create);
    mgr->register_creature_script(BOSS_GENERAL_KAAL_ALT, &SanguineDepthsGeneralKaalAI::Create);
}
