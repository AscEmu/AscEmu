/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "Instance_Den_Of_Nalorakk.hpp"

#include "Objects/GameObject.h"
#include "Objects/Units/Creatures/Creature.h"
#include "Server/Script/CreatureAIScript.hpp"
#include "Server/Script/InstanceScript.hpp"
#include "Server/Script/ScriptMgr.hpp"
#include "Utilities/Random.hpp"

#include <cmath>
#include <vector>

enum DenOfNalorakkEvents
{
    EVENT_HOARD_SLAM = 1,
    EVENT_HOARD_BELLOW,
    EVENT_HOARD_GATHER,

    EVENT_SENTINEL_GLACIAL_TORMENT,
    EVENT_SENTINEL_RAGING_SQUALL,
    EVENT_SENTINEL_SHATTERING_FROSTSPIKE,
    EVENT_SENTINEL_FROZEN_TEMPEST,

    EVENT_SHIVERCORE_WINTERS_SHROUD,

    EVENT_LOA_ECHOING_MAUL,
    EVENT_LOA_FORCEFUL_ROAR
};

namespace
{
    float const DenTwoPi = 6.2831853f;

    // the three heaps of the hoard, one is raided at each health mark
    enum HoardHeap : uint8_t
    {
        HOARD_HEAP_BONES = 0,
        HOARD_HEAP_MEAT,
        HOARD_HEAP_MUSHROOMS,
        HOARD_HEAP_COUNT
    };

    uint32_t const HoardHeapEntries[HOARD_HEAP_COUNT] = { NPC_HOARD_BONE_PILE, NPC_HOARD_MEAT_PILE, NPC_HOARD_MUSHROOM_PILE };
    int32_t const HoardMeasureMarks[3] = { 90, 70, 40 };

    uint32_t const MushroomGrowMs = 12000;
    float const MushroomTouchReach = 2.0f;

    uint32_t const SquallLifetimeMs = 30000;
    uint32_t const EchoLifetimeMs = 25000;
    float const EchoCollideReach = 3.0f;
    uint8_t const EchoCount = 3;

    // room doors shut during the fight, passages open after the kill
    struct DenDoor
    {
        uint32_t entry;
        uint32_t dataId;
        bool passage;
    };

    DenDoor const DenDoors[] =
    {
        { GO_HOARD_WATER_DOOR,  DATA_HOARDMONGER,   true },
        { GO_LOA_VINE_DOOR,     DATA_LOA_NALORAKK,  false }
    };

    DenDoor const* findDenDoor(uint32_t entry)
    {
        for (DenDoor const& door : DenDoors)
        {
            if (door.entry == entry)
                return &door;
        }

        return nullptr;
    }

    LocationVector denRingPoint(LocationVector const& origin, float minRange, float maxRange)
    {
        float const angle = Util::getRandomFloat(DenTwoPi);
        float const range = Util::getRandomFloat(minRange, maxRange);
        return LocationVector(origin.x + range * std::cos(angle), origin.y + range * std::sin(angle), origin.z, angle);
    }

    // first living player within reach of a creature
    Unit* playerTouching(Creature* pCreature, float reach)
    {
        for (Object* pObject : pCreature->getInRangePlayersSet())
        {
            Unit* pPlayer = static_cast<Unit*>(pObject);
            if (pPlayer != nullptr && pPlayer->isAlive() && pCreature->getDistance(pPlayer) <= reach)
                return pPlayer;
        }

        return nullptr;
    }

    // true once when the countdown runs out
    bool denCountdown(uint32_t& leftMs, uint32_t diff, uint32_t restartMs)
    {
        if (leftMs > diff)
        {
            leftMs -= diff;
            return false;
        }

        leftMs = restartMs;
        return true;
    }
}

//////////////////////////////////////////////////////////////////////////////////////////
// Instance

class DenOfNalorakkInstanceScript : public InstanceScript
{
public:
    explicit DenOfNalorakkInstanceScript(WorldMap* pMapMgr) : InstanceScript(pMapMgr)
    {
        setBossNumber(DenOfNalorakkEncounterCount);
    }

    static InstanceScript* Create(WorldMap* pMapMgr) { return new DenOfNalorakkInstanceScript(pMapMgr); }

    void OnGameObjectPushToWorld(GameObject* pGameObject) override
    {
        if (findDenDoor(pGameObject->getEntry()) == nullptr)
            return;

        mDoorGuids.push_back(pGameObject->GetNewGUID());
        applyDoor(pGameObject);
    }

    void OnEncounterStateChange(uint32_t /*id*/, uint32_t /*state*/) override
    {
        for (WoWGuid const& guid : mDoorGuids)
        {
            if (GameObject* pDoor = getGameObjectByGuid(guid))
                applyDoor(pDoor);
        }
    }

private:
    void applyDoor(GameObject* pDoor)
    {
        DenDoor const* pRule = findDenDoor(pDoor->getEntry());
        if (pRule == nullptr)
            return;

        EncounterStates const state = getBossState(pRule->dataId);
        bool const open = pRule->passage ? state == EncounterStates::Performed : state != EncounterStates::InProgress;
        uint8_t const wanted = open ? GO_STATE_OPEN : GO_STATE_CLOSED;
        if (pDoor->getState() != wanted)
            pDoor->setState(wanted);
    }

    std::vector<WoWGuid> mDoorGuids;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Shared combat base: event clock, energy bar and target helpers

class DenOfNalorakkCombatAI : public CreatureAIScript
{
public:
    explicit DenOfNalorakkCombatAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        mEnergyFillMs = 0;
        onEngage();
    }

    void OnCombatStop(Unit* /*pTarget*/) override
    {
        scriptEvents.resetEvents();
        summons.despawnAll();
        mEnergyFillMs = 0;

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
        tickEnergy(diff);
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
    virtual void onEnergyFull() {}

    Unit* anyPlayer() { return selectUnitTarget(FilterArgs(TargetFilter_Player)); }

    // anyone but the tank, the tank when alone
    Unit* anyPlayerButTank()
    {
        FilterArgs args(TargetFilter_Player);
        args.addFilter(TargetFilter_NotCurrent);
        if (Unit* pTarget = selectUnitTarget(args))
            return pTarget;

        return anyPlayer();
    }

    Creature* callAdd(uint32_t entry, LocationVector const& position)
    {
        Creature* pAdd = summonCreature(entry, position, CORPSE_DESPAWN);
        if (pAdd != nullptr)
            setZoneWideCombat(pAdd);

        return pAdd;
    }

    LocationVector aroundSelf(float minRange, float maxRange) { return denRingPoint(getCreature()->GetPosition(), minRange, maxRange); }

    void warn(char const* text) { sendChatMessage(CHAT_MSG_RAID_BOSS_EMOTE, 0, text); }

    // the bar fills from empty in fillMs
    void armEnergy(uint32_t fillMs)
    {
        mEnergyFillMs = fillMs;
        mEnergyMs = 0;
        mEnergyFull = false;
    }

private:
    void tickEnergy(uint32_t diff)
    {
        if (mEnergyFillMs == 0)
            return;

        if (!mEnergyFull)
        {
            mEnergyMs += diff;
            mEnergyFull = mEnergyMs >= mEnergyFillMs;
        }

        // a full bar waits for the running cast
        if (!mEnergyFull || _isCasting())
            return;

        mEnergyFull = false;
        mEnergyMs = 0;
        onEnergyFull();
    }

    uint32_t mEnergyFillMs = 0;
    uint32_t mEnergyMs = 0;
    bool mEnergyFull = false;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Encounter base: frame and boss state

class DenOfNalorakkBossAI : public DenOfNalorakkCombatAI
{
public:
    DenOfNalorakkBossAI(Creature* pCreature, uint32_t dataId) : DenOfNalorakkCombatAI(pCreature), mDataId(dataId) {}

    void OnCombatStart(Unit* pTarget) override
    {
        if (InstanceScript* pInstance = getInstanceScript())
        {
            pInstance->sendUnitEncounter(EncounterFrameEngage, getCreature());
            pInstance->setBossState(mDataId, EncounterStates::InProgress);
        }

        DenOfNalorakkCombatAI::OnCombatStart(pTarget);
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

        DenOfNalorakkCombatAI::OnCombatStop(pTarget);
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
// The Hoardmonger, each raided heap empowers his next use of one ability

class DenOfNalorakkHoardmongerAI : public DenOfNalorakkBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new DenOfNalorakkHoardmongerAI(c); }
    explicit DenOfNalorakkHoardmongerAI(Creature* pCreature) : DenOfNalorakkBossAI(pCreature, DATA_HOARDMONGER) {}

protected:
    void onEngage() override
    {
        mMeasures = 0;
        mGathering = false;
        mGatherHeap = HOARD_HEAP_COUNT;
        for (uint8_t i = 0; i < HOARD_HEAP_COUNT; ++i)
        {
            mHeapRaided[i] = false;
            mEmpowered[i] = false;
        }

        armEnergy(35000);
        scriptEvents.addEvent(EVENT_HOARD_SLAM, 8000);
        scriptEvents.addEvent(EVENT_HOARD_BELLOW, 16000);
    }

    void onCombatTick(uint32_t /*diff*/) override
    {
        if (mGathering || mMeasures >= 3 || _isCasting() || _getHealthPercent() > HoardMeasureMarks[mMeasures])
            return;

        ++mMeasures;
        raidHoard();
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_HOARD_SLAM:
                if (spendEmpowerment(HOARD_HEAP_BONES))
                {
                    warn("The Hoardmonger slams down with a bone-spiked fist!");
                    castSpellOnVictim(SPELL_HOARD_BONESPIKE_SLAM);
                }
                else
                {
                    castSpellOnVictim(SPELL_HOARD_EARTHSHATTER_SLAM);
                }
                scriptEvents.addEvent(EVENT_HOARD_SLAM, 16000);
                break;
            case EVENT_HOARD_BELLOW:
                castSpellAOE(spendEmpowerment(HOARD_HEAP_MEAT) ? SPELL_HOARD_HEARTY_BELLOW : SPELL_HOARD_RAVENOUS_BELLOW);
                scriptEvents.addEvent(EVENT_HOARD_BELLOW, 24000);
                break;
            case EVENT_HOARD_GATHER:
                castSpellOnSelf(SPELL_HOARD_RESOURCEFUL_MEASURES);
                if (mGatherHeap < HOARD_HEAP_COUNT)
                    mEmpowered[mGatherHeap] = true;
                mGathering = false;
                break;
            default:
                break;
        }
    }

    // spoiled goods rain down and sprout Rotten Mushrooms
    void onEnergyFull() override
    {
        if (spendEmpowerment(HOARD_HEAP_MUSHROOMS))
        {
            warn("The Hoardmonger flings his Overflowing Supplies across the den!");
            castSpellOnSelf(SPELL_HOARD_OVERFLOWING_SUPPLIES);
            scatterMushrooms(6);
            return;
        }

        warn("The Hoardmonger tosses Spoiled Supplies around!");
        castSpellOnSelf(SPELL_HOARD_SPOILED_SUPPLIES);
        scatterMushrooms(4);
    }

private:
    // runs to the closest heap not raided yet
    void raidHoard()
    {
        uint8_t chosen = HOARD_HEAP_COUNT;
        Creature* pChosenHeap = nullptr;
        float chosenRange = 0.0f;

        for (uint8_t heap = 0; heap < HOARD_HEAP_COUNT; ++heap)
        {
            if (mHeapRaided[heap])
                continue;

            Creature* pHeap = findNearestCreature(HoardHeapEntries[heap], 120.0f);
            float const range = pHeap != nullptr ? getCreature()->getDistance(pHeap) : 1000.0f;
            if (chosen == HOARD_HEAP_COUNT || range < chosenRange)
            {
                chosen = heap;
                pChosenHeap = pHeap;
                chosenRange = range;
            }
        }

        if (chosen == HOARD_HEAP_COUNT)
            return;

        mHeapRaided[chosen] = true;
        mGatherHeap = chosen;
        mGathering = true;

        warn("The Hoardmonger dives into his hoard with Resourceful Measures!");
        if (pChosenHeap != nullptr)
            moveTo(pChosenHeap->GetPositionX(), pChosenHeap->GetPositionY(), pChosenHeap->GetPositionZ(), true);

        scriptEvents.addEvent(EVENT_HOARD_GATHER, 3000);
    }

    // outside mythic an empowerment is good for one use
    bool spendEmpowerment(uint8_t heap)
    {
        if (!mEmpowered[heap])
            return false;

        mEmpowered[heap] = false;
        return true;
    }

    void scatterMushrooms(uint8_t count)
    {
        for (uint8_t i = 0; i < count; ++i)
            summonCreature(NPC_HOARD_ROTTEN_MUSHROOM, aroundSelf(8.0f, 25.0f), CORPSE_DESPAWN);
    }

    uint8_t mMeasures = 0;
    uint8_t mGatherHeap = HOARD_HEAP_COUNT;
    bool mGathering = false;
    bool mHeapRaided[HOARD_HEAP_COUNT] = {};
    bool mEmpowered[HOARD_HEAP_COUNT] = {};
};

// grows for a while, then bursts unless a player stomps it first
class DenOfNalorakkRottenMushroomAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new DenOfNalorakkRottenMushroomAI(c); }
    explicit DenOfNalorakkRottenMushroomAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnLoad() override
    {
        setReactState(REACT_PASSIVE);
        setRooted(true);
        _setMeleeDisabled(true);
    }

    void AIUpdate(unsigned long time_passed) override
    {
        if (mSpent || !isAlive())
            return;

        uint32_t const diff = static_cast<uint32_t>(time_passed);
        mGrownMs += diff;

        if (!denCountdown(mCheckMs, diff, 500))
            return;

        // the spores cling to whoever touches it
        if (Unit* pPlayer = playerTouching(getCreature(), MushroomTouchReach))
        {
            castSpell(pPlayer, SPELL_MUSHROOM_TOXIC_SPORES, true);
            finish(200);
            return;
        }

        if (mGrownMs < MushroomGrowMs)
            return;

        getCreature()->castSpellLoc(getCreature()->GetPosition(), SPELL_MUSHROOM_PUTRID_BURST, true);
        _castOnInrangePlayers(SPELL_MUSHROOM_TOXIC_SPORES, true);
        finish(1000);
    }

private:
    void finish(uint32_t delayMs)
    {
        mSpent = true;
        despawn(delayMs, 0);
    }

    uint32_t mGrownMs = 0;
    uint32_t mCheckMs = 500;
    bool mSpent = false;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Sentinel of Winter

class DenOfNalorakkSentinelAI : public DenOfNalorakkBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new DenOfNalorakkSentinelAI(c); }
    explicit DenOfNalorakkSentinelAI(Creature* pCreature) : DenOfNalorakkBossAI(pCreature, DATA_SENTINEL_OF_WINTER) {}

    // a fallen shivercore leaves a snowdrift behind
    void OnSummonDies(Creature* pSummon, Unit* /*pKiller*/) override
    {
        if (pSummon != nullptr && pSummon->getEntry() == NPC_FRACTURED_SHIVERCORE && isAlive())
            getCreature()->castSpellLoc(pSummon->GetPosition(), SPELL_SHIVERCORE_SNOWDRIFT, true);
    }

protected:
    void onEngage() override
    {
        scriptEvents.addEvent(EVENT_SENTINEL_GLACIAL_TORMENT, 8000);
        scriptEvents.addEvent(EVENT_SENTINEL_RAGING_SQUALL, 10000);
        scriptEvents.addEvent(EVENT_SENTINEL_SHATTERING_FROSTSPIKE, 15000);
        scriptEvents.addEvent(EVENT_SENTINEL_FROZEN_TEMPEST, 30000);
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_SENTINEL_GLACIAL_TORMENT:
                castSpellAOE(SPELL_SENTINEL_GLACIAL_TORMENT);
                scriptEvents.addEvent(EVENT_SENTINEL_GLACIAL_TORMENT, 20000);
                break;
            case EVENT_SENTINEL_RAGING_SQUALL:
                castSpellOnSelf(SPELL_SENTINEL_RAGING_SQUALL, true);
                summonCreature(NPC_RAGING_SQUALL, aroundSelf(10.0f, 20.0f), TIMED_DESPAWN, SquallLifetimeMs);
                scriptEvents.addEvent(EVENT_SENTINEL_RAGING_SQUALL, 26000);
                break;
            case EVENT_SENTINEL_SHATTERING_FROSTSPIKE:
                castSpellOnSelf(SPELL_SENTINEL_SHATTERING_FROSTSPIKE);
                for (uint8_t i = 0; i < 2; ++i)
                    callAdd(NPC_FRACTURED_SHIVERCORE, aroundSelf(8.0f, 16.0f));
                scriptEvents.addEvent(EVENT_SENTINEL_SHATTERING_FROSTSPIKE, 32000);
                break;
            case EVENT_SENTINEL_FROZEN_TEMPEST:
                warn("The Sentinel of Winter calls a Frozen Tempest!");
                castSpellOnSelf(SPELL_SENTINEL_FROZEN_TEMPEST);
                scriptEvents.addEvent(EVENT_SENTINEL_FROZEN_TEMPEST, 45000);
                break;
            default:
                break;
        }
    }
};

// pulses Winter's Shroud until destroyed
class DenOfNalorakkShivercoreAI : public DenOfNalorakkCombatAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new DenOfNalorakkShivercoreAI(c); }
    explicit DenOfNalorakkShivercoreAI(Creature* pCreature) : DenOfNalorakkCombatAI(pCreature) {}

protected:
    void onEngage() override { scriptEvents.addEvent(EVENT_SHIVERCORE_WINTERS_SHROUD, 2000); }

    void onEvent(uint32_t eventId) override
    {
        if (eventId != EVENT_SHIVERCORE_WINTERS_SHROUD)
            return;

        castSpellAOE(SPELL_SHIVERCORE_WINTERS_SHROUD);
        scriptEvents.addEvent(EVENT_SHIVERCORE_WINTERS_SHROUD, 6000);
    }
};

// wanders over the battlefield
class DenOfNalorakkRagingSquallAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new DenOfNalorakkRagingSquallAI(c); }
    explicit DenOfNalorakkRagingSquallAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnLoad() override
    {
        setReactState(REACT_PASSIVE);
        _setMeleeDisabled(true);
        mOrigin = getCreature()->GetPosition();
    }

    void AIUpdate(unsigned long time_passed) override
    {
        if (!isAlive() || !denCountdown(mWanderMs, static_cast<uint32_t>(time_passed), 5000))
            return;

        LocationVector const spot = denRingPoint(mOrigin, 4.0f, 18.0f);
        moveTo(spot.x, spot.y, spot.z, false);
    }

private:
    LocationVector mOrigin;
    uint32_t mWanderMs = 1000;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Nalorakk, Loa of War

class DenOfNalorakkLoaAI : public DenOfNalorakkBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new DenOfNalorakkLoaAI(c); }
    explicit DenOfNalorakkLoaAI(Creature* pCreature) : DenOfNalorakkBossAI(pCreature, DATA_LOA_NALORAKK) {}

protected:
    void onEngage() override
    {
        mEchoGuids.clear();
        armEnergy(40000);
        scriptEvents.addEvent(EVENT_LOA_ECHOING_MAUL, 7000);
        scriptEvents.addEvent(EVENT_LOA_FORCEFUL_ROAR, 20000);
    }

    void onReset() override { mEchoGuids.clear(); }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_LOA_ECHOING_MAUL:
                castSpellOnVictim(SPELL_LOA_ECHOING_MAUL);
                commandEchoes();
                scriptEvents.addEvent(EVENT_LOA_ECHOING_MAUL, 17000);
                break;
            case EVENT_LOA_FORCEFUL_ROAR:
                castSpellAOE(SPELL_LOA_FORCEFUL_ROAR);
                scriptEvents.addEvent(EVENT_LOA_FORCEFUL_ROAR, 34000);
                break;
            default:
                break;
        }
    }

    // Zul'jarra is knocked down and the echoes charge at her
    void onEnergyFull() override
    {
        warn("Nalorakk unleashes the Fury of the War God! Intercept his echoes!");
        castSpellOnSelf(SPELL_LOA_FURY_OF_THE_WAR_GOD, true);

        Creature* pZuljarra = findNearestCreature(NPC_LOA_ZULJARRA, 150.0f);
        if (pZuljarra != nullptr && pZuljarra->isAlive())
            castSpell(pZuljarra, SPELL_LOA_CONCUSSIVE_SHOCK, true);

        castSpellAOE(SPELL_LOA_DEMORALIZING_SCREAM, true);

        for (uint8_t i = 0; i < EchoCount; ++i)
        {
            if (Creature* pEcho = summonCreature(NPC_ECHO_OF_NALORAKK, aroundSelf(5.0f, 9.0f), TIMED_DESPAWN, EchoLifetimeMs))
                mEchoGuids.push_back(pEcho->getGuid());
        }
    }

private:
    // living echoes repeat his maul
    void commandEchoes()
    {
        std::vector<uint64_t> living;
        for (uint64_t guid : mEchoGuids)
        {
            Creature* pEcho = getCreature()->getWorldMapCreature(guid);
            if (pEcho == nullptr || !pEcho->isAlive())
                continue;

            living.push_back(guid);
            if (pEcho->GetScript() != nullptr)
                pEcho->GetScript()->DoAction(ACTION_ECHO_JOINS_MAUL);
        }

        mEchoGuids.swap(living);
    }

    std::vector<uint64_t> mEchoGuids;
};

// charges Zul'jarra and stops at the first player in its way
class DenOfNalorakkEchoAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new DenOfNalorakkEchoAI(c); }
    explicit DenOfNalorakkEchoAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnLoad() override
    {
        mCharging = true;
        setReactState(REACT_PASSIVE);
    }

    void DoAction(int32_t action) override
    {
        if (action == ACTION_ECHO_JOINS_MAUL && !mCharging && isAlive())
            castSpellOnVictim(SPELL_LOA_ECHOING_MAUL);
    }

    void AIUpdate(unsigned long time_passed) override
    {
        if (!isAlive())
            return;

        uint32_t const diff = static_cast<uint32_t>(time_passed);
        if (!mCharging)
        {
            if (_isInCombat() && !_isCasting() && denCountdown(mSlashMs, diff, 8000))
                castSpellOnVictim(SPELL_ECHO_SPECTRAL_SLASH);
            return;
        }

        if (!denCountdown(mCheckMs, diff, 400))
            return;

        if (Unit* pPlayer = playerTouching(getCreature(), EchoCollideReach))
        {
            castSpell(pPlayer, SPELL_ECHO_ECHOING_FURY, true);
            engage(pPlayer);
            return;
        }

        Creature* pZuljarra = findNearestCreature(NPC_LOA_ZULJARRA, 150.0f);
        if (pZuljarra != nullptr && pZuljarra->isAlive())
        {
            if (getCreature()->getDistance(pZuljarra) <= EchoCollideReach)
            {
                castSpell(pZuljarra, SPELL_ECHO_ECHOING_FURY, true);
                mCharging = false;
                despawn(1500, 0);
                return;
            }

            steerTo(pZuljarra);
            return;
        }

        // nobody to protect, the echo hunts a player instead
        if (Unit* pPrey = selectUnitTarget(FilterArgs(TargetFilter_Player)))
            steerTo(pPrey);
    }

private:
    void steerTo(Unit* pTarget)
    {
        if (++mSteerCount % 5 != 1)
            return;

        moveTo(pTarget->GetPositionX(), pTarget->GetPositionY(), pTarget->GetPositionZ(), true);
    }

    void engage(Unit* pPlayer)
    {
        mCharging = false;
        setReactState(REACT_AGGRESSIVE);
        setZoneWideCombat();
        attackStart(pPlayer);
    }

    uint32_t mCheckMs = 400;
    uint32_t mSlashMs = 3000;
    uint32_t mSteerCount = 0;
    bool mCharging = true;
};

void SetupDenOfNalorakk(ScriptMgr* mgr)
{
    mgr->register_instance_script(MAP_DEN_OF_NALORAKK, &DenOfNalorakkInstanceScript::Create);

    mgr->register_creature_script(BOSS_HOARDMONGER, &DenOfNalorakkHoardmongerAI::Create);
    mgr->register_creature_script(BOSS_HOARDMONGER_ALT, &DenOfNalorakkHoardmongerAI::Create);
    mgr->register_creature_script(NPC_HOARD_ROTTEN_MUSHROOM, &DenOfNalorakkRottenMushroomAI::Create);

    mgr->register_creature_script(BOSS_SENTINEL_OF_WINTER, &DenOfNalorakkSentinelAI::Create);
    mgr->register_creature_script(BOSS_SENTINEL_OF_WINTER_ALT, &DenOfNalorakkSentinelAI::Create);
    mgr->register_creature_script(NPC_FRACTURED_SHIVERCORE, &DenOfNalorakkShivercoreAI::Create);
    mgr->register_creature_script(NPC_RAGING_SQUALL, &DenOfNalorakkRagingSquallAI::Create);
    mgr->register_creature_script(NPC_RAGING_SQUALL_ALT, &DenOfNalorakkRagingSquallAI::Create);

    mgr->register_creature_script(BOSS_LOA_NALORAKK, &DenOfNalorakkLoaAI::Create);
    mgr->register_creature_script(NPC_ECHO_OF_NALORAKK, &DenOfNalorakkEchoAI::Create);
    mgr->register_creature_script(NPC_ECHO_OF_NALORAKK_ALT, &DenOfNalorakkEchoAI::Create);
    mgr->register_creature_script(NPC_ECHO_OF_NALORAKK_THIRD, &DenOfNalorakkEchoAI::Create);
}
