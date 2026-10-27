/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "Instance_Operation_Floodgate.hpp"

#include "Objects/GameObject.h"
#include "Objects/Units/Creatures/Creature.h"
#include "Server/Script/CreatureAIScript.hpp"
#include "Server/Script/InstanceScript.hpp"
#include "Server/Script/ScriptMgr.hpp"
#include "Utilities/Random.hpp"

#include <cmath>
#include <cstddef>
#include <list>
#include <map>
#include <utility>
#include <vector>

enum OperationFloodgateEvents
{
    EVENT_KEEZA_QUICK_SHOT = 1,
    EVENT_KEEZA_BIG_BADA_BOOM,
    EVENT_KEEZA_BBBFG,
    EVENT_KEEZA_BBBFG_BLAST,

    EVENT_BRONT_WALLOP,
    EVENT_BRONT_BARRELING_CHARGE,
    EVENT_BRONT_CHARGE_RUSH,

    EVENT_MOMMA_MOBILIZE,
    EVENT_MOMMA_ELECTROCRUSH,
    EVENT_MOMMA_SONIC_BOOM,
    EVENT_MOMMA_SONIC_BOOM_HIT,

    EVENT_MECHADRONE_SHOOT,
    EVENT_MECHADRONE_DOOM_STORM,

    EVENT_SWAMPFACE_SLUDGE_CLAWS,
    EVENT_SWAMPFACE_CLAWS_RAKE,
    EVENT_SWAMPFACE_MUDSLIDE,
    EVENT_SWAMPFACE_MUDSLIDE_HIT,
    EVENT_SWAMPFACE_AWAKEN_THE_SWAMP,

    EVENT_GEEZLE_THUNDER_PUNCH,
    EVENT_GEEZLE_PUNCH_LANDS,
    EVENT_GEEZLE_GIGAZAP,
    EVENT_GEEZLE_GIGAZAP_HIT,
    EVENT_GEEZLE_LEAPING_SPARKS,
    EVENT_GEEZLE_TURBO_CHARGE,
    EVENT_GEEZLE_TURBO_RELEASE,
    EVENT_GEEZLE_DAM
};

namespace
{
    float const TwoPi = 6.28318531f;
    float const FightReach = 60.0f;

    uint32_t const DuoEntries[] = { BOSS_BRONT, BOSS_KEEZA_QUICKFUSE };

    // room doors shut during the fight
    struct FloodgateDoor
    {
        uint32_t entry;
        uint32_t dataId;
    };

    FloodgateDoor const FloodgateDoors[] =
    {
        { GO_DUO_FIRE_BARRIER, DATA_DEMOLITION_DUO }
    };

    FloodgateDoor const* findFloodgateDoor(uint32_t entry)
    {
        for (FloodgateDoor const& door : FloodgateDoors)
        {
            if (door.entry == entry)
                return &door;
        }

        return nullptr;
    }

    // Demolition Duo
    uint32_t const BombFuseMs = 15000;
    float const ChargeTrampleWidth = 3.0f;
    float const ChargeBombWidth = 4.0f;
    float const BombSearchRange = 100.0f;

    // Big M.O.M.M.A.
    uint32_t const SquadCheckMs = 1000;
    uint32_t const SquadRequisitionMs = 20000;

    // Geezle Gigazap
    uint32_t const SparkLifetimeMs = 20000;
    uint32_t const SparkStepMs = 500;
    float const SparkTouchRange = 2.0f;

    LocationVector pointAround(LocationVector const& origin, float minRange, float maxRange)
    {
        float const angle = Util::getRandomFloat(TwoPi);
        float const range = Util::getRandomFloat(minRange, maxRange);
        return LocationVector(origin.x + range * std::cos(angle), origin.y + range * std::sin(angle), origin.z, angle);
    }

    std::vector<Unit*> playersNear(Creature* pCreature, LocationVector const& spot, float range)
    {
        std::vector<Unit*> players;
        for (Object* pObject : pCreature->getInRangePlayersSet())
        {
            Unit* pPlayer = static_cast<Unit*>(pObject);
            if (pPlayer != nullptr && pPlayer->isAlive() && pPlayer->getDistance(spot) <= range)
                players.push_back(pPlayer);
        }

        return players;
    }

    // 2d distance of a point to the segment from to
    float distanceToSegment(LocationVector const& from, LocationVector const& to, LocationVector const& point)
    {
        float const dx = to.x - from.x;
        float const dy = to.y - from.y;
        float const lengthSq = dx * dx + dy * dy;

        float t = 0.0f;
        if (lengthSq > 0.0f)
        {
            t = ((point.x - from.x) * dx + (point.y - from.y) * dy) / lengthSq;
            t = t < 0.0f ? 0.0f : (t > 1.0f ? 1.0f : t);
        }

        float const offX = from.x + t * dx - point.x;
        float const offY = from.y + t * dy - point.y;
        return std::sqrt(offX * offX + offY * offY);
    }
}

//////////////////////////////////////////////////////////////////////////////////////////
// Instance

class OperationFloodgateInstanceScript : public InstanceScript
{
public:
    explicit OperationFloodgateInstanceScript(WorldMap* pMapMgr) : InstanceScript(pMapMgr)
    {
        setBossNumber(OperationFloodgateEncounterCount);
    }

    static InstanceScript* Create(WorldMap* pMapMgr) { return new OperationFloodgateInstanceScript(pMapMgr); }

    // a fallen partner must not end the encounter while the other stands
    bool setBossState(uint32_t id, EncounterStates state) override
    {
        if (state == EncounterStates::Performed && getBossState(id) == EncounterStates::InProgress && !isGroupDown(id, nullptr))
            return false;

        return InstanceScript::setBossState(id, state);
    }

    void OnCreaturePushToWorld(Creature* pCreature) override
    {
        for (uint32_t entry : DuoEntries)
        {
            if (entry == pCreature->getEntry())
            {
                mMemberGuids[entry] = pCreature->GetNewGUID();
                return;
            }
        }
    }

    void OnGameObjectPushToWorld(GameObject* pGameObject) override
    {
        if (findFloodgateDoor(pGameObject->getEntry()) == nullptr)
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

        // a wiped pair fights again in full
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
        if (dataId != DATA_DEMOLITION_DUO)
            return group;

        for (uint32_t entry : DuoEntries)
        {
            if (Creature* pMember = getMember(entry))
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

    // pulling one partner pulls the other
    void rallyGroup(uint32_t dataId, Creature* pPuller)
    {
        if (mRallying)
            return;

        mRallying = true;
        for (Creature* pMember : getGroup(dataId))
        {
            if (pMember == pPuller || !pMember->isAlive() || pMember->isInCombat() || pMember->GetScript() == nullptr)
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
    void applyDoorState(GameObject* pDoor)
    {
        FloodgateDoor const* pRule = findFloodgateDoor(pDoor->getEntry());
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
// Shared combat base: event clock and target helpers

class FloodgateCombatAI : public CreatureAIScript
{
public:
    explicit FloodgateCombatAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

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
    virtual void onReset() {}
    virtual void onCombatTick(uint32_t /*diff*/) {}
    virtual void onEvent(uint32_t /*eventId*/) {}

    OperationFloodgateInstanceScript* getFloodgate()
    {
        if (getCreature()->GetMapId() != MAP_OPERATION_FLOODGATE)
            return nullptr;

        return static_cast<OperationFloodgateInstanceScript*>(getInstanceScript());
    }

    Unit* victim() { return getCreature()->getAIInterface()->getCurrentTarget(); }

    // distinct random players in reach, the tank only when nobody else is left
    std::vector<Unit*> pickPlayers(size_t count, bool skipTank = false)
    {
        Unit* pTank = skipTank ? victim() : nullptr;
        std::vector<Unit*> pool;
        for (Unit* pPlayer : playersNear(getCreature(), getCreature()->GetPosition(), FightReach))
        {
            if (pPlayer != pTank)
                pool.push_back(pPlayer);
        }

        if (pool.empty() && pTank != nullptr && pTank->isAlive())
            pool.push_back(pTank);

        for (size_t i = 0; i < pool.size() && i < count; ++i)
        {
            size_t const pick = i + Util::getRandomUInt(static_cast<uint32_t>(pool.size() - 1 - i));
            std::swap(pool[i], pool[pick]);
        }

        if (pool.size() > count)
            pool.resize(count);

        return pool;
    }

    Unit* randomPlayer(bool skipTank = false)
    {
        std::vector<Unit*> const picked = pickPlayers(1, skipTank);
        return picked.empty() ? nullptr : picked.front();
    }

    Unit* castAtRandomPlayer(uint32_t spellId, bool skipTank = false, bool triggered = false)
    {
        Unit* pTarget = randomPlayer(skipTank);
        if (pTarget != nullptr)
            castSpell(pTarget, spellId, triggered);

        return pTarget;
    }

    std::vector<Creature*> livingSummons(uint32_t entry)
    {
        std::vector<Creature*> found;
        for (uint64_t const guid : summons)
        {
            Creature* pSummon = getCreature()->getWorldMapCreature(guid);
            if (pSummon != nullptr && pSummon->getEntry() == entry && pSummon->isAlive())
                found.push_back(pSummon);
        }

        return found;
    }

    Creature* callAdd(uint32_t entry, LocationVector const& spot)
    {
        Creature* pAdd = summonCreature(entry, spot, CORPSE_DESPAWN);
        if (pAdd != nullptr)
            setZoneWideCombat(pAdd);

        return pAdd;
    }

    Unit* unitByGuid(uint64_t guid)
    {
        Unit* pUnit = guid != 0 ? getCreature()->getWorldMapUnit(guid) : nullptr;
        return pUnit != nullptr && pUnit->isAlive() ? pUnit : nullptr;
    }

    void warn(char const* text) { sendChatMessage(CHAT_MSG_RAID_BOSS_EMOTE, 0, text); }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Encounter base: frame, boss state and group pull

class FloodgateBossAI : public FloodgateCombatAI
{
public:
    FloodgateBossAI(Creature* pCreature, uint32_t dataId) : FloodgateCombatAI(pCreature), mDataId(dataId) {}

    void OnCombatStart(Unit* pTarget) override
    {
        if (InstanceScript* pInstance = getInstanceScript())
        {
            pInstance->sendUnitEncounter(EncounterFrameEngage, getCreature());
            pInstance->setBossState(mDataId, EncounterStates::InProgress);
        }

        if (OperationFloodgateInstanceScript* pFloodgate = getFloodgate())
            pFloodgate->rallyGroup(mDataId, getCreature());

        FloodgateCombatAI::OnCombatStart(pTarget);
    }

    void OnCombatStop(Unit* pTarget) override
    {
        if (isAlive())
        {
            if (InstanceScript* pInstance = getInstanceScript())
            {
                pInstance->sendUnitEncounter(EncounterFrameDisengaged, getCreature());

                // the wipe counts once the last partner has left the fight
                OperationFloodgateInstanceScript* pFloodgate = getFloodgate();
                if (pFloodgate == nullptr || !pFloodgate->isGroupFighting(mDataId, getCreature()))
                    pInstance->setBossState(mDataId, EncounterStates::Failed);
            }
        }

        FloodgateCombatAI::OnCombatStop(pTarget);
    }

    void OnDied(Unit* /*pKiller*/) override
    {
        onDefeat();

        if (InstanceScript* pInstance = getInstanceScript())
        {
            pInstance->sendUnitEncounter(EncounterFrameDisengaged, getCreature());

            OperationFloodgateInstanceScript* pFloodgate = getFloodgate();
            if (pFloodgate == nullptr || pFloodgate->isGroupDown(mDataId, getCreature()))
                pInstance->setBossState(mDataId, EncounterStates::Performed);
        }

        scriptEvents.resetEvents();
        summons.despawnAll();
    }

protected:
    virtual void onDefeat() {}

    uint32_t mDataId;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Demolition Duo, the survivor fights on as a Divided Duo

class FloodgateDuoMemberAI : public FloodgateBossAI
{
public:
    FloodgateDuoMemberAI(Creature* pCreature, uint32_t partnerEntry) : FloodgateBossAI(pCreature, DATA_DEMOLITION_DUO), mPartnerEntry(partnerEntry) {}

    void DoAction(int32_t action) override
    {
        if (action == ACTION_DUO_PARTNER_FELL && isAlive() && _isInCombat())
            castSpellOnSelf(SPELL_DUO_DIVIDED_DUO, true);
    }

protected:
    void onDefeat() override
    {
        if (OperationFloodgateInstanceScript* pFloodgate = getFloodgate())
            pFloodgate->sendAction(mPartnerEntry, ACTION_DUO_PARTNER_FELL);
    }

    uint32_t mPartnerEntry;
};

class FloodgateKeezaAI : public FloodgateDuoMemberAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new FloodgateKeezaAI(c); }
    explicit FloodgateKeezaAI(Creature* pCreature) : FloodgateDuoMemberAI(pCreature, BOSS_BRONT) {}

protected:
    void onEngage() override
    {
        mBbbfgTarget = 0;
        scriptEvents.addEvent(EVENT_KEEZA_QUICK_SHOT, 3000);
        scriptEvents.addEvent(EVENT_KEEZA_BIG_BADA_BOOM, 10000);
        scriptEvents.addEvent(EVENT_KEEZA_BBBFG, 20000);
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_KEEZA_QUICK_SHOT:
                castAtRandomPlayer(SPELL_KEEZA_QUICK_SHOT);
                scriptEvents.addEvent(EVENT_KEEZA_QUICK_SHOT, 7000);
                break;
            case EVENT_KEEZA_BIG_BADA_BOOM:
            {
                warn("Keeza Quickfuse plants Ticking Time Bombs! Lure Bront through them!");
                castSpellAOE(SPELL_KEEZA_BIG_BADA_BOOM, true);
                for (Unit* pTarget : pickPlayers(isHeroic() ? 3 : 2))
                    summonCreature(NPC_KEEZA_TICKING_TIME_BOMB, pointAround(pTarget->GetPosition(), 4.0f, 8.0f), CORPSE_DESPAWN);
                scriptEvents.addEvent(EVENT_KEEZA_BIG_BADA_BOOM, 30000);
                break;
            }
            case EVENT_KEEZA_BBBFG:
                if (Unit* pTarget = castAtRandomPlayer(SPELL_KEEZA_BBBFG))
                {
                    mBbbfgTarget = pTarget->getGuid();
                    scriptEvents.addEvent(EVENT_KEEZA_BBBFG_BLAST, 300);
                }
                scriptEvents.addEvent(EVENT_KEEZA_BBBFG, 30000);
                break;
            case EVENT_KEEZA_BBBFG_BLAST:
                if (Unit* pTarget = unitByGuid(mBbbfgTarget))
                    castSpell(pTarget, SPELL_KEEZA_BBBFG_BLAST, true);
                break;
            default:
                break;
        }
    }

private:
    uint64_t mBbbfgTarget = 0;
};

class FloodgateBrontAI : public FloodgateDuoMemberAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new FloodgateBrontAI(c); }
    explicit FloodgateBrontAI(Creature* pCreature) : FloodgateDuoMemberAI(pCreature, BOSS_KEEZA_QUICKFUSE) {}

protected:
    void onEngage() override
    {
        scriptEvents.addEvent(EVENT_BRONT_WALLOP, 6000);
        scriptEvents.addEvent(EVENT_BRONT_BARRELING_CHARGE, 15000);
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_BRONT_WALLOP:
                castSpellOnVictim(SPELL_BRONT_WALLOP);
                scriptEvents.addEvent(EVENT_BRONT_WALLOP, 14000);
                break;
            case EVENT_BRONT_BARRELING_CHARGE:
                if (Unit* pTarget = randomPlayer(true))
                {
                    warn("Bront lowers his head for a Barreling Charge!");
                    mChargeTo = pTarget->GetPosition();
                    castSpell(pTarget, SPELL_BRONT_BARRELING_CHARGE);
                    scriptEvents.addEvent(EVENT_BRONT_CHARGE_RUSH, 200);
                }
                scriptEvents.addEvent(EVENT_BRONT_BARRELING_CHARGE, 30000);
                break;
            case EVENT_BRONT_CHARGE_RUSH:
                rush();
                break;
            default:
                break;
        }
    }

private:
    // everything on the line is run over, Keeza's bombs included
    void rush()
    {
        LocationVector const from = getCreature()->GetPosition();
        moveCharge(mChargeTo);

        for (Unit* pPlayer : playersNear(getCreature(), from, FightReach))
        {
            if (distanceToSegment(from, mChargeTo, pPlayer->GetPosition()) <= ChargeTrampleWidth)
                castSpell(pPlayer, SPELL_BRONT_BARRELING_CHARGE_HIT, true);
        }

        std::list<Creature*> bombs;
        GetCreatureListWithEntryInGrid(bombs, NPC_KEEZA_TICKING_TIME_BOMB, BombSearchRange);
        for (Creature* pBomb : bombs)
        {
            if (!pBomb->isAlive() || pBomb->GetScript() == nullptr)
                continue;

            if (distanceToSegment(from, mChargeTo, pBomb->GetPosition()) <= ChargeBombWidth)
                pBomb->GetScript()->DoAction(ACTION_KEEZA_BOMB_TRAMPLED);
        }
    }

    LocationVector mChargeTo;
};

// goes off for the whole party unless Bront runs it over first
class FloodgateTickingTimeBombAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new FloodgateTickingTimeBombAI(c); }
    explicit FloodgateTickingTimeBombAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnLoad() override
    {
        setRooted(true);
        setReactState(REACT_PASSIVE);
        _setMeleeDisabled(true);
    }

    void DoAction(int32_t action) override
    {
        if (action != ACTION_KEEZA_BOMB_TRAMPLED || mSpent || !isAlive())
            return;

        mSpent = true;
        getCreature()->castSpellLoc(getCreature()->GetPosition(), SPELL_KEEZA_BOMB_SHRAPNEL, true);
        despawn(300, 0);
    }

    void AIUpdate(unsigned long time_passed) override
    {
        if (!isAlive() || mSpent)
            return;

        uint32_t const diff = static_cast<uint32_t>(time_passed);
        if (mFuseMs > diff)
        {
            mFuseMs -= diff;
            return;
        }

        mSpent = true;
        sendChatMessage(CHAT_MSG_RAID_BOSS_EMOTE, 0, "A Ticking Time Bomb detonates!");
        getCreature()->castSpellLoc(getCreature()->GetPosition(), SPELL_KEEZA_BOMB_DEFLAGRATION, true);
        despawn(500, 0);
    }

private:
    uint32_t mFuseMs = BombFuseMs;
    bool mSpent = false;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Big M.O.M.M.A., Jumpstart once her squadron is gone

class FloodgateBigMommaAI : public FloodgateBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new FloodgateBigMommaAI(c); }
    explicit FloodgateBigMommaAI(Creature* pCreature) : FloodgateBossAI(pCreature, DATA_BIG_MOMMA) {}

protected:
    void onEngage() override
    {
        mSquadUp = false;
        mSquadCheckMs = 0;
        mSonicTarget = 0;

        scriptEvents.addEvent(EVENT_MOMMA_MOBILIZE, 1000);
        scriptEvents.addEvent(EVENT_MOMMA_ELECTROCRUSH, 8000);
        scriptEvents.addEvent(EVENT_MOMMA_SONIC_BOOM, 14000);
    }

    void onCombatTick(uint32_t diff) override
    {
        if (!mSquadUp)
            return;

        mSquadCheckMs += diff;
        if (mSquadCheckMs < SquadCheckMs || _isCasting())
            return;
        mSquadCheckMs = 0;

        if (!livingSummons(NPC_MOMMA_DARKFUSE_MECHADRONE).empty())
            return;

        // the last drone fell, she overloads and calls the next squadron later
        mSquadUp = false;
        _removeAura(SPELL_MOMMA_KILL_O_BLOCK_BARRIER);
        warn("Big M.O.M.M.A. overloads with a Jumpstart!");
        castSpellOnSelf(SPELL_MOMMA_JUMPSTART);
        scriptEvents.addEvent(EVENT_MOMMA_MOBILIZE, SquadRequisitionMs);
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_MOMMA_MOBILIZE:
            {
                warn("Big M.O.M.M.A. mobilizes a squadron of Darkfuse Mechadrones!");
                castSpellAOE(SPELL_MOMMA_MOBILIZE_MECHADRONES, true);
                castSpellOnSelf(SPELL_MOMMA_KILL_O_BLOCK_BARRIER, true);

                uint8_t const drones = isHeroic() ? 4 : 3;
                for (uint8_t i = 0; i < drones; ++i)
                    callAdd(NPC_MOMMA_DARKFUSE_MECHADRONE, pointAround(getCreature()->GetPosition(), 12.0f, 18.0f));

                mSquadUp = true;
                mSquadCheckMs = 0;
                break;
            }
            case EVENT_MOMMA_ELECTROCRUSH:
                castSpellOnVictim(SPELL_MOMMA_ELECTROCRUSH);
                scriptEvents.addEvent(EVENT_MOMMA_ELECTROCRUSH, 18000);
                break;
            case EVENT_MOMMA_SONIC_BOOM:
                if (Unit* pTarget = castAtRandomPlayer(SPELL_MOMMA_SONIC_BOOM))
                {
                    mSonicTarget = pTarget->getGuid();
                    scriptEvents.addEvent(EVENT_MOMMA_SONIC_BOOM_HIT, 300);
                }
                scriptEvents.addEvent(EVENT_MOMMA_SONIC_BOOM, 22000);
                break;
            case EVENT_MOMMA_SONIC_BOOM_HIT:
                if (Unit* pTarget = unitByGuid(mSonicTarget))
                    castSpell(pTarget, SPELL_MOMMA_SONIC_BOOM_HIT, true);
                break;
            default:
                break;
        }
    }

private:
    bool mSquadUp = false;
    uint32_t mSquadCheckMs = 0;
    uint64_t mSonicTarget = 0;
};

class FloodgateDarkfuseMechadroneAI : public FloodgateCombatAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new FloodgateDarkfuseMechadroneAI(c); }
    explicit FloodgateDarkfuseMechadroneAI(Creature* pCreature) : FloodgateCombatAI(pCreature) {}

protected:
    void onEngage() override
    {
        scriptEvents.addEvent(EVENT_MECHADRONE_SHOOT, 2000);
        scriptEvents.addEvent(EVENT_MECHADRONE_DOOM_STORM, 8000);
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_MECHADRONE_SHOOT:
                castSpellOnVictim(SPELL_MECHADRONE_SHOOT);
                scriptEvents.addEvent(EVENT_MECHADRONE_SHOOT, 3000);
                break;
            case EVENT_MECHADRONE_DOOM_STORM:
                if (Unit* pTarget = randomPlayer())
                    getCreature()->castSpellLoc(pTarget->GetPosition(), SPELL_MECHADRONE_DOOM_STORM, false);
                scriptEvents.addEvent(EVENT_MECHADRONE_DOOM_STORM, 16000);
                break;
            default:
                break;
        }
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Swampface

class FloodgateSwampfaceAI : public FloodgateBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new FloodgateSwampfaceAI(c); }
    explicit FloodgateSwampfaceAI(Creature* pCreature) : FloodgateBossAI(pCreature, DATA_SWAMPFACE) {}

protected:
    void onEngage() override
    {
        mMudslideTarget = 0;
        scriptEvents.addEvent(EVENT_SWAMPFACE_SLUDGE_CLAWS, 5000);
        scriptEvents.addEvent(EVENT_SWAMPFACE_MUDSLIDE, 12000);
        scriptEvents.addEvent(EVENT_SWAMPFACE_AWAKEN_THE_SWAMP, 20000);
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_SWAMPFACE_SLUDGE_CLAWS:
                castSpellOnVictim(SPELL_SWAMPFACE_SLUDGE_CLAWS);
                scriptEvents.addEvent(EVENT_SWAMPFACE_CLAWS_RAKE, 300);
                scriptEvents.addEvent(EVENT_SWAMPFACE_SLUDGE_CLAWS, 15000);
                break;
            case EVENT_SWAMPFACE_CLAWS_RAKE:
                castSpellOnVictim(SPELL_SWAMPFACE_SLUDGE_CLAWS_HIT, true);
                castSpellOnVictim(SPELL_SWAMPFACE_SLUDGE_CLAWS_ABSORB, true);
                break;
            case EVENT_SWAMPFACE_MUDSLIDE:
                if (Unit* pTarget = castAtRandomPlayer(SPELL_SWAMPFACE_MUDSLIDE))
                {
                    warn("Swampface brings down a Mudslide!");
                    mMudslideTarget = pTarget->getGuid();
                    scriptEvents.addEvent(EVENT_SWAMPFACE_MUDSLIDE_HIT, 300);
                }
                scriptEvents.addEvent(EVENT_SWAMPFACE_MUDSLIDE, 25000);
                break;
            case EVENT_SWAMPFACE_MUDSLIDE_HIT:
                if (Unit* pTarget = unitByGuid(mMudslideTarget))
                    castSpell(pTarget, SPELL_SWAMPFACE_MUDSLIDE_HIT, true);
                break;
            case EVENT_SWAMPFACE_AWAKEN_THE_SWAMP:
                warn("Swampface awakens the swamp!");
                castSpellOnSelf(SPELL_SWAMPFACE_AWAKEN_THE_SWAMP);
                scriptEvents.addEvent(EVENT_SWAMPFACE_AWAKEN_THE_SWAMP, 30000);
                break;
            default:
                break;
        }
    }

private:
    uint64_t mMudslideTarget = 0;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Geezle Gigazap

class FloodgateGeezleAI : public FloodgateBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new FloodgateGeezleAI(c); }
    explicit FloodgateGeezleAI(Creature* pCreature) : FloodgateBossAI(pCreature, DATA_GEEZLE_GIGAZAP) {}

protected:
    void onEngage() override
    {
        mGigazapTarget = 0;
        scriptEvents.addEvent(EVENT_GEEZLE_THUNDER_PUNCH, 5000);
        scriptEvents.addEvent(EVENT_GEEZLE_GIGAZAP, 8000);
        scriptEvents.addEvent(EVENT_GEEZLE_LEAPING_SPARKS, 12000);
        scriptEvents.addEvent(EVENT_GEEZLE_TURBO_CHARGE, 25000);
        scriptEvents.addEvent(EVENT_GEEZLE_DAM, 35000);
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_GEEZLE_THUNDER_PUNCH:
                castSpellOnVictim(SPELL_GEEZLE_THUNDER_PUNCH);
                scriptEvents.addEvent(EVENT_GEEZLE_PUNCH_LANDS, 300);
                scriptEvents.addEvent(EVENT_GEEZLE_THUNDER_PUNCH, 14000);
                break;
            case EVENT_GEEZLE_PUNCH_LANDS:
                castSpellOnVictim(SPELL_GEEZLE_THUNDER_PUNCH_HIT, true);
                castSpellOnVictim(SPELL_GEEZLE_THUNDER_PUNCH_KNOCK, true);
                break;
            case EVENT_GEEZLE_GIGAZAP:
                if (Unit* pTarget = castAtRandomPlayer(SPELL_GEEZLE_GIGAZAP))
                {
                    mGigazapTarget = pTarget->getGuid();
                    scriptEvents.addEvent(EVENT_GEEZLE_GIGAZAP_HIT, 300);
                }
                scriptEvents.addEvent(EVENT_GEEZLE_GIGAZAP, 16000);
                break;
            case EVENT_GEEZLE_GIGAZAP_HIT:
                if (Unit* pTarget = unitByGuid(mGigazapTarget))
                    castSpell(pTarget, SPELL_GEEZLE_GIGAZAP_HIT, true);
                break;
            case EVENT_GEEZLE_LEAPING_SPARKS:
            {
                warn("Leaping Sparks jump from Geezle Gigazap!");
                castSpellOnSelf(SPELL_GEEZLE_LEAPING_SPARKS, true);
                uint8_t const sparks = isHeroic() ? 4 : 3;
                for (uint8_t i = 0; i < sparks; ++i)
                    summonCreature(NPC_GEEZLE_LEAPING_SPARK, pointAround(getCreature()->GetPosition(), 4.0f, 8.0f), TIMED_DESPAWN, SparkLifetimeMs);
                scriptEvents.addEvent(EVENT_GEEZLE_LEAPING_SPARKS, 25000);
                break;
            }
            case EVENT_GEEZLE_TURBO_CHARGE:
                warn("Geezle Gigazap siphons the generator with a Turbo Charge!");
                castSpellOnSelf(SPELL_GEEZLE_TURBO_CHARGE);
                scriptEvents.addEvent(EVENT_GEEZLE_TURBO_RELEASE, 300);
                scriptEvents.addEvent(EVENT_GEEZLE_TURBO_CHARGE, 45000);
                break;
            case EVENT_GEEZLE_TURBO_RELEASE:
                castSpellAOE(SPELL_GEEZLE_TURBO_CHARGE_BLAST, true);
                for (Unit* pTarget : pickPlayers(2))
                    castSpell(pTarget, SPELL_GEEZLE_TURBO_BOLT, true);
                break;
            case EVENT_GEEZLE_DAM:
                warn("Geezle Gigazap blasts the Dam!");
                castSpellOnSelf(SPELL_GEEZLE_DAM);
                scriptEvents.addEvent(EVENT_GEEZLE_DAM, 45000);
                break;
            default:
                break;
        }
    }

private:
    uint64_t mGigazapTarget = 0;
};

// chases one player and stuns on contact
class FloodgateLeapingSparkAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new FloodgateLeapingSparkAI(c); }
    explicit FloodgateLeapingSparkAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnLoad() override
    {
        setReactState(REACT_PASSIVE);
        _setMeleeDisabled(true);
    }

    void AIUpdate(unsigned long time_passed) override
    {
        if (!isAlive() || mSpent)
            return;

        uint32_t const diff = static_cast<uint32_t>(time_passed);
        if (mStepMs > diff)
        {
            mStepMs -= diff;
            return;
        }
        mStepMs = SparkStepMs;

        Unit* pPrey = mPrey != 0 ? getCreature()->getWorldMapUnit(mPrey) : nullptr;
        if (pPrey == nullptr || !pPrey->isAlive())
        {
            std::vector<Unit*> const players = playersNear(getCreature(), getCreature()->GetPosition(), FightReach);
            if (players.empty())
                return;

            pPrey = players[Util::getRandomUInt(static_cast<uint32_t>(players.size() - 1))];
            mPrey = pPrey->getGuid();
        }

        if (getCreature()->getDistance(pPrey) <= SparkTouchRange)
        {
            mSpent = true;
            castSpell(pPrey, SPELL_GEEZLE_SPARK_JOLT, true);
            despawn(300, 0);
            return;
        }

        moveTo(pPrey->GetPositionX(), pPrey->GetPositionY(), pPrey->GetPositionZ(), true);
    }

private:
    uint64_t mPrey = 0;
    uint32_t mStepMs = 0;
    bool mSpent = false;
};

void SetupOperationFloodgate(ScriptMgr* mgr)
{
    mgr->register_instance_script(MAP_OPERATION_FLOODGATE, &OperationFloodgateInstanceScript::Create);

    mgr->register_creature_script(BOSS_KEEZA_QUICKFUSE, &FloodgateKeezaAI::Create);
    mgr->register_creature_script(BOSS_BRONT, &FloodgateBrontAI::Create);
    mgr->register_creature_script(NPC_KEEZA_TICKING_TIME_BOMB, &FloodgateTickingTimeBombAI::Create);

    mgr->register_creature_script(BOSS_BIG_MOMMA, &FloodgateBigMommaAI::Create);
    mgr->register_creature_script(NPC_MOMMA_DARKFUSE_MECHADRONE, &FloodgateDarkfuseMechadroneAI::Create);

    mgr->register_creature_script(BOSS_SWAMPFACE, &FloodgateSwampfaceAI::Create);

    mgr->register_creature_script(BOSS_GEEZLE_GIGAZAP, &FloodgateGeezleAI::Create);
    mgr->register_creature_script(NPC_GEEZLE_LEAPING_SPARK, &FloodgateLeapingSparkAI::Create);
}
