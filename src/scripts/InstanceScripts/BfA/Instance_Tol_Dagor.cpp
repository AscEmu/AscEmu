/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "Instance_Tol_Dagor.hpp"

#include "Map/Maps/WorldMap.hpp"
#include "Objects/GameObject.h"
#include "Objects/Units/Creatures/Creature.h"
#include "Server/Script/CreatureAIScript.hpp"
#include "Server/Script/InstanceScript.hpp"
#include "Server/Script/ScriptMgr.hpp"
#include "Utilities/Random.hpp"

#include <vector>

enum TolDagorEvents
{
    EVENT_SAND_QUEEN_SAND_TRAP = 1,
    EVENT_SAND_QUEEN_UPHEAVAL,
    EVENT_SAND_QUEEN_UPHEAVAL_TRACK,
    EVENT_SAND_QUEEN_UPHEAVAL_ERUPT,
    EVENT_SAND_QUEEN_SANDSTORM,

    EVENT_JES_CRIPPLING_SHIV,
    EVENT_JES_HOWLING_FEAR,
    EVENT_JES_FLASHING_DAGGERS,

    EVENT_VALYRI_IGNITION,
    EVENT_VALYRI_FUSELIGHTER,
    EVENT_VALYRI_CINDERFLAME,

    EVENT_KORGUS_AZERITE_ROUNDS,
    EVENT_KORGUS_EXPLOSIVE_BURST,
    EVENT_KORGUS_CROSS_IGNITION,
    EVENT_KORGUS_MASSIVE_BLAST,
    EVENT_KORGUS_DEADEYE
};

namespace
{
    // a room door shuts while its boss fights
    struct TolDagorDoor
    {
        uint32_t entry;
        uint32_t dataId;
    };

    TolDagorDoor const TolDagorDoors[] =
    {
        { GO_SAND_QUEEN_SEWER_GRATE,    DATA_SAND_QUEEN },
        { GO_VALYRI_ARMORY_GATE_1,      DATA_KNIGHT_CAPTAIN_VALYRI },
        { GO_VALYRI_ARMORY_GATE_2,      DATA_KNIGHT_CAPTAIN_VALYRI }
    };

    TolDagorDoor const* findTolDagorDoor(uint32_t entry)
    {
        for (TolDagorDoor const& door : TolDagorDoors)
        {
            if (door.entry == entry)
                return &door;
        }

        return nullptr;
    }

    float const SandTrapTriggerRange = 2.5f;
    uint32_t const SandTrapLifetimeMs = 60000;
    uint32_t const MarkGraceMs = 1000;
}

//////////////////////////////////////////////////////////////////////////////////////////
// Instance

class TolDagorInstanceScript : public InstanceScript
{
public:
    explicit TolDagorInstanceScript(WorldMap* pMapMgr) : InstanceScript(pMapMgr)
    {
        setBossNumber(TolDagorEncounterCount);
    }

    static InstanceScript* Create(WorldMap* pMapMgr) { return new TolDagorInstanceScript(pMapMgr); }

    void OnGameObjectPushToWorld(GameObject* pGameObject) override
    {
        if (findTolDagorDoor(pGameObject->getEntry()) == nullptr)
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

private:
    void applyDoorState(GameObject* pDoor)
    {
        TolDagorDoor const* pRule = findTolDagorDoor(pDoor->getEntry());
        if (pRule == nullptr)
            return;

        bool const open = getBossState(pRule->dataId) != EncounterStates::InProgress;
        uint8_t const wanted = open ? GO_STATE_OPEN : GO_STATE_CLOSED;
        if (pDoor->getState() != wanted)
            pDoor->setState(wanted);
    }

    std::vector<WoWGuid> mDoorGuids;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Shared combat base: event clock and marks that strike when they fade

class TolDagorCombatAI : public CreatureAIScript
{
public:
    explicit TolDagorCombatAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

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
        resolveMarks(diff);
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

    void warn(char const* text) { sendChatMessage(CHAT_MSG_RAID_BOSS_EMOTE, 0, text); }

    // the strike follows once the mark aura is gone or the timeout ran out
    void markForStrike(Unit* pTarget, uint32_t markAura, uint32_t strikeSpell, uint32_t timeoutMs)
    {
        if (pTarget == nullptr)
            return;

        castSpell(pTarget, markAura, true);
        mMarks.push_back({ pTarget->GetNewGUID(), markAura, strikeSpell, MarkGraceMs, timeoutMs });
    }

private:
    struct PendingMark
    {
        WoWGuid target;
        uint32_t markAura;
        uint32_t strikeSpell;
        uint32_t graceMs;
        uint32_t timeoutMs;
    };

    static uint32_t countDown(uint32_t value, uint32_t diff) { return value > diff ? value - diff : 0; }

    void resolveMarks(uint32_t diff)
    {
        for (auto itr = mMarks.begin(); itr != mMarks.end();)
        {
            itr->graceMs = countDown(itr->graceMs, diff);
            itr->timeoutMs = countDown(itr->timeoutMs, diff);

            Unit* pTarget = getCreature()->getWorldMap() != nullptr ? getCreature()->getWorldMap()->getUnit(itr->target) : nullptr;
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

            castSpell(pTarget, itr->strikeSpell, true);
            itr = mMarks.erase(itr);
        }
    }

    std::vector<PendingMark> mMarks;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Encounter base: frame and boss state

class TolDagorBossAI : public TolDagorCombatAI
{
public:
    TolDagorBossAI(Creature* pCreature, uint32_t dataId) : TolDagorCombatAI(pCreature), mDataId(dataId) {}

    void OnCombatStart(Unit* pTarget) override
    {
        if (InstanceScript* pInstance = getInstanceScript())
        {
            pInstance->sendUnitEncounter(EncounterFrameEngage, getCreature());
            pInstance->setBossState(mDataId, EncounterStates::InProgress);
        }

        TolDagorCombatAI::OnCombatStart(pTarget);
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

        TolDagorCombatAI::OnCombatStop(pTarget);
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
// The Sand Queen, burrows and erupts beneath a player

class TolDagorSandQueenAI : public TolDagorBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new TolDagorSandQueenAI(c); }
    explicit TolDagorSandQueenAI(Creature* pCreature) : TolDagorBossAI(pCreature, DATA_SAND_QUEEN) {}

protected:
    void onEngage() override
    {
        mBurrowed = false;
        scriptEvents.addEvent(EVENT_SAND_QUEEN_SAND_TRAP, 8500);
        scriptEvents.addEvent(EVENT_SAND_QUEEN_UPHEAVAL, 20500);
        scriptEvents.addEvent(EVENT_SAND_QUEEN_SANDSTORM, 30300);
    }

    void onReset() override { surface(); }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_SAND_QUEEN_SAND_TRAP:
                if (Unit* pTarget = anyPlayer())
                {
                    castSpell(pTarget, SPELL_SAND_QUEEN_SAND_TRAP, true);
                    summonCreature(NPC_BUZZING_DRONE, pTarget->GetPosition(), TIMED_DESPAWN, SandTrapLifetimeMs);
                }
                scriptEvents.addEvent(EVENT_SAND_QUEEN_SAND_TRAP, 14200);
                break;
            case EVENT_SAND_QUEEN_UPHEAVAL:
                burrow();
                scriptEvents.addEvent(EVENT_SAND_QUEEN_UPHEAVAL_TRACK, 2000);
                scriptEvents.addEvent(EVENT_SAND_QUEEN_UPHEAVAL, 43400);
                break;
            case EVENT_SAND_QUEEN_UPHEAVAL_TRACK:
                if (Unit* pTarget = anyPlayer())
                    mEruptionSpot = pTarget->GetPosition();
                else
                    mEruptionSpot = getCreature()->GetPosition();
                scriptEvents.addEvent(EVENT_SAND_QUEEN_UPHEAVAL_ERUPT, 3000);
                break;
            case EVENT_SAND_QUEEN_UPHEAVAL_ERUPT:
                MoveTeleport(mEruptionSpot);
                surface();
                castSpellAOE(SPELL_SAND_QUEEN_UPHEAVAL_ERUPTION, true);
                break;
            case EVENT_SAND_QUEEN_SANDSTORM:
                warn("The Sand Queen whips up a Sandstorm!");
                castSpellOnSelf(SPELL_SAND_QUEEN_SANDSTORM);
                scriptEvents.addEvent(EVENT_SAND_QUEEN_SANDSTORM, 34000);
                break;
            default:
                break;
        }
    }

private:
    void burrow()
    {
        mBurrowed = true;
        warn("The Sand Queen burrows beneath the sand!");
        attackStop();
        setReactState(REACT_PASSIVE);
        setRooted(true);
        castSpellOnSelf(SPELL_SAND_QUEEN_UPHEAVAL_BURROW, true);
        addUnitFlags(UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
    }

    void surface()
    {
        if (!mBurrowed)
            return;

        mBurrowed = false;
        _removeAura(SPELL_SAND_QUEEN_UPHEAVAL_BURROW);
        removeUnitFlags(UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
        setRooted(false);
        setReactState(REACT_AGGRESSIVE);
    }

    bool mBurrowed = false;
    LocationVector mEruptionSpot{ 0.0f, 0.0f, 0.0f, 0.0f };
};

// sand trap: the first player stepping on it is caught
class TolDagorBuzzingDroneAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new TolDagorBuzzingDroneAI(c); }
    explicit TolDagorBuzzingDroneAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnLoad() override
    {
        setReactState(REACT_PASSIVE);
        setRooted(true);
        _setMeleeDisabled(true);
        addUnitFlags(UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
    }

    void AIUpdate(unsigned long time_passed) override
    {
        if (mSprung || !isAlive())
            return;

        uint32_t const diff = static_cast<uint32_t>(time_passed);
        if (mCheckClock > diff)
        {
            mCheckClock -= diff;
            return;
        }

        mCheckClock = 500;
        for (Object* pObject : getCreature()->getInRangePlayersSet())
        {
            Unit* pPlayer = static_cast<Unit*>(pObject);
            if (pPlayer == nullptr || !pPlayer->isAlive() || getCreature()->getDistance(pPlayer) > SandTrapTriggerRange)
                continue;

            mSprung = true;
            pPlayer->castSpell(pPlayer, SPELL_SAND_QUEEN_SAND_TRAP_SPRUNG, true);
            despawn(500, 0);
            return;
        }
    }

private:
    uint32_t mCheckClock = 1000;
    bool mSprung = false;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Jes Howlis

class TolDagorJesHowlisAI : public TolDagorBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new TolDagorJesHowlisAI(c); }
    explicit TolDagorJesHowlisAI(Creature* pCreature) : TolDagorBossAI(pCreature, DATA_JES_HOWLIS) {}

protected:
    void onEngage() override
    {
        scriptEvents.addEvent(EVENT_JES_CRIPPLING_SHIV, 7200);
        scriptEvents.addEvent(EVENT_JES_HOWLING_FEAR, 8500);
        scriptEvents.addEvent(EVENT_JES_FLASHING_DAGGERS, 12100);
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_JES_CRIPPLING_SHIV:
                castSpellOnVictim(SPELL_JES_CRIPPLING_SHIV);
                scriptEvents.addEvent(EVENT_JES_CRIPPLING_SHIV, 16100);
                break;
            case EVENT_JES_HOWLING_FEAR:
                castSpellAOE(SPELL_JES_HOWLING_FEAR);
                scriptEvents.addEvent(EVENT_JES_HOWLING_FEAR, 13400);
                break;
            case EVENT_JES_FLASHING_DAGGERS:
                warn("Jes Howlis readies Flashing Daggers!");
                castSpellAOE(SPELL_JES_FLASHING_DAGGERS);
                scriptEvents.addEvent(EVENT_JES_FLASHING_DAGGERS, 31600);
                break;
            default:
                break;
        }
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Knight Captain Valyri

class TolDagorValyriAI : public TolDagorBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new TolDagorValyriAI(c); }
    explicit TolDagorValyriAI(Creature* pCreature) : TolDagorBossAI(pCreature, DATA_KNIGHT_CAPTAIN_VALYRI) {}

protected:
    void onEngage() override
    {
        scriptEvents.addEvent(EVENT_VALYRI_IGNITION, 6100);
        scriptEvents.addEvent(EVENT_VALYRI_FUSELIGHTER, 14200);
        scriptEvents.addEvent(EVENT_VALYRI_CINDERFLAME, 18200);
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_VALYRI_IGNITION:
                castSpellAOE(SPELL_VALYRI_IGNITION);
                scriptEvents.addEvent(EVENT_VALYRI_IGNITION, 32700);
                break;
            case EVENT_VALYRI_FUSELIGHTER:
                castAt(anyPlayer(), SPELL_VALYRI_FUSELIGHTER);
                scriptEvents.addEvent(EVENT_VALYRI_FUSELIGHTER, 14700);
                break;
            case EVENT_VALYRI_CINDERFLAME:
                warn("Knight Captain Valyri unleashes Cinderflame!");
                castSpellOnVictim(SPELL_VALYRI_CINDERFLAME);
                scriptEvents.addEvent(EVENT_VALYRI_CINDERFLAME, 20500);
                break;
            default:
                break;
        }
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Overseer Korgus

class TolDagorKorgusAI : public TolDagorBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new TolDagorKorgusAI(c); }
    explicit TolDagorKorgusAI(Creature* pCreature) : TolDagorBossAI(pCreature, DATA_OVERSEER_KORGUS) {}

protected:
    void onEngage() override
    {
        scriptEvents.addEvent(EVENT_KORGUS_AZERITE_ROUNDS, 5100);
        scriptEvents.addEvent(EVENT_KORGUS_EXPLOSIVE_BURST, 11500);
        scriptEvents.addEvent(EVENT_KORGUS_CROSS_IGNITION, 16000);
        scriptEvents.addEvent(EVENT_KORGUS_MASSIVE_BLAST, 17000);
        scriptEvents.addEvent(EVENT_KORGUS_DEADEYE, 23300);
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_KORGUS_AZERITE_ROUNDS:
                castSpellOnSelf(Util::getRandomUInt(1) == 0 ? SPELL_KORGUS_ROUNDS_BLAST : SPELL_KORGUS_ROUNDS_INCENDIARY);
                scriptEvents.addEvent(EVENT_KORGUS_AZERITE_ROUNDS, 44800);
                break;
            case EVENT_KORGUS_EXPLOSIVE_BURST:
                markForStrike(anyPlayer(), SPELL_KORGUS_EXPLOSIVE_BURST_MARK, SPELL_KORGUS_EXPLOSIVE_BURST_BLAST, 8000);
                scriptEvents.addEvent(EVENT_KORGUS_EXPLOSIVE_BURST, 44800);
                break;
            case EVENT_KORGUS_CROSS_IGNITION:
                warn("Overseer Korgus fires a Cross Ignition!");
                castSpellOnVictim(SPELL_KORGUS_CROSS_IGNITION);
                scriptEvents.addEvent(EVENT_KORGUS_CROSS_IGNITION, 44800);
                break;
            case EVENT_KORGUS_MASSIVE_BLAST:
                fireHeavyCannon();
                scriptEvents.addEvent(EVENT_KORGUS_MASSIVE_BLAST, 22000);
                break;
            case EVENT_KORGUS_DEADEYE:
                warn("Overseer Korgus takes aim with Deadeye!");
                markForStrike(anyPlayer(), SPELL_KORGUS_DEADEYE_MARK, SPELL_KORGUS_DEADEYE_SHOT, 8000);
                scriptEvents.addEvent(EVENT_KORGUS_DEADEYE, 23000);
                break;
            default:
                break;
        }
    }

private:
    // the yard cannon fires only when one is placed nearby
    void fireHeavyCannon()
    {
        Creature* pCannon = findNearestCreature(NPC_HEAVY_CANNON, 50.0f);
        Unit* pTarget = anyPlayer();
        if (pCannon == nullptr || !pCannon->isAlive() || pTarget == nullptr)
            return;

        pCannon->castSpell(pTarget, SPELL_HEAVY_CANNON_MASSIVE_BLAST, false);
    }
};

void SetupTolDagor(ScriptMgr* mgr)
{
    mgr->register_instance_script(MAP_TOL_DAGOR, &TolDagorInstanceScript::Create);

    mgr->register_creature_script(BOSS_SAND_QUEEN, &TolDagorSandQueenAI::Create);
    mgr->register_creature_script(NPC_BUZZING_DRONE, &TolDagorBuzzingDroneAI::Create);

    mgr->register_creature_script(BOSS_JES_HOWLIS, &TolDagorJesHowlisAI::Create);

    mgr->register_creature_script(BOSS_KNIGHT_CAPTAIN_VALYRI, &TolDagorValyriAI::Create);

    mgr->register_creature_script(BOSS_OVERSEER_KORGUS, &TolDagorKorgusAI::Create);
}
