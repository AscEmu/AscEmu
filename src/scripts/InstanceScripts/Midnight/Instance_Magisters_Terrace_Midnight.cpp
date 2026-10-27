/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "Instance_Magisters_Terrace_Midnight.hpp"

#include "Objects/Units/Creatures/Creature.h"
#include "Server/Script/CreatureAIScript.hpp"
#include "Server/Script/InstanceScript.hpp"
#include "Server/Script/ScriptMgr.hpp"
#include "Utilities/Random.hpp"

#include <cmath>
#include <vector>

enum MagistersTerraceMidnightEvents
{
    EVENT_CUSTOS_REPULSING_SLAM = 1,
    EVENT_CUSTOS_ETHEREAL_SHACKLES,
    EVENT_CUSTOS_ARCANE_EXPULSION,
    EVENT_CUSTOS_REFUEL_DONE,

    EVENT_SERANEL_RUNIC_MARK,
    EVENT_SERANEL_SUPPRESSION_ZONE,
    EVENT_SERANEL_HASTENING_WARD,

    EVENT_GEMELLUS_COSMIC_STING,
    EVENT_GEMELLUS_VOID_SECRETIONS,
    EVENT_GEMELLUS_ASTRAL_GRASP,

    EVENT_DEGENTRIUS_HULKING_FRAGMENT,
    EVENT_DEGENTRIUS_DEVOURING_ENTROPY,
    EVENT_DEGENTRIUS_VOID_ESSENCE,
    EVENT_DEGENTRIUS_ESSENCE_IMPACT,
    EVENT_DEGENTRIUS_VOID_TORRENT
};

namespace
{
    float const TerraceTwoPi = 6.2831853f;

    // Arcanotron Custos runs dry after this long and refuels for the second value
    uint32_t const CustosFuelMs = 45000;
    uint32_t const CustosRefuelMs = 15000;
    uint8_t const CustosOrbCount = 4;
    float const OrbSoakReach = 2.5f;
    float const OrbFeedReach = 3.5f;

    int32_t const GemellusSecondSplitPct = 50;

    float const EssenceCatchReach = 5.0f;

    LocationVector terraceRingPoint(LocationVector const& origin, float minRange, float maxRange)
    {
        float const angle = Util::getRandomFloat(TerraceTwoPi);
        float const range = Util::getRandomFloat(minRange, maxRange);
        return LocationVector(origin.x + range * std::cos(angle), origin.y + range * std::sin(angle), origin.z, angle);
    }

    // living players around a point
    bool anyPlayerNear(Creature* pCreature, LocationVector const& spot, float range)
    {
        for (Object* pObject : pCreature->getInRangePlayersSet())
        {
            Unit* pPlayer = static_cast<Unit*>(pObject);
            if (pPlayer != nullptr && pPlayer->isAlive() && pPlayer->getDistance(spot) <= range)
                return true;
        }

        return false;
    }
}

//////////////////////////////////////////////////////////////////////////////////////////
// Instance

class MagistersTerraceMidnightInstanceScript : public InstanceScript
{
public:
    explicit MagistersTerraceMidnightInstanceScript(WorldMap* pMapMgr) : InstanceScript(pMapMgr)
    {
        setBossNumber(MagistersTerraceMidnightEncounterCount);
    }

    static InstanceScript* Create(WorldMap* pMapMgr) { return new MagistersTerraceMidnightInstanceScript(pMapMgr); }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Shared combat base: event clock, energy bar and target helpers

class MagistersTerraceMidnightCombatAI : public CreatureAIScript
{
public:
    explicit MagistersTerraceMidnightCombatAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

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

    void castAt(Unit* pTarget, uint32_t spellId, bool triggered = false)
    {
        if (pTarget != nullptr)
            castSpell(pTarget, spellId, triggered);
    }

    void castUnder(Unit* pTarget, uint32_t spellId, bool triggered = false)
    {
        if (pTarget != nullptr)
            getCreature()->castSpellLoc(pTarget->GetPosition(), spellId, triggered);
    }

    Creature* callAdd(uint32_t entry, LocationVector const& position)
    {
        Creature* pAdd = summonCreature(entry, position, CORPSE_DESPAWN);
        if (pAdd != nullptr)
            setZoneWideCombat(pAdd);

        return pAdd;
    }

    LocationVector aroundSelf(float minRange, float maxRange) { return terraceRingPoint(getCreature()->GetPosition(), minRange, maxRange); }

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

class MagistersTerraceMidnightBossAI : public MagistersTerraceMidnightCombatAI
{
public:
    MagistersTerraceMidnightBossAI(Creature* pCreature, uint32_t dataId) : MagistersTerraceMidnightCombatAI(pCreature), mDataId(dataId) {}

    void OnCombatStart(Unit* pTarget) override
    {
        if (InstanceScript* pInstance = getInstanceScript())
        {
            pInstance->sendUnitEncounter(EncounterFrameEngage, getCreature());
            pInstance->setBossState(mDataId, EncounterStates::InProgress);
        }

        MagistersTerraceMidnightCombatAI::OnCombatStart(pTarget);
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

        MagistersTerraceMidnightCombatAI::OnCombatStop(pTarget);
    }

    void OnDied(Unit* /*pKiller*/) override
    {
        if (InstanceScript* pInstance = getInstanceScript())
        {
            pInstance->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
            pInstance->setBossState(mDataId, EncounterStates::Performed);
        }

        onDefeat();
        scriptEvents.resetEvents();
        summons.despawnAll();
    }

protected:
    virtual void onDefeat() {}

    uint32_t mDataId;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Arcanotron Custos, refuels from Energy Orbs once its tank runs dry

class MagistersTerraceMidnightCustosAI : public MagistersTerraceMidnightBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new MagistersTerraceMidnightCustosAI(c); }
    explicit MagistersTerraceMidnightCustosAI(Creature* pCreature) : MagistersTerraceMidnightBossAI(pCreature, DATA_ARCANOTRON_CUSTOS) {}

    void DoAction(int32_t action) override
    {
        // an orb that slipped through empowers it
        if (action == ACTION_CUSTOS_ORB_ABSORBED && isAlive() && _isInCombat())
            castSpellOnSelf(SPELL_CUSTOS_ARCANE_EMPOWERMENT, true);
    }

protected:
    void onEngage() override
    {
        mFuelLeftMs = CustosFuelMs;
        mRefueling = false;

        scriptEvents.addEvent(EVENT_CUSTOS_REPULSING_SLAM, 7000);
        scriptEvents.addEvent(EVENT_CUSTOS_ETHEREAL_SHACKLES, 11000);
        scriptEvents.addEvent(EVENT_CUSTOS_ARCANE_EXPULSION, 20000);
    }

    void onReset() override
    {
        mRefueling = false;
        _removeAura(SPELL_CUSTOS_REFUELING_PROTOCOL);
        _removeAura(SPELL_CUSTOS_ARCANE_EMPOWERMENT);
    }

    void onCombatTick(uint32_t diff) override
    {
        if (mRefueling)
            return;

        mFuelLeftMs = mFuelLeftMs > diff ? mFuelLeftMs - diff : 0;
        if (mFuelLeftMs == 0 && !_isCasting())
            beginRefueling();
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_CUSTOS_REPULSING_SLAM:
                castSpellOnVictim(SPELL_CUSTOS_REPULSING_SLAM);
                scriptEvents.addEvent(EVENT_CUSTOS_REPULSING_SLAM, 15000);
                break;
            case EVENT_CUSTOS_ETHEREAL_SHACKLES:
                castAt(anyPlayerButTank(), SPELL_CUSTOS_ETHEREAL_SHACKLES);
                scriptEvents.addEvent(EVENT_CUSTOS_ETHEREAL_SHACKLES, 20000);
                break;
            case EVENT_CUSTOS_ARCANE_EXPULSION:
                warn("Arcanotron Custos charges an Arcane Expulsion!");
                castSpellAOE(SPELL_CUSTOS_ARCANE_EXPULSION);
                scriptEvents.addEvent(EVENT_CUSTOS_ARCANE_EXPULSION, 30000);
                break;
            case EVENT_CUSTOS_REFUEL_DONE:
                endRefueling();
                break;
            default:
                break;
        }
    }

private:
    // orbs drift in from the edge of the platform
    void beginRefueling()
    {
        mRefueling = true;
        warn("Arcanotron Custos runs dry and begins its Refueling Protocol!");
        castSpellOnSelf(SPELL_CUSTOS_REFUELING_PROTOCOL, true);

        for (uint8_t i = 0; i < CustosOrbCount; ++i)
            summonCreature(NPC_CUSTOS_ENERGY_ORB, aroundSelf(18.0f, 25.0f), CORPSE_DESPAWN);

        scriptEvents.addEvent(EVENT_CUSTOS_REFUEL_DONE, CustosRefuelMs);
    }

    void endRefueling()
    {
        mRefueling = false;
        mFuelLeftMs = CustosFuelMs;
        _removeAura(SPELL_CUSTOS_REFUELING_PROTOCOL);
        summons.despawnEntry(NPC_CUSTOS_ENERGY_ORB);
    }

    uint32_t mFuelLeftMs = 0;
    bool mRefueling = false;
};

// drifts to Arcanotron Custos, a player in its path soaks it
class MagistersTerraceMidnightEnergyOrbAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new MagistersTerraceMidnightEnergyOrbAI(c); }
    explicit MagistersTerraceMidnightEnergyOrbAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnLoad() override
    {
        setReactState(REACT_PASSIVE);
        _setMeleeDisabled(true);
    }

    void AIUpdate(unsigned long time_passed) override
    {
        if (mSpent || !isAlive())
            return;

        uint32_t const diff = static_cast<uint32_t>(time_passed);
        if (mCheckMs > diff)
        {
            mCheckMs -= diff;
            return;
        }

        mCheckMs = 500;

        for (Object* pObject : getCreature()->getInRangePlayersSet())
        {
            Unit* pPlayer = static_cast<Unit*>(pObject);
            if (pPlayer != nullptr && pPlayer->isAlive() && getCreature()->getDistance(pPlayer) <= OrbSoakReach)
            {
                castSpell(pPlayer, SPELL_CUSTOS_UNSTABLE_ENERGY, true);
                expire();
                return;
            }
        }

        Creature* pCustos = findNearestCreature(BOSS_ARCANOTRON_CUSTOS, 80.0f);
        if (pCustos == nullptr || !pCustos->isAlive())
        {
            expire();
            return;
        }

        if (getCreature()->getDistance(pCustos) <= OrbFeedReach)
        {
            if (pCustos->GetScript() != nullptr)
                pCustos->GetScript()->DoAction(ACTION_CUSTOS_ORB_ABSORBED);

            expire();
            return;
        }

        // the boss keeps moving, steer again every other second
        if (++mSteerCount % 4 == 1)
            moveTo(pCustos->GetPositionX(), pCustos->GetPositionY(), pCustos->GetPositionZ(), false);
    }

private:
    void expire()
    {
        mSpent = true;
        despawn(200, 0);
    }

    uint32_t mCheckMs = 500;
    uint32_t mSteerCount = 0;
    bool mSpent = false;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Seranel Sunlash

class MagistersTerraceMidnightSeranelAI : public MagistersTerraceMidnightBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new MagistersTerraceMidnightSeranelAI(c); }
    explicit MagistersTerraceMidnightSeranelAI(Creature* pCreature) : MagistersTerraceMidnightBossAI(pCreature, DATA_SERANEL_SUNLASH) {}

protected:
    void onEngage() override
    {
        armEnergy(40000);
        scriptEvents.addEvent(EVENT_SERANEL_RUNIC_MARK, 6000);
        scriptEvents.addEvent(EVENT_SERANEL_SUPPRESSION_ZONE, 12000);
        scriptEvents.addEvent(EVENT_SERANEL_HASTENING_WARD, 18000);
    }

    void onReset() override { _removeAura(SPELL_SERANEL_HASTENING_WARD); }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_SERANEL_RUNIC_MARK:
                castAt(anyPlayer(), SPELL_SERANEL_RUNIC_MARK);
                scriptEvents.addEvent(EVENT_SERANEL_RUNIC_MARK, 20000);
                break;
            case EVENT_SERANEL_SUPPRESSION_ZONE:
                castUnder(anyPlayer(), SPELL_SERANEL_SUPPRESSION_ZONE);
                scriptEvents.addEvent(EVENT_SERANEL_SUPPRESSION_ZONE, 24000);
                break;
            case EVENT_SERANEL_HASTENING_WARD:
                castSpellOnSelf(SPELL_SERANEL_HASTENING_WARD);
                scriptEvents.addEvent(EVENT_SERANEL_HASTENING_WARD, 30000);
                break;
            default:
                break;
        }
    }

    // only the Suppression Zones give shelter
    void onEnergyFull() override
    {
        warn("Seranel Sunlash sends out a Wave of Silence! Seek a Suppression Zone!");
        castSpellAOE(SPELL_SERANEL_WAVE_OF_SILENCE);
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Gemellus, splits into three that act as one

class MagistersTerraceMidnightGemellusAI : public MagistersTerraceMidnightBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new MagistersTerraceMidnightGemellusAI(c); }
    explicit MagistersTerraceMidnightGemellusAI(Creature* pCreature) : MagistersTerraceMidnightBossAI(pCreature, DATA_GEMELLUS) {}

protected:
    void onEngage() override
    {
        mSecondSplit = false;
        mStingGuid = 0;
        castSpellOnSelf(SPELL_GEMELLUS_SYNAPTIC_NEXUS, true);
        triplicate();

        scriptEvents.addEvent(EVENT_GEMELLUS_COSMIC_STING, 8000);
        scriptEvents.addEvent(EVENT_GEMELLUS_ASTRAL_GRASP, 14000);
    }

    void onReset() override
    {
        mCloneGuids.clear();
        _removeAura(SPELL_GEMELLUS_SYNAPTIC_NEXUS);
    }

    void onDefeat() override { mCloneGuids.clear(); }

    void onCombatTick(uint32_t /*diff*/) override
    {
        if (mSecondSplit || _getHealthPercent() > GemellusSecondSplitPct || _isCasting())
            return;

        mSecondSplit = true;
        triplicate();
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_GEMELLUS_COSMIC_STING:
                if (Unit* pTarget = anyPlayer())
                {
                    castSpell(pTarget, SPELL_GEMELLUS_COSMIC_STING);
                    mStingGuid = pTarget->getGuid();
                    scriptEvents.addEvent(EVENT_GEMELLUS_VOID_SECRETIONS, 5000);
                }
                scriptEvents.addEvent(EVENT_GEMELLUS_COSMIC_STING, 16000);
                break;
            case EVENT_GEMELLUS_VOID_SECRETIONS:
                // the sting leaves a pool where its victim stands
                if (Unit* pTarget = getCreature()->getWorldMapUnit(mStingGuid))
                {
                    if (pTarget->isAlive())
                        getCreature()->castSpellLoc(pTarget->GetPosition(), SPELL_GEMELLUS_VOID_SECRETIONS, true);
                }
                mStingGuid = 0;
                break;
            case EVENT_GEMELLUS_ASTRAL_GRASP:
                castAt(anyPlayer(), SPELL_GEMELLUS_ASTRAL_GRASP);
                commandClones(ACTION_GEMELLUS_ASTRAL_GRASP);
                scriptEvents.addEvent(EVENT_GEMELLUS_ASTRAL_GRASP, 26000);
                break;
            default:
                break;
        }
    }

private:
    // the old pair fades, two fresh clones drift away from the original
    void triplicate()
    {
        warn("Gemellus splits apart with Triplicate!");
        castSpellOnSelf(SPELL_GEMELLUS_TRIPLICATE, true);

        summons.despawnEntry(NPC_GEMELLUS_CLONE);
        summons.despawnEntry(NPC_GEMELLUS_CLONE_SECOND);
        mCloneGuids.clear();

        uint32_t const clones[2] = { NPC_GEMELLUS_CLONE, NPC_GEMELLUS_CLONE_SECOND };
        for (uint32_t entry : clones)
        {
            if (Creature* pClone = summonCreature(entry, aroundSelf(10.0f, 15.0f), CORPSE_DESPAWN))
                mCloneGuids.push_back(pClone->getGuid());
        }
    }

    void commandClones(int32_t action)
    {
        for (uint64_t guid : mCloneGuids)
        {
            Creature* pClone = getCreature()->getWorldMapCreature(guid);
            if (pClone != nullptr && pClone->isAlive() && pClone->GetScript() != nullptr)
                pClone->GetScript()->DoAction(action);
        }
    }

    std::vector<uint64_t> mCloneGuids;
    uint64_t mStingGuid = 0;
    bool mSecondSplit = false;
};

// shares the original's health and grasps its own victim
class MagistersTerraceMidnightGemellusCloneAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new MagistersTerraceMidnightGemellusCloneAI(c); }
    explicit MagistersTerraceMidnightGemellusCloneAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnLoad() override
    {
        setReactState(REACT_PASSIVE);
        _setMeleeDisabled(true);
        castSpellOnSelf(SPELL_GEMELLUS_SYNAPTIC_NEXUS, true);
    }

    // every blow lands on the original
    void DamageTaken(Unit* /*pAttacker*/, uint32_t* damage) override
    {
        if (damage == nullptr)
            return;

        Creature* pOriginal = original();
        if (pOriginal != nullptr && pOriginal->isAlive())
        {
            uint32_t const health = pOriginal->getHealth();
            pOriginal->setHealth(health > *damage ? health - *damage : 1);
        }

        *damage = 0;
    }

    void DoAction(int32_t action) override
    {
        if (action == ACTION_GEMELLUS_ASTRAL_GRASP && isAlive())
        {
            if (Unit* pTarget = selectUnitTarget(FilterArgs(TargetFilter_Player)))
                castSpell(pTarget, SPELL_GEMELLUS_ASTRAL_GRASP, true);
        }
    }

    void AIUpdate(unsigned long time_passed) override
    {
        if (!isAlive())
            return;

        uint32_t const diff = static_cast<uint32_t>(time_passed);
        if (mSyncMs > diff)
        {
            mSyncMs -= diff;
            return;
        }

        mSyncMs = 1000;

        Creature* pOriginal = original();
        if (pOriginal == nullptr || !pOriginal->isAlive())
        {
            despawn(500, 0);
            return;
        }

        uint32_t const healthPct = pOriginal->getHealthPct();
        getCreature()->setHealthPct(healthPct > 0 ? healthPct : 1);

        // drift to another spot every few seconds
        if (++mDriftCount % 6 == 0)
        {
            LocationVector const spot = terraceRingPoint(pOriginal->GetPosition(), 6.0f, 18.0f);
            moveTo(spot.x, spot.y, spot.z, false);
        }
    }

private:
    Creature* original() { return findNearestCreature(BOSS_GEMELLUS, 150.0f); }

    uint32_t mSyncMs = 1000;
    uint32_t mDriftCount = 0;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Degentrius

class MagistersTerraceMidnightDegentriusAI : public MagistersTerraceMidnightBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new MagistersTerraceMidnightDegentriusAI(c); }
    explicit MagistersTerraceMidnightDegentriusAI(Creature* pCreature) : MagistersTerraceMidnightBossAI(pCreature, DATA_DEGENTRIUS) {}

protected:
    void onEngage() override
    {
        scriptEvents.addEvent(EVENT_DEGENTRIUS_HULKING_FRAGMENT, 6000);
        scriptEvents.addEvent(EVENT_DEGENTRIUS_DEVOURING_ENTROPY, 10000);
        scriptEvents.addEvent(EVENT_DEGENTRIUS_VOID_ESSENCE, 15000);
        scriptEvents.addEvent(EVENT_DEGENTRIUS_VOID_TORRENT, 25000);
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_DEGENTRIUS_HULKING_FRAGMENT:
                castSpellOnVictim(SPELL_DEGENTRIUS_HULKING_FRAGMENT);
                scriptEvents.addEvent(EVENT_DEGENTRIUS_HULKING_FRAGMENT, 15000);
                break;
            case EVENT_DEGENTRIUS_DEVOURING_ENTROPY:
                castAt(anyPlayerButTank(), SPELL_DEGENTRIUS_DEVOURING_ENTROPY);
                scriptEvents.addEvent(EVENT_DEGENTRIUS_DEVOURING_ENTROPY, 21000);
                break;
            case EVENT_DEGENTRIUS_VOID_ESSENCE:
                launchEssence();
                scriptEvents.addEvent(EVENT_DEGENTRIUS_VOID_ESSENCE, 24000);
                break;
            case EVENT_DEGENTRIUS_ESSENCE_IMPACT:
                // nobody under the essence, the whole platform pays
                if (!anyPlayerNear(getCreature(), mEssenceSpot, EssenceCatchReach))
                {
                    warn("The Unstable Void Essence lands unchecked!");
                    castSpellAOE(SPELL_DEGENTRIUS_VOID_DESTRUCTION, true);
                }
                break;
            case EVENT_DEGENTRIUS_VOID_TORRENT:
                warn("Degentrius draws in a Void Torrent!");
                castSpellOnSelf(SPELL_DEGENTRIUS_VOID_TORRENT);
                scriptEvents.addEvent(EVENT_DEGENTRIUS_VOID_TORRENT, 38000);
                break;
            default:
                break;
        }
    }

private:
    // the essence comes down where a player stood
    void launchEssence()
    {
        Unit* pTarget = anyPlayer();
        if (pTarget == nullptr)
            return;

        mEssenceSpot = pTarget->GetPosition();
        castSpellOnSelf(SPELL_DEGENTRIUS_UNSTABLE_VOID_ESSENCE);
        scriptEvents.addEvent(EVENT_DEGENTRIUS_ESSENCE_IMPACT, 5000);
    }

    LocationVector mEssenceSpot;
};

void SetupMagistersTerraceMidnight(ScriptMgr* mgr)
{
    mgr->register_instance_script(MAP_MAGISTERS_TERRACE_MIDNIGHT, &MagistersTerraceMidnightInstanceScript::Create);

    mgr->register_creature_script(BOSS_ARCANOTRON_CUSTOS, &MagistersTerraceMidnightCustosAI::Create);
    mgr->register_creature_script(NPC_CUSTOS_ENERGY_ORB, &MagistersTerraceMidnightEnergyOrbAI::Create);

    mgr->register_creature_script(BOSS_SERANEL_SUNLASH, &MagistersTerraceMidnightSeranelAI::Create);
    mgr->register_creature_script(BOSS_SERANEL_SUNLASH_ALT, &MagistersTerraceMidnightSeranelAI::Create);

    mgr->register_creature_script(BOSS_GEMELLUS, &MagistersTerraceMidnightGemellusAI::Create);
    mgr->register_creature_script(NPC_GEMELLUS_CLONE, &MagistersTerraceMidnightGemellusCloneAI::Create);
    mgr->register_creature_script(NPC_GEMELLUS_CLONE_SECOND, &MagistersTerraceMidnightGemellusCloneAI::Create);

    mgr->register_creature_script(BOSS_DEGENTRIUS, &MagistersTerraceMidnightDegentriusAI::Create);
}
