/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "Instance_The_Motherlode.hpp"

#include "Objects/Units/Creatures/Creature.h"
#include "Server/Script/CreatureAIScript.hpp"
#include "Server/Script/InstanceScript.hpp"
#include "Server/Script/ScriptMgr.hpp"
#include "Utilities/Random.hpp"

#include <cmath>
#include <list>
#include <vector>

enum TheMotherlodeEvents
{
    EVENT_PUMMELER_STATIC_PULSE = 1,
    EVENT_PUMMELER_FOOTBOMBS,
    EVENT_PUMMELER_SHOCKING_CLAW,
    EVENT_PUMMELER_COIN_MAGNET,
    EVENT_PUMMELER_PAY_TO_WIN,

    EVENT_AZEROKK_RESONANT_PULSE,
    EVENT_AZEROKK_QUAKE_ECHO,
    EVENT_AZEROKK_INFUSION,
    EVENT_AZEROKK_TECTONIC_SMASH,
    EVENT_AZEROKK_CALL_EARTHRAGER,

    EVENT_EARTHRAGER_FIXATE,

    EVENT_RIXXA_SEARING_REAGENT,
    EVENT_RIXXA_PROPELLANT_BLAST,
    EVENT_RIXXA_CHEMICAL_BURN,

    EVENT_RAZDUNK_GATLING_GUN,
    EVENT_RAZDUNK_ALPHA_CANNON,
    EVENT_RAZDUNK_DRILL_SMASH,
    EVENT_RAZDUNK_DRILL_IMPACT,
    EVENT_RAZDUNK_MICRO_MISSILES
};

namespace
{
    // quarry floor around Azerokk
    LocationVector const EarthragerSpots[] =
    {
        { 1233.36f, -3321.13f, 57.50f },
        { 1216.14f, -3340.71f, 55.34f },
        { 1199.50f, -3330.84f, 55.80f },
        { 1202.84f, -3315.90f, 55.58f }
    };
    uint32_t const EarthragerSpotCount = 4;
    float const EarthragerSearchRange = 45.0f;

    uint32_t const FootbombVolley = 3;
    uint32_t const FootbombFuseMs = 15000;

    float const MotherlodeFullCircle = 6.2831853f;

    // random point on a ring around origin
    LocationVector motherlodeRingPoint(LocationVector const& origin, float minRange, float maxRange)
    {
        float const angle = Util::getRandomFloat(MotherlodeFullCircle);
        float const range = Util::getRandomFloat(minRange, maxRange);
        return LocationVector(origin.x + range * std::cos(angle), origin.y + range * std::sin(angle), origin.z, angle);
    }
}

//////////////////////////////////////////////////////////////////////////////////////////
// Instance

class TheMotherlodeInstanceScript : public InstanceScript
{
public:
    explicit TheMotherlodeInstanceScript(WorldMap* pMapMgr) : InstanceScript(pMapMgr)
    {
        setBossNumber(TheMotherlodeEncounterCount);
    }

    static InstanceScript* Create(WorldMap* pMapMgr) { return new TheMotherlodeInstanceScript(pMapMgr); }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Shared combat base: event clock while in combat

class TheMotherlodeCombatAI : public CreatureAIScript
{
public:
    explicit TheMotherlodeCombatAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override { onEngage(); }

    void OnCombatStop(Unit* /*pTarget*/) override
    {
        scriptEvents.resetEvents();
        summons.despawnAll();

        if (isAlive())
            onReset();
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
    virtual void onReset() {}
    virtual void onCombatTick(uint32_t /*diff*/) {}
    virtual void onEvent(uint32_t /*eventId*/) {}

    Unit* anyPlayer() { return selectUnitTarget(FilterArgs(TargetFilter_Player)); }

    void castAt(Unit* pTarget, uint32_t spellId, bool triggered = false)
    {
        if (pTarget != nullptr)
            castSpell(pTarget, spellId, triggered);
    }

    Creature* callAdd(uint32_t entry, LocationVector const& position)
    {
        Creature* pAdd = summonCreature(entry, position, CORPSE_DESPAWN);
        if (pAdd != nullptr)
            setZoneWideCombat(pAdd);

        return pAdd;
    }

    void warn(char const* text) { sendChatMessage(CHAT_MSG_RAID_BOSS_EMOTE, 0, text); }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Encounter base: frame and boss state

class TheMotherlodeBossAI : public TheMotherlodeCombatAI
{
public:
    TheMotherlodeBossAI(Creature* pCreature, uint32_t dataId) : TheMotherlodeCombatAI(pCreature), mDataId(dataId) {}

    void OnCombatStart(Unit* pTarget) override
    {
        if (InstanceScript* pInstance = getInstanceScript())
        {
            pInstance->sendUnitEncounter(EncounterFrameEngage, getCreature());
            pInstance->setBossState(mDataId, EncounterStates::InProgress);
        }

        TheMotherlodeCombatAI::OnCombatStart(pTarget);
    }

    void OnCombatStop(Unit* pTarget) override
    {
        if (isAlive())
        {
            if (InstanceScript* pInstance = getInstanceScript())
            {
                pInstance->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
                pInstance->setBossState(mDataId, EncounterStates::Failed);
            }
        }

        TheMotherlodeCombatAI::OnCombatStop(pTarget);
    }

    void OnDied(Unit* /*pKiller*/) override
    {
        if (InstanceScript* pInstance = getInstanceScript())
        {
            pInstance->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
            pInstance->setBossState(mDataId, EncounterStates::Performed);
        }

        scriptEvents.resetEvents();
        summons.despawnAll();
    }

protected:
    uint32_t mDataId;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Coin-Operated Crowd Pummeler

class TheMotherlodePummelerAI : public TheMotherlodeBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new TheMotherlodePummelerAI(c); }
    explicit TheMotherlodePummelerAI(Creature* pCreature) : TheMotherlodeBossAI(pCreature, DATA_CROWD_PUMMELER) {}

protected:
    void onEngage() override
    {
        scriptEvents.addEvent(EVENT_PUMMELER_FOOTBOMBS, 4000);
        scriptEvents.addEvent(EVENT_PUMMELER_SHOCKING_CLAW, 7000);
        scriptEvents.addEvent(EVENT_PUMMELER_STATIC_PULSE, 8000);

        // heroic: coin magnet and pay to win
        if (isHeroic())
        {
            scriptEvents.addEvent(EVENT_PUMMELER_PAY_TO_WIN, 1000);
            scriptEvents.addEvent(EVENT_PUMMELER_COIN_MAGNET, 8000);
        }
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_PUMMELER_STATIC_PULSE:
                warn("The Coin-Operated Crowd Pummeler releases a Static Pulse!");
                castSpellOnSelf(SPELL_PUMMELER_STATIC_PULSE);
                scriptEvents.addEvent(EVENT_PUMMELER_STATIC_PULSE, 20000);
                break;
            case EVENT_PUMMELER_FOOTBOMBS:
                launchFootbombs();
                scriptEvents.addEvent(EVENT_PUMMELER_FOOTBOMBS, 38000);
                break;
            case EVENT_PUMMELER_SHOCKING_CLAW:
                castSpellOnVictim(SPELL_PUMMELER_SHOCKING_CLAW);
                scriptEvents.addEvent(EVENT_PUMMELER_SHOCKING_CLAW, 30000);
                break;
            case EVENT_PUMMELER_COIN_MAGNET:
                castSpellOnSelf(SPELL_PUMMELER_COIN_MAGNET);
                scriptEvents.addEvent(EVENT_PUMMELER_COIN_MAGNET, 30000);
                break;
            case EVENT_PUMMELER_PAY_TO_WIN:
                castSpellOnSelf(SPELL_PUMMELER_PAY_TO_WIN, true);
                scriptEvents.addEvent(EVENT_PUMMELER_PAY_TO_WIN, 30000);
                break;
            default:
                break;
        }
    }

private:
    // bombs land around the pit and blow after their fuse
    void launchFootbombs()
    {
        castSpellOnSelf(SPELL_PUMMELER_FOOTBOMB_LAUNCHER, true);

        LocationVector const origin = getCreature()->GetPosition();
        for (uint32_t i = 0; i < FootbombVolley; ++i)
            summonCreature(NPC_AZERITE_FOOTBOMB, motherlodeRingPoint(origin, 6.0f, 14.0f), TIMED_DESPAWN, FootbombFuseMs + 3000);
    }
};

// sits still and detonates when the fuse runs out
class TheMotherlodeFootbombAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new TheMotherlodeFootbombAI(c); }
    explicit TheMotherlodeFootbombAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnLoad() override
    {
        setReactState(REACT_PASSIVE);
        setRooted(true);
        _setMeleeDisabled(true);
    }

    void AIUpdate(unsigned long time_passed) override
    {
        if (mFuseLeft == 0 || !isAlive())
            return;

        uint32_t const diff = static_cast<uint32_t>(time_passed);
        if (mFuseLeft > diff)
        {
            mFuseLeft -= diff;
            return;
        }

        mFuseLeft = 0;
        castSpellAOE(SPELL_FOOTBOMB_TIMED_DETONATION, true);
        despawn(1000, 0);
    }

private:
    uint32_t mFuseLeft = FootbombFuseMs;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Azerokk, fights alongside his Earthragers

class TheMotherlodeAzerokkAI : public TheMotherlodeBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new TheMotherlodeAzerokkAI(c); }
    explicit TheMotherlodeAzerokkAI(Creature* pCreature) : TheMotherlodeBossAI(pCreature, DATA_AZEROKK) {}

protected:
    void onEngage() override
    {
        rallyEarthragers();

        scriptEvents.addEvent(EVENT_AZEROKK_RESONANT_PULSE, 10600);
        scriptEvents.addEvent(EVENT_AZEROKK_INFUSION, 19600);
        scriptEvents.addEvent(EVENT_AZEROKK_CALL_EARTHRAGER, 60000);

        if (isHeroic())
            scriptEvents.addEvent(EVENT_AZEROKK_TECTONIC_SMASH, 5000);
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_AZEROKK_RESONANT_PULSE:
                warn("Azerokk prepares to cast Resonant Pulse!");
                castSpellOnSelf(SPELL_AZEROKK_RESONANT_PULSE);
                scriptEvents.addEvent(EVENT_AZEROKK_QUAKE_ECHO, 500);
                scriptEvents.addEvent(EVENT_AZEROKK_RESONANT_PULSE, 33200);
                break;
            case EVENT_AZEROKK_QUAKE_ECHO:
                // every rager answers the pulse with a quake
                for (Creature* pRager : livingEarthragers())
                    pRager->castSpell(pRager, SPELL_EARTHRAGER_RESONANT_QUAKE, false);
                break;
            case EVENT_AZEROKK_INFUSION:
                infuseEarthrager();
                scriptEvents.addEvent(EVENT_AZEROKK_INFUSION, 13000);
                break;
            case EVENT_AZEROKK_TECTONIC_SMASH:
                castSpellOnVictim(SPELL_AZEROKK_TECTONIC_SMASH);
                scriptEvents.addEvent(EVENT_AZEROKK_TECTONIC_SMASH, 23000);
                break;
            case EVENT_AZEROKK_CALL_EARTHRAGER:
                castSpellOnSelf(SPELL_AZEROKK_CALL_EARTHRAGER);
                callAdd(NPC_EARTHRAGER, EarthragerSpots[Util::getRandomUInt(EarthragerSpotCount - 1)]);
                scriptEvents.addEvent(EVENT_AZEROKK_CALL_EARTHRAGER, 60400);
                break;
            default:
                break;
        }
    }

private:
    std::vector<Creature*> livingEarthragers()
    {
        std::list<Creature*> found;
        GetCreatureListWithEntryInGrid(found, NPC_EARTHRAGER, EarthragerSearchRange);

        std::vector<Creature*> living;
        for (Creature* pRager : found)
        {
            if (pRager->isAlive())
                living.push_back(pRager);
        }

        return living;
    }

    // placed ragers join, otherwise the four of the quarry are raised
    void rallyEarthragers()
    {
        std::vector<Creature*> ragers = livingEarthragers();
        if (ragers.empty())
        {
            for (LocationVector const& spot : EarthragerSpots)
                callAdd(NPC_EARTHRAGER, spot);
            return;
        }

        for (Creature* pRager : ragers)
        {
            if (!pRager->isInCombat())
                setZoneWideCombat(pRager);
        }
    }

    void infuseEarthrager()
    {
        std::vector<Creature*> unbound;
        for (Creature* pRager : livingEarthragers())
        {
            if (!pRager->hasAurasWithId(SPELL_FRACKING_TOTEM_STUN))
                unbound.push_back(pRager);
        }

        if (unbound.empty())
            return;

        castSpell(unbound[Util::getRandomUInt(static_cast<uint32_t>(unbound.size() - 1))], SPELL_AZEROKK_AZERITE_INFUSION);
    }
};

// fixates a random player
class TheMotherlodeEarthragerAI : public TheMotherlodeCombatAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new TheMotherlodeEarthragerAI(c); }
    explicit TheMotherlodeEarthragerAI(Creature* pCreature) : TheMotherlodeCombatAI(pCreature) {}

protected:
    void onEngage() override
    {
        castSpellOnSelf(SPELL_EARTHRAGER_JAGGED_CUT, true);
        scriptEvents.addEvent(EVENT_EARTHRAGER_FIXATE, 2000);
    }

    void onEvent(uint32_t eventId) override
    {
        if (eventId != EVENT_EARTHRAGER_FIXATE)
            return;

        if (Unit* pTarget = anyPlayer())
            addThreat(pTarget, 1000000.0f);
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Rixxa Fluxflame

class TheMotherlodeRixxaAI : public TheMotherlodeBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new TheMotherlodeRixxaAI(c); }
    explicit TheMotherlodeRixxaAI(Creature* pCreature) : TheMotherlodeBossAI(pCreature, DATA_RIXXA_FLUXFLAME) {}

protected:
    void onEngage() override
    {
        scriptEvents.addEvent(EVENT_RIXXA_SEARING_REAGENT, 1500);
        scriptEvents.addEvent(EVENT_RIXXA_PROPELLANT_BLAST, 6000);
        scriptEvents.addEvent(EVENT_RIXXA_CHEMICAL_BURN, 12000);
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_RIXXA_SEARING_REAGENT:
                castSpellOnVictim(SPELL_RIXXA_SEARING_REAGENT);
                scriptEvents.addEvent(EVENT_RIXXA_SEARING_REAGENT, 15800);
                break;
            case EVENT_RIXXA_PROPELLANT_BLAST:
                castSpellOnVictim(SPELL_RIXXA_PROPELLANT_BLAST);
                scriptEvents.addEvent(EVENT_RIXXA_PROPELLANT_BLAST, 10000);
                break;
            case EVENT_RIXXA_CHEMICAL_BURN:
                warn("Rixxa Fluxflame splashes a Chemical Burn!");
                castAt(anyPlayer(), SPELL_RIXXA_CHEMICAL_BURN);
                scriptEvents.addEvent(EVENT_RIXXA_CHEMICAL_BURN, 34500);
                break;
            default:
                break;
        }
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Mogul Razdunk, guns first and the drill below half health

class TheMotherlodeRazdunkAI : public TheMotherlodeBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new TheMotherlodeRazdunkAI(c); }
    explicit TheMotherlodeRazdunkAI(Creature* pCreature) : TheMotherlodeBossAI(pCreature, DATA_MOGUL_RAZDUNK) {}

protected:
    void onEngage() override
    {
        mDrilling = false;
        scriptEvents.addEvent(EVENT_RAZDUNK_GATLING_GUN, 5000);
        scriptEvents.addEvent(EVENT_RAZDUNK_ALPHA_CANNON, 11000);

        if (isHeroic())
            scriptEvents.addEvent(EVENT_RAZDUNK_MICRO_MISSILES, 15800);
    }

    void onReset() override
    {
        mDrilling = false;
        _removeAura(SPELL_RAZDUNK_CONFIGURATION_DRILL);
    }

    void onCombatTick(uint32_t /*diff*/) override
    {
        if (mDrilling || _getHealthPercent() > 50)
            return;

        // second stage: the gun is stowed for the drill
        mDrilling = true;
        scriptEvents.removeEvent(EVENT_RAZDUNK_GATLING_GUN);
        warn("Mogul Razdunk switches to Configuration: Drill!");
        castSpellOnSelf(SPELL_RAZDUNK_CONFIGURATION_DRILL, true);
        scriptEvents.addEvent(EVENT_RAZDUNK_DRILL_SMASH, 5000);
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_RAZDUNK_GATLING_GUN:
                castAt(anyPlayer(), SPELL_RAZDUNK_HOMING_MISSILE, true);
                castSpellOnSelf(SPELL_RAZDUNK_GATLING_GUN);
                scriptEvents.addEvent(EVENT_RAZDUNK_GATLING_GUN, 40400);
                break;
            case EVENT_RAZDUNK_ALPHA_CANNON:
                castSpellOnVictim(SPELL_RAZDUNK_ALPHA_CANNON);
                scriptEvents.addEvent(EVENT_RAZDUNK_ALPHA_CANNON, 15000);
                break;
            case EVENT_RAZDUNK_DRILL_SMASH:
                if (Unit* pTarget = anyPlayer())
                {
                    warn("Mogul Razdunk leaps in for a Drill Smash!");
                    moveJump(pTarget->GetPosition(), 25.0f, 12.0f);
                    scriptEvents.addEvent(EVENT_RAZDUNK_DRILL_IMPACT, 1500);
                }
                scriptEvents.addEvent(EVENT_RAZDUNK_DRILL_SMASH, 25000);
                break;
            case EVENT_RAZDUNK_DRILL_IMPACT:
                castSpellAOE(SPELL_RAZDUNK_DRILL_SMASH);
                break;
            case EVENT_RAZDUNK_MICRO_MISSILES:
                castAt(anyPlayer(), SPELL_RAZDUNK_MICRO_MISSILES);
                scriptEvents.addEvent(EVENT_RAZDUNK_MICRO_MISSILES, 35000);
                break;
            default:
                break;
        }
    }

private:
    bool mDrilling = false;
};

void SetupTheMotherlode(ScriptMgr* mgr)
{
    mgr->register_instance_script(MAP_THE_MOTHERLODE, &TheMotherlodeInstanceScript::Create);

    mgr->register_creature_script(BOSS_CROWD_PUMMELER, &TheMotherlodePummelerAI::Create);
    mgr->register_creature_script(NPC_AZERITE_FOOTBOMB, &TheMotherlodeFootbombAI::Create);

    mgr->register_creature_script(BOSS_AZEROKK, &TheMotherlodeAzerokkAI::Create);
    mgr->register_creature_script(NPC_EARTHRAGER, &TheMotherlodeEarthragerAI::Create);

    mgr->register_creature_script(BOSS_RIXXA_FLUXFLAME, &TheMotherlodeRixxaAI::Create);

    mgr->register_creature_script(BOSS_MOGUL_RAZDUNK, &TheMotherlodeRazdunkAI::Create);
}
