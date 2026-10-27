/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "Instance_The_Underrot.hpp"

#include "Map/Maps/WorldMap.hpp"
#include "Objects/GameObject.h"
#include "Objects/Units/Creatures/Creature.h"
#include "Server/Script/CreatureAIScript.hpp"
#include "Server/Script/InstanceScript.hpp"
#include "Server/Script/ScriptMgr.hpp"
#include "Utilities/Random.hpp"

#include <cmath>
#include <map>
#include <vector>

enum TheUnderrotEvents
{
    EVENT_LEAXA_BLOOD_BOLT = 1,
    EVENT_LEAXA_SANGUINE_FEAST,
    EVENT_LEAXA_FEAST_LANDING,
    EVENT_LEAXA_CREEPING_ROT,
    EVENT_LEAXA_BLOOD_MIRROR,

    EVENT_ZANCHA_SHOCKWAVE,
    EVENT_ZANCHA_UPHEAVAL,
    EVENT_ZANCHA_BOUNDLESS_ROT,

    EVENT_CRAGMAW_CHARGE,
    EVENT_CRAGMAW_INDIGESTION,
    EVENT_CRAGMAW_REQUEUE,

    EVENT_BLOOD_TICK_SERRATED_FANGS,

    EVENT_UNBOUND_PUTRID_BLOOD,
    EVENT_UNBOUND_VILE_EXPULSION,

    EVENT_HEZREL_HOLY_BOLT,
    EVENT_HEZREL_PURGE_CORRUPTION,
    EVENT_HEZREL_CLEANSING_LIGHT
};

namespace
{
    // both stay shut until Sporecaller Zancha is slain
    uint32_t const UnderrotPassageDoors[] =
    {
        GO_UNDERROT_SHORTCUT_WALL,
        GO_UNDERROT_PYRAMID_DOOR
    };

    bool isUnderrotPassageDoor(uint32_t entry)
    {
        for (uint32_t doorEntry : UnderrotPassageDoors)
        {
            if (doorEntry == entry)
                return true;
        }

        return false;
    }

    uint32_t const MarkGraceMs = 1000;

    // blood pools on Leaxa's platform
    LocationVector const LeaxaMirrorSpots[] =
    {
        { 879.80f, 1222.62f, 56.48f },
        { 856.18f, 1232.54f, 56.52f },
        { 859.34f, 1238.02f, 56.52f },
        { 857.83f, 1220.65f, 56.47f },
        { 876.17f, 1240.28f, 56.41f },
        { 864.26f, 1240.59f, 56.52f },
        { 863.10f, 1218.06f, 56.47f }
    };
    uint32_t const LeaxaMirrorSpotCount = 7;
    float const CreepingRotStartRange = 5.0f;
    float const CreepingRotTravel = 58.0f;
    uint32_t const CreepingRotLifetimeMs = 15000;

    // shockwave and upheaval cadence: opening delays, then a repeating cycle
    struct ZanchaCadence
    {
        uint32_t const* opening;
        uint32_t openingCount;
        uint32_t const* cycle;
        uint32_t cycleCount;
    };

    uint32_t const ZanchaShockwaveOpening[] = { 14600, 15800, 30400 };
    uint32_t const ZanchaShockwaveCycle[] = { 15800, 40100 };
    uint32_t const ZanchaShockwaveHeroicOpening[] = { 14600, 45100 };
    uint32_t const ZanchaShockwaveHeroicCycle[] = { 20700, 30400 };
    uint32_t const ZanchaUpheavalOpening[] = { 15800, 30400 };
    uint32_t const ZanchaUpheavalCycle[] = { 15800, 15800, 24300 };
    uint32_t const ZanchaUpheavalHeroicOpening[] = { 20700, 24300 };
    uint32_t const ZanchaUpheavalHeroicCycle[] = { 20700, 30400 };

    ZanchaCadence const ZanchaShockwaveNormal = { ZanchaShockwaveOpening, 3, ZanchaShockwaveCycle, 2 };
    ZanchaCadence const ZanchaShockwaveHeroic = { ZanchaShockwaveHeroicOpening, 2, ZanchaShockwaveHeroicCycle, 2 };
    ZanchaCadence const ZanchaUpheavalNormal = { ZanchaUpheavalOpening, 2, ZanchaUpheavalCycle, 3 };
    ZanchaCadence const ZanchaUpheavalHeroic = { ZanchaUpheavalHeroicOpening, 2, ZanchaUpheavalHeroicCycle, 2 };

    uint32_t zanchaDelay(ZanchaCadence const& cadence, uint32_t step)
    {
        if (step < cadence.openingCount)
            return cadence.opening[step];

        return cadence.cycle[(step - cadence.openingCount) % cadence.cycleCount];
    }

    // festering harvest bar: four ticks of two, the fifth of three
    uint32_t const ZanchaHarvestFull = 100;

    uint32_t const CragmawTantrumFull = 100;
    uint32_t const CragmawTantrumGain = 2;
    uint32_t const LarvaHatchMs = 10000;
    float const LarvaCrushRange = 2.0f;

    LocationVector const UnboundPitCentre(1199.42f, 1481.94f, -181.51f);
    LocationVector const BloodVisageRises[] =
    {
        { 1228.46f, 1459.84f, -181.44f },
        { 1168.01f, 1499.57f, -181.48f }
    };
    uint32_t const BloodBarrierFull = 100;
    uint32_t const BloodBarrierDecay = 5;
    uint32_t const BloodBarrierDecayMs = 5000;
    uint32_t const FatalLinkShares = 6;
    float const HezrelReach = 80.0f;
    float const CleansingLightRadius = 10.0f;

    LocationVector pointTowards(LocationVector const& from, LocationVector const& to, float range)
    {
        float const angle = std::atan2(to.y - from.y, to.x - from.x);
        return LocationVector(from.x + range * std::cos(angle), from.y + range * std::sin(angle), from.z, angle);
    }

    LocationVector pointAround(LocationVector const& origin, float minRange, float maxRange)
    {
        float const angle = Util::getRandomFloat(6.2831853f);
        float const range = Util::getRandomFloat(minRange, maxRange);
        return LocationVector(origin.x + range * std::cos(angle), origin.y + range * std::sin(angle), origin.z, angle);
    }

    // living players in range of a creature
    std::vector<Unit*> playersNear(Creature* pCreature, float range)
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

    // a blood wave rolls out from the caster towards the target
    void releaseCreepingRot(CreatureAIScript* pCaster, Unit* pTarget)
    {
        if (pTarget == nullptr)
            return;

        pCaster->castSpell(pTarget, SPELL_LEAXA_CREEPING_ROT, true);
        LocationVector const start = pointTowards(pCaster->getCreature()->GetPosition(), pTarget->GetPosition(), CreepingRotStartRange);
        pCaster->summonCreature(NPC_BLOOD_WAVE_STALKER, start, TIMED_DESPAWN, CreepingRotLifetimeMs);
    }
}

//////////////////////////////////////////////////////////////////////////////////////////
// Instance

class TheUnderrotInstanceScript : public InstanceScript
{
public:
    explicit TheUnderrotInstanceScript(WorldMap* pMapMgr) : InstanceScript(pMapMgr)
    {
        setBossNumber(TheUnderrotEncounterCount);
    }

    static InstanceScript* Create(WorldMap* pMapMgr) { return new TheUnderrotInstanceScript(pMapMgr); }

    void OnCreaturePushToWorld(Creature* pCreature) override
    {
        switch (pCreature->getEntry())
        {
            case BOSS_CRAGMAW_THE_INFESTED:
            case BOSS_UNBOUND_ABOMINATION:
                mBossGuids[pCreature->getEntry()] = pCreature->GetNewGUID();
                break;
            default:
                break;
        }
    }

    void OnGameObjectPushToWorld(GameObject* pGameObject) override
    {
        if (!isUnderrotPassageDoor(pGameObject->getEntry()))
            return;

        mDoorGuids.push_back(pGameObject->GetNewGUID());
        applyDoorState(pGameObject);
    }

    void OnEncounterStateChange(uint32_t /*id*/, uint32_t /*state*/) override
    {
        for (WoWGuid const& guid : mDoorGuids)
        {
            if (GameObject* pDoor = getGameObjectByGuid(guid))
                applyDoorState(pDoor);
        }
    }

    Creature* getBoss(uint32_t entry)
    {
        auto const itr = mBossGuids.find(entry);
        return itr != mBossGuids.end() ? getCreatureByGuid(itr->second) : nullptr;
    }

private:
    void applyDoorState(GameObject* pDoor)
    {
        bool const open = getBossState(DATA_SPORECALLER_ZANCHA) == EncounterStates::Performed;
        uint8_t const wanted = open ? GO_STATE_OPEN : GO_STATE_CLOSED;
        if (pDoor->getState() != wanted)
            pDoor->setState(wanted);
    }

    std::map<uint32_t, WoWGuid> mBossGuids;
    std::vector<WoWGuid> mDoorGuids;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Shared combat base: event clock and marks that burst once they fade

class TheUnderrotCombatAI : public CreatureAIScript
{
public:
    explicit TheUnderrotCombatAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        mMarks.clear();
        onEngage();
    }

    void OnCombatStop(Unit* /*pTarget*/) override
    {
        scriptEvents.resetEvents();
        summons.despawnAll();
        mMarks.clear();

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
        burstFadedMarks(diff);
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

    Creature* callAdd(uint32_t entry, LocationVector const& position)
    {
        Creature* pAdd = summonCreature(entry, position, CORPSE_DESPAWN);
        if (pAdd != nullptr)
            setZoneWideCombat(pAdd);

        return pAdd;
    }

    void warn(char const* text) { sendChatMessage(CHAT_MSG_RAID_BOSS_EMOTE, 0, text); }

    // the burst follows once the mark aura is gone or the timeout ran out
    void markForBurst(Unit* pTarget, uint32_t markAura, uint32_t burstSpell, uint32_t timeoutMs)
    {
        if (pTarget == nullptr)
            return;

        castSpell(pTarget, markAura, true);
        mMarks.push_back({ pTarget->GetNewGUID(), markAura, burstSpell, MarkGraceMs, timeoutMs });
    }

private:
    struct FadingMark
    {
        WoWGuid target;
        uint32_t markAura;
        uint32_t burstSpell;
        uint32_t graceMs;
        uint32_t timeoutMs;
    };

    static uint32_t tickDown(uint32_t value, uint32_t diff) { return value > diff ? value - diff : 0; }

    void burstFadedMarks(uint32_t diff)
    {
        WorldMap* pMap = getCreature()->getWorldMap();
        for (auto itr = mMarks.begin(); itr != mMarks.end();)
        {
            itr->graceMs = tickDown(itr->graceMs, diff);
            itr->timeoutMs = tickDown(itr->timeoutMs, diff);

            Unit* pTarget = pMap != nullptr ? pMap->getUnit(itr->target) : nullptr;
            if (pTarget == nullptr || !pTarget->isAlive())
            {
                itr = mMarks.erase(itr);
                continue;
            }

            bool const faded = itr->graceMs == 0 && !pTarget->hasAurasWithId(itr->markAura);
            if (!faded && itr->timeoutMs != 0)
            {
                ++itr;
                continue;
            }

            castSpell(pTarget, itr->burstSpell, true);
            itr = mMarks.erase(itr);
        }
    }

    std::vector<FadingMark> mMarks;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Encounter base: frame and boss state

class TheUnderrotBossAI : public TheUnderrotCombatAI
{
public:
    TheUnderrotBossAI(Creature* pCreature, uint32_t dataId) : TheUnderrotCombatAI(pCreature), mDataId(dataId) {}

    void OnCombatStart(Unit* pTarget) override
    {
        if (InstanceScript* pInstance = getInstanceScript())
        {
            pInstance->sendUnitEncounter(EncounterFrameEngage, getCreature());
            pInstance->setBossState(mDataId, EncounterStates::InProgress);
        }

        TheUnderrotCombatAI::OnCombatStart(pTarget);
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

        TheUnderrotCombatAI::OnCombatStop(pTarget);
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
// Elder Leaxa

class TheUnderrotLeaxaAI : public TheUnderrotBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new TheUnderrotLeaxaAI(c); }
    explicit TheUnderrotLeaxaAI(Creature* pCreature) : TheUnderrotBossAI(pCreature, DATA_ELDER_LEAXA) {}

protected:
    void onEngage() override
    {
        scriptEvents.addEvent(EVENT_LEAXA_BLOOD_BOLT, 1000);
        scriptEvents.addEvent(EVENT_LEAXA_CREEPING_ROT, 12000);
        scriptEvents.addEvent(EVENT_LEAXA_BLOOD_MIRROR, 17000);

        if (isHeroic())
            scriptEvents.addEvent(EVENT_LEAXA_SANGUINE_FEAST, 8000);
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_LEAXA_BLOOD_BOLT:
                castSpellOnVictim(SPELL_LEAXA_BLOOD_BOLT);
                scriptEvents.addEvent(EVENT_LEAXA_BLOOD_BOLT, 6000);
                break;
            case EVENT_LEAXA_SANGUINE_FEAST:
                if (Unit* pTarget = anyPlayer())
                {
                    moveJump(pTarget->GetPosition(), 20.0f, 10.0f);
                    scriptEvents.addEvent(EVENT_LEAXA_FEAST_LANDING, 1200);
                }
                scriptEvents.addEvent(EVENT_LEAXA_SANGUINE_FEAST, 30000);
                break;
            case EVENT_LEAXA_FEAST_LANDING:
                castSpellAOE(SPELL_LEAXA_SANGUINE_FEAST, true);
                break;
            case EVENT_LEAXA_CREEPING_ROT:
                warn("Elder Leaxa sends a wave of Creeping Rot!");
                releaseCreepingRot(this, anyPlayer());
                scriptEvents.addEvent(EVENT_LEAXA_CREEPING_ROT, 16000);
                break;
            case EVENT_LEAXA_BLOOD_MIRROR:
                castSpellOnSelf(SPELL_LEAXA_BLOOD_MIRROR, true);
                callAdd(NPC_BLOOD_EFFIGY, LeaxaMirrorSpots[Util::getRandomUInt(LeaxaMirrorSpotCount - 1)]);
                scriptEvents.addEvent(EVENT_LEAXA_BLOOD_MIRROR, 47000);
                break;
            default:
                break;
        }
    }
};

// mirror image of Leaxa with a faster bolt
class TheUnderrotBloodEffigyAI : public TheUnderrotCombatAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new TheUnderrotBloodEffigyAI(c); }
    explicit TheUnderrotBloodEffigyAI(Creature* pCreature) : TheUnderrotCombatAI(pCreature) {}

protected:
    void onEngage() override
    {
        scriptEvents.addEvent(EVENT_LEAXA_BLOOD_BOLT, 1000);
        scriptEvents.addEvent(EVENT_LEAXA_SANGUINE_FEAST, 8000);
        scriptEvents.addEvent(EVENT_LEAXA_CREEPING_ROT, 12000);
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_LEAXA_BLOOD_BOLT:
                castSpellOnVictim(SPELL_LEAXA_BLOOD_BOLT);
                scriptEvents.addEvent(EVENT_LEAXA_BLOOD_BOLT, 2000);
                break;
            case EVENT_LEAXA_SANGUINE_FEAST:
                if (Unit* pTarget = anyPlayer())
                {
                    moveJump(pTarget->GetPosition(), 20.0f, 10.0f);
                    scriptEvents.addEvent(EVENT_LEAXA_FEAST_LANDING, 1200);
                }
                scriptEvents.addEvent(EVENT_LEAXA_SANGUINE_FEAST, 30000);
                break;
            case EVENT_LEAXA_FEAST_LANDING:
                castSpellAOE(SPELL_LEAXA_SANGUINE_FEAST, true);
                break;
            case EVENT_LEAXA_CREEPING_ROT:
                releaseCreepingRot(this, anyPlayer());
                scriptEvents.addEvent(EVENT_LEAXA_CREEPING_ROT, 16000);
                break;
            default:
                break;
        }
    }
};

// rotting wave that walks straight ahead
class TheUnderrotBloodWaveAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new TheUnderrotBloodWaveAI(c); }
    explicit TheUnderrotBloodWaveAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnLoad() override
    {
        setReactState(REACT_PASSIVE);
        _setMeleeDisabled(true);
        addUnitFlags(UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
        castSpellOnSelf(SPELL_LEAXA_CREEPING_ROT_WAVE, true);

        Creature* pSelf = getCreature();
        float const angle = pSelf->GetOrientation();
        moveTo(pSelf->GetPositionX() + CreepingRotTravel * std::cos(angle), pSelf->GetPositionY() + CreepingRotTravel * std::sin(angle), pSelf->GetPositionZ(), false);
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Sporecaller Zancha, Festering Harvest at a full bar

class TheUnderrotZanchaAI : public TheUnderrotBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new TheUnderrotZanchaAI(c); }
    explicit TheUnderrotZanchaAI(Creature* pCreature) : TheUnderrotBossAI(pCreature, DATA_SPORECALLER_ZANCHA) {}

protected:
    void onEngage() override
    {
        mHarvest = 0;
        mHarvestClock = 0;
        mHarvestTicks = 0;
        mShockwaveStep = 0;
        mUpheavalStep = 0;

        castSpellOnSelf(SPELL_ZANCHA_FESTERING_HARVEST_BAR, true);
        castSpellOnSelf(SPELL_ZANCHA_BOUNDLESS_ROT, true);

        scriptEvents.addEvent(EVENT_ZANCHA_SHOCKWAVE, isHeroic() ? 10600 : 10500);
        scriptEvents.addEvent(EVENT_ZANCHA_UPHEAVAL, isHeroic() ? 16600 : 16000);
    }

    void onCombatTick(uint32_t diff) override
    {
        mHarvestClock += diff;
        while (mHarvestClock >= 1000)
        {
            mHarvestClock -= 1000;
            ++mHarvestTicks;
            mHarvest += (mHarvestTicks % 5 == 0) ? 3 : 2;
        }

        if (mHarvest < ZanchaHarvestFull || _isCasting())
            return;

        mHarvest = 0;
        warn("Sporecaller Zancha unleashes a Festering Harvest!");
        castSpellAOE(SPELL_ZANCHA_FESTERING_HARVEST);
        scriptEvents.addEvent(EVENT_ZANCHA_BOUNDLESS_ROT, 9000);
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_ZANCHA_SHOCKWAVE:
                castSpellOnSelf(SPELL_ZANCHA_SHOCKWAVE);
                scriptEvents.addEvent(EVENT_ZANCHA_SHOCKWAVE, zanchaDelay(isHeroic() ? ZanchaShockwaveHeroic : ZanchaShockwaveNormal, mShockwaveStep++));
                break;
            case EVENT_ZANCHA_UPHEAVAL:
                markForBurst(anyPlayer(), SPELL_ZANCHA_UPHEAVAL_MARK, SPELL_ZANCHA_UPHEAVAL_BLAST, 10000);
                scriptEvents.addEvent(EVENT_ZANCHA_UPHEAVAL, zanchaDelay(isHeroic() ? ZanchaUpheavalHeroic : ZanchaUpheavalNormal, mUpheavalStep++));
                break;
            case EVENT_ZANCHA_BOUNDLESS_ROT:
                castSpellOnSelf(SPELL_ZANCHA_BOUNDLESS_ROT, true);
                break;
            default:
                break;
        }
    }

private:
    uint32_t mHarvest = 0;
    uint32_t mHarvestClock = 0;
    uint32_t mHarvestTicks = 0;
    uint32_t mShockwaveStep = 0;
    uint32_t mUpheavalStep = 0;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Cragmaw the Infested, every attack leaves larvae behind

class TheUnderrotCragmawAI : public TheUnderrotBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new TheUnderrotCragmawAI(c); }
    explicit TheUnderrotCragmawAI(Creature* pCreature) : TheUnderrotBossAI(pCreature, DATA_CRAGMAW_THE_INFESTED) {}

    void OnLoad() override
    {
        // vehicle template: the core keeps it passive unless the AI is switched on
        getCreature()->setAItoUse(true);
        setCanEnterCombat(true);
    }

    // larvae hatch through Cragmaw so the ticks leave with him
    void hatchTick(LocationVector const& position)
    {
        if (isAlive() && _isInCombat())
            callAdd(NPC_BLOOD_TICK, position);
    }

protected:
    void onEngage() override
    {
        mTantrum = 0;
        mTantrumClock = 0;
        queueFeeding();
    }

    void onCombatTick(uint32_t diff) override
    {
        // heroic: the tantrum bar fills on its own
        if (!isHeroic())
            return;

        mTantrumClock += diff;
        while (mTantrumClock >= 1000)
        {
            mTantrumClock -= 1000;
            mTantrum += CragmawTantrumGain;
        }

        if (mTantrum < CragmawTantrumFull || _isCasting())
            return;

        mTantrum = 0;
        warn("Cragmaw the Infested throws a Tantrum!");
        castSpellOnSelf(SPELL_CRAGMAW_TANTRUM);
        for (uint8_t i = 0; i < 5; ++i)
            dropLarva(pointAround(getCreature()->GetPosition(), 5.0f, 15.0f));
        queueFeeding();
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_CRAGMAW_CHARGE:
                chargeThrough(anyPlayer());
                if (mSecondChargeMs != 0)
                {
                    scriptEvents.addEvent(EVENT_CRAGMAW_CHARGE, mSecondChargeMs);
                    mSecondChargeMs = 0;
                }
                break;
            case EVENT_CRAGMAW_INDIGESTION:
                if (Unit* pVictim = getCreature()->getAIInterface()->getCurrentTarget())
                {
                    castSpell(pVictim, SPELL_CRAGMAW_INDIGESTION);
                    for (uint8_t i = 0; i < 4; ++i)
                        dropLarva(pointAround(pVictim->GetPosition(), 3.0f, 7.0f));
                }
                break;
            case EVENT_CRAGMAW_REQUEUE:
                queueFeeding();
                break;
            default:
                break;
        }
    }

private:
    // one of two openings, normal repeats it every 45 seconds
    void queueFeeding()
    {
        scriptEvents.removeEvent(EVENT_CRAGMAW_CHARGE);
        scriptEvents.removeEvent(EVENT_CRAGMAW_INDIGESTION);
        scriptEvents.removeEvent(EVENT_CRAGMAW_REQUEUE);

        if (Util::getRandomUInt(1) == 0)
        {
            scriptEvents.addEvent(EVENT_CRAGMAW_CHARGE, 9000);
            scriptEvents.addEvent(EVENT_CRAGMAW_INDIGESTION, 20000);
            mSecondChargeMs = 24000;
        }
        else
        {
            scriptEvents.addEvent(EVENT_CRAGMAW_INDIGESTION, 9000);
            scriptEvents.addEvent(EVENT_CRAGMAW_CHARGE, 21000);
            mSecondChargeMs = 20000;
        }

        if (!isHeroic())
            scriptEvents.addEvent(EVENT_CRAGMAW_REQUEUE, 45000);
    }

    // larvae are left along the charge lane
    void chargeThrough(Unit* pTarget)
    {
        if (pTarget == nullptr)
            return;

        LocationVector const from = getCreature()->GetPosition();
        LocationVector const to = pTarget->GetPosition();
        castSpell(pTarget, SPELL_CRAGMAW_CHARGE);

        for (uint8_t i = 1; i <= 6; ++i)
        {
            float const step = static_cast<float>(i) / 7.0f;
            dropLarva(LocationVector(from.x + (to.x - from.x) * step, from.y + (to.y - from.y) * step, from.z, from.o));
        }
    }

    void dropLarva(LocationVector const& position)
    {
        summonCreature(NPC_BLOOD_TICK_LARVA, position, TIMED_DESPAWN, LarvaHatchMs + 2000);
    }

    uint32_t mTantrum = 0;
    uint32_t mTantrumClock = 0;
    uint32_t mSecondChargeMs = 0;
};

// crushed underfoot or it hatches into a Blood Tick
class TheUnderrotLarvaAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new TheUnderrotLarvaAI(c); }
    explicit TheUnderrotLarvaAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnLoad() override
    {
        setReactState(REACT_PASSIVE);
        setRooted(true);
        _setMeleeDisabled(true);
    }

    void AIUpdate(unsigned long time_passed) override
    {
        if (mDone || !isAlive())
            return;

        uint32_t const diff = static_cast<uint32_t>(time_passed);
        mHatchLeft = mHatchLeft > diff ? mHatchLeft - diff : 0;
        if (mHatchLeft == 0)
        {
            hatch();
            return;
        }

        if (mCheckClock > diff)
        {
            mCheckClock -= diff;
            return;
        }

        mCheckClock = 500;
        if (!playersNear(getCreature(), LarvaCrushRange).empty())
        {
            mDone = true;
            castSpellOnSelf(SPELL_LARVA_DESTROY_LARVA, true);
            despawn(300, 0);
        }
    }

private:
    void hatch()
    {
        mDone = true;

        InstanceScript* pInstance = getInstanceScript();
        if (pInstance != nullptr && getCreature()->GetMapId() == MAP_THE_UNDERROT)
        {
            Creature* pCragmaw = static_cast<TheUnderrotInstanceScript*>(pInstance)->getBoss(BOSS_CRAGMAW_THE_INFESTED);
            if (pCragmaw != nullptr && pCragmaw->isAlive() && pCragmaw->GetScript() != nullptr)
                static_cast<TheUnderrotCragmawAI*>(pCragmaw->GetScript())->hatchTick(getCreature()->GetPosition());
        }

        despawn(300, 0);
    }

    uint32_t mHatchLeft = LarvaHatchMs;
    uint32_t mCheckClock = 500;
    bool mDone = false;
};

class TheUnderrotBloodTickAI : public TheUnderrotCombatAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new TheUnderrotBloodTickAI(c); }
    explicit TheUnderrotBloodTickAI(Creature* pCreature) : TheUnderrotCombatAI(pCreature) {}

protected:
    void onEngage() override { scriptEvents.addEvent(EVENT_BLOOD_TICK_SERRATED_FANGS, 1000); }

    void onEvent(uint32_t eventId) override
    {
        if (eventId != EVENT_BLOOD_TICK_SERRATED_FANGS)
            return;

        castSpellOnVictim(SPELL_BLOOD_TICK_SERRATED_FANGS);
        scriptEvents.addEvent(EVENT_BLOOD_TICK_SERRATED_FANGS, 6000);
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Unbound Abomination, shielded until its Blood Visages are cut down

class TheUnderrotUnboundAbominationAI : public TheUnderrotBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new TheUnderrotUnboundAbominationAI(c); }
    explicit TheUnderrotUnboundAbominationAI(Creature* pCreature) : TheUnderrotBossAI(pCreature, DATA_UNBOUND_ABOMINATION) {}

    // blood barrier: almost every hit is soaked and feeds the barrier
    void DamageTaken(Unit* /*pAttacker*/, uint32_t* damage) override
    {
        if (damage == nullptr || mFatalLink || !_isInCombat())
            return;

        if (mBarrier < BloodBarrierFull)
            ++mBarrier;

        *damage /= 100;
    }

    void OnSummonDies(Creature* pSummon, Unit* pKiller) override
    {
        if (pSummon != nullptr && pSummon->getEntry() == NPC_BLOOD_VISAGE)
            sufferFatalLink(pKiller);
    }

protected:
    void onEngage() override
    {
        mBarrier = 0;
        mBarrierClock = 0;
        mFatalLink = false;

        // the keeper placed in the pit helps, otherwise one is called
        if (findNearestCreature(NPC_TITAN_KEEPER_HEZREL, HezrelReach) == nullptr)
        {
            if (Creature* pKeeper = summonCreature(NPC_TITAN_KEEPER_HEZREL, UnboundPitCentre, MANUAL_DESPAWN))
                pKeeper->setFaction(35);
        }

        scriptEvents.addEvent(EVENT_UNBOUND_PUTRID_BLOOD, 10000);
        scriptEvents.addEvent(EVENT_UNBOUND_VILE_EXPULSION, 15000);
    }

    void onCombatTick(uint32_t diff) override
    {
        mBarrierClock += diff;
        if (mBarrierClock >= BloodBarrierDecayMs)
        {
            mBarrierClock = 0;
            mBarrier = mBarrier > BloodBarrierDecay ? mBarrier - BloodBarrierDecay : 0;
        }

        if (mBarrier < BloodBarrierFull)
            return;

        mBarrier = 0;
        warn("Blood Visages tear free from the Unbound Abomination!");
        for (LocationVector const& rise : BloodVisageRises)
            callAdd(NPC_BLOOD_VISAGE, rise);
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_UNBOUND_PUTRID_BLOOD:
                castSpellAOE(SPELL_UNBOUND_PUTRID_BLOOD, true);
                scriptEvents.addEvent(EVENT_UNBOUND_PUTRID_BLOOD, 8000);
                break;
            case EVENT_UNBOUND_VILE_EXPULSION:
                castSpellOnVictim(SPELL_UNBOUND_VILE_EXPULSION);
                scriptEvents.addEvent(EVENT_UNBOUND_VILE_EXPULSION, 15000);
                break;
            default:
                break;
        }
    }

private:
    // each fallen visage tears off a sixth of the abomination's health
    void sufferFatalLink(Unit* pKiller)
    {
        if (!isAlive() || !_isInCombat())
            return;

        Creature* pSelf = getCreature();
        uint32_t const share = pSelf->getMaxHealth() / FatalLinkShares + 1;
        if (share < pSelf->getHealth())
        {
            pSelf->setHealth(pSelf->getHealth() - share);
            return;
        }

        Unit* pAttacker = pKiller != nullptr && pKiller->isAlive() ? pKiller : pSelf;
        mFatalLink = true;
        pAttacker->dealDamage(pSelf, pSelf->getHealth(), 0);
        mFatalLink = false;
    }

    uint32_t mBarrier = 0;
    uint32_t mBarrierClock = 0;
    bool mFatalLink = false;
};

// keeper in the pit, aids the party while the abomination fights
class TheUnderrotHezrelAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new TheUnderrotHezrelAI(c); }
    explicit TheUnderrotHezrelAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnLoad() override { setReactState(REACT_PASSIVE); }

    void AIUpdate(unsigned long time_passed) override
    {
        Creature* pAbomination = activeAbomination();
        if (pAbomination == nullptr)
        {
            if (mAssisting)
            {
                mAssisting = false;
                scriptEvents.resetEvents();
            }
            return;
        }

        if (!mAssisting)
        {
            mAssisting = true;
            scriptEvents.addEvent(EVENT_HEZREL_HOLY_BOLT, 5000);
            scriptEvents.addEvent(EVENT_HEZREL_CLEANSING_LIGHT, 20000);
        }

        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);
        if (_isCasting())
            return;

        switch (scriptEvents.getFinishedEvent())
        {
            case EVENT_HEZREL_HOLY_BOLT:
                castSpell(pAbomination, SPELL_HEZREL_HOLY_BOLT);
                scriptEvents.addEvent(EVENT_HEZREL_PURGE_CORRUPTION, 7000);
                break;
            case EVENT_HEZREL_PURGE_CORRUPTION:
                if (Creature* pVisage = findNearestCreature(NPC_BLOOD_VISAGE, HezrelReach))
                {
                    if (pVisage->isAlive())
                        castSpell(pVisage, SPELL_HEZREL_PURGE_CORRUPTION);
                }
                scriptEvents.addEvent(EVENT_HEZREL_HOLY_BOLT, 5000);
                break;
            case EVENT_HEZREL_CLEANSING_LIGHT:
                cleanseAround();
                scriptEvents.addEvent(EVENT_HEZREL_CLEANSING_LIGHT, 20000);
                break;
            default:
                break;
        }
    }

private:
    Creature* activeAbomination()
    {
        InstanceScript* pInstance = getInstanceScript();
        if (pInstance == nullptr || getCreature()->GetMapId() != MAP_THE_UNDERROT)
            return nullptr;

        if (pInstance->getBossState(DATA_UNBOUND_ABOMINATION) != EncounterStates::InProgress)
            return nullptr;

        Creature* pAbomination = static_cast<TheUnderrotInstanceScript*>(pInstance)->getBoss(BOSS_UNBOUND_ABOMINATION);
        if (pAbomination == nullptr || !pAbomination->isAlive() || getCreature()->getDistance(pAbomination) > HezrelReach)
            return nullptr;

        return pAbomination;
    }

    // purges Putrid Blood from everyone close to the chosen player
    void cleanseAround()
    {
        std::vector<Unit*> players = playersNear(getCreature(), HezrelReach);
        if (players.empty())
            return;

        Unit* pTarget = players[Util::getRandomUInt(static_cast<uint32_t>(players.size() - 1))];
        sendChatMessage(CHAT_MSG_RAID_BOSS_EMOTE, 0, "Titan Keeper Hezrel begins to cast Cleansing Light!");
        castSpell(pTarget, SPELL_HEZREL_CLEANSING_LIGHT);

        for (Unit* pPlayer : players)
        {
            if (pPlayer->getDistance(pTarget) <= CleansingLightRadius)
                pPlayer->removeAllAurasById(SPELL_UNBOUND_PUTRID_BLOOD);
        }
    }

    bool mAssisting = false;
};

void SetupTheUnderrot(ScriptMgr* mgr)
{
    mgr->register_instance_script(MAP_THE_UNDERROT, &TheUnderrotInstanceScript::Create);

    mgr->register_creature_script(BOSS_ELDER_LEAXA, &TheUnderrotLeaxaAI::Create);
    mgr->register_creature_script(NPC_BLOOD_EFFIGY, &TheUnderrotBloodEffigyAI::Create);
    mgr->register_creature_script(NPC_BLOOD_WAVE_STALKER, &TheUnderrotBloodWaveAI::Create);

    mgr->register_creature_script(BOSS_SPORECALLER_ZANCHA, &TheUnderrotZanchaAI::Create);

    mgr->register_creature_script(BOSS_CRAGMAW_THE_INFESTED, &TheUnderrotCragmawAI::Create);
    mgr->register_creature_script(NPC_BLOOD_TICK_LARVA, &TheUnderrotLarvaAI::Create);
    mgr->register_creature_script(NPC_BLOOD_TICK, &TheUnderrotBloodTickAI::Create);

    mgr->register_creature_script(BOSS_UNBOUND_ABOMINATION, &TheUnderrotUnboundAbominationAI::Create);
    mgr->register_creature_script(NPC_TITAN_KEEPER_HEZREL, &TheUnderrotHezrelAI::Create);
}
