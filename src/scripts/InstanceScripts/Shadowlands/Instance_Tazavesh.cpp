/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "Instance_Tazavesh.hpp"

#include "Objects/GameObject.h"
#include "Objects/Units/Creatures/Creature.h"
#include "Server/Script/CreatureAIScript.hpp"
#include "Server/Script/InstanceScript.hpp"
#include "Server/Script/ScriptMgr.hpp"
#include "Utilities/Random.hpp"

#include <cmath>
#include <list>
#include <map>
#include <vector>

enum TazaveshEvents
{
    EVENT_HOOKTAIL_INFINITE_BREATH = 1,
    EVENT_HOOKTAIL_BREATH_IMPACT,
    EVENT_HOOKTAIL_TIME_BOMB,
    EVENT_HOOKTAIL_TEMPORAL_DETONATION,
    EVENT_HOOKTAIL_CANNON_BARRAGE,
    EVENT_HOOKTAIL_ANCHOR_SHOT,
    EVENT_HOOKTAIL_CORSAIR_BRUTES,

    EVENT_POST_MASTER_MONEY_ORDER,
    EVENT_POST_MASTER_HAZARDOUS_LIQUIDS,
    EVENT_POST_MASTER_LIQUIDS_SPILL,
    EVENT_POST_MASTER_FAN_MAIL,

    EVENT_ZOPHEX_CHARGED_SLASH,
    EVENT_ZOPHEX_IMPOUND_CONTRABAND,
    EVENT_ZOPHEX_ARMED_SECURITY,
    EVENT_ZOPHEX_LIMBS_RETURN,

    EVENT_HYLBRANDE_SHEARING_SWINGS,
    EVENT_HYLBRANDE_TITANIC_CRASH,
    EVENT_HYLBRANDE_PURGED_BY_FIRE,
    EVENT_HYLBRANDE_PURIFYING_BURST,
    EVENT_HYLBRANDE_VAULT_PURIFIERS,
    EVENT_PURIFIER_VALOROUS_BOLT,
    EVENT_PURIFIER_EMPOWERED_DEFENSE,

    EVENT_SOAZMI_PHASE_SLASH,
    EVENT_SOAZMI_SHURI,

    EVENT_ZOGRON_SECURITY_SLAM,
    EVENT_ZOGRON_CROWD_CONTROL,
    EVENT_ZOGRON_MENACING_SHOUT,
    EVENT_ZOGRON_SUPPRESSION_SPARK,

    EVENT_ALCRUUX_GLUTTONY,
    EVENT_ALCRUUX_GRIP_OF_HUNGER,
    EVENT_ALCRUUX_GRAND_CONSUMPTION,
    EVENT_ACHILLITE_FLAGELLATION_PROTOCOL,
    EVENT_ACHILLITE_PURIFICATION_PROTOCOL,
    EVENT_ACHILLITE_VENTING_PROTOCOL,
    EVENT_VENZA_CHAINS_OF_DAMNATION,
    EVENT_VENZA_WHIRLING_ANNIHILATION,

    EVENT_SOLEAH_HYPERLIGHT_SPARK,
    EVENT_SOLEAH_COLLAPSING_STAR,
    EVENT_SOLEAH_ASSASSINS,
    EVENT_SOLEAH_HYPERLIGHT_JOLT,
    EVENT_SOLEAH_HYPERLIGHT_NOVA,
    EVENT_SOLEAH_ENERGY_FRAGMENTATION,
    EVENT_SOLEAH_POWER_FADES,
    EVENT_ASSASSIN_SHURIKEN_BLITZ
};

namespace
{
    // members of the encounters fought as one
    struct TazaveshGroupSlot
    {
        uint32_t dataId;
        uint32_t entry;
    };

    TazaveshGroupSlot const TazaveshGroupSlots[] =
    {
        { DATA_GRAND_MENAGERIE, BOSS_ALCRUUX },
        { DATA_GRAND_MENAGERIE, BOSS_ACHILLITE },
        { DATA_GRAND_MENAGERIE, BOSS_VENZA_GOLDFUSE }
    };

    // room doors, shut while their encounter runs
    struct TazaveshDoor
    {
        uint32_t entry;
        uint32_t dataId;
    };

    TazaveshDoor const TazaveshDoors[] =
    {
        { GO_CUSTOMS_ENFORCEMENT_DOOR,  DATA_ZOPHEX },
        { GO_CUSTOMS_MARKET_DOOR,       DATA_ZOPHEX },
        { GO_MAILROOM_ENTRANCE,         DATA_MAILROOM_MAYHEM },
        { GO_MENAGERIE_DOOR,            DATA_GRAND_MENAGERIE }
    };

    TazaveshDoor const* findTazaveshDoor(uint32_t entry)
    {
        for (TazaveshDoor const& door : TazaveshDoors)
        {
            if (door.entry == entry)
                return &door;
        }

        return nullptr;
    }

    float const TazaveshTwoPi = 6.2831853f;

    // Hooktail
    uint32_t const BrutesPerGrapple = 2;
    float const InfiniteBreathReach = 20.0f;

    // Zo'phex
    uint32_t const ZophexEnergyFull = 100;
    uint32_t const ZophexEnergyPerSecond = 5;
    uint32_t const ArmedSecurityLimbs = 2;
    uint32_t const ArmedSecurityMs = 15000;
    uint32_t const ContainmentCellHoldMs = 15000;

    // Hylbrande
    uint32_t const PurifiersPerCall = 2;

    // So'azmi
    uint32_t const SoazmiEnergyFull = 100;
    uint32_t const SoazmiEnergyPerSecond = 4;
    int32_t const SoazmiDivideMarks[] = { 70, 40 };
    uint32_t const SoazmiDivideMarkCount = 2;

    // Zo'gron
    int32_t const ZogronFinalWarningPct = 50;

    // the next Menagerie member steps in at this health
    int32_t const MenagerieHandOverPct = 50;

    // So'leah
    int32_t const SoleahStageTwoPct = 40;
    uint32_t const AssassinsPerCall = 2;
    uint32_t const PowerOverwhelmingMs = 15000;

    LocationVector tazaveshPointAround(LocationVector const& origin, float minRange, float maxRange)
    {
        float const angle = Util::getRandomFloat(TazaveshTwoPi);
        float const range = Util::getRandomFloat(minRange, maxRange);
        return LocationVector(origin.x + range * std::cos(angle), origin.y + range * std::sin(angle), origin.z, angle);
    }

    // energy filled once per second, true when full
    bool fillEnergy(uint32_t& energy, uint32_t& clock, uint32_t diff, uint32_t perSecond, uint32_t full)
    {
        clock += diff;
        while (clock >= 1000)
        {
            clock -= 1000;
            energy += perSecond;
        }

        return energy >= full;
    }
}

//////////////////////////////////////////////////////////////////////////////////////////
// Instance

class TazaveshInstanceScript : public InstanceScript
{
public:
    explicit TazaveshInstanceScript(WorldMap* pMapMgr) : InstanceScript(pMapMgr)
    {
        setBossNumber(TazaveshEncounterCount);
    }

    static InstanceScript* Create(WorldMap* pMapMgr) { return new TazaveshInstanceScript(pMapMgr); }

    // a fallen Menagerie member must not end the encounter while another stands
    bool setBossState(uint32_t id, EncounterStates state) override
    {
        if (state == EncounterStates::Performed && getBossState(id) == EncounterStates::InProgress && !isGroupDown(id, nullptr))
            return false;

        return InstanceScript::setBossState(id, state);
    }

    void OnCreaturePushToWorld(Creature* pCreature) override
    {
        for (TazaveshGroupSlot const& slot : TazaveshGroupSlots)
        {
            if (slot.entry == pCreature->getEntry())
            {
                mMemberGuids[slot.entry] = pCreature->GetNewGUID();
                return;
            }
        }
    }

    void OnGameObjectPushToWorld(GameObject* pGameObject) override
    {
        if (findTazaveshDoor(pGameObject->getEntry()) == nullptr)
            return;

        mDoorGuids.push_back(pGameObject->GetNewGUID());
        applyDoorState(pGameObject);
    }

    void OnEncounterStateChange(uint32_t id, uint32_t state) override
    {
        for (WoWGuid const& guid : mDoorGuids)
        {
            if (GameObject* pDoor = getGameObjectByGuid(guid))
                applyDoorState(pDoor);
        }

        if (state != EncounterStates::Failed)
            return;

        // a wiped group fights again in full
        for (Creature* pMember : getGroup(id))
        {
            if (!pMember->isAlive())
                pMember->despawn(1000, 5000);
        }
    }

    Creature* getMember(uint32_t entry)
    {
        auto const itr = mMemberGuids.find(entry);
        return itr != mMemberGuids.end() ? getCreatureByGuid(itr->second) : nullptr;
    }

    std::vector<Creature*> getGroup(uint32_t dataId)
    {
        std::vector<Creature*> group;
        for (TazaveshGroupSlot const& slot : TazaveshGroupSlots)
        {
            if (slot.dataId != dataId)
                continue;

            if (Creature* pMember = getMember(slot.entry))
                group.push_back(pMember);
        }

        return group;
    }

    bool isGroupDown(uint32_t dataId, Creature* pExcept)
    {
        for (Creature* pMember : getGroup(dataId))
        {
            if (pMember != pExcept && pMember->isAlive())
                return false;
        }

        return true;
    }

    bool isGroupFighting(uint32_t dataId, Creature* pExcept)
    {
        for (Creature* pMember : getGroup(dataId))
        {
            if (pMember != pExcept && pMember->isAlive() && pMember->isInCombat())
                return true;
        }

        return false;
    }

    // pulling one member pulls the rest, members still held back wait for their turn
    void rallyGroup(uint32_t dataId, Creature* pPuller)
    {
        if (mRallying)
            return;

        mRallying = true;
        for (Creature* pMember : getGroup(dataId))
        {
            if (pMember == pPuller || !pMember->isAlive() || pMember->isInCombat() || pMember->GetScript() == nullptr)
                continue;

            if (pMember->hasUnitFlags(UNIT_FLAG_NON_ATTACKABLE))
                continue;

            pMember->GetScript()->setZoneWideCombat();
        }
        mRallying = false;
    }

    void sendAction(uint32_t entry, int32_t action)
    {
        Creature* pMember = getMember(entry);
        if (pMember != nullptr && pMember->isAlive() && pMember->GetScript() != nullptr)
            pMember->GetScript()->DoAction(action);
    }

private:
    // the encounter decides, the spawn state of a door is not reliable
    void applyDoorState(GameObject* pDoor)
    {
        TazaveshDoor const* pRule = findTazaveshDoor(pDoor->getEntry());
        if (pRule == nullptr)
            return;

        bool const open = getBossState(pRule->dataId) != EncounterStates::InProgress;
        uint8_t const wanted = open ? GO_STATE_OPEN : GO_STATE_CLOSED;
        if (pDoor->getState() != wanted)
            pDoor->setState(wanted);
    }

    std::map<uint32_t, WoWGuid> mMemberGuids;
    std::vector<WoWGuid> mDoorGuids;
    bool mRallying = false;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Shared combat base: event clock while in combat

class TazaveshCombatAI : public CreatureAIScript
{
public:
    explicit TazaveshCombatAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

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

    TazaveshInstanceScript* getTazavesh()
    {
        if (getCreature()->GetMapId() != MAP_TAZAVESH)
            return nullptr;

        return static_cast<TazaveshInstanceScript*>(getInstanceScript());
    }

    Unit* randomPlayer() { return selectUnitTarget(FilterArgs(TargetFilter_Player)); }

    // anyone but the tank, the tank when alone
    Unit* randomNonTankPlayer()
    {
        FilterArgs args(TargetFilter_Player);
        args.addFilter(TargetFilter_NotCurrent);
        if (Unit* pTarget = selectUnitTarget(args))
            return pTarget;

        return randomPlayer();
    }

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
// Encounter base: frame, boss state and group pull

class TazaveshBossAI : public TazaveshCombatAI
{
public:
    TazaveshBossAI(Creature* pCreature, uint32_t dataId) : TazaveshCombatAI(pCreature), mDataId(dataId) {}

    void OnCombatStart(Unit* pTarget) override
    {
        if (TazaveshInstanceScript* pTazavesh = getTazavesh())
        {
            pTazavesh->sendUnitEncounter(EncounterFrameEngage, getCreature());
            pTazavesh->setBossState(mDataId, EncounterStates::InProgress);
            pTazavesh->rallyGroup(mDataId, getCreature());
        }

        TazaveshCombatAI::OnCombatStart(pTarget);
    }

    void OnCombatStop(Unit* pTarget) override
    {
        if (isAlive())
        {
            if (TazaveshInstanceScript* pTazavesh = getTazavesh())
            {
                pTazavesh->sendUnitEncounter(EncounterFrameDisengaged, getCreature());

                // the wipe counts once the last member has left the fight
                if (!pTazavesh->isGroupFighting(mDataId, getCreature()))
                    pTazavesh->setBossState(mDataId, EncounterStates::Failed);
            }
        }

        TazaveshCombatAI::OnCombatStop(pTarget);
    }

    void OnDied(Unit* /*pKiller*/) override
    {
        onDefeat();

        // the instance holds the credit back while a member stands
        if (TazaveshInstanceScript* pTazavesh = getTazavesh())
        {
            pTazavesh->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
            pTazavesh->setBossState(mDataId, EncounterStates::Performed);
        }

        scriptEvents.resetEvents();
        summons.despawnAll();
    }

protected:
    virtual void onDefeat() {}

    uint32_t mDataId;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Timecap'n Hooktail

class TazaveshHooktailAI : public TazaveshBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new TazaveshHooktailAI(c); }
    explicit TazaveshHooktailAI(Creature* pCreature) : TazaveshBossAI(pCreature, DATA_TIMECAPN_HOOKTAIL) {}

protected:
    void onEngage() override
    {
        mBombTargetGuid = 0;
        scriptEvents.addEvent(EVENT_HOOKTAIL_CORSAIR_BRUTES, 6000);
        scriptEvents.addEvent(EVENT_HOOKTAIL_TIME_BOMB, 8000);
        scriptEvents.addEvent(EVENT_HOOKTAIL_INFINITE_BREATH, 12000);
        scriptEvents.addEvent(EVENT_HOOKTAIL_CANNON_BARRAGE, 15000);
        scriptEvents.addEvent(EVENT_HOOKTAIL_ANCHOR_SHOT, 20000);
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_HOOKTAIL_INFINITE_BREATH:
                warn("Timecap'n Hooktail takes a deep Infinite Breath!");
                castSpellOnVictim(SPELL_HOOKTAIL_INFINITE_BREATH);
                scriptEvents.addEvent(EVENT_HOOKTAIL_BREATH_IMPACT, 2000);
                scriptEvents.addEvent(EVENT_HOOKTAIL_INFINITE_BREATH, 22000);
                break;
            case EVENT_HOOKTAIL_BREATH_IMPACT:
                burnCrewInFront();
                break;
            case EVENT_HOOKTAIL_TIME_BOMB:
                if (Unit* pTarget = randomPlayer())
                {
                    castSpell(pTarget, SPELL_HOOKTAIL_TIME_BOMB);
                    mBombTargetGuid = pTarget->getGuid();
                    scriptEvents.addEvent(EVENT_HOOKTAIL_TEMPORAL_DETONATION, 6000);
                }
                scriptEvents.addEvent(EVENT_HOOKTAIL_TIME_BOMB, 26000);
                break;
            case EVENT_HOOKTAIL_TEMPORAL_DETONATION:
                if (Unit* pTarget = getCreature()->getWorldMapUnit(mBombTargetGuid))
                {
                    if (pTarget->isAlive())
                        castSpell(pTarget, SPELL_HOOKTAIL_TEMPORAL_DETONATION, true);
                }
                mBombTargetGuid = 0;
                break;
            case EVENT_HOOKTAIL_CANNON_BARRAGE:
                // the cannoneers fire from the deck
                castAt(randomPlayer(), SPELL_HOOKTAIL_CANNON_BARRAGE, true);
                scriptEvents.addEvent(EVENT_HOOKTAIL_CANNON_BARRAGE, 18000);
                break;
            case EVENT_HOOKTAIL_ANCHOR_SHOT:
                castAt(randomPlayer(), SPELL_HOOKTAIL_ANCHOR_SHOT, true);
                scriptEvents.addEvent(EVENT_HOOKTAIL_ANCHOR_SHOT, 24000);
                break;
            case EVENT_HOOKTAIL_CORSAIR_BRUTES:
                warn("Corsair Brutes grapple down from The Hourglass!");
                for (uint32_t i = 0; i < BrutesPerGrapple; ++i)
                    callAdd(NPC_CORSAIR_BRUTE, tazaveshPointAround(getCreature()->GetPosition(), 8.0f, 14.0f));
                scriptEvents.addEvent(EVENT_HOOKTAIL_CORSAIR_BRUTES, 35000);
                break;
            default:
                break;
        }
    }

private:
    // her own crew does not survive the breath
    void burnCrewInFront()
    {
        std::list<Creature*> brutes;
        GetCreatureListWithEntryInGrid(brutes, NPC_CORSAIR_BRUTE, InfiniteBreathReach);

        for (Creature* pBrute : brutes)
        {
            if (pBrute->isAlive() && getCreature()->isInFront(pBrute))
                getCreature()->dealDamage(pBrute, pBrute->getHealth(), 0);
        }
    }

    uint64_t mBombTargetGuid = 0;
};

class TazaveshCorsairBruteAI : public TazaveshCombatAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new TazaveshCorsairBruteAI(c); }
    explicit TazaveshCorsairBruteAI(Creature* pCreature) : TazaveshCombatAI(pCreature) {}

protected:
    void onEngage() override { castSpellOnSelf(SPELL_CORSAIR_BRUTE_DOUBLE_TIME, true); }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Mailroom Mayhem, the P.O.S.T. Master

class TazaveshPostMasterAI : public TazaveshBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new TazaveshPostMasterAI(c); }
    explicit TazaveshPostMasterAI(Creature* pCreature) : TazaveshBossAI(pCreature, DATA_MAILROOM_MAYHEM) {}

protected:
    void onEngage() override
    {
        scriptEvents.addEvent(EVENT_POST_MASTER_MONEY_ORDER, 9000);
        scriptEvents.addEvent(EVENT_POST_MASTER_HAZARDOUS_LIQUIDS, 14000);
        scriptEvents.addEvent(EVENT_POST_MASTER_FAN_MAIL, 25000);
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_POST_MASTER_MONEY_ORDER:
                castAt(randomPlayer(), SPELL_POST_MASTER_MONEY_ORDER);
                scriptEvents.addEvent(EVENT_POST_MASTER_MONEY_ORDER, 20000);
                break;
            case EVENT_POST_MASTER_HAZARDOUS_LIQUIDS:
                castSpellAOE(SPELL_POST_MASTER_HAZARDOUS_LIQUIDS);
                scriptEvents.addEvent(EVENT_POST_MASTER_LIQUIDS_SPILL, 2000);
                scriptEvents.addEvent(EVENT_POST_MASTER_HAZARDOUS_LIQUIDS, 22000);
                break;
            case EVENT_POST_MASTER_LIQUIDS_SPILL:
                // the unidentified liquid is either residue or a spill
                castAt(randomPlayer(), Util::getRandomUInt(1) == 0 ? SPELL_POST_MASTER_ALCHEMICAL_RESIDUE : SPELL_POST_MASTER_SPILLED_LIQUIDS, true);
                break;
            case EVENT_POST_MASTER_FAN_MAIL:
                warn("The P.O.S.T. Master sends out Fan Mail!");
                castSpellOnSelf(SPELL_POST_MASTER_FAN_MAIL);
                scriptEvents.addEvent(EVENT_POST_MASTER_FAN_MAIL, 30000);
                break;
            default:
                break;
        }
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Zo'phex the Sentinel

class TazaveshZophexAI : public TazaveshBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new TazaveshZophexAI(c); }
    explicit TazaveshZophexAI(Creature* pCreature) : TazaveshBossAI(pCreature, DATA_ZOPHEX) {}

    void DoAction(int32_t action) override
    {
        if (action != ACTION_TAZAVESH_CELL_BROKEN || !isAlive() || !_isInCombat())
            return;

        // a destroyed cell breaks off the Interrogation
        getCreature()->interruptSpell();
        castSpellAOE(SPELL_ZOPHEX_SECURITY_MEASURE, true);
    }

protected:
    void onEngage() override
    {
        mEnergy = 0;
        mEnergyClock = 0;
        castSpellOnSelf(SPELL_ZOPHEX_FULLY_ARMED, true);

        scriptEvents.addEvent(EVENT_ZOPHEX_CHARGED_SLASH, 8000);
        scriptEvents.addEvent(EVENT_ZOPHEX_IMPOUND_CONTRABAND, 14000);
        scriptEvents.addEvent(EVENT_ZOPHEX_ARMED_SECURITY, 20000);
    }

    void onReset() override { _removeAura(SPELL_ZOPHEX_FULLY_ARMED); }

    void onCombatTick(uint32_t diff) override
    {
        if (!fillEnergy(mEnergy, mEnergyClock, diff, ZophexEnergyPerSecond, ZophexEnergyFull) || _isCasting())
            return;

        mEnergy = 0;
        interrogate();
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_ZOPHEX_CHARGED_SLASH:
                castSpellOnVictim(SPELL_ZOPHEX_CHARGED_SLASH);
                scriptEvents.addEvent(EVENT_ZOPHEX_CHARGED_SLASH, 15000);
                break;
            case EVENT_ZOPHEX_IMPOUND_CONTRABAND:
                castAt(randomNonTankPlayer(), SPELL_ZOPHEX_IMPOUND_CONTRABAND);
                scriptEvents.addEvent(EVENT_ZOPHEX_IMPOUND_CONTRABAND, 26000);
                break;
            case EVENT_ZOPHEX_ARMED_SECURITY:
                // his limbs leave him and patrol on their own
                warn("Zo'phex deploys Armed Security!");
                _removeAura(SPELL_ZOPHEX_FULLY_ARMED);
                castSpellOnSelf(SPELL_ZOPHEX_ARMED_SECURITY);
                for (uint32_t i = 0; i < ArmedSecurityLimbs; ++i)
                    callAdd(NPC_ARMED_SECURITY, tazaveshPointAround(getCreature()->GetPosition(), 5.0f, 10.0f));
                scriptEvents.addEvent(EVENT_ZOPHEX_LIMBS_RETURN, ArmedSecurityMs);
                scriptEvents.addEvent(EVENT_ZOPHEX_ARMED_SECURITY, 40000);
                break;
            case EVENT_ZOPHEX_LIMBS_RETURN:
                summons.despawnEntry(NPC_ARMED_SECURITY);
                castSpellOnSelf(SPELL_ZOPHEX_FULLY_ARMED, true);
                break;
            default:
                break;
        }
    }

private:
    // the suspect is locked into a cell at its current spot
    void interrogate()
    {
        Unit* pSuspect = randomNonTankPlayer();
        warn("Zo'phex begins an Interrogation!");

        if (pSuspect != nullptr)
        {
            castSpell(pSuspect, SPELL_ZOPHEX_CONTAINMENT_CELL, true);

            Creature* pCell = summonCreature(NPC_CONTAINMENT_CELL, pSuspect->GetPosition(), CORPSE_DESPAWN);
            if (pCell != nullptr && pCell->GetScript() != nullptr)
                pCell->GetScript()->SetCreatureData64(TAZAVESH_CELL_PRISONER, pSuspect->getGuid());
        }

        castSpellAOE(SPELL_ZOPHEX_INTERROGATION);
    }

    uint32_t mEnergy = 0;
    uint32_t mEnergyClock = 0;
};

// holds its prisoner until destroyed or until the hold runs out
class TazaveshContainmentCellAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new TazaveshContainmentCellAI(c); }
    explicit TazaveshContainmentCellAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnLoad() override
    {
        mHolding = true;
        mHoldLeftMs = ContainmentCellHoldMs;
        setReactState(REACT_PASSIVE);
        setRooted(true);
        _setMeleeDisabled(true);
    }

    void SetCreatureData64(uint32_t type, uint64_t data) override
    {
        if (type == TAZAVESH_CELL_PRISONER)
            mPrisonerGuid = data;
    }

    void AIUpdate(unsigned long time_passed) override
    {
        if (!mHolding || !isAlive())
            return;

        uint32_t const diff = static_cast<uint32_t>(time_passed);
        if (mHoldLeftMs > diff)
        {
            mHoldLeftMs -= diff;
            return;
        }

        mHolding = false;
        releasePrisoner();
        despawn(500, 0);
    }

    void OnDied(Unit* /*pKiller*/) override
    {
        if (!mHolding)
            return;

        mHolding = false;
        releasePrisoner();

        Creature* pZophex = findNearestCreature(BOSS_ZOPHEX, 150.0f);
        if (pZophex != nullptr && pZophex->isAlive() && pZophex->GetScript() != nullptr)
            pZophex->GetScript()->DoAction(ACTION_TAZAVESH_CELL_BROKEN);
    }

private:
    void releasePrisoner()
    {
        if (mPrisonerGuid == 0)
            return;

        if (Unit* pPrisoner = getCreature()->getWorldMapUnit(mPrisonerGuid))
            pPrisoner->removeAllAurasById(SPELL_ZOPHEX_CONTAINMENT_CELL);

        mPrisonerGuid = 0;
    }

    bool mHolding = false;
    uint32_t mHoldLeftMs = 0;
    uint64_t mPrisonerGuid = 0;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Hylbrande

class TazaveshHylbrandeAI : public TazaveshBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new TazaveshHylbrandeAI(c); }
    explicit TazaveshHylbrandeAI(Creature* pCreature) : TazaveshBossAI(pCreature, DATA_HYLBRANDE) {}

protected:
    void onEngage() override
    {
        scriptEvents.addEvent(EVENT_HYLBRANDE_SHEARING_SWINGS, 6000);
        scriptEvents.addEvent(EVENT_HYLBRANDE_PURGED_BY_FIRE, 10000);
        scriptEvents.addEvent(EVENT_HYLBRANDE_TITANIC_CRASH, 14000);
        scriptEvents.addEvent(EVENT_HYLBRANDE_VAULT_PURIFIERS, 15000);
        scriptEvents.addEvent(EVENT_HYLBRANDE_PURIFYING_BURST, 22000);
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_HYLBRANDE_SHEARING_SWINGS:
                castSpellOnVictim(SPELL_HYLBRANDE_SHEARING_SWINGS);
                scriptEvents.addEvent(EVENT_HYLBRANDE_SHEARING_SWINGS, 10000);
                break;
            case EVENT_HYLBRANDE_PURGED_BY_FIRE:
                castAt(randomPlayer(), SPELL_HYLBRANDE_PURGED_BY_FIRE);
                scriptEvents.addEvent(EVENT_HYLBRANDE_PURGED_BY_FIRE, 18000);
                break;
            case EVENT_HYLBRANDE_TITANIC_CRASH:
                castSpellOnVictim(SPELL_HYLBRANDE_TITANIC_CRASH);
                scriptEvents.addEvent(EVENT_HYLBRANDE_TITANIC_CRASH, 20000);
                break;
            case EVENT_HYLBRANDE_VAULT_PURIFIERS:
                warn("Hylbrande calls for Vault Purifiers!");
                for (uint32_t i = 0; i < PurifiersPerCall; ++i)
                    callAdd(NPC_VAULT_PURIFIER, tazaveshPointAround(getCreature()->GetPosition(), 10.0f, 15.0f));
                scriptEvents.addEvent(EVENT_HYLBRANDE_VAULT_PURIFIERS, 45000);
                break;
            case EVENT_HYLBRANDE_PURIFYING_BURST:
                castSpellAOE(SPELL_HYLBRANDE_PURIFYING_BURST);
                scriptEvents.addEvent(EVENT_HYLBRANDE_PURIFYING_BURST, 26000);
                break;
            default:
                break;
        }
    }
};

class TazaveshVaultPurifierAI : public TazaveshCombatAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new TazaveshVaultPurifierAI(c); }
    explicit TazaveshVaultPurifierAI(Creature* pCreature) : TazaveshCombatAI(pCreature) {}

protected:
    void onEngage() override
    {
        scriptEvents.addEvent(EVENT_PURIFIER_VALOROUS_BOLT, 2000);
        scriptEvents.addEvent(EVENT_PURIFIER_EMPOWERED_DEFENSE, 6000);
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_PURIFIER_VALOROUS_BOLT:
                castSpellOnVictim(SPELL_PURIFIER_VALOROUS_BOLT);
                scriptEvents.addEvent(EVENT_PURIFIER_VALOROUS_BOLT, 4000);
                break;
            case EVENT_PURIFIER_EMPOWERED_DEFENSE:
            {
                Creature* pHylbrande = findNearestCreature(BOSS_HYLBRANDE, 60.0f);
                if (pHylbrande != nullptr && pHylbrande->isAlive())
                    castSpell(pHylbrande, SPELL_PURIFIER_EMPOWERED_DEFENSE);
                scriptEvents.addEvent(EVENT_PURIFIER_EMPOWERED_DEFENSE, 15000);
                break;
            }
            default:
                break;
        }
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// So'azmi

class TazaveshSoazmiAI : public TazaveshBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new TazaveshSoazmiAI(c); }
    explicit TazaveshSoazmiAI(Creature* pCreature) : TazaveshBossAI(pCreature, DATA_SOAZMI) {}

protected:
    void onEngage() override
    {
        mEnergy = 0;
        mEnergyClock = 0;
        mDivideIndex = 0;

        scriptEvents.addEvent(EVENT_SOAZMI_PHASE_SLASH, 7000);
        scriptEvents.addEvent(EVENT_SOAZMI_SHURI, 12000);
    }

    void onCombatTick(uint32_t diff) override
    {
        bool const full = fillEnergy(mEnergy, mEnergyClock, diff, SoazmiEnergyPerSecond, SoazmiEnergyFull);
        if (_isCasting())
            return;

        if (mDivideIndex < SoazmiDivideMarkCount && _getHealthPercent() <= SoazmiDivideMarks[mDivideIndex])
        {
            ++mDivideIndex;
            warn("So'azmi Divides the chamber!");
            castSpellOnSelf(SPELL_SOAZMI_DIVIDE);
            return;
        }

        if (!full)
            return;

        mEnergy = 0;
        warn("So'azmi prepares his Double Technique! Interrupt him!");
        castSpellAOE(SPELL_SOAZMI_DOUBLE_TECHNIQUE);
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_SOAZMI_PHASE_SLASH:
                castSpellOnVictim(SPELL_SOAZMI_PHASE_SLASH);
                scriptEvents.addEvent(EVENT_SOAZMI_PHASE_SLASH, 12000);
                break;
            case EVENT_SOAZMI_SHURI:
                castAt(randomPlayer(), SPELL_SOAZMI_SHURI);
                scriptEvents.addEvent(EVENT_SOAZMI_SHURI, 20000);
                break;
            default:
                break;
        }
    }

private:
    uint32_t mEnergy = 0;
    uint32_t mEnergyClock = 0;
    uint32_t mDivideIndex = 0;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Myza's Oasis, Zo'gron closes the show

class TazaveshZogronAI : public TazaveshBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new TazaveshZogronAI(c); }
    explicit TazaveshZogronAI(Creature* pCreature) : TazaveshBossAI(pCreature, DATA_MYZAS_OASIS) {}

protected:
    void onEngage() override
    {
        mWarned = false;
        scriptEvents.addEvent(EVENT_ZOGRON_SECURITY_SLAM, 6000);
        scriptEvents.addEvent(EVENT_ZOGRON_CROWD_CONTROL, 10000);
        scriptEvents.addEvent(EVENT_ZOGRON_MENACING_SHOUT, 15000);
        scriptEvents.addEvent(EVENT_ZOGRON_SUPPRESSION_SPARK, 18000);
    }

    void onReset() override { _removeAura(SPELL_ZOGRON_FINAL_WARNING); }

    void onCombatTick(uint32_t /*diff*/) override
    {
        if (mWarned || _isCasting() || _getHealthPercent() > ZogronFinalWarningPct)
            return;

        mWarned = true;
        warn("Zo'gron issues a Final Warning!");
        castSpellOnSelf(SPELL_ZOGRON_FINAL_WARNING);
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_ZOGRON_SECURITY_SLAM:
                castSpellOnVictim(SPELL_ZOGRON_SECURITY_SLAM);
                scriptEvents.addEvent(EVENT_ZOGRON_SECURITY_SLAM, 12000);
                break;
            case EVENT_ZOGRON_CROWD_CONTROL:
                castSpellOnVictim(SPELL_ZOGRON_CROWD_CONTROL);
                scriptEvents.addEvent(EVENT_ZOGRON_CROWD_CONTROL, 20000);
                break;
            case EVENT_ZOGRON_MENACING_SHOUT:
                castSpellAOE(SPELL_ZOGRON_MENACING_SHOUT);
                scriptEvents.addEvent(EVENT_ZOGRON_MENACING_SHOUT, 24000);
                break;
            case EVENT_ZOGRON_SUPPRESSION_SPARK:
                castSpellOnSelf(SPELL_ZOGRON_SUPPRESSION_SPARK);
                scriptEvents.addEvent(EVENT_ZOGRON_SUPPRESSION_SPARK, 26000);
                break;
            default:
                break;
        }
    }

private:
    bool mWarned = false;
};

//////////////////////////////////////////////////////////////////////////////////////////
// The Grand Menagerie, each member brings the next one in

class TazaveshMenagerieMemberAI : public TazaveshBossAI
{
public:
    TazaveshMenagerieMemberAI(Creature* pCreature, bool startsHeldBack, uint32_t nextEntry, char const* entranceText)
        : TazaveshBossAI(pCreature, DATA_GRAND_MENAGERIE), mStartsHeldBack(startsHeldBack), mNextEntry(nextEntry), mEntranceText(entranceText) {}

    void OnLoad() override
    {
        if (mStartsHeldBack)
            setHeldBack(true);
    }

    void DoAction(int32_t action) override
    {
        if (action != ACTION_TAZAVESH_MENAGERIE_ENTER || !mHeldBack || !isAlive())
            return;

        setHeldBack(false);
        if (mEntranceText != nullptr)
            warn(mEntranceText);

        setZoneWideCombat();
    }

protected:
    void onReset() override
    {
        mNextCalled = false;
        if (mStartsHeldBack)
            setHeldBack(true);
    }

    void onDefeat() override { callNext(); }

    void onCombatTick(uint32_t /*diff*/) override
    {
        if (!mNextCalled && _getHealthPercent() <= MenagerieHandOverPct)
            callNext();
    }

private:
    void callNext()
    {
        if (mNextCalled || mNextEntry == 0)
            return;

        mNextCalled = true;
        if (TazaveshInstanceScript* pTazavesh = getTazavesh())
            pTazavesh->sendAction(mNextEntry, ACTION_TAZAVESH_MENAGERIE_ENTER);
    }

    // waits out of reach until called in
    void setHeldBack(bool heldBack)
    {
        mHeldBack = heldBack;
        if (heldBack)
        {
            getCreature()->addUnitFlags(UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
            setReactState(REACT_PASSIVE);
        }
        else
        {
            getCreature()->removeUnitFlags(UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
            setReactState(REACT_AGGRESSIVE);
        }
    }

    bool mStartsHeldBack;
    uint32_t mNextEntry;
    char const* mEntranceText;
    bool mHeldBack = false;
    bool mNextCalled = false;
};

class TazaveshAlcruuxAI : public TazaveshMenagerieMemberAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new TazaveshAlcruuxAI(c); }
    explicit TazaveshAlcruuxAI(Creature* pCreature) : TazaveshMenagerieMemberAI(pCreature, false, BOSS_ACHILLITE, nullptr) {}

protected:
    void onEngage() override
    {
        scriptEvents.addEvent(EVENT_ALCRUUX_GLUTTONY, 1000);
        scriptEvents.addEvent(EVENT_ALCRUUX_GRIP_OF_HUNGER, 10000);
        scriptEvents.addEvent(EVENT_ALCRUUX_GRAND_CONSUMPTION, 25000);
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_ALCRUUX_GLUTTONY:
                // stays on the closest player for the whole fight
                castSpellOnVictim(SPELL_ALCRUUX_GLUTTONY, true);
                break;
            case EVENT_ALCRUUX_GRIP_OF_HUNGER:
                castSpellAOE(SPELL_ALCRUUX_GRIP_OF_HUNGER);
                scriptEvents.addEvent(EVENT_ALCRUUX_GRIP_OF_HUNGER, 20000);
                break;
            case EVENT_ALCRUUX_GRAND_CONSUMPTION:
                warn("Alcruux begins a Grand Consumption!");
                castSpellOnSelf(SPELL_ALCRUUX_GRAND_CONSUMPTION);
                scriptEvents.addEvent(EVENT_ALCRUUX_GRAND_CONSUMPTION, 30000);
                break;
            default:
                break;
        }
    }
};

class TazaveshAchilliteAI : public TazaveshMenagerieMemberAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new TazaveshAchilliteAI(c); }
    explicit TazaveshAchilliteAI(Creature* pCreature) : TazaveshMenagerieMemberAI(pCreature, true, BOSS_VENZA_GOLDFUSE, "Achillite powers up and enters the fight!") {}

protected:
    void onEngage() override
    {
        scriptEvents.addEvent(EVENT_ACHILLITE_FLAGELLATION_PROTOCOL, 6000);
        scriptEvents.addEvent(EVENT_ACHILLITE_PURIFICATION_PROTOCOL, 12000);
        scriptEvents.addEvent(EVENT_ACHILLITE_VENTING_PROTOCOL, 20000);
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_ACHILLITE_FLAGELLATION_PROTOCOL:
                castSpellOnVictim(SPELL_ACHILLITE_FLAGELLATION_PROTOCOL);
                scriptEvents.addEvent(EVENT_ACHILLITE_FLAGELLATION_PROTOCOL, 18000);
                break;
            case EVENT_ACHILLITE_PURIFICATION_PROTOCOL:
                castAt(randomPlayer(), SPELL_ACHILLITE_PURIFICATION_PROTOCOL);
                scriptEvents.addEvent(EVENT_ACHILLITE_PURIFICATION_PROTOCOL, 22000);
                break;
            case EVENT_ACHILLITE_VENTING_PROTOCOL:
                castSpellOnSelf(SPELL_ACHILLITE_VENTING_PROTOCOL);
                scriptEvents.addEvent(EVENT_ACHILLITE_VENTING_PROTOCOL, 30000);
                break;
            default:
                break;
        }
    }
};

class TazaveshVenzaGoldfuseAI : public TazaveshMenagerieMemberAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new TazaveshVenzaGoldfuseAI(c); }
    explicit TazaveshVenzaGoldfuseAI(Creature* pCreature) : TazaveshMenagerieMemberAI(pCreature, true, 0, "Venza Goldfuse joins the fight!") {}

protected:
    void onEngage() override
    {
        scriptEvents.addEvent(EVENT_VENZA_CHAINS_OF_DAMNATION, 5000);
        scriptEvents.addEvent(EVENT_VENZA_WHIRLING_ANNIHILATION, 12000);
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_VENZA_CHAINS_OF_DAMNATION:
                castAt(randomNonTankPlayer(), SPELL_VENZA_CHAINS_OF_DAMNATION);
                scriptEvents.addEvent(EVENT_VENZA_CHAINS_OF_DAMNATION, 18000);
                break;
            case EVENT_VENZA_WHIRLING_ANNIHILATION:
                warn("Venza Goldfuse unleashes Whirling Annihilation!");
                castSpellAOE(SPELL_VENZA_WHIRLING_ANNIHILATION);
                scriptEvents.addEvent(EVENT_VENZA_WHIRLING_ANNIHILATION, 24000);
                break;
            default:
                break;
        }
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// So'leah

class TazaveshSoleahAI : public TazaveshBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new TazaveshSoleahAI(c); }
    explicit TazaveshSoleahAI(Creature* pCreature) : TazaveshBossAI(pCreature, DATA_SOLEAH) {}

protected:
    void onEngage() override
    {
        mStageTwo = false;
        scriptEvents.addEvent(EVENT_SOLEAH_HYPERLIGHT_SPARK, 6000);
        scriptEvents.addEvent(EVENT_SOLEAH_COLLAPSING_STAR, 14000);
        scriptEvents.addEvent(EVENT_SOLEAH_ASSASSINS, 18000);
    }

    void onReset() override { _removeAura(SPELL_SOLEAH_POWER_OVERWHELMING); }

    void onCombatTick(uint32_t /*diff*/) override
    {
        if (mStageTwo || _isCasting() || _getHealthPercent() > SoleahStageTwoPct)
            return;

        enterStageTwo();
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_SOLEAH_HYPERLIGHT_SPARK:
                castAt(randomPlayer(), SPELL_SOLEAH_HYPERLIGHT_SPARK);
                scriptEvents.addEvent(EVENT_SOLEAH_HYPERLIGHT_SPARK, 9000);
                break;
            case EVENT_SOLEAH_COLLAPSING_STAR:
                castAt(randomPlayer(), SPELL_SOLEAH_COLLAPSING_STAR);
                scriptEvents.addEvent(EVENT_SOLEAH_COLLAPSING_STAR, 22000);
                break;
            case EVENT_SOLEAH_ASSASSINS:
                warn("So'leah calls in her assassins!");
                for (uint32_t i = 0; i < AssassinsPerCall; ++i)
                    callAdd(NPC_SO_CARTEL_ASSASSIN, tazaveshPointAround(getCreature()->GetPosition(), 10.0f, 16.0f));
                scriptEvents.addEvent(EVENT_SOLEAH_ASSASSINS, 40000);
                break;
            case EVENT_SOLEAH_HYPERLIGHT_JOLT:
                castSpellOnVictim(SPELL_SOLEAH_HYPERLIGHT_JOLT);
                scriptEvents.addEvent(EVENT_SOLEAH_HYPERLIGHT_JOLT, 12000);
                break;
            case EVENT_SOLEAH_HYPERLIGHT_NOVA:
                castSpellAOE(SPELL_SOLEAH_HYPERLIGHT_NOVA);
                scriptEvents.addEvent(EVENT_SOLEAH_HYPERLIGHT_NOVA, 20000);
                break;
            case EVENT_SOLEAH_ENERGY_FRAGMENTATION:
                castAt(randomPlayer(), SPELL_SOLEAH_ENERGY_FRAGMENTATION);
                scriptEvents.addEvent(EVENT_SOLEAH_ENERGY_FRAGMENTATION, 26000);
                break;
            case EVENT_SOLEAH_POWER_FADES:
                _removeAura(SPELL_SOLEAH_POWER_OVERWHELMING);
                warn("So'leah's Power Overwhelming falters!");
                break;
            default:
                break;
        }
    }

private:
    // she draws the relics in and wields their power for a while
    void enterStageTwo()
    {
        mStageTwo = true;
        scriptEvents.removeEvent(EVENT_SOLEAH_HYPERLIGHT_SPARK);
        scriptEvents.removeEvent(EVENT_SOLEAH_ASSASSINS);

        warn("So'leah draws her relics into the arena!");
        castSpellOnSelf(SPELL_SOLEAH_POWER_OVERWHELMING, true);
        scriptEvents.addEvent(EVENT_SOLEAH_POWER_FADES, PowerOverwhelmingMs);

        scriptEvents.addEvent(EVENT_SOLEAH_HYPERLIGHT_JOLT, 6000);
        scriptEvents.addEvent(EVENT_SOLEAH_HYPERLIGHT_NOVA, 12000);
        scriptEvents.addEvent(EVENT_SOLEAH_ENERGY_FRAGMENTATION, 18000);
    }

    bool mStageTwo = false;
};

class TazaveshCartelAssassinAI : public TazaveshCombatAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new TazaveshCartelAssassinAI(c); }
    explicit TazaveshCartelAssassinAI(Creature* pCreature) : TazaveshCombatAI(pCreature) {}

protected:
    void onEngage() override { scriptEvents.addEvent(EVENT_ASSASSIN_SHURIKEN_BLITZ, 3000); }

    void onEvent(uint32_t eventId) override
    {
        if (eventId != EVENT_ASSASSIN_SHURIKEN_BLITZ)
            return;

        castSpellAOE(SPELL_ASSASSIN_SHURIKEN_BLITZ);
        scriptEvents.addEvent(EVENT_ASSASSIN_SHURIKEN_BLITZ, 10000);
    }
};

void SetupTazavesh(ScriptMgr* mgr)
{
    mgr->register_instance_script(MAP_TAZAVESH, &TazaveshInstanceScript::Create);

    mgr->register_creature_script(BOSS_TIMECAPN_HOOKTAIL, &TazaveshHooktailAI::Create);
    mgr->register_creature_script(NPC_CORSAIR_BRUTE, &TazaveshCorsairBruteAI::Create);

    mgr->register_creature_script(BOSS_POST_MASTER, &TazaveshPostMasterAI::Create);

    mgr->register_creature_script(BOSS_ZOPHEX, &TazaveshZophexAI::Create);
    mgr->register_creature_script(NPC_CONTAINMENT_CELL, &TazaveshContainmentCellAI::Create);

    mgr->register_creature_script(BOSS_HYLBRANDE, &TazaveshHylbrandeAI::Create);
    mgr->register_creature_script(NPC_VAULT_PURIFIER, &TazaveshVaultPurifierAI::Create);

    mgr->register_creature_script(BOSS_SOAZMI, &TazaveshSoazmiAI::Create);

    mgr->register_creature_script(BOSS_ZOGRON, &TazaveshZogronAI::Create);
    mgr->register_creature_script(BOSS_ZOGRON_ALT, &TazaveshZogronAI::Create);

    mgr->register_creature_script(BOSS_ALCRUUX, &TazaveshAlcruuxAI::Create);
    mgr->register_creature_script(BOSS_ACHILLITE, &TazaveshAchilliteAI::Create);
    mgr->register_creature_script(BOSS_VENZA_GOLDFUSE, &TazaveshVenzaGoldfuseAI::Create);

    mgr->register_creature_script(BOSS_SOLEAH, &TazaveshSoleahAI::Create);
    mgr->register_creature_script(NPC_SO_CARTEL_ASSASSIN, &TazaveshCartelAssassinAI::Create);
}
