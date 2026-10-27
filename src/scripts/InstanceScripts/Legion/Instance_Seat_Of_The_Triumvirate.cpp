/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "Instance_Seat_Of_The_Triumvirate.hpp"

#include "Map/Maps/WorldMap.hpp"
#include "Objects/GameObject.h"
#include "Objects/Units/Creatures/Creature.h"
#include "Objects/Units/Players/Player.hpp"
#include "Server/Script/CreatureAIScript.hpp"
#include "Server/Script/InstanceScript.hpp"
#include "Server/Script/ScriptMgr.hpp"
#include "Utilities/Random.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <list>
#include <map>
#include <vector>

enum SeatOfTheTriumvirateEvents
{
    EVENT_SOTT_ZURAAL_NULL_PALM = 1,
    EVENT_SOTT_ZURAAL_DECIMATE,
    EVENT_SOTT_ZURAAL_COALESCED_VOID,

    EVENT_SOTT_SAPRISH_VOID_TRAPS,
    EVENT_SOTT_SAPRISH_OVERLOAD_TRAP,
    EVENT_SOTT_SAPRISH_UMBRAL_FLANKING,
    EVENT_SOTT_SAPRISH_FLANKING_STRIKE,
    EVENT_SOTT_DARKFANG_SHADOW_POUNCE,
    EVENT_SOTT_DARKFANG_RAVAGING_DARKNESS,
    EVENT_SOTT_SHADEWING_SWOOP,
    EVENT_SOTT_SHADEWING_DREAD_SCREECH,

    EVENT_SOTT_NEZHAR_MIND_BLAST,
    EVENT_SOTT_NEZHAR_TENTACLES,
    EVENT_SOTT_NEZHAR_HOWLING_DARK,
    EVENT_SOTT_NEZHAR_ENTROPIC_FORCE,
    EVENT_SOTT_TENTACLE_VOID_LASHING,

    EVENT_SOTT_LURA_CALL_TO_THE_VOID,
    EVENT_SOTT_LURA_BACKLASH_FADES,
    EVENT_SOTT_LURA_VOID_BLAST,
    EVENT_SOTT_LURA_UMBRAL_CADENCE,
    EVENT_SOTT_WARDEN_FRAGMENT,
    EVENT_SOTT_WARDEN_SORROW_SHOCK
};

namespace
{
    std::array<LocationVector, 8> const ZuraalVoidSpots =
    { {
        { 5483.56f, 10765.00f, 18.26f, 1.21f },
        { 5494.43f, 10762.32f, 17.16f, 1.39f },
        { 5502.32f, 10760.47f, 17.20f, 1.34f },
        { 5519.31f, 10756.85f, 17.83f, 1.41f },
        { 5537.28f, 10821.35f, 17.16f, 4.52f },
        { 5522.23f, 10824.61f, 17.16f, 4.53f },
        { 5510.59f, 10827.11f, 17.16f, 4.54f },
        { 5497.86f, 10829.84f, 17.24f, 4.51f }
    } };

    std::array<LocationVector, 2> const LuraRiftSpots =
    { {
        { 6018.16f, 10221.20f, 13.22f, 2.48f },
        { 5978.56f, 10258.90f, 14.13f, 5.58f }
    } };

    std::array<uint32_t, 3> const SaprishPackEntries = { { BOSS_SAPRISH, BOSS_DARKFANG, BOSS_SHADEWING } };

    // shroud waves before L'ura stops channeling for good
    uint32_t const LuraShroudWaves = 2;

    LocationVector offsetFrom(LocationVector const& origin, float minRange, float maxRange)
    {
        float const angle = Util::getRandomFloat(0.0f, 6.283f);
        float const range = Util::getRandomFloat(minRange, maxRange);
        return LocationVector(origin.x + range * std::cos(angle), origin.y + range * std::sin(angle), origin.z, origin.o);
    }
}

class SeatOfTriumvirateInstanceScript : public InstanceScript
{
public:
    explicit SeatOfTriumvirateInstanceScript(WorldMap* pMapMgr) : InstanceScript(pMapMgr)
    {
        setBossNumber(SeatOfTheTriumvirateEncounterCount);
    }

    static InstanceScript* Create(WorldMap* pMapMgr) { return new SeatOfTriumvirateInstanceScript(pMapMgr); }

    // the pack ends only with its last member
    bool setBossState(uint32_t id, EncounterStates state) override
    {
        if (id == DATA_SAPRISH && state == EncounterStates::Performed && getBossState(id) == EncounterStates::InProgress && !isPackDown(nullptr))
            return false;

        return InstanceScript::setBossState(id, state);
    }

    void OnCreaturePushToWorld(Creature* pCreature) override
    {
        if (std::find(SaprishPackEntries.begin(), SaprishPackEntries.end(), pCreature->getEntry()) != SaprishPackEntries.end())
            mPackGuids[pCreature->getEntry()] = pCreature->GetNewGUID();
    }

    void OnGameObjectPushToWorld(GameObject* pGameObject) override
    {
        switch (pGameObject->getEntry())
        {
            case GO_SOTT_VOID_WALL:
            case GO_SOTT_KARABOR_DOOR:
            case GO_SOTT_HALLWAY_DOOR:
                mDoorGuids.push_back(pGameObject->GetNewGUID());
                alignDoor(pGameObject);
                break;
            default:
                break;
        }
    }

    void OnEncounterStateChange(uint32_t id, uint32_t state) override
    {
        for (WoWGuid const& guid : mDoorGuids)
        {
            if (GameObject* pDoor = getGameObjectByGuid(guid))
                alignDoor(pDoor);
        }

        // a wiped pack returns complete
        if (id == DATA_SAPRISH && state == EncounterStates::Failed)
        {
            for (Creature* pMember : getPack())
            {
                if (!pMember->isAlive())
                    pMember->despawn(1000, 5000);
            }
        }
    }

    std::vector<Creature*> getPack()
    {
        std::vector<Creature*> pack;
        for (auto const& entryGuid : mPackGuids)
        {
            if (Creature* pMember = getCreatureByGuid(entryGuid.second))
                pack.push_back(pMember);
        }

        return pack;
    }

    bool isPackDown(Creature* pExcept)
    {
        for (Creature* pMember : getPack())
        {
            if (pMember != pExcept && pMember->isAlive())
                return false;
        }

        return true;
    }

private:
    bool shouldBeOpen(uint32_t entry) const
    {
        switch (entry)
        {
            // the Alleria events are not scripted, the passages follow the bosses
            case GO_SOTT_VOID_WALL:
                return getBossState(DATA_ZURAAL_THE_ASCENDED) == EncounterStates::Performed;
            case GO_SOTT_KARABOR_DOOR:
                return getBossState(DATA_VICEROY_NEZHAR) == EncounterStates::Performed;
            case GO_SOTT_HALLWAY_DOOR:
                return getBossState(DATA_LURA) != EncounterStates::InProgress;
            default:
                return true;
        }
    }

    void alignDoor(GameObject* pDoor)
    {
        pDoor->setState(shouldBeOpen(pDoor->getEntry()) ? GO_STATE_OPEN : GO_STATE_CLOSED);
    }

    std::map<uint32_t, WoWGuid> mPackGuids;
    std::vector<WoWGuid> mDoorGuids;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Shared fighter base: event clock while engaged

class SeatOfTriumvirateFighterAI : public CreatureAIScript
{
public:
    explicit SeatOfTriumvirateFighterAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        scriptEvents.resetEvents();
        onPull();
    }

    void OnCombatStop(Unit* /*pTarget*/) override
    {
        scriptEvents.resetEvents();
        summons.despawnAll();
    }

    void onSummonedCreature(Creature* pSummon) override { summons.summon(pSummon); }

    void AIUpdate(unsigned long time_passed) override
    {
        if (!_isInCombat())
            return;

        uint32_t const diff = static_cast<uint32_t>(time_passed);
        scriptEvents.updateEvents(diff, 0);
        onFightTick(diff);

        if (_isCasting())
            return;

        if (uint32_t const eventId = scriptEvents.getFinishedEvent())
            onEvent(eventId);
    }

protected:
    virtual void onPull() {}
    virtual void onFightTick(uint32_t /*diff*/) {}
    virtual void onEvent(uint32_t /*eventId*/) {}

    SeatOfTriumvirateInstanceScript* getSeat()
    {
        if (getCreature()->GetMapId() != MAP_SEAT_OF_THE_TRIUMVIRATE)
            return nullptr;

        return static_cast<SeatOfTriumvirateInstanceScript*>(getInstanceScript());
    }

    Unit* anyPlayer() { return selectUnitTarget(FilterArgs(TargetFilter_Player)); }

    Unit* anyPlayerButTank()
    {
        if (Unit* pTarget = selectUnitTarget(FilterArgs(TargetFilter(TargetFilter_Player | TargetFilter_NotCurrent))))
            return pTarget;

        return anyPlayer();
    }

    void castAtAnyPlayer(uint32_t spellId)
    {
        if (Unit* pTarget = anyPlayer())
            castSpell(pTarget, spellId);
    }

    Creature* bringAdd(uint32_t entry, LocationVector const& spot)
    {
        Creature* pAdd = summonCreature(entry, spot, CORPSE_DESPAWN);
        if (pAdd != nullptr)
            setZoneWideCombat(pAdd);

        return pAdd;
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Encounter base: frame and boss state

class SeatOfTriumvirateBossAI : public SeatOfTriumvirateFighterAI
{
public:
    SeatOfTriumvirateBossAI(Creature* pCreature, uint32_t encounterId) : SeatOfTriumvirateFighterAI(pCreature), mEncounterId(encounterId) {}

    void OnCombatStart(Unit* pTarget) override
    {
        if (InstanceScript* pInstance = getInstanceScript())
        {
            pInstance->sendUnitEncounter(EncounterFrameEngage, getCreature());
            pInstance->setBossState(mEncounterId, EncounterStates::InProgress);
        }

        SeatOfTriumvirateFighterAI::OnCombatStart(pTarget);
    }

    void OnCombatStop(Unit* pTarget) override
    {
        if (!isAlive())
            return;

        if (InstanceScript* pInstance = getInstanceScript())
        {
            pInstance->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
            pInstance->setBossState(mEncounterId, EncounterStates::Failed);
        }

        onReset();
        SeatOfTriumvirateFighterAI::OnCombatStop(pTarget);
    }

    void OnDied(Unit* pKiller) override
    {
        if (InstanceScript* pInstance = getInstanceScript())
        {
            pInstance->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
            if (isEncounterOver())
                pInstance->setBossState(mEncounterId, EncounterStates::Performed);
        }

        onDefeated(pKiller);
    }

protected:
    virtual void onReset() {}
    virtual void onDefeated(Unit* /*pKiller*/) {}
    virtual bool isEncounterOver() { return true; }

    uint32_t mEncounterId;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Zuraal the Ascended

class SeatOfTriumvirateZuraalAI : public SeatOfTriumvirateBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new SeatOfTriumvirateZuraalAI(c); }
    explicit SeatOfTriumvirateZuraalAI(Creature* pCreature) : SeatOfTriumvirateBossAI(pCreature, DATA_ZURAAL_THE_ASCENDED) {}

protected:
    void onPull() override
    {
        scriptEvents.addEvent(EVENT_SOTT_ZURAAL_NULL_PALM, 11000);
        scriptEvents.addEvent(EVENT_SOTT_ZURAAL_DECIMATE, 18000);
        scriptEvents.addEvent(EVENT_SOTT_ZURAAL_COALESCED_VOID, 20000);
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_SOTT_ZURAAL_NULL_PALM:
                if (Unit* pTarget = selectUnitTarget(FilterArgs(TargetFilter_Player, 0.0f, 39.0f)))
                {
                    sendChatMessage(CHAT_MSG_RAID_BOSS_EMOTE, 0, "Zuraal the Ascended strikes out with a Null Palm!");
                    castSpell(pTarget, SPELL_ZURAAL_NULL_PALM);
                }
                scriptEvents.addEvent(EVENT_SOTT_ZURAAL_NULL_PALM, 55000);
                break;
            case EVENT_SOTT_ZURAAL_DECIMATE:
                castSpellOnVictim(SPELL_ZURAAL_DECIMATE);
                scriptEvents.addEvent(EVENT_SOTT_ZURAAL_DECIMATE, 12000);
                break;
            case EVENT_SOTT_ZURAAL_COALESCED_VOID:
                sendChatMessage(CHAT_MSG_RAID_BOSS_EMOTE, 0, "Coalesced Voids drift towards Zuraal the Ascended!");
                for (LocationVector const& spot : ZuraalVoidSpots)
                    summonCreature(NPC_COALESCED_VOID, spot, CORPSE_DESPAWN);
                scriptEvents.addEvent(EVENT_SOTT_ZURAAL_COALESCED_VOID, 55000);
                break;
            default:
                break;
        }
    }
};

class SeatOfTriumvirateCoalescedVoidAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new SeatOfTriumvirateCoalescedVoidAI(c); }
    explicit SeatOfTriumvirateCoalescedVoidAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnLoad() override
    {
        setReactState(REACT_PASSIVE);
        _setMeleeDisabled(true);
        setSpeedRate(TYPE_WALK, 0.3f, true);
        setSpeedRate(TYPE_RUN, 0.3f, true);
        castSpellOnSelf(SPELL_COALESCED_VOID_SPAWN, true);
    }

    // creeps to Zuraal and empowers him on contact
    void AIUpdate(unsigned long time_passed) override
    {
        if (mSpent)
            return;

        mStepClock += static_cast<uint32_t>(time_passed);
        if (mStepClock < 1000)
            return;

        mStepClock = 0;

        Creature* pZuraal = findNearestCreature(BOSS_ZURAAL_THE_ASCENDED, 60.0f);
        if (pZuraal == nullptr || !pZuraal->isAlive())
        {
            mSpent = true;
            despawn(500, 0);
            return;
        }

        if (getRangeToObject(pZuraal) <= 2.0f)
        {
            mSpent = true;
            castSpellAOE(SPELL_COALESCED_VOID_EXPULSION, true);
            despawn(200, 0);
            return;
        }

        moveTo(pZuraal->GetPositionX(), pZuraal->GetPositionY(), pZuraal->GetPositionZ(), false);
    }

private:
    uint32_t mStepClock = 0;
    bool mSpent = false;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Saprish, Darkfang and Shadewing

class SeatOfTriumvirateSaprishPackAI : public SeatOfTriumvirateBossAI
{
public:
    explicit SeatOfTriumvirateSaprishPackAI(Creature* pCreature) : SeatOfTriumvirateBossAI(pCreature, DATA_SAPRISH) {}

    void OnCombatStart(Unit* pTarget) override
    {
        mWoundClock = 0;
        SeatOfTriumvirateBossAI::OnCombatStart(pTarget);

        // one pulled, all pulled
        if (SeatOfTriumvirateInstanceScript* pSeat = getSeat())
        {
            for (Creature* pMember : pSeat->getPack())
            {
                if (pMember != getCreature() && pMember->isAlive() && !pMember->isInCombat())
                    setZoneWideCombat(pMember);
            }
        }
    }

protected:
    bool isEncounterOver() override
    {
        SeatOfTriumvirateInstanceScript* pSeat = getSeat();
        return pSeat == nullptr || pSeat->isPackDown(getCreature());
    }

    // they share one life, the fallen takes the others along
    void onDefeated(Unit* pKiller) override
    {
        SeatOfTriumvirateInstanceScript* pSeat = getSeat();
        if (pSeat == nullptr)
            return;

        for (Creature* pMember : pSeat->getPack())
        {
            if (pMember == getCreature() || !pMember->isAlive())
                continue;

            Unit* pSource = pKiller != nullptr && pKiller->isAlive() ? pKiller : pMember;
            pSource->dealDamage(pMember, pMember->getHealth(), 0);
        }
    }

    void onFightTick(uint32_t diff) override
    {
        mWoundClock += diff;
        if (mWoundClock < 1000)
            return;

        mWoundClock = 0;
        shareWounds();
    }

    Unit* resolveUnit(WoWGuid const& guid) { return getCreature()->getWorldMap()->getUnit(guid); }

private:
    // every member is cut down to the most wounded one
    void shareWounds()
    {
        SeatOfTriumvirateInstanceScript* pSeat = getSeat();
        if (pSeat == nullptr)
            return;

        std::vector<Creature*> living;
        float lowest = 1.0f;
        for (Creature* pMember : pSeat->getPack())
        {
            if (!pMember->isAlive() || pMember->getMaxHealth() == 0)
                continue;

            living.push_back(pMember);
            lowest = std::min(lowest, static_cast<float>(pMember->getHealth()) / static_cast<float>(pMember->getMaxHealth()));
        }

        for (Creature* pMember : living)
        {
            uint32_t const shared = std::max<uint32_t>(1, static_cast<uint32_t>(static_cast<float>(pMember->getMaxHealth()) * lowest));
            if (pMember->getHealth() > shared)
                pMember->setHealth(shared);
        }
    }

    uint32_t mWoundClock = 0;
};

class SeatOfTriumvirateSaprishAI : public SeatOfTriumvirateSaprishPackAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new SeatOfTriumvirateSaprishAI(c); }
    explicit SeatOfTriumvirateSaprishAI(Creature* pCreature) : SeatOfTriumvirateSaprishPackAI(pCreature) {}

protected:
    void onPull() override
    {
        mFlankTarget = WoWGuid();
        scriptEvents.addEvent(EVENT_SOTT_SAPRISH_VOID_TRAPS, 8000);
        scriptEvents.addEvent(EVENT_SOTT_SAPRISH_OVERLOAD_TRAP, 15000);
        scriptEvents.addEvent(EVENT_SOTT_SAPRISH_UMBRAL_FLANKING, 20000);
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_SOTT_SAPRISH_VOID_TRAPS:
                scatterTraps();
                scriptEvents.addEvent(EVENT_SOTT_SAPRISH_VOID_TRAPS, 16000);
                break;
            case EVENT_SOTT_SAPRISH_OVERLOAD_TRAP:
                sendChatMessage(CHAT_MSG_RAID_BOSS_EMOTE, 0, "Saprish overloads his Void Traps!");
                castSpellOnSelf(SPELL_SAPRISH_OVERLOAD_TRAP, true);
                overloadTraps();
                scriptEvents.addEvent(EVENT_SOTT_SAPRISH_OVERLOAD_TRAP, 20000);
                break;
            case EVENT_SOTT_SAPRISH_UMBRAL_FLANKING:
                if (Unit* pTarget = anyPlayerButTank())
                {
                    sendChatMessage(CHAT_MSG_RAID_BOSS_EMOTE, 0, "Saprish marks his prey for Umbral Flanking!");
                    castSpell(pTarget, SPELL_SAPRISH_UMBRAL_FLANKING_MARK);
                    mFlankTarget = pTarget->GetNewGUID();
                    scriptEvents.addEvent(EVENT_SOTT_SAPRISH_FLANKING_STRIKE, 6000);
                }
                scriptEvents.addEvent(EVENT_SOTT_SAPRISH_UMBRAL_FLANKING, 35000);
                break;
            case EVENT_SOTT_SAPRISH_FLANKING_STRIKE:
                if (Unit* pTarget = resolveUnit(mFlankTarget))
                {
                    if (pTarget->isAlive())
                    {
                        castSpell(pTarget, SPELL_SAPRISH_UMBRAL_FLANKING, true);
                        castSpellOnSelf(SPELL_SAPRISH_HUNTERS_RUSH, true);
                    }
                }
                mFlankTarget = WoWGuid();
                break;
            default:
                break;
        }
    }

private:
    // three to six traps around the current target
    void scatterTraps()
    {
        Unit* pVictim = getCreature()->getAIInterface()->getCurrentTarget();
        if (pVictim == nullptr)
            pVictim = anyPlayer();

        if (pVictim == nullptr)
            return;

        uint32_t const count = Util::getRandomUInt(3, 6);
        for (uint32_t i = 0; i < count; ++i)
            summonCreature(NPC_VOID_TRAP, offsetFrom(pVictim->GetPosition(), 4.0f, 9.0f));
    }

    void overloadTraps()
    {
        std::list<Creature*> traps;
        GetCreatureListWithEntryInGrid(traps, NPC_VOID_TRAP, 100.0f);

        for (Creature* pTrap : traps)
        {
            if (pTrap->isAlive() && pTrap->GetScript() != nullptr)
                pTrap->GetScript()->DoAction(ACTION_SOTT_OVERLOAD_TRAP);
        }
    }

    WoWGuid mFlankTarget;
};

class SeatOfTriumvirateDarkfangAI : public SeatOfTriumvirateSaprishPackAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new SeatOfTriumvirateDarkfangAI(c); }
    explicit SeatOfTriumvirateDarkfangAI(Creature* pCreature) : SeatOfTriumvirateSaprishPackAI(pCreature) {}

protected:
    void onPull() override
    {
        scriptEvents.addEvent(EVENT_SOTT_DARKFANG_SHADOW_POUNCE, 1500);
        scriptEvents.addEvent(EVENT_SOTT_DARKFANG_RAVAGING_DARKNESS, 3000);
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_SOTT_DARKFANG_SHADOW_POUNCE:
                castAtAnyPlayer(SPELL_DARKFANG_SHADOW_POUNCE);
                scriptEvents.addEvent(EVENT_SOTT_DARKFANG_SHADOW_POUNCE, 11000);
                break;
            case EVENT_SOTT_DARKFANG_RAVAGING_DARKNESS:
                castSpellOnSelf(SPELL_DARKFANG_RAVAGING_DARKNESS);
                scriptEvents.addEvent(EVENT_SOTT_DARKFANG_RAVAGING_DARKNESS, 11000);
                break;
            default:
                break;
        }
    }
};

class SeatOfTriumvirateShadewingAI : public SeatOfTriumvirateSaprishPackAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new SeatOfTriumvirateShadewingAI(c); }
    explicit SeatOfTriumvirateShadewingAI(Creature* pCreature) : SeatOfTriumvirateSaprishPackAI(pCreature) {}

protected:
    void onPull() override { scriptEvents.addEvent(EVENT_SOTT_SHADEWING_SWOOP, 3000); }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_SOTT_SHADEWING_SWOOP:
                castAtAnyPlayer(SPELL_SHADEWING_SWOOP);
                scriptEvents.addEvent(EVENT_SOTT_SHADEWING_DREAD_SCREECH, 1500);
                scriptEvents.addEvent(EVENT_SOTT_SHADEWING_SWOOP, Util::getRandomUInt(13000, 15000));
                break;
            case EVENT_SOTT_SHADEWING_DREAD_SCREECH:
                castSpellAOE(SPELL_SHADEWING_DREAD_SCREECH);
                break;
            default:
                break;
        }
    }
};

class SeatOfTriumvirateVoidTrapAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new SeatOfTriumvirateVoidTrapAI(c); }
    explicit SeatOfTriumvirateVoidTrapAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnLoad() override
    {
        setReactState(REACT_PASSIVE);
        setRooted(true);
        _setMeleeDisabled(true);
        addUnitFlags(UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
        castSpellOnSelf(SPELL_VOID_TRAP_AREA, true);
    }

    void DoAction(int32_t action) override
    {
        if (action != ACTION_SOTT_OVERLOAD_TRAP || mSprung)
            return;

        mSprung = true;
        castSpellAOE(SPELL_VOID_TRAP_OVERLOAD, true);
        despawn(1000, 0);
    }

    // springs on the first player stepping in
    void AIUpdate(unsigned long time_passed) override
    {
        if (mSprung)
            return;

        mScanClock += static_cast<uint32_t>(time_passed);
        if (mScanClock < 500)
            return;

        mScanClock = 0;

        Player* pPlayer = getNearestPlayer();
        if (pPlayer == nullptr || !pPlayer->isAlive() || getRangeToObject(pPlayer) > 2.5f)
            return;

        mSprung = true;
        castSpellAOE(SPELL_VOID_TRAP_SNARE, true);
        despawn(1000, 0);
    }

private:
    uint32_t mScanClock = 0;
    bool mSprung = false;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Viceroy Nezhar

class SeatOfTriumvirateNezharAI : public SeatOfTriumvirateBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new SeatOfTriumvirateNezharAI(c); }
    explicit SeatOfTriumvirateNezharAI(Creature* pCreature) : SeatOfTriumvirateBossAI(pCreature, DATA_VICEROY_NEZHAR) {}

protected:
    void onPull() override
    {
        scriptEvents.addEvent(EVENT_SOTT_NEZHAR_MIND_BLAST, 1000);
        scriptEvents.addEvent(EVENT_SOTT_NEZHAR_TENTACLES, 12000);
        scriptEvents.addEvent(EVENT_SOTT_NEZHAR_HOWLING_DARK, 16000);
        scriptEvents.addEvent(EVENT_SOTT_NEZHAR_ENTROPIC_FORCE, 32000);
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_SOTT_NEZHAR_MIND_BLAST:
                castSpellOnVictim(SPELL_NEZHAR_MIND_BLAST);
                scriptEvents.addEvent(EVENT_SOTT_NEZHAR_MIND_BLAST, 3000);
                break;
            case EVENT_SOTT_NEZHAR_TENTACLES:
                raiseTentacles();
                scriptEvents.addEvent(EVENT_SOTT_NEZHAR_TENTACLES, 30000);
                break;
            case EVENT_SOTT_NEZHAR_HOWLING_DARK:
                sendChatMessage(CHAT_MSG_RAID_BOSS_EMOTE, 0, "Viceroy Nezhar begins to cast Howling Dark!");
                castSpellAOE(SPELL_NEZHAR_HOWLING_DARK);
                scriptEvents.addEvent(EVENT_SOTT_NEZHAR_HOWLING_DARK, 33000);
                break;
            case EVENT_SOTT_NEZHAR_ENTROPIC_FORCE:
                sendChatMessage(CHAT_MSG_RAID_BOSS_EMOTE, 0, "Viceroy Nezhar channels Entropic Force!");
                castSpellOnSelf(SPELL_NEZHAR_ENTROPIC_FORCE);
                scriptEvents.addEvent(EVENT_SOTT_NEZHAR_ENTROPIC_FORCE, 61000);
                break;
            default:
                break;
        }
    }

private:
    // three Umbral Tentacles around the platform centre
    void raiseTentacles()
    {
        LocationVector centre = getCreature()->GetPosition();
        if (Creature* pCentre = findNearestCreature(NPC_NEZHAR_CENTER_POINT, 100.0f))
            centre = pCentre->GetPosition();

        for (uint8_t i = 0; i < 3; ++i)
            bringAdd(NPC_UMBRAL_TENTACLE, offsetFrom(centre, 12.0f, 25.0f));
    }
};

class SeatOfTriumvirateUmbralTentacleAI : public SeatOfTriumvirateFighterAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new SeatOfTriumvirateUmbralTentacleAI(c); }
    explicit SeatOfTriumvirateUmbralTentacleAI(Creature* pCreature) : SeatOfTriumvirateFighterAI(pCreature) {}

    void OnLoad() override
    {
        setRooted(true);
        castSpellOnSelf(SPELL_TENTACLE_BASE_VISUAL, true);
    }

protected:
    void onPull() override { scriptEvents.addEvent(EVENT_SOTT_TENTACLE_VOID_LASHING, Util::getRandomUInt(2500, 5000)); }

    void onEvent(uint32_t eventId) override
    {
        if (eventId != EVENT_SOTT_TENTACLE_VOID_LASHING)
            return;

        castSpellOnVictim(SPELL_TENTACLE_VOID_LASHING);
        scriptEvents.addEvent(EVENT_SOTT_TENTACLE_VOID_LASHING, 6000);
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// L'ura

class SeatOfTriumvirateLuraAI : public SeatOfTriumvirateBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new SeatOfTriumvirateLuraAI(c); }
    explicit SeatOfTriumvirateLuraAI(Creature* pCreature) : SeatOfTriumvirateBossAI(pCreature, DATA_LURA) {}

    // untouchable behind the Darkened Shroud
    void DamageTaken(Unit* /*pAttacker*/, uint32_t* damage) override
    {
        if (mShrouded)
            *damage = 0;
    }

    void OnSummonDies(Creature* pSummon, Unit* /*pKiller*/) override
    {
        if (!mShrouded || pSummon->getEntry() != NPC_GREATER_RIFT_WARDEN)
            return;

        if (mWardensLeft > 0 && --mWardensLeft == 0)
            shatterShroud();
    }

protected:
    void onPull() override
    {
        mWave = 0;
        mWardensLeft = 0;
        mShrouded = false;
        setRooted(true);
        scriptEvents.addEvent(EVENT_SOTT_LURA_CALL_TO_THE_VOID, 1500);
        scriptEvents.addEvent(EVENT_SOTT_LURA_VOID_BLAST, Util::getRandomUInt(10000, 18000));
    }

    void onReset() override
    {
        mShrouded = false;
        setRooted(false);
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_SOTT_LURA_CALL_TO_THE_VOID:
                callToTheVoid();
                break;
            case EVENT_SOTT_LURA_BACKLASH_FADES:
                if (mWave < LuraShroudWaves)
                {
                    callToTheVoid();
                }
                else
                {
                    sendChatMessage(CHAT_MSG_RAID_BOSS_EMOTE, 0, "L'ura is unbound and turns her lament upon you!");
                    setRooted(false);
                    castSpellOnSelf(SPELL_LURA_NAARUS_LAMENT, true);
                    scriptEvents.addEvent(EVENT_SOTT_LURA_UMBRAL_CADENCE, 25000);
                }
                break;
            case EVENT_SOTT_LURA_VOID_BLAST:
                if (!hasAura(SPELL_LURA_BACKLASH))
                    castAtAnyPlayer(SPELL_LURA_VOID_BLAST);
                scriptEvents.addEvent(EVENT_SOTT_LURA_VOID_BLAST, Util::getRandomUInt(10000, 18000));
                break;
            case EVENT_SOTT_LURA_UMBRAL_CADENCE:
                castSpellAOE(SPELL_LURA_UMBRAL_CADENCE);
                scriptEvents.addEvent(EVENT_SOTT_LURA_UMBRAL_CADENCE, Util::getRandomUInt(13000, 15000));
                break;
            default:
                break;
        }
    }

private:
    // each wave opens one more rift, its wardens keep the shroud up
    void callToTheVoid()
    {
        ++mWave;
        sendChatMessage(CHAT_MSG_RAID_BOSS_EMOTE, 0, "L'ura sounds a Call to the Void!");
        castSpellAOE(SPELL_LURA_CALL_TO_THE_VOID, true);
        castSpellOnSelf(SPELL_LURA_DARKENED_SHROUD, true);
        castSpellOnSelf(SPELL_LURA_NAARUS_LAMENT, true);
        mShrouded = true;

        mWardensLeft = 0;
        for (uint32_t i = 0; i < mWave && i < LuraRiftSpots.size(); ++i)
        {
            if (bringAdd(NPC_GREATER_RIFT_WARDEN, LuraRiftSpots[i]) != nullptr)
                ++mWardensLeft;
        }

        if (mWardensLeft == 0)
            shatterShroud();
    }

    void shatterShroud()
    {
        mShrouded = false;
        _removeAura(SPELL_LURA_DARKENED_SHROUD);
        _removeAura(SPELL_LURA_NAARUS_LAMENT);
        sendChatMessage(CHAT_MSG_RAID_BOSS_EMOTE, 0, "The Darkened Shroud collapses and L'ura reels from the Backlash!");
        castSpellOnSelf(SPELL_LURA_BACKLASH, true);
        scriptEvents.addEvent(EVENT_SOTT_LURA_BACKLASH_FADES, 12000);
    }

    uint32_t mWave = 0;
    uint32_t mWardensLeft = 0;
    bool mShrouded = false;
};

class SeatOfTriumvirateRiftWardenAI : public SeatOfTriumvirateFighterAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new SeatOfTriumvirateRiftWardenAI(c); }
    explicit SeatOfTriumvirateRiftWardenAI(Creature* pCreature) : SeatOfTriumvirateFighterAI(pCreature) {}

protected:
    void onPull() override
    {
        scriptEvents.addEvent(EVENT_SOTT_WARDEN_SORROW_SHOCK, Util::getRandomUInt(5000, 8000));
        scriptEvents.addEvent(EVENT_SOTT_WARDEN_FRAGMENT, Util::getRandomUInt(8000, 10000));
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_SOTT_WARDEN_SORROW_SHOCK:
                castSpellOnVictim(SPELL_RIFT_WARDEN_SORROW_SHOCK);
                scriptEvents.addEvent(EVENT_SOTT_WARDEN_SORROW_SHOCK, Util::getRandomUInt(10000, 12000));
                break;
            case EVENT_SOTT_WARDEN_FRAGMENT:
                castSpellAOE(SPELL_RIFT_WARDEN_FRAGMENT);
                scriptEvents.addEvent(EVENT_SOTT_WARDEN_FRAGMENT, Util::getRandomUInt(18000, 22000));
                break;
            default:
                break;
        }
    }
};

void SetupSeatOfTheTriumvirate(ScriptMgr* mgr)
{
    mgr->register_instance_script(MAP_SEAT_OF_THE_TRIUMVIRATE, &SeatOfTriumvirateInstanceScript::Create);

    mgr->register_creature_script(BOSS_ZURAAL_THE_ASCENDED, &SeatOfTriumvirateZuraalAI::Create);
    mgr->register_creature_script(NPC_COALESCED_VOID, &SeatOfTriumvirateCoalescedVoidAI::Create);

    mgr->register_creature_script(BOSS_SAPRISH, &SeatOfTriumvirateSaprishAI::Create);
    mgr->register_creature_script(BOSS_DARKFANG, &SeatOfTriumvirateDarkfangAI::Create);
    mgr->register_creature_script(BOSS_SHADEWING, &SeatOfTriumvirateShadewingAI::Create);
    mgr->register_creature_script(NPC_VOID_TRAP, &SeatOfTriumvirateVoidTrapAI::Create);

    mgr->register_creature_script(BOSS_VICEROY_NEZHAR, &SeatOfTriumvirateNezharAI::Create);
    mgr->register_creature_script(NPC_UMBRAL_TENTACLE, &SeatOfTriumvirateUmbralTentacleAI::Create);

    mgr->register_creature_script(BOSS_LURA, &SeatOfTriumvirateLuraAI::Create);
    mgr->register_creature_script(NPC_GREATER_RIFT_WARDEN, &SeatOfTriumvirateRiftWardenAI::Create);
}
