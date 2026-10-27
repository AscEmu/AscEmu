/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "Instance_Halls_Of_Infusion.hpp"

#include "Objects/Units/Creatures/Creature.h"
#include "Server/Script/CreatureAIScript.hpp"
#include "Server/Script/InstanceScript.hpp"
#include "Server/Script/ScriptMgr.hpp"
#include "Utilities/Random.hpp"

#include <cmath>
#include <list>
#include <set>
#include <vector>

enum HallsOfInfusionEvents
{
    EVENT_IRIDEUS_TITANIC_FIST = 1,
    EVENT_IRIDEUS_STATIC_SURGE,
    EVENT_IRIDEUS_SPARK_VOLLEY,
    EVENT_IRIDEUS_POWER_OVERLOAD,

    EVENT_DEVICE_PURIFYING_BLAST,

    EVENT_GOLIATH_TOXIC_EFFLUVIA,
    EVENT_GOLIATH_GULP,
    EVENT_GOLIATH_GULP_MISSED,
    EVENT_GOLIATH_OVERPOWERING_CROAK,
    EVENT_GOLIATH_BELLY_SLAM,

    EVENT_SWOGLET_GULP_SWOG_TOXIN,

    EVENT_KHAJIN_FROST_SHOCK,
    EVENT_KHAJIN_POLAR_WINDS,
    EVENT_KHAJIN_ICE_TURN,

    EVENT_TSUNAMI_INFUSED_GLOBULE,
    EVENT_TSUNAMI_SQUALL_BUFFET,
    EVENT_TSUNAMI_FOCUSED_DELUGE,
    EVENT_TSUNAMI_TEMPESTS_FURY,
    EVENT_TSUNAMI_CRASHING_TSUNAMI,
    EVENT_TSUNAMI_WATERS_CALM,

    EVENT_INFUSER_INFUSE,
    EVENT_INFUSER_INUNDATE
};

namespace
{
    float const HallsOfInfusionTwoPi = 6.28318531f;

    int32_t const IrideusLastStandPct = 50;
    uint32_t const IrideusDeviceCount = 3;

    float const GoliathGulpReach = 10.0f;
    uint32_t const GoliathSwogletsPerCaveIn = 3;

    uint32_t const KhajinBouldersPerSurge = 3;
    uint32_t const KhajinIceTurnMs = 22000;

    int32_t const TsunamiInfusedWatersPct = 50;
    uint32_t const TsunamiInfuserCount = 3;
    uint32_t const TsunamiEmptyWatersMs = 20000;
    float const TsunamiMeleeReach = 8.0f;

    std::vector<Unit*> playersAround(Creature* pCreature, float range)
    {
        std::vector<Unit*> players;
        for (Object* pObject : pCreature->getInRangePlayersSet())
        {
            Unit* pPlayer = static_cast<Unit*>(pObject);
            if (pPlayer != nullptr && pPlayer->isAlive() && pCreature->getDistance(pPlayer) <= range)
                players.push_back(pPlayer);
        }

        return players;
    }

    LocationVector ringPoint(LocationVector const& centre, float radius, float angle)
    {
        return LocationVector(centre.x + radius * std::cos(angle), centre.y + radius * std::sin(angle), centre.z, angle);
    }

    LocationVector pointNear(LocationVector const& origin, float minRange, float maxRange)
    {
        return ringPoint(origin, Util::getRandomFloat(minRange, maxRange), Util::getRandomFloat(HallsOfInfusionTwoPi));
    }
}

//////////////////////////////////////////////////////////////////////////////////////////
// Instance

class HallsOfInfusionInstanceScript : public InstanceScript
{
public:
    explicit HallsOfInfusionInstanceScript(WorldMap* pMapMgr) : InstanceScript(pMapMgr)
    {
        setBossNumber(HallsOfInfusionEncounterCount);
    }

    static InstanceScript* Create(WorldMap* pMapMgr) { return new HallsOfInfusionInstanceScript(pMapMgr); }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Shared combat base: event clock while in combat

class HallsOfInfusionCombatAI : public CreatureAIScript
{
public:
    explicit HallsOfInfusionCombatAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

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

class HallsOfInfusionBossAI : public HallsOfInfusionCombatAI
{
public:
    HallsOfInfusionBossAI(Creature* pCreature, uint32_t dataId) : HallsOfInfusionCombatAI(pCreature), mDataId(dataId) {}

    void OnCombatStart(Unit* pTarget) override
    {
        if (InstanceScript* pInstance = getInstanceScript())
        {
            pInstance->sendUnitEncounter(EncounterFrameEngage, getCreature());
            pInstance->setBossState(mDataId, EncounterStates::InProgress);
        }

        HallsOfInfusionCombatAI::OnCombatStart(pTarget);
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

        HallsOfInfusionCombatAI::OnCombatStop(pTarget);
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
// Watcher Irideus

class HallsOfInfusionIrideusAI : public HallsOfInfusionBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new HallsOfInfusionIrideusAI(c); }
    explicit HallsOfInfusionIrideusAI(Creature* pCreature) : HallsOfInfusionBossAI(pCreature, DATA_WATCHER_IRIDEUS) {}

    // each slain device strips one layer of the barrier
    void OnSummonDies(Creature* pSummon, Unit* /*pKiller*/) override
    {
        if (pSummon == nullptr || pSummon->getEntry() != NPC_NULLIFICATION_DEVICE || mBarrierLayers == 0)
            return;

        if (--mBarrierLayers == 0)
            dropBarrier();
    }

protected:
    void onEngage() override
    {
        mLastStand = false;
        mBarrierLayers = 0;
        scriptEvents.addEvent(EVENT_IRIDEUS_TITANIC_FIST, 6000);
        scriptEvents.addEvent(EVENT_IRIDEUS_STATIC_SURGE, 10000);
        scriptEvents.addEvent(EVENT_IRIDEUS_SPARK_VOLLEY, 15000);
        scriptEvents.addEvent(EVENT_IRIDEUS_POWER_OVERLOAD, 20000);
    }

    void onReset() override
    {
        mBarrierLayers = 0;
        _removeAura(SPELL_IRIDEUS_ABLATIVE_BARRIER);
        _removeAura(SPELL_IRIDEUS_SIPHON_POWER);
    }

    void onCombatTick(uint32_t /*diff*/) override
    {
        if (!mLastStand && _getHealthPercent() <= IrideusLastStandPct && !_isCasting())
            beginLastStand();
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_IRIDEUS_TITANIC_FIST:
                castSpellOnVictim(SPELL_IRIDEUS_TITANIC_FIST);
                scriptEvents.addEvent(EVENT_IRIDEUS_TITANIC_FIST, 14000);
                break;
            case EVENT_IRIDEUS_STATIC_SURGE:
                castSpellAOE(SPELL_IRIDEUS_STATIC_SURGE);
                scriptEvents.addEvent(EVENT_IRIDEUS_STATIC_SURGE, 18000);
                break;
            case EVENT_IRIDEUS_SPARK_VOLLEY:
                castSpellOnSelf(SPELL_IRIDEUS_SPARK_VOLLEY);
                scriptEvents.addEvent(EVENT_IRIDEUS_SPARK_VOLLEY, 25000);
                break;
            case EVENT_IRIDEUS_POWER_OVERLOAD:
                castSpellAOE(SPELL_IRIDEUS_POWER_OVERLOAD);
                scriptEvents.addEvent(EVENT_IRIDEUS_POWER_OVERLOAD, 28000);
                break;
            default:
                break;
        }
    }

private:
    void beginLastStand()
    {
        mLastStand = true;
        warn("Watcher Irideus siphons power from the facility! Destroy the Nullification Devices!");
        castSpellOnSelf(SPELL_IRIDEUS_SIPHON_POWER, true);

        mBarrierLayers = 0;
        for (uint32_t i = 0; i < IrideusDeviceCount; ++i)
        {
            if (callAdd(NPC_NULLIFICATION_DEVICE, ringPoint(getCreature()->GetPosition(), 15.0f, i * HallsOfInfusionTwoPi / IrideusDeviceCount)) != nullptr)
                ++mBarrierLayers;
        }

        // no device to break it, no barrier
        if (mBarrierLayers > 0)
            castSpellOnSelf(SPELL_IRIDEUS_ABLATIVE_BARRIER, true);
    }

    void dropBarrier()
    {
        _removeAura(SPELL_IRIDEUS_ABLATIVE_BARRIER);
        warn("Watcher Irideus' Ablative Barrier collapses!");
    }

    bool mLastStand = false;
    uint32_t mBarrierLayers = 0;
};

// pulses its nullification on the way down
class HallsOfInfusionNullificationDeviceAI : public HallsOfInfusionCombatAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new HallsOfInfusionNullificationDeviceAI(c); }
    explicit HallsOfInfusionNullificationDeviceAI(Creature* pCreature) : HallsOfInfusionCombatAI(pCreature) {}

    void DamageTaken(Unit* /*pAttacker*/, uint32_t* damage) override
    {
        if (damage == nullptr || mPulsed || *damage < getCreature()->getHealth())
            return;

        mPulsed = true;
        castSpellAOE(SPELL_DEVICE_NULLIFYING_PULSE, true);
    }

protected:
    void onEngage() override
    {
        setRooted(true);
        _setMeleeDisabled(true);
        scriptEvents.addEvent(EVENT_DEVICE_PURIFYING_BLAST, 3000);
    }

    void onEvent(uint32_t eventId) override
    {
        if (eventId != EVENT_DEVICE_PURIFYING_BLAST)
            return;

        castAt(anyPlayer(), SPELL_DEVICE_PURIFYING_BLAST);
        scriptEvents.addEvent(EVENT_DEVICE_PURIFYING_BLAST, 6000);
    }

private:
    bool mPulsed = false;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Gulping Goliath

class HallsOfInfusionGoliathAI : public HallsOfInfusionBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new HallsOfInfusionGoliathAI(c); }
    explicit HallsOfInfusionGoliathAI(Creature* pCreature) : HallsOfInfusionBossAI(pCreature, DATA_GULPING_GOLIATH) {}

protected:
    void onEngage() override
    {
        scriptEvents.addEvent(EVENT_GOLIATH_TOXIC_EFFLUVIA, 8000);
        scriptEvents.addEvent(EVENT_GOLIATH_GULP, 12000);
        scriptEvents.addEvent(EVENT_GOLIATH_OVERPOWERING_CROAK, 20000);
        scriptEvents.addEvent(EVENT_GOLIATH_BELLY_SLAM, 28000);
    }

    void onReset() override { _removeAura(SPELL_GOLIATH_HANGRY); }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_GOLIATH_TOXIC_EFFLUVIA:
                castSpellAOE(SPELL_GOLIATH_TOXIC_EFFLUVIA);
                scriptEvents.addEvent(EVENT_GOLIATH_TOXIC_EFFLUVIA, 18000);
                break;
            case EVENT_GOLIATH_GULP:
                warn("The Gulping Goliath prepares to Gulp!");
                castSpellOnVictim(SPELL_GOLIATH_GULP);
                scriptEvents.addEvent(EVENT_GOLIATH_GULP_MISSED, 500);
                scriptEvents.addEvent(EVENT_GOLIATH_GULP, 25000);
                break;
            case EVENT_GOLIATH_GULP_MISSED:
                // nobody close enough to swallow
                if (playersAround(getCreature(), GoliathGulpReach).empty())
                {
                    warn("The Gulping Goliath becomes Hangry!");
                    castSpellOnSelf(SPELL_GOLIATH_HANGRY, true);
                }
                break;
            case EVENT_GOLIATH_OVERPOWERING_CROAK:
                castSpellAOE(SPELL_GOLIATH_OVERPOWERING_CROAK);
                caveIn();
                scriptEvents.addEvent(EVENT_GOLIATH_OVERPOWERING_CROAK, 30000);
                break;
            case EVENT_GOLIATH_BELLY_SLAM:
                castSpellAOE(SPELL_GOLIATH_BELLY_SLAM);
                scriptEvents.addEvent(EVENT_GOLIATH_BELLY_SLAM, 30000);
                break;
            default:
                break;
        }
    }

private:
    // the croak shakes loose stones and swoglets drop with them
    void caveIn()
    {
        castAt(anyPlayer(), SPELL_GOLIATH_CAVE_IN, true);
        for (uint32_t i = 0; i < GoliathSwogletsPerCaveIn; ++i)
            callAdd(NPC_CURIOUS_SWOGLET, pointNear(getCreature()->GetPosition(), 10.0f, 20.0f));
    }
};

class HallsOfInfusionCuriousSwogletAI : public HallsOfInfusionCombatAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new HallsOfInfusionCuriousSwogletAI(c); }
    explicit HallsOfInfusionCuriousSwogletAI(Creature* pCreature) : HallsOfInfusionCombatAI(pCreature) {}

protected:
    void onEngage() override
    {
        // picks one player and sticks to it
        if (Unit* pPrey = anyPlayer())
        {
            castSpell(pPrey, SPELL_SWOGLET_FIXATE, true);
            attackStart(pPrey);
        }

        scriptEvents.addEvent(EVENT_SWOGLET_GULP_SWOG_TOXIN, 2000);
    }

    void onEvent(uint32_t eventId) override
    {
        if (eventId != EVENT_SWOGLET_GULP_SWOG_TOXIN)
            return;

        Unit* pVictim = getCreature()->getAIInterface()->getCurrentTarget();
        if (pVictim != nullptr && getCreature()->getDistance(pVictim) <= 5.0f)
            castSpell(pVictim, SPELL_SWOGLET_GULP_SWOG_TOXIN, true);

        scriptEvents.addEvent(EVENT_SWOGLET_GULP_SWOG_TOXIN, 2500);
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Khajin the Unyielding

class HallsOfInfusionKhajinAI : public HallsOfInfusionBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new HallsOfInfusionKhajinAI(c); }
    explicit HallsOfInfusionKhajinAI(Creature* pCreature) : HallsOfInfusionBossAI(pCreature, DATA_KHAJIN_THE_UNYIELDING) {}

protected:
    void onEngage() override
    {
        mSurgeNext = true;
        mStruckBoulders.clear();
        scriptEvents.addEvent(EVENT_KHAJIN_FROST_SHOCK, 6000);
        scriptEvents.addEvent(EVENT_KHAJIN_ICE_TURN, 10000);
        scriptEvents.addEvent(EVENT_KHAJIN_POLAR_WINDS, 20000);
    }

    void onReset() override { mStruckBoulders.clear(); }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_KHAJIN_FROST_SHOCK:
                castAt(anyPlayer(), SPELL_KHAJIN_FROST_SHOCK);
                scriptEvents.addEvent(EVENT_KHAJIN_FROST_SHOCK, 12000);
                break;
            case EVENT_KHAJIN_POLAR_WINDS:
                castAt(anyPlayer(), SPELL_KHAJIN_POLAR_WINDS);
                scriptEvents.addEvent(EVENT_KHAJIN_POLAR_WINDS, 30000);
                break;
            case EVENT_KHAJIN_ICE_TURN:
                // surge and storm take turns
                if (mSurgeNext)
                    glacialSurge();
                else
                    hailstorm();
                mSurgeNext = !mSurgeNext;
                scriptEvents.addEvent(EVENT_KHAJIN_ICE_TURN, KhajinIceTurnMs);
                break;
            default:
                break;
        }
    }

private:
    void glacialSurge()
    {
        castAt(anyPlayer(), SPELL_KHAJIN_GLACIAL_SURGE);
        for (uint32_t i = 0; i < KhajinBouldersPerSurge; ++i)
            summonCreature(NPC_ICE_BOULDER, pointNear(getCreature()->GetPosition(), 8.0f, 18.0f), CORPSE_DESPAWN);
    }

    // every boulder is struck, the second strike shatters it
    void hailstorm()
    {
        warn("Khajin the Unyielding calls down a Hailstorm! Take cover behind an Ice Boulder!");
        castSpellAOE(SPELL_KHAJIN_HAILSTORM);

        std::list<Creature*> boulders;
        GetCreatureListWithEntryInGrid(boulders, NPC_ICE_BOULDER, 80.0f);
        for (Creature* pBoulder : boulders)
        {
            if (pBoulder == nullptr || !pBoulder->isAlive())
                continue;

            if (mStruckBoulders.insert(pBoulder->getGuid()).second)
                continue;

            mStruckBoulders.erase(pBoulder->getGuid());
            pBoulder->castSpell(pBoulder, SPELL_BOULDER_AVALANCHE, true);
            pBoulder->despawn(1500, 0);
        }
    }

    bool mSurgeNext = true;
    std::set<uint64_t> mStruckBoulders;
};

// cover against the hailstorm, cannot be attacked
class HallsOfInfusionIceBoulderAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new HallsOfInfusionIceBoulderAI(c); }
    explicit HallsOfInfusionIceBoulderAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnLoad() override
    {
        setReactState(REACT_PASSIVE);
        setRooted(true);
        _setMeleeDisabled(true);
        setCanEnterCombat(false);
        addUnitFlags(UNIT_FLAG_NON_ATTACKABLE);
        castSpellOnSelf(SPELL_BOULDER_ICE_BOULDER, true);
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Primal Tsunami

class HallsOfInfusionTsunamiAI : public HallsOfInfusionBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new HallsOfInfusionTsunamiAI(c); }
    explicit HallsOfInfusionTsunamiAI(Creature* pCreature) : HallsOfInfusionBossAI(pCreature, DATA_PRIMAL_TSUNAMI) {}

    void OnSummonDies(Creature* pSummon, Unit* /*pKiller*/) override
    {
        if (pSummon == nullptr || pSummon->getEntry() != NPC_PRIMALIST_INFUSER || mInfusersLeft == 0)
            return;

        if (--mInfusersLeft == 0 && mInfusedWaters)
            calmWaters();
    }

protected:
    void onEngage() override
    {
        mWatersTurned = false;
        mInfusedWaters = false;
        mInfusersLeft = 0;
        mReachClock = 0;
        queueViolentSwells();
    }

    void onReset() override
    {
        mInfusedWaters = false;
        setRooted(false);
    }

    void onCombatTick(uint32_t diff) override
    {
        if (!mWatersTurned && _getHealthPercent() <= TsunamiInfusedWatersPct && !_isCasting())
        {
            beginInfusedWaters();
            return;
        }

        if (mInfusedWaters)
            return;

        // nobody in reach, the tide drags them back
        mReachClock += diff;
        if (mReachClock < 3000 || _isCasting())
            return;

        mReachClock = 0;
        if (playersAround(getCreature(), TsunamiMeleeReach).empty() && !hasAura(SPELL_TSUNAMI_UNDERTOW))
            castSpellOnSelf(SPELL_TSUNAMI_UNDERTOW);
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_TSUNAMI_INFUSED_GLOBULE:
                castSpellAOE(SPELL_TSUNAMI_INFUSED_GLOBULE);
                scriptEvents.addEvent(EVENT_TSUNAMI_INFUSED_GLOBULE, 18000);
                break;
            case EVENT_TSUNAMI_SQUALL_BUFFET:
                castSpellOnVictim(SPELL_TSUNAMI_SQUALL_BUFFET);
                scriptEvents.addEvent(EVENT_TSUNAMI_FOCUSED_DELUGE, 500);
                scriptEvents.addEvent(EVENT_TSUNAMI_SQUALL_BUFFET, 25000);
                break;
            case EVENT_TSUNAMI_FOCUSED_DELUGE:
                castAt(anyPlayer(), SPELL_TSUNAMI_FOCUSED_DELUGE);
                break;
            case EVENT_TSUNAMI_TEMPESTS_FURY:
                castSpellAOE(SPELL_TSUNAMI_TEMPESTS_FURY);
                scriptEvents.addEvent(EVENT_TSUNAMI_TEMPESTS_FURY, 30000);
                break;
            case EVENT_TSUNAMI_CRASHING_TSUNAMI:
                castAt(anyPlayer(), SPELL_TSUNAMI_CRASHING_TSUNAMI);
                scriptEvents.addEvent(EVENT_TSUNAMI_CRASHING_TSUNAMI, 10000);
                break;
            case EVENT_TSUNAMI_WATERS_CALM:
                if (mInfusedWaters)
                    calmWaters();
                break;
            default:
                break;
        }
    }

private:
    void queueViolentSwells()
    {
        scriptEvents.addEvent(EVENT_TSUNAMI_INFUSED_GLOBULE, 8000);
        scriptEvents.addEvent(EVENT_TSUNAMI_SQUALL_BUFFET, 14000);
        scriptEvents.addEvent(EVENT_TSUNAMI_TEMPESTS_FURY, 22000);
    }

    void beginInfusedWaters()
    {
        mWatersTurned = true;
        mInfusedWaters = true;
        scriptEvents.resetEvents();

        warn("The Primal Tsunami Casts Away its foes! Interrupt the Primalist Infusers!");
        setRooted(true);
        castSpellAOE(SPELL_TSUNAMI_CAST_AWAY);

        mInfusersLeft = 0;
        for (uint32_t i = 0; i < TsunamiInfuserCount; ++i)
        {
            if (callAdd(NPC_PRIMALIST_INFUSER, ringPoint(getCreature()->GetPosition(), 18.0f, i * HallsOfInfusionTwoPi / TsunamiInfuserCount)) != nullptr)
                ++mInfusersLeft;
        }

        scriptEvents.addEvent(EVENT_TSUNAMI_CRASHING_TSUNAMI, 6000);
        scriptEvents.addEvent(EVENT_TSUNAMI_INFUSED_GLOBULE, 9000);

        // without infusers the waters settle on their own
        if (mInfusersLeft == 0)
            scriptEvents.addEvent(EVENT_TSUNAMI_WATERS_CALM, TsunamiEmptyWatersMs);
    }

    void calmWaters()
    {
        mInfusedWaters = false;
        scriptEvents.resetEvents();
        setRooted(false);
        warn("The infused waters calm down.");
        queueViolentSwells();
    }

    bool mWatersTurned = false;
    bool mInfusedWaters = false;
    uint32_t mInfusersLeft = 0;
    uint32_t mReachClock = 0;
};

class HallsOfInfusionPrimalistInfuserAI : public HallsOfInfusionCombatAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new HallsOfInfusionPrimalistInfuserAI(c); }
    explicit HallsOfInfusionPrimalistInfuserAI(Creature* pCreature) : HallsOfInfusionCombatAI(pCreature) {}

protected:
    void onEngage() override
    {
        scriptEvents.addEvent(EVENT_INFUSER_INFUSE, 3000);
        scriptEvents.addEvent(EVENT_INFUSER_INUNDATE, 7000);
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_INFUSER_INFUSE:
            {
                Creature* pTsunami = findNearestCreature(BOSS_PRIMAL_TSUNAMI, 80.0f);
                if (pTsunami != nullptr && pTsunami->isAlive())
                    castSpell(pTsunami, SPELL_INFUSER_INFUSE);
                scriptEvents.addEvent(EVENT_INFUSER_INFUSE, 8000);
                break;
            }
            case EVENT_INFUSER_INUNDATE:
                castSpellAOE(SPELL_INFUSER_INUNDATE);
                scriptEvents.addEvent(EVENT_INFUSER_INUNDATE, 12000);
                break;
            default:
                break;
        }
    }
};

void SetupHallsOfInfusion(ScriptMgr* mgr)
{
    mgr->register_instance_script(MAP_HALLS_OF_INFUSION, &HallsOfInfusionInstanceScript::Create);

    mgr->register_creature_script(BOSS_WATCHER_IRIDEUS, &HallsOfInfusionIrideusAI::Create);
    mgr->register_creature_script(NPC_NULLIFICATION_DEVICE, &HallsOfInfusionNullificationDeviceAI::Create);

    mgr->register_creature_script(BOSS_GULPING_GOLIATH, &HallsOfInfusionGoliathAI::Create);
    mgr->register_creature_script(NPC_CURIOUS_SWOGLET, &HallsOfInfusionCuriousSwogletAI::Create);
    mgr->register_creature_script(NPC_CURIOUS_SWOGLET_ALT, &HallsOfInfusionCuriousSwogletAI::Create);

    mgr->register_creature_script(BOSS_KHAJIN_THE_UNYIELDING, &HallsOfInfusionKhajinAI::Create);
    mgr->register_creature_script(NPC_ICE_BOULDER, &HallsOfInfusionIceBoulderAI::Create);

    mgr->register_creature_script(BOSS_PRIMAL_TSUNAMI, &HallsOfInfusionTsunamiAI::Create);
    mgr->register_creature_script(NPC_PRIMALIST_INFUSER, &HallsOfInfusionPrimalistInfuserAI::Create);
}
