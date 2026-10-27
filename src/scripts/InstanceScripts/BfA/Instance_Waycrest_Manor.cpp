/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "Instance_Waycrest_Manor.hpp"

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

enum WaycrestManorEvents
{
    EVENT_SISTER_BOLT = 1,
    EVENT_SISTER_FOCUSED_BOLT,
    EVENT_SISTER_SIGNATURE,
    EVENT_SISTER_CLAIM_IRIS,

    EVENT_GOLIATH_CRUSH,
    EVENT_GOLIATH_SOUL_THORNS,

    EVENT_RAAL_TENDERIZE,
    EVENT_RAAL_ROTTEN_EXPULSION,
    EVENT_RAAL_CALL_SERVANT,

    EVENT_LORD_WASTING_STRIKE,
    EVENT_LORD_VIRULENT_PATHOGEN,

    EVENT_LADY_WRACKING_CHORD,
    EVENT_LADY_DISCORDANT_CADENZA,
    EVENT_LADY_TRANSFER_BEGIN,
    EVENT_LADY_TRANSFER_DONE,
    EVENT_LADY_DESCEND,

    EVENT_GORAK_DARKENED_LIGHTNING,
    EVENT_GORAK_SUMMON_SLAVER,
    EVENT_GORAK_DREAD_ESSENCE,

    EVENT_SLAVER_DARK_LEAP,
    EVENT_SLAVER_DREAD_BOLT,
    EVENT_SLAVER_DEATH_LENS
};

namespace
{
    // members of the encounters fought as one
    struct WaycrestGroupSlot
    {
        uint32_t dataId;
        uint32_t entry;
    };

    WaycrestGroupSlot const WaycrestGroupSlots[] =
    {
        { DATA_HEARTSBANE_TRIAD,        BOSS_SISTER_SOLENA },
        { DATA_HEARTSBANE_TRIAD,        BOSS_SISTER_MALADY },
        { DATA_HEARTSBANE_TRIAD,        BOSS_SISTER_BRIAR },
        { DATA_LORD_AND_LADY_WAYCREST,  BOSS_LORD_WAYCREST },
        { DATA_LORD_AND_LADY_WAYCREST,  BOSS_LADY_WAYCREST }
    };

    // the iris goes round in this order
    uint32_t const IrisRotation[] = { BOSS_SISTER_SOLENA, BOSS_SISTER_MALADY, BOSS_SISTER_BRIAR };
    uint32_t const IrisRotationSize = 3;

    // room doors shut during the fight, passages open after the kill
    struct WaycrestDoor
    {
        uint32_t entry;
        uint32_t dataId;
        bool passage;
    };

    WaycrestDoor const WaycrestDoors[] =
    {
        { GO_HEARTSBANE_TRIAD_DOOR,     DATA_HEARTSBANE_TRIAD,          false },
        { GO_WAYCREST_ORGAN_BARRIER,    DATA_LORD_AND_LADY_WAYCREST,    true }
    };

    WaycrestDoor const* findWaycrestDoor(uint32_t entry)
    {
        for (WaycrestDoor const& door : WaycrestDoors)
        {
            if (door.entry == entry)
                return &door;
        }

        return nullptr;
    }

    // what each sister casts, with and without the Focusing Iris
    struct WaycrestSisterKit
    {
        uint32_t bolt;
        uint32_t focusedBolt;
        uint32_t focusedBoltMs;
        uint32_t signature;
        uint32_t signatureMs;
        bool signatureAtPlayer;
        uint32_t ward;
    };

    WaycrestSisterKit const BriarKit = { SPELL_BRIAR_BRAMBLE_BOLT, SPELL_BRIAR_BRAMBLE_BOLT_FOCUSED, 7300, SPELL_BRIAR_JAGGED_NETTLES, 14600, true, SPELL_BRIAR_IRONBARK_SHIELD };
    WaycrestSisterKit const MaladyKit = { SPELL_MALADY_RUINOUS_BOLT, SPELL_MALADY_RUINOUS_BOLT_FOCUSED, 7300, SPELL_MALADY_UNSTABLE_RUNIC_MARK, 12100, false, SPELL_MALADY_RUNIC_WARD };
    WaycrestSisterKit const SolenaKit = { SPELL_SOLENA_SOUL_BOLT, SPELL_SOLENA_SOUL_BOLT_FOCUSED, 4900, 0, 0, false, SPELL_SOLENA_SOUL_ARMOR };

    uint32_t const DireRitualFull = 100;
    uint32_t const DireRitualGainPerSecond = 2;

    LocationVector const LordArenaSpot(-549.78f, -260.09f, 185.27f);
    uint32_t const LordTransferCount = 3;
    uint32_t const LordTransferThresholdPct = 30;
    uint32_t const LadyTransferCostPct = 30;

    // servants shuffle in from the kitchen doors
    LocationVector const RaalServantDoors[] =
    {
        { -461.0f, -330.0f, 237.0f, 3.1f },
        { -481.0f, -308.0f, 237.0f, 4.7f },
        { -525.0f, -330.0f, 237.0f, 0.0f }
    };
    float const RaalFeedingReach = 8.0f;
    float const RaalMeleeReach = 7.0f;
    uint32_t const RaalMeleeCheckMs = 2600;

    float const SoulThornsReach = 5.0f;

    // living players in range of a creature
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

    LocationVector pointNear(LocationVector const& origin, float minRange, float maxRange)
    {
        float const angle = Util::getRandomFloat(6.2831853f);
        float const range = Util::getRandomFloat(minRange, maxRange);
        return LocationVector(origin.x + range * std::cos(angle), origin.y + range * std::sin(angle), origin.z, angle);
    }

    uint32_t percentOf(uint32_t value, uint32_t pct)
    {
        return static_cast<uint32_t>(static_cast<uint64_t>(value) * pct / 100);
    }
}

//////////////////////////////////////////////////////////////////////////////////////////
// Instance

class WaycrestManorInstanceScript : public InstanceScript
{
public:
    explicit WaycrestManorInstanceScript(WorldMap* pMapMgr) : InstanceScript(pMapMgr)
    {
        setBossNumber(WaycrestManorEncounterCount);
    }

    static InstanceScript* Create(WorldMap* pMapMgr) { return new WaycrestManorInstanceScript(pMapMgr); }

    // a credited member falling before its partners must not end the encounter
    bool setBossState(uint32_t id, EncounterStates state) override
    {
        if (state == EncounterStates::Performed && getBossState(id) == EncounterStates::InProgress && !isGroupDown(id, nullptr))
            return false;

        return InstanceScript::setBossState(id, state);
    }

    void OnCreaturePushToWorld(Creature* pCreature) override
    {
        for (WaycrestGroupSlot const& slot : WaycrestGroupSlots)
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
        if (findWaycrestDoor(pGameObject->getEntry()) == nullptr)
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
        for (WaycrestGroupSlot const& slot : WaycrestGroupSlots)
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

    // pulling one member pulls the rest
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

    // the next living sister in line claims the iris
    void passIris(uint32_t fromEntry)
    {
        uint32_t from = 0;
        for (uint32_t i = 0; i < IrisRotationSize; ++i)
        {
            if (IrisRotation[i] == fromEntry)
                from = i;
        }

        for (uint32_t step = 1; step < IrisRotationSize; ++step)
        {
            Creature* pSister = getMember(IrisRotation[(from + step) % IrisRotationSize]);
            if (pSister == nullptr || !pSister->isAlive() || pSister->GetScript() == nullptr)
                continue;

            pSister->GetScript()->DoAction(ACTION_TRIAD_CLAIM_IRIS);
            return;
        }
    }

private:
    void applyDoorState(GameObject* pDoor)
    {
        WaycrestDoor const* pRule = findWaycrestDoor(pDoor->getEntry());
        if (pRule == nullptr)
            return;

        EncounterStates const state = getBossState(pRule->dataId);
        bool const open = pRule->passage ? state == EncounterStates::Performed : state != EncounterStates::InProgress;
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

class WaycrestManorCombatAI : public CreatureAIScript
{
public:
    explicit WaycrestManorCombatAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

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

    WaycrestManorInstanceScript* getManor()
    {
        if (getCreature()->GetMapId() != MAP_WAYCREST_MANOR)
            return nullptr;

        return static_cast<WaycrestManorInstanceScript*>(getInstanceScript());
    }

    Unit* anyPlayer() { return selectUnitTarget(FilterArgs(TargetFilter_Player)); }

    // anyone but the tank, the tank when alone
    Unit* offTankPlayer()
    {
        FilterArgs args(TargetFilter_Player);
        args.addFilter(TargetFilter_NotCurrent);
        if (Unit* pTarget = selectUnitTarget(args))
            return pTarget;

        return anyPlayer();
    }

    Unit* farthestPlayer()
    {
        Unit* pFarthest = nullptr;
        float farthestRange = -1.0f;
        for (Unit* pPlayer : playersAround(getCreature(), 100.0f))
        {
            float const range = getCreature()->getDistance(pPlayer);
            if (range > farthestRange)
            {
                farthestRange = range;
                pFarthest = pPlayer;
            }
        }

        return pFarthest;
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

class WaycrestManorBossAI : public WaycrestManorCombatAI
{
public:
    WaycrestManorBossAI(Creature* pCreature, uint32_t dataId) : WaycrestManorCombatAI(pCreature), mDataId(dataId) {}

    void OnCombatStart(Unit* pTarget) override
    {
        if (InstanceScript* pInstance = getInstanceScript())
        {
            pInstance->sendUnitEncounter(EncounterFrameEngage, getCreature());
            pInstance->setBossState(mDataId, EncounterStates::InProgress);
        }

        if (WaycrestManorInstanceScript* pManor = getManor())
            pManor->rallyGroup(mDataId, getCreature());

        WaycrestManorCombatAI::OnCombatStart(pTarget);
    }

    void OnCombatStop(Unit* pTarget) override
    {
        if (isAlive())
        {
            if (InstanceScript* pInstance = getInstanceScript())
            {
                pInstance->sendUnitEncounter(EncounterFrameDisengaged, getCreature());

                // the wipe counts once the last partner has left the fight
                WaycrestManorInstanceScript* pManor = getManor();
                if (pManor == nullptr || !pManor->isGroupFighting(mDataId, getCreature()))
                    pInstance->setBossState(mDataId, EncounterStates::Failed);
            }
        }

        WaycrestManorCombatAI::OnCombatStop(pTarget);
    }

    void OnDied(Unit* /*pKiller*/) override
    {
        onDefeat();

        if (InstanceScript* pInstance = getInstanceScript())
        {
            pInstance->sendUnitEncounter(EncounterFrameDisengaged, getCreature());

            WaycrestManorInstanceScript* pManor = getManor();
            if (pManor == nullptr || pManor->isGroupDown(mDataId, getCreature()))
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
// Heartsbane Triad, only the bearer of the Focusing Iris can fall

class WaycrestManorSisterAI : public WaycrestManorBossAI
{
public:
    static CreatureAIScript* CreateBriar(Creature* c) { return new WaycrestManorSisterAI(c, BriarKit); }
    static CreatureAIScript* CreateMalady(Creature* c) { return new WaycrestManorSisterAI(c, MaladyKit); }
    static CreatureAIScript* CreateSolena(Creature* c) { return new WaycrestManorSisterAI(c, SolenaKit); }

    WaycrestManorSisterAI(Creature* pCreature, WaycrestSisterKit const& kit) : WaycrestManorBossAI(pCreature, DATA_HEARTSBANE_TRIAD), mKit(kit) {}

    void DamageTaken(Unit* /*pAttacker*/, uint32_t* damage) override
    {
        if (damage == nullptr)
            return;

        Creature* pSelf = getCreature();
        uint32_t const health = pSelf->getHealth();
        if (!mHoldsIris && *damage >= health)
            *damage = health > 1 ? health - 1 : 0;

        if (mPastHalf || !_isInCombat())
            return;

        // first time below half: the bearer lets go of the iris
        uint32_t const left = health > *damage ? health - *damage : 0;
        if (left <= percentOf(pSelf->getMaxHealth(), 50))
        {
            mPastHalf = true;
            if (mHoldsIris)
                mDropPending = true;
        }
    }

    void DoAction(int32_t action) override
    {
        if (action == ACTION_TRIAD_CLAIM_IRIS && isAlive() && _isInCombat() && !mHoldsIris)
            scriptEvents.addEvent(EVENT_SISTER_CLAIM_IRIS, 1500);
    }

protected:
    void onEngage() override
    {
        mHoldsIris = false;
        mPastHalf = false;
        mDropPending = false;
        mRitual = 0;
        mRitualClock = 0;

        scriptEvents.addEvent(EVENT_SISTER_BOLT, 2000);

        // Solena starts with the iris
        if (getCreature()->getEntry() == BOSS_SISTER_SOLENA)
            claimIris();
    }

    void onReset() override
    {
        mHoldsIris = false;
        mPastHalf = false;
        mDropPending = false;
        _removeAura(SPELL_TRIAD_FOCUSING_IRIS);
        _removeAura(mKit.ward);
    }

    void onDefeat() override
    {
        if (!mHoldsIris)
            return;

        mHoldsIris = false;
        if (WaycrestManorInstanceScript* pManor = getManor())
            pManor->passIris(getCreature()->getEntry());
    }

    void onCombatTick(uint32_t diff) override
    {
        if (mDropPending && !_isCasting())
        {
            mDropPending = false;
            dropIris();
        }

        if (!mHoldsIris)
            return;

        mRitualClock += diff;
        while (mRitualClock >= 1000)
        {
            mRitualClock -= 1000;
            mRitual += DireRitualGainPerSecond;
        }

        if (mRitual < DireRitualFull || _isCasting())
            return;

        mRitual = 0;
        warn("The Focusing Iris overflows, a Dire Ritual begins!");
        castSpellAOE(SPELL_TRIAD_DIRE_RITUAL);
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_SISTER_BOLT:
                castAt(anyPlayer(), mKit.bolt);
                scriptEvents.addEvent(EVENT_SISTER_BOLT, 2400);
                break;
            case EVENT_SISTER_FOCUSED_BOLT:
                castAt(anyPlayer(), mKit.focusedBolt);
                scriptEvents.addEvent(EVENT_SISTER_FOCUSED_BOLT, mKit.focusedBoltMs);
                break;
            case EVENT_SISTER_SIGNATURE:
                if (mKit.signatureAtPlayer)
                    castAt(anyPlayer(), mKit.signature);
                else
                    castSpellAOE(mKit.signature);
                scriptEvents.addEvent(EVENT_SISTER_SIGNATURE, mKit.signatureMs);
                break;
            case EVENT_SISTER_CLAIM_IRIS:
                claimIris();
                break;
            default:
                break;
        }
    }

private:
    void claimIris()
    {
        if (mHoldsIris)
            return;

        mHoldsIris = true;
        mRitual = 0;
        mRitualClock = 0;

        _removeAura(mKit.ward);
        castSpellOnSelf(SPELL_TRIAD_CLAIM_THE_IRIS, true);
        castSpellOnSelf(SPELL_TRIAD_FOCUSING_IRIS, true);

        scriptEvents.removeEvent(EVENT_SISTER_BOLT);
        scriptEvents.addEvent(EVENT_SISTER_FOCUSED_BOLT, 4700);
        if (mKit.signature != 0)
            scriptEvents.addEvent(EVENT_SISTER_SIGNATURE, 14600);
    }

    // shielded again, the next sister takes the iris
    void dropIris()
    {
        if (!mHoldsIris)
            return;

        mHoldsIris = false;
        _removeAura(SPELL_TRIAD_FOCUSING_IRIS);
        scriptEvents.removeEvent(EVENT_SISTER_FOCUSED_BOLT);
        scriptEvents.removeEvent(EVENT_SISTER_SIGNATURE);

        castSpellOnSelf(mKit.ward, true);
        scriptEvents.addEvent(EVENT_SISTER_BOLT, 3200);

        if (WaycrestManorInstanceScript* pManor = getManor())
            pManor->passIris(getCreature()->getEntry());
    }

    WaycrestSisterKit const& mKit;
    bool mHoldsIris = false;
    bool mPastHalf = false;
    bool mDropPending = false;
    uint32_t mRitual = 0;
    uint32_t mRitualClock = 0;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Soulbound Goliath

class WaycrestManorGoliathAI : public WaycrestManorBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new WaycrestManorGoliathAI(c); }
    explicit WaycrestManorGoliathAI(Creature* pCreature) : WaycrestManorBossAI(pCreature, DATA_SOULBOUND_GOLIATH) {}

protected:
    void onEngage() override
    {
        scriptEvents.addEvent(EVENT_GOLIATH_CRUSH, 5800);
        scriptEvents.addEvent(EVENT_GOLIATH_SOUL_THORNS, 9800);
    }

    void onReset() override { _removeAuraOnPlayers(SPELL_GOLIATH_SOUL_THORNS); }

    void onDefeat() override { _removeAuraOnPlayers(SPELL_GOLIATH_SOUL_THORNS); }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_GOLIATH_CRUSH:
                castSpellOnVictim(SPELL_GOLIATH_CRUSH);
                scriptEvents.addEvent(EVENT_GOLIATH_CRUSH, 16200);
                break;
            case EVENT_GOLIATH_SOUL_THORNS:
                if (Unit* pTarget = offTankPlayer())
                {
                    warn("The Soulbound Goliath traps a player in Soul Thorns!");
                    castSpell(pTarget, SPELL_GOLIATH_SOUL_THORNS, true);
                    summonCreature(NPC_SOUL_THORNS, pTarget->GetPosition(), CORPSE_DESPAWN);
                }
                scriptEvents.addEvent(EVENT_GOLIATH_SOUL_THORNS, 22500);
                break;
            default:
                break;
        }
    }
};

// cutting the thorns frees the player caught inside
class WaycrestManorSoulThornsAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new WaycrestManorSoulThornsAI(c); }
    explicit WaycrestManorSoulThornsAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnLoad() override
    {
        setReactState(REACT_PASSIVE);
        setRooted(true);
        _setMeleeDisabled(true);
    }

    void OnDied(Unit* /*pKiller*/) override
    {
        for (Unit* pPlayer : playersAround(getCreature(), SoulThornsReach))
            pPlayer->removeAllAurasById(SPELL_GOLIATH_SOUL_THORNS);
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Raal the Gluttonous, rooted at his table

class WaycrestManorRaalAI : public WaycrestManorBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new WaycrestManorRaalAI(c); }
    explicit WaycrestManorRaalAI(Creature* pCreature) : WaycrestManorBossAI(pCreature, DATA_RAAL_THE_GLUTTONOUS) {}

protected:
    void onEngage() override
    {
        mFeedClock = 0;
        mReachClock = 0;
        setRooted(true);

        scriptEvents.addEvent(EVENT_RAAL_TENDERIZE, 3000);
        scriptEvents.addEvent(EVENT_RAAL_ROTTEN_EXPULSION, 6000);
        scriptEvents.addEvent(EVENT_RAAL_CALL_SERVANT, 9000);
    }

    void onReset() override { setRooted(false); }

    void onCombatTick(uint32_t diff) override
    {
        mFeedClock += diff;
        if (mFeedClock >= 1000)
        {
            mFeedClock = 0;
            devourArrivedServants();
        }

        mReachClock += diff;
        if (mReachClock < RaalMeleeCheckMs || _isCasting())
            return;

        // nobody at the table: he eats everything in the room
        mReachClock = 0;
        if (!playersAround(getCreature(), RaalMeleeReach).empty())
            return;

        warn("Raal the Gluttonous has no one in reach and begins to Consume All!");
        castSpellAOE(SPELL_RAAL_CONSUME_ALL);
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_RAAL_TENDERIZE:
                castAt(anyPlayer(), SPELL_RAAL_TENDERIZE);
                scriptEvents.addEvent(EVENT_RAAL_TENDERIZE, 15000);
                break;
            case EVENT_RAAL_ROTTEN_EXPULSION:
                if (Unit* pTarget = anyPlayer())
                {
                    castSpell(pTarget, SPELL_RAAL_ROTTEN_EXPULSION);

                    // heroic: oozelings crawl out of the bile
                    if (isHeroic())
                    {
                        for (uint8_t i = 0; i < 2; ++i)
                            callAdd(NPC_BILE_OOZELING, pointNear(pTarget->GetPosition(), 2.0f, 5.0f));
                    }
                }
                scriptEvents.addEvent(EVENT_RAAL_ROTTEN_EXPULSION, 20000);
                break;
            case EVENT_RAAL_CALL_SERVANT:
                warn("Raal the Gluttonous calls for his servants!");
                castSpellOnSelf(SPELL_RAAL_CALL_SERVANT, true);
                for (LocationVector const& door : RaalServantDoors)
                    summonCreature(NPC_WASTING_SERVANT, door, CORPSE_DESPAWN);
                scriptEvents.addEvent(EVENT_RAAL_CALL_SERVANT, 30000);
                break;
            default:
                break;
        }
    }

private:
    void devourArrivedServants()
    {
        std::list<Creature*> servants;
        GetCreatureListWithEntryInGrid(servants, NPC_WASTING_SERVANT, RaalFeedingReach);

        for (Creature* pServant : servants)
        {
            if (!pServant->isAlive())
                continue;

            castSpell(pServant, SPELL_RAAL_CONSUME_SERVANT, true);
            if (pServant->isAlive())
                getCreature()->dealDamage(pServant, pServant->getHealth(), 0);

            castSpellOnSelf(SPELL_RAAL_CONSUMED_SERVANT, true);
        }
    }

    uint32_t mFeedClock = 0;
    uint32_t mReachClock = 0;
};

// walks to Raal and gets eaten unless slain first
class WaycrestManorWastingServantAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new WaycrestManorWastingServantAI(c); }
    explicit WaycrestManorWastingServantAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnLoad() override
    {
        setReactState(REACT_PASSIVE);
        _setMeleeDisabled(true);
        shuffleToRaal();
    }

    void AIUpdate(unsigned long time_passed) override
    {
        if (!isAlive())
            return;

        uint32_t const diff = static_cast<uint32_t>(time_passed);
        if (mMoveClock > diff)
        {
            mMoveClock -= diff;
            return;
        }

        mMoveClock = 2000;
        shuffleToRaal();
    }

private:
    void shuffleToRaal()
    {
        Creature* pRaal = findNearestCreature(BOSS_RAAL_THE_GLUTTONOUS, 100.0f);
        if (pRaal != nullptr && pRaal->isAlive())
            moveTo(pRaal->GetPositionX(), pRaal->GetPositionY(), pRaal->GetPositionZ(), false);
    }

    uint32_t mMoveClock = 2000;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Lord and Lady Waycrest

// unkillable until his wife has given up her vitality three times
class WaycrestManorLordAI : public WaycrestManorBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new WaycrestManorLordAI(c); }
    explicit WaycrestManorLordAI(Creature* pCreature) : WaycrestManorBossAI(pCreature, DATA_LORD_AND_LADY_WAYCREST) {}

    void DamageTaken(Unit* /*pAttacker*/, uint32_t* damage) override
    {
        if (damage == nullptr || mTransfers >= LordTransferCount)
            return;

        Creature* pSelf = getCreature();
        uint32_t const health = pSelf->getHealth();
        if (*damage >= health)
            *damage = health > 1 ? health - 1 : 0;

        if (mAwaitingTransfer || !_isInCombat() || health - *damage > percentOf(pSelf->getMaxHealth(), LordTransferThresholdPct))
            return;

        WaycrestManorInstanceScript* pManor = getManor();
        Creature* pLady = pManor != nullptr ? pManor->getMember(BOSS_LADY_WAYCREST) : nullptr;
        if (pLady == nullptr || !pLady->isAlive())
        {
            // nobody left to heal him
            mTransfers = LordTransferCount;
            return;
        }

        mAwaitingTransfer = true;
        ++mTransfers;
        pManor->sendAction(BOSS_LADY_WAYCREST, mTransfers == LordTransferCount ? ACTION_LADY_TRANSFER_AND_DESCEND : ACTION_LADY_VITALITY_TRANSFER);
    }

    void DoAction(int32_t action) override
    {
        if (action == ACTION_LORD_REVITALIZED)
            mAwaitingTransfer = false;
    }

protected:
    void onEngage() override
    {
        mTransfers = 0;
        mAwaitingTransfer = false;
        moveJump(LordArenaSpot, 20.0f, 5.0f);

        scriptEvents.addEvent(EVENT_LORD_WASTING_STRIKE, 7200);
        scriptEvents.addEvent(EVENT_LORD_VIRULENT_PATHOGEN, 10800);
    }

    void onReset() override
    {
        mTransfers = 0;
        mAwaitingTransfer = false;
        _removeAura(SPELL_LORD_PUTRID_VITALITY);
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_LORD_WASTING_STRIKE:
                castSpellOnVictim(SPELL_LORD_WASTING_STRIKE);
                scriptEvents.addEvent(EVENT_LORD_WASTING_STRIKE, 17000);
                break;
            case EVENT_LORD_VIRULENT_PATHOGEN:
                castAt(anyPlayer(), SPELL_LORD_VIRULENT_PATHOGEN);
                scriptEvents.addEvent(EVENT_LORD_VIRULENT_PATHOGEN, 17000);
                break;
            default:
                break;
        }
    }

private:
    uint32_t mTransfers = 0;
    bool mAwaitingTransfer = false;
};

// plays from the side under Soul Armor and comes down after the third transfer
class WaycrestManorLadyAI : public WaycrestManorBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new WaycrestManorLadyAI(c); }
    explicit WaycrestManorLadyAI(Creature* pCreature) : WaycrestManorBossAI(pCreature, DATA_LORD_AND_LADY_WAYCREST) {}

    void DoAction(int32_t action) override
    {
        if (!isAlive() || !_isInCombat())
            return;

        if (action != ACTION_LADY_VITALITY_TRANSFER && action != ACTION_LADY_TRANSFER_AND_DESCEND)
            return;

        if (action == ACTION_LADY_TRANSFER_AND_DESCEND && !mInArena)
            mDescendAfterTransfer = true;

        scriptEvents.addEvent(EVENT_LADY_TRANSFER_BEGIN, 100);
    }

protected:
    void onEngage() override
    {
        mInArena = false;
        mDescendAfterTransfer = false;

        castSpellOnSelf(SPELL_LADY_SOUL_ARMOR, true);
        setRooted(true);
        _setMeleeDisabled(true);

        scriptEvents.addEvent(EVENT_LADY_WRACKING_CHORD, 1100);
        scriptEvents.addEvent(EVENT_LADY_DISCORDANT_CADENZA, 13300);
    }

    void onReset() override
    {
        mInArena = false;
        mDescendAfterTransfer = false;
        _removeAura(SPELL_LADY_SOUL_ARMOR);
        setRooted(false);
        _setMeleeDisabled(false);
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_LADY_WRACKING_CHORD:
                castAt(anyPlayer(), SPELL_LADY_WRACKING_CHORD);
                scriptEvents.addEvent(EVENT_LADY_WRACKING_CHORD, mInArena ? 8500 : 2400);
                break;
            case EVENT_LADY_DISCORDANT_CADENZA:
                warn("Lady Waycrest plays a Discordant Cadenza!");
                castAt(anyPlayer(), SPELL_LADY_DISCORDANT_CADENZA);
                scriptEvents.addEvent(EVENT_LADY_DISCORDANT_CADENZA, 21800);
                break;
            case EVENT_LADY_TRANSFER_BEGIN:
                if (Creature* pLord = lord())
                {
                    warn("Lady Waycrest pours her Vitality into Lord Waycrest!");
                    castSpell(pLord, SPELL_LADY_VITALITY_TRANSFER, true);
                }
                scriptEvents.addEvent(EVENT_LADY_TRANSFER_DONE, 2500);
                if (mDescendAfterTransfer)
                {
                    mDescendAfterTransfer = false;
                    scriptEvents.addEvent(EVENT_LADY_DESCEND, 3000);
                }
                break;
            case EVENT_LADY_TRANSFER_DONE:
                completeTransfer();
                break;
            case EVENT_LADY_DESCEND:
                descend();
                break;
            default:
                break;
        }
    }

private:
    Creature* lord()
    {
        WaycrestManorInstanceScript* pManor = getManor();
        Creature* pLord = pManor != nullptr ? pManor->getMember(BOSS_LORD_WAYCREST) : nullptr;
        return pLord != nullptr && pLord->isAlive() ? pLord : nullptr;
    }

    // she pays a share of her own health, he is restored and empowered
    void completeTransfer()
    {
        Creature* pSelf = getCreature();
        uint32_t const cost = percentOf(pSelf->getMaxHealth(), LadyTransferCostPct);
        pSelf->setHealth(pSelf->getHealth() > cost ? pSelf->getHealth() - cost : 1);

        if (Creature* pLord = lord())
        {
            pLord->setHealth(pLord->getMaxHealth());
            pLord->castSpell(pLord, SPELL_LORD_PUTRID_VITALITY, true);
        }

        if (WaycrestManorInstanceScript* pManor = getManor())
            pManor->sendAction(BOSS_LORD_WAYCREST, ACTION_LORD_REVITALIZED);
    }

    void descend()
    {
        mInArena = true;
        warn("Lady Waycrest joins the fight!");
        _removeAura(SPELL_LADY_SOUL_ARMOR);
        setRooted(false);
        _setMeleeDisabled(false);

        Creature* pLord = lord();
        moveJump(pointNear(pLord != nullptr ? pLord->GetPosition() : LordArenaSpot, 3.0f, 6.0f), 20.0f, 5.0f);
        scriptEvents.addEvent(EVENT_LADY_WRACKING_CHORD, 10100);
    }

    bool mInArena = false;
    bool mDescendAfterTransfer = false;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Gorak Tul

class WaycrestManorGorakTulAI : public WaycrestManorBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new WaycrestManorGorakTulAI(c); }
    explicit WaycrestManorGorakTulAI(Creature* pCreature) : WaycrestManorBossAI(pCreature, DATA_GORAK_TUL) {}

protected:
    void onEngage() override
    {
        scriptEvents.addEvent(EVENT_GORAK_DARKENED_LIGHTNING, 8000);
        scriptEvents.addEvent(EVENT_GORAK_SUMMON_SLAVER, 13000);
        scriptEvents.addEvent(EVENT_GORAK_DREAD_ESSENCE, 24000);
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_GORAK_DARKENED_LIGHTNING:
                castAt(anyPlayer(), SPELL_GORAK_DARKENED_LIGHTNING);
                scriptEvents.addEvent(EVENT_GORAK_DARKENED_LIGHTNING, 15000);
                break;
            case EVENT_GORAK_SUMMON_SLAVER:
                summonSlaver();
                scriptEvents.addEvent(EVENT_GORAK_SUMMON_SLAVER, 20000);
                break;
            case EVENT_GORAK_DREAD_ESSENCE:
                warn("Gorak Tul unleashes Dread Essence!");
                castSpellAOE(SPELL_GORAK_DREAD_ESSENCE);
                scriptEvents.addEvent(EVENT_GORAK_DREAD_ESSENCE, 25000);
                break;
            default:
                break;
        }
    }

private:
    // slavers step out of the drust portal
    void summonSlaver()
    {
        castSpellOnSelf(SPELL_GORAK_SUMMON_SLAVER, true);

        Creature* pPortal = findNearestCreature(NPC_DRUST_SUMMONING_PORTAL, 100.0f);
        LocationVector const spot = pPortal != nullptr ? pPortal->GetPosition() : pointNear(getCreature()->GetPosition(), 5.0f, 10.0f);
        callAdd(NPC_DEATHTOUCHED_SLAVER, spot);
    }
};

class WaycrestManorDeathtouchedSlaverAI : public WaycrestManorCombatAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new WaycrestManorDeathtouchedSlaverAI(c); }
    explicit WaycrestManorDeathtouchedSlaverAI(Creature* pCreature) : WaycrestManorCombatAI(pCreature) {}

protected:
    void onEngage() override
    {
        scriptEvents.addEvent(EVENT_SLAVER_DARK_LEAP, 3000);
        scriptEvents.addEvent(EVENT_SLAVER_DREAD_BOLT, 6000);

        if (isHeroic())
            scriptEvents.addEvent(EVENT_SLAVER_DEATH_LENS, 9000);
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_SLAVER_DARK_LEAP:
                castAt(farthestPlayer(), SPELL_SLAVER_DARK_LEAP);
                scriptEvents.addEvent(EVENT_SLAVER_DARK_LEAP, 15000);
                break;
            case EVENT_SLAVER_DREAD_BOLT:
                castAt(anyPlayer(), SPELL_SLAVER_DREAD_BOLT);
                scriptEvents.addEvent(EVENT_SLAVER_DREAD_BOLT, 2000);
                break;
            case EVENT_SLAVER_DEATH_LENS:
                castAt(farthestPlayer(), SPELL_SLAVER_DEATH_LENS);
                scriptEvents.addEvent(EVENT_SLAVER_DEATH_LENS, 20000);
                break;
            default:
                break;
        }
    }
};

void SetupWaycrestManor(ScriptMgr* mgr)
{
    mgr->register_instance_script(MAP_WAYCREST_MANOR, &WaycrestManorInstanceScript::Create);

    mgr->register_creature_script(BOSS_SISTER_BRIAR, &WaycrestManorSisterAI::CreateBriar);
    mgr->register_creature_script(BOSS_SISTER_MALADY, &WaycrestManorSisterAI::CreateMalady);
    mgr->register_creature_script(BOSS_SISTER_SOLENA, &WaycrestManorSisterAI::CreateSolena);

    mgr->register_creature_script(BOSS_SOULBOUND_GOLIATH, &WaycrestManorGoliathAI::Create);
    mgr->register_creature_script(NPC_SOUL_THORNS, &WaycrestManorSoulThornsAI::Create);

    mgr->register_creature_script(BOSS_RAAL_THE_GLUTTONOUS, &WaycrestManorRaalAI::Create);
    mgr->register_creature_script(NPC_WASTING_SERVANT, &WaycrestManorWastingServantAI::Create);

    mgr->register_creature_script(BOSS_LORD_WAYCREST, &WaycrestManorLordAI::Create);
    mgr->register_creature_script(BOSS_LADY_WAYCREST, &WaycrestManorLadyAI::Create);

    mgr->register_creature_script(BOSS_GORAK_TUL, &WaycrestManorGorakTulAI::Create);
    mgr->register_creature_script(NPC_DEATHTOUCHED_SLAVER, &WaycrestManorDeathtouchedSlaverAI::Create);
}
