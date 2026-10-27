/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "Instance_Halls_Of_Valor.hpp"

#include "Objects/GameObject.h"
#include "Objects/Units/Creatures/Creature.h"
#include "Objects/Units/Players/Player.hpp"
#include "Server/Script/CreatureAIScript.hpp"
#include "Server/Script/InstanceScript.hpp"
#include "Server/Script/ScriptMgr.hpp"
#include "Utilities/Random.hpp"

#include <cmath>
#include <set>
#include <utility>
#include <vector>

enum HallsOfValorEvents
{
    EVENT_HYMDALL_HORN_OF_VALOR = 1,
    EVENT_HYMDALL_DANCING_BLADE,
    EVENT_HYMDALL_BLOODLETTING_SWEEP,

    EVENT_HYRJA_EXPEL_LIGHT,
    EVENT_HYRJA_SHIELD_OF_LIGHT,
    EVENT_HYRJA_ARCING_BOLT,
    EVENT_HYRJA_EMPOWERED_ABILITY,

    EVENT_OLMYR_SEARING_LIGHT,
    EVENT_OLMYR_SANCTIFY,
    EVENT_SOLSTEN_EYE_OF_THE_STORM,

    EVENT_FENRYR_UNNERVING_HOWL,
    EVENT_FENRYR_RAVENOUS_LEAP,
    EVENT_FENRYR_CLAW_FRENZY,
    EVENT_FENRYR_SCENT_OF_BLOOD,
    EVENT_FENRYR_PACK,

    EVENT_SKOVALD_FELBLAZE_RUSH,
    EVENT_SKOVALD_SAVAGE_BLADE,
    EVENT_SKOVALD_RAGNAROK,

    EVENT_KING_CALL_ANCESTOR,
    EVENT_KING_WICKED_DAGGER,
    EVENT_KING_UNRULY_YELL,
    EVENT_KING_SEVER,

    EVENT_ODYN_SPEAR_OF_LIGHT,
    EVENT_ODYN_RADIANT_TEMPEST,
    EVENT_ODYN_SHATTER_SPEARS,
    EVENT_ODYN_RUNIC_BRAND,
    EVENT_ODYN_STORMFORGED
};

namespace
{
    float const HallsOfValorTwoPi = 6.28318531f;
    uint32_t const HallsOfValorHostileFaction = 16;

    // Skovald comes through the gate once the four kings are slain
    LocationVector const SkovaldGateArrival(2500.0f, 528.9f, 749.0f, 3.14159f);
    LocationVector const SkovaldHallSpot(2411.03f, 528.72f, 748.99f);
    uint32_t const SkovaldArrivalMs = 20000;
    // energy starts at 80 and fills by five every three seconds
    uint32_t const SkovaldRagnarokFirstMs = 12000;
    uint32_t const SkovaldRagnarokCycleMs = 60000;

    // a king takes up the challenge when a player walks up to him
    float const KingChallengeRange = 8.0f;
    uint32_t const OdynKings[] = { NPC_KING_HALDOR, NPC_KING_BJORN, NPC_KING_RANULF, NPC_KING_TOR };

    LocationVector const OdynHallCentre(2429.13f, 528.55f, 748.99f);
    uint32_t const OdynRunicBrands[] =
    {
        SPELL_ODYN_RUNIC_BRAND_PURE,
        SPELL_ODYN_RUNIC_BRAND_RED,
        SPELL_ODYN_RUNIC_BRAND_YELLOW,
        SPELL_ODYN_RUNIC_BRAND_BLUE,
        SPELL_ODYN_RUNIC_BRAND_GREEN
    };
    uint32_t const OdynRunicBrandCount = 5;

    // grounded point on a ring around the anchor
    LocationVector hallsOfValorRingPoint(Creature* pAnchor, float minRange, float maxRange)
    {
        float const angle = Util::getRandomFloat(HallsOfValorTwoPi);
        float const range = Util::getRandomFloat(minRange, maxRange);
        float const x = pAnchor->GetPositionX() + range * std::cos(angle);
        float const y = pAnchor->GetPositionY() + range * std::sin(angle);
        float z = pAnchor->GetPositionZ();
        pAnchor->updateGroundPositionZ(x, y, z);
        return LocationVector(x, y, z, angle);
    }
}

//////////////////////////////////////////////////////////////////////////////////////////
// Instance

class HallsOfValorInstanceScript : public InstanceScript
{
public:
    explicit HallsOfValorInstanceScript(WorldMap* pMapMgr) : InstanceScript(pMapMgr)
    {
        setBossNumber(HallsOfValorEncounterCount);
    }

    static InstanceScript* Create(WorldMap* pMapMgr) { return new HallsOfValorInstanceScript(pMapMgr); }

    void OnCreaturePushToWorld(Creature* pCreature) override
    {
        switch (pCreature->getEntry())
        {
            case BOSS_GOD_KING_SKOVALD:
                mSkovaldGuid = pCreature->GetNewGUID();
                break;
            case NPC_OLMYR_THE_ENLIGHTENED:
            case NPC_SOLSTEN:
                mDefenderGuids.insert(pCreature->GetNewGUID());
                break;
            case NPC_KING_HALDOR:
            case NPC_KING_BJORN:
            case NPC_KING_RANULF:
            case NPC_KING_TOR:
                mKingGuids.insert(pCreature->GetNewGUID());
                break;
            default:
                break;
        }
    }

    void OnGameObjectPushToWorld(GameObject* pGameObject) override
    {
        bool open = false;
        if (!gateShouldBeOpen(pGameObject->getEntry(), open))
            return;

        mGateGuids.push_back(pGameObject->GetNewGUID());
        alignGate(pGameObject, open);
    }

    void OnEncounterStateChange(uint32_t /*id*/, uint32_t /*state*/) override
    {
        for (WoWGuid const& guid : mGateGuids)
        {
            GameObject* pGate = getGameObjectByGuid(guid);
            bool open = false;
            if (pGate != nullptr && gateShouldBeOpen(pGate->getEntry(), open))
                alignGate(pGate, open);
        }
    }

    // calls Skovald once no king is left standing
    void UpdateEvent() override
    {
        if (mSkovaldCalled)
            return;

        // spawns settle during the first seconds
        if (mUpdateTicks < 5)
        {
            ++mUpdateTicks;
            return;
        }

        if (isDone(DATA_GOD_KING_SKOVALD) || getCreatureByGuid(mSkovaldGuid) != nullptr)
        {
            mSkovaldCalled = true;
            return;
        }

        if (countLiving(mKingGuids) > 0)
            return;

        mSkovaldCalled = true;
        if (Creature* pSkovald = spawnCreature(BOSS_GOD_KING_SKOVALD, SkovaldGateArrival.x, SkovaldGateArrival.y, SkovaldGateArrival.z, SkovaldGateArrival.o))
            mSkovaldGuid = pSkovald->GetNewGUID();
    }

    uint32_t livingDefenders() { return countLiving(mDefenderGuids); }

private:
    bool isDone(uint32_t id) const { return getBossState(id) == EncounterStates::Performed; }
    bool isFighting(uint32_t id) const { return getBossState(id) == EncounterStates::InProgress; }

    // false for objects that are no encounter gate
    bool gateShouldBeOpen(uint32_t entry, bool& open) const
    {
        switch (entry)
        {
            case GO_HYMDALL_ENTRANCE_GATE:
                open = !isFighting(DATA_HYMDALL);
                return true;
            case GO_HYMDALL_EXIT_GATE:
                open = isDone(DATA_HYMDALL);
                return true;
            case GO_HYRJA_GATE:
                open = !isFighting(DATA_HYRJA);
                return true;
            case GO_GATEWAY_OF_SPLENDOR:
            case GO_BRIDGE_OF_GLORY:
                open = isDone(DATA_HYRJA) && isDone(DATA_FENRYR);
                return true;
            case GO_ODYN_HALL_GATE:
                open = !isFighting(DATA_GOD_KING_SKOVALD) && !isFighting(DATA_ODYN);
                return true;
            default:
                return false;
        }
    }

    // changes the state only when it differs
    void alignGate(GameObject* pGate, bool open)
    {
        bool const isOpen = pGate->getState() != GO_STATE_CLOSED;
        if (isOpen != open)
            pGate->setState(open ? GO_STATE_OPEN : GO_STATE_CLOSED);
    }

    uint32_t countLiving(std::set<WoWGuid> const& guids)
    {
        uint32_t living = 0;
        for (WoWGuid const& guid : guids)
        {
            Creature* pCreature = getCreatureByGuid(guid);
            if (pCreature != nullptr && pCreature->isAlive())
                ++living;
        }

        return living;
    }

    bool mSkovaldCalled = false;
    uint32_t mUpdateTicks = 0;
    WoWGuid mSkovaldGuid;
    std::set<WoWGuid> mDefenderGuids;
    std::set<WoWGuid> mKingGuids;
    std::vector<WoWGuid> mGateGuids;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Shared combat base: event tick in combat, idle tick outside

class HallsOfValorCombatAI : public CreatureAIScript
{
public:
    explicit HallsOfValorCombatAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override { onEngage(); }

    void OnCombatStop(Unit* /*pTarget*/) override
    {
        scriptEvents.resetEvents();
        summons.despawnAll();
        onDisengage();
    }

    void onSummonedCreature(Creature* pSummon) override { summons.summon(pSummon); }

    void AIUpdate(unsigned long time_passed) override
    {
        uint32_t const diff = static_cast<uint32_t>(time_passed);
        if (!_isInCombat())
        {
            onIdleTick(diff);
            return;
        }

        scriptEvents.updateEvents(diff, 0);

        if (_isCasting())
            return;

        if (uint32_t eventId = scriptEvents.getFinishedEvent())
            onEvent(eventId);
    }

protected:
    virtual void onEngage() {}
    virtual void onDisengage() {}
    virtual void onIdleTick(uint32_t /*diff*/) {}
    virtual void onEvent(uint32_t /*eventId*/) {}

    HallsOfValorInstanceScript* getHallsOfValor()
    {
        if (getCreature()->GetMapId() != MAP_HALLS_OF_VALOR)
            return nullptr;

        return static_cast<HallsOfValorInstanceScript*>(getInstanceScript());
    }

    Unit* randomPlayer() { return selectUnitTarget(FilterArgs(TargetFilter_Player)); }

    void castOnRandomPlayer(uint32_t spellId, bool triggered = false)
    {
        if (Unit* pTarget = randomPlayer())
            castSpell(pTarget, spellId, triggered);
    }

    // living players in range in random order
    std::vector<Unit*> playersAround(float range)
    {
        std::vector<Unit*> players;
        for (Object* pObject : getCreature()->getInRangePlayersSet())
        {
            Unit* pPlayer = pObject != nullptr ? pObject->ToUnit() : nullptr;
            if (pPlayer != nullptr && pPlayer->isAlive() && getCreature()->getDistance(pPlayer) <= range)
                players.push_back(pPlayer);
        }

        for (size_t i = players.size(); i > 1; --i)
            std::swap(players[i - 1], players[Util::getRandomUInt(static_cast<uint32_t>(i - 1))]);

        return players;
    }

    // out of reach and out of combat until released
    void seal(bool sealed)
    {
        setCanEnterCombat(!sealed);
        if (sealed)
            addUnitFlags(UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
        else
            removeUnitFlags(UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
    }

    Creature* callAdd(uint32_t entry, LocationVector const& position)
    {
        Creature* pAdd = summonCreature(entry, position, CORPSE_DESPAWN);
        if (pAdd != nullptr)
            setZoneWideCombat(pAdd);

        return pAdd;
    }

    // counts down and reports when it ran out
    static bool countDown(uint32_t& remainingMs, uint32_t diff)
    {
        if (remainingMs > diff)
        {
            remainingMs -= diff;
            return false;
        }

        remainingMs = 0;
        return true;
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Encounter base: frame and boss state

class HallsOfValorBossAI : public HallsOfValorCombatAI
{
public:
    HallsOfValorBossAI(Creature* pCreature, uint32_t encounterId) : HallsOfValorCombatAI(pCreature), mEncounterId(encounterId) {}

    void OnCombatStart(Unit* pTarget) override
    {
        if (InstanceScript* pInstance = getInstanceScript())
        {
            pInstance->sendUnitEncounter(EncounterFrameEngage, getCreature());
            pInstance->setBossState(mEncounterId, EncounterStates::InProgress);
        }

        HallsOfValorCombatAI::OnCombatStart(pTarget);
    }

    void OnCombatStop(Unit* pTarget) override
    {
        // a dying boss leaves combat as well
        if (isAlive())
        {
            if (InstanceScript* pInstance = getInstanceScript())
            {
                pInstance->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
                pInstance->setBossState(mEncounterId, EncounterStates::Failed);
            }
        }

        HallsOfValorCombatAI::OnCombatStop(pTarget);
    }

    void OnDied(Unit* /*pKiller*/) override
    {
        if (InstanceScript* pInstance = getInstanceScript())
        {
            pInstance->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
            pInstance->setBossState(mEncounterId, EncounterStates::Performed);
        }
    }

protected:
    uint32_t mEncounterId;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Hymdall

class HallsOfValorHymdallAI : public HallsOfValorBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new HallsOfValorHymdallAI(c); }
    explicit HallsOfValorHymdallAI(Creature* pCreature) : HallsOfValorBossAI(pCreature, DATA_HYMDALL) {}

protected:
    void onEngage() override
    {
        scriptEvents.addEvent(EVENT_HYMDALL_HORN_OF_VALOR, 6000);
        scriptEvents.addEvent(EVENT_HYMDALL_DANCING_BLADE, 15000);
        scriptEvents.addEvent(EVENT_HYMDALL_BLOODLETTING_SWEEP, 20000);
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_HYMDALL_HORN_OF_VALOR:
                sendChatMessage(CHAT_MSG_RAID_BOSS_EMOTE, 0, "Hymdall sounds the Horn of Valor!");
                castSpellAOE(SPELL_HYMDALL_HORN_OF_VALOR);
                scriptEvents.addEvent(EVENT_HYMDALL_HORN_OF_VALOR, 30000);
                break;
            case EVENT_HYMDALL_DANCING_BLADE:
                castOnRandomPlayer(SPELL_HYMDALL_DANCING_BLADE);
                scriptEvents.addEvent(EVENT_HYMDALL_DANCING_BLADE, 11000);
                break;
            case EVENT_HYMDALL_BLOODLETTING_SWEEP:
                castSpellOnVictim(SPELL_HYMDALL_BLOODLETTING_SWEEP, true);
                scriptEvents.addEvent(EVENT_HYMDALL_BLOODLETTING_SWEEP, 26000);
                break;
            default:
                break;
        }
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Hyrja, descends once Olmyr and Solsten have fallen

class HallsOfValorHyrjaAI : public HallsOfValorBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new HallsOfValorHyrjaAI(c); }
    explicit HallsOfValorHyrjaAI(Creature* pCreature) : HallsOfValorBossAI(pCreature, DATA_HYRJA) {}

    void OnLoad() override
    {
        mAscended = false;
        mWatchMs = 5000;
        seal(true);
    }

protected:
    void onIdleTick(uint32_t diff) override
    {
        if (mAscended || !countDown(mWatchMs, diff))
            return;

        mWatchMs = 2000;

        HallsOfValorInstanceScript* pHalls = getHallsOfValor();
        if (pHalls != nullptr && pHalls->livingDefenders() > 0)
            return;

        mAscended = true;
        seal(false);
        castSpellOnSelf(SPELL_HYRJA_ASCENDANCE, true);
        sendChatMessage(CHAT_MSG_RAID_BOSS_EMOTE, 0, "Hyrja ascends with the power of her fallen defenders!");
    }

    void onEngage() override
    {
        mStormNext = true;
        scriptEvents.addEvent(EVENT_HYRJA_EXPEL_LIGHT, 3000);
        scriptEvents.addEvent(EVENT_HYRJA_SHIELD_OF_LIGHT, 10000);
        scriptEvents.addEvent(EVENT_HYRJA_ARCING_BOLT, 12000);
        scriptEvents.addEvent(EVENT_HYRJA_EMPOWERED_ABILITY, 20000);
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_HYRJA_EXPEL_LIGHT:
                castOnRandomPlayer(SPELL_HYRJA_EXPEL_LIGHT, true);
                scriptEvents.addEvent(EVENT_HYRJA_EXPEL_LIGHT, 30000);
                break;
            case EVENT_HYRJA_SHIELD_OF_LIGHT:
                castSpellOnVictim(SPELL_HYRJA_SHIELD_OF_LIGHT);
                scriptEvents.addEvent(EVENT_HYRJA_SHIELD_OF_LIGHT, 20000);
                break;
            case EVENT_HYRJA_ARCING_BOLT:
                castOnRandomPlayer(SPELL_HYRJA_ARCING_BOLT);
                scriptEvents.addEvent(EVENT_HYRJA_ARCING_BOLT, 12000);
                break;
            case EVENT_HYRJA_EMPOWERED_ABILITY:
                // storm and light in turn
                if (mStormNext)
                {
                    sendChatMessage(CHAT_MSG_RAID_BOSS_EMOTE, 0, "Hyrja calls down the Eye of the Storm!");
                    castSpellAOE(SPELL_HYRJA_EYE_OF_THE_STORM);
                }
                else
                {
                    sendChatMessage(CHAT_MSG_RAID_BOSS_EMOTE, 0, "Hyrja begins to cast Sanctify!");
                    castSpellOnSelf(SPELL_HYRJA_SANCTIFY);
                }
                mStormNext = !mStormNext;
                scriptEvents.addEvent(EVENT_HYRJA_EMPOWERED_ABILITY, 30000);
                break;
            default:
                break;
        }
    }

private:
    bool mAscended = false;
    bool mStormNext = true;
    uint32_t mWatchMs = 5000;
};

// Olmyr the Enlightened and Solsten
class HallsOfValorDefenderAI : public HallsOfValorCombatAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new HallsOfValorDefenderAI(c); }
    explicit HallsOfValorDefenderAI(Creature* pCreature) : HallsOfValorCombatAI(pCreature) {}

protected:
    void onEngage() override
    {
        if (getCreature()->getEntry() == NPC_OLMYR_THE_ENLIGHTENED)
        {
            scriptEvents.addEvent(EVENT_OLMYR_SEARING_LIGHT, 5000);
            scriptEvents.addEvent(EVENT_OLMYR_SANCTIFY, 30000);
        }
        else
        {
            scriptEvents.addEvent(EVENT_SOLSTEN_EYE_OF_THE_STORM, 30000);
        }
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_OLMYR_SEARING_LIGHT:
                castSpellOnVictim(SPELL_OLMYR_SEARING_LIGHT);
                scriptEvents.addEvent(EVENT_OLMYR_SEARING_LIGHT, 7000);
                break;
            case EVENT_OLMYR_SANCTIFY:
                castSpellOnSelf(SPELL_HYRJA_SANCTIFY);
                scriptEvents.addEvent(EVENT_OLMYR_SANCTIFY, 30000);
                break;
            case EVENT_SOLSTEN_EYE_OF_THE_STORM:
                castSpellAOE(SPELL_HYRJA_EYE_OF_THE_STORM);
                scriptEvents.addEvent(EVENT_SOLSTEN_EYE_OF_THE_STORM, 30000);
                break;
            default:
                break;
        }
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Fenryr

class HallsOfValorFenryrAI : public HallsOfValorBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new HallsOfValorFenryrAI(c); }
    explicit HallsOfValorFenryrAI(Creature* pCreature) : HallsOfValorBossAI(pCreature, DATA_FENRYR) {}

protected:
    void onEngage() override
    {
        scriptEvents.addEvent(EVENT_FENRYR_UNNERVING_HOWL, 3000);
        scriptEvents.addEvent(EVENT_FENRYR_SCENT_OF_BLOOD, 9000);
        scriptEvents.addEvent(EVENT_FENRYR_RAVENOUS_LEAP, 28000);
        scriptEvents.addEvent(EVENT_FENRYR_CLAW_FRENZY, 40000);
        if (isHeroic())
            scriptEvents.addEvent(EVENT_FENRYR_PACK, 4000);
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_FENRYR_UNNERVING_HOWL:
                castSpellAOE(SPELL_FENRYR_UNNERVING_HOWL);
                scriptEvents.addEvent(EVENT_FENRYR_UNNERVING_HOWL, 36000);
                break;
            case EVENT_FENRYR_SCENT_OF_BLOOD:
                castSpellOnSelf(SPELL_FENRYR_SCENT_OF_BLOOD, true);
                scriptEvents.addEvent(EVENT_FENRYR_SCENT_OF_BLOOD, 38000);
                break;
            case EVENT_FENRYR_RAVENOUS_LEAP:
                sendChatMessage(CHAT_MSG_RAID_BOSS_EMOTE, 0, "Fenryr leaps upon his prey!");
                castSpellAOE(SPELL_FENRYR_RAVENOUS_LEAP);
                scriptEvents.addEvent(EVENT_FENRYR_RAVENOUS_LEAP, 36000);
                break;
            case EVENT_FENRYR_CLAW_FRENZY:
                castSpellAOE(SPELL_FENRYR_CLAW_FRENZY);
                scriptEvents.addEvent(EVENT_FENRYR_CLAW_FRENZY, Util::getRandomUInt(10000, 20000));
                break;
            case EVENT_FENRYR_PACK:
                for (uint8_t i = 0; i < 3; ++i)
                    callAdd(NPC_EBONCLAW_PACKMATE, hallsOfValorRingPoint(getCreature(), 30.0f, 40.0f));
                scriptEvents.addEvent(EVENT_FENRYR_PACK, 36000);
                break;
            default:
                break;
        }
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// The four kings, God-King Skovald

class HallsOfValorKingAI : public HallsOfValorCombatAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new HallsOfValorKingAI(c); }
    explicit HallsOfValorKingAI(Creature* pCreature) : HallsOfValorCombatAI(pCreature) {}

    void OnLoad() override
    {
        mPeacefulFaction = getCreature()->getFactionTemplate();
        mChallenged = false;
        mWatchMs = 1000;
    }

protected:
    void onIdleTick(uint32_t diff) override
    {
        if (mChallenged || !countDown(mWatchMs, diff))
            return;

        mWatchMs = 500;

        Player* pPlayer = getNearestPlayer();
        if (pPlayer == nullptr || !pPlayer->isAlive() || getCreature()->getDistance(pPlayer) > KingChallengeRange)
            return;

        mChallenged = true;
        getCreature()->setFaction(HallsOfValorHostileFaction);
        attackStart(pPlayer);
    }

    // back to waiting for a challenger
    void onDisengage() override
    {
        if (!isAlive())
            return;

        mChallenged = false;
        mWatchMs = 5000;
        getCreature()->setFaction(mPeacefulFaction);
    }

    void onEngage() override
    {
        scriptEvents.addEvent(EVENT_KING_UNRULY_YELL, Util::getRandomUInt(3000, 5000));
        scriptEvents.addEvent(EVENT_KING_SEVER, Util::getRandomUInt(11000, 15000));
        scriptEvents.addEvent(EVENT_KING_WICKED_DAGGER, Util::getRandomUInt(16000, 18000));
        scriptEvents.addEvent(EVENT_KING_CALL_ANCESTOR, Util::getRandomUInt(24000, 27000));
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_KING_UNRULY_YELL:
                castSpellAOE(SPELL_KING_UNRULY_YELL);
                break;
            case EVENT_KING_SEVER:
                castSpellOnVictim(SPELL_KING_SEVER);
                scriptEvents.addEvent(EVENT_KING_SEVER, Util::getRandomUInt(11000, 15000));
                break;
            case EVENT_KING_WICKED_DAGGER:
                castSpellOnVictim(SPELL_KING_WICKED_DAGGER);
                scriptEvents.addEvent(EVENT_KING_WICKED_DAGGER, Util::getRandomUInt(16000, 18000));
                break;
            case EVENT_KING_CALL_ANCESTOR:
                castSpellAOE(SPELL_KING_CALL_ANCESTOR);
                scriptEvents.addEvent(EVENT_KING_CALL_ANCESTOR, Util::getRandomUInt(22000, 26000));
                break;
            default:
                break;
        }
    }

private:
    uint32_t mPeacefulFaction = 0;
    bool mChallenged = false;
    uint32_t mWatchMs = 1000;
};

class HallsOfValorSkovaldAI : public HallsOfValorBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new HallsOfValorSkovaldAI(c); }
    explicit HallsOfValorSkovaldAI(Creature* pCreature) : HallsOfValorBossAI(pCreature, DATA_GOD_KING_SKOVALD) {}

    void OnLoad() override
    {
        mArrival = ArrivalStep::AtGate;
        mArrivalMs = 1000;
        seal(true);
    }

protected:
    // walks into the hall and takes up the fight after a while
    void onIdleTick(uint32_t diff) override
    {
        if (mArrival == ArrivalStep::Arrived || !countDown(mArrivalMs, diff))
            return;

        if (mArrival == ArrivalStep::AtGate)
        {
            mArrival = ArrivalStep::Walking;
            mArrivalMs = SkovaldArrivalMs;
            moveTo(SkovaldHallSpot.x, SkovaldHallSpot.y, SkovaldHallSpot.z, false);
            sendChatMessage(CHAT_MSG_RAID_BOSS_EMOTE, 0, "God-King Skovald strides into the hall to claim the Aegis of Aggramar!");
            return;
        }

        mArrival = ArrivalStep::Arrived;
        seal(false);
    }

    void onEngage() override
    {
        scriptEvents.addEvent(EVENT_SKOVALD_FELBLAZE_RUSH, 7000);
        scriptEvents.addEvent(EVENT_SKOVALD_RAGNAROK, SkovaldRagnarokFirstMs);
        scriptEvents.addEvent(EVENT_SKOVALD_SAVAGE_BLADE, 24000);
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_SKOVALD_FELBLAZE_RUSH:
                castSpellAOE(SPELL_SKOVALD_FELBLAZE_RUSH);
                scriptEvents.addEvent(EVENT_SKOVALD_FELBLAZE_RUSH, Util::getRandomUInt(10000, 26000));
                break;
            case EVENT_SKOVALD_RAGNAROK:
                sendChatMessage(CHAT_MSG_RAID_BOSS_EMOTE, 0, "God-King Skovald begins to cast Ragnarok!");
                castSpellAOE(SPELL_SKOVALD_RAGNAROK);
                scriptEvents.addEvent(EVENT_SKOVALD_RAGNAROK, SkovaldRagnarokCycleMs);
                break;
            case EVENT_SKOVALD_SAVAGE_BLADE:
                castSpellOnVictim(SPELL_SKOVALD_SAVAGE_BLADE);
                scriptEvents.addEvent(EVENT_SKOVALD_SAVAGE_BLADE, 24000);
                break;
            default:
                break;
        }
    }

private:
    enum class ArrivalStep : uint8_t
    {
        AtGate,
        Walking,
        Arrived
    };

    ArrivalStep mArrival = ArrivalStep::AtGate;
    uint32_t mArrivalMs = 1000;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Odyn, watches until Skovald is beaten

class HallsOfValorOdynAI : public HallsOfValorBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new HallsOfValorOdynAI(c); }
    explicit HallsOfValorOdynAI(Creature* pCreature) : HallsOfValorBossAI(pCreature, DATA_ODYN) {}

    void OnLoad() override
    {
        mTrialOpen = false;
        mWatchMs = 2000;
        seal(true);
    }

protected:
    void onIdleTick(uint32_t diff) override
    {
        if (mTrialOpen || !countDown(mWatchMs, diff))
            return;

        mWatchMs = 2000;

        InstanceScript* pInstance = getInstanceScript();
        if (pInstance != nullptr && pInstance->getBossState(DATA_GOD_KING_SKOVALD) != EncounterStates::Performed)
            return;

        mTrialOpen = true;
        getCreature()->setFaction(HallsOfValorHostileFaction);
        seal(false);
        sendChatMessage(CHAT_MSG_RAID_BOSS_EMOTE, 0, "Odyn descends to test the champions himself!");
    }

    void onEngage() override
    {
        scriptEvents.addEvent(EVENT_ODYN_SPEAR_OF_LIGHT, 8000);
        scriptEvents.addEvent(EVENT_ODYN_RADIANT_TEMPEST, 24000);
        scriptEvents.addEvent(EVENT_ODYN_SHATTER_SPEARS, 40000);
        scriptEvents.addEvent(EVENT_ODYN_RUNIC_BRAND, 44000);
        if (isHeroic())
            scriptEvents.addEvent(EVENT_ODYN_STORMFORGED, 18000);
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_ODYN_SPEAR_OF_LIGHT:
                castSpellOnSelf(SPELL_ODYN_SPEAR_OF_LIGHT);
                scriptEvents.addEvent(EVENT_ODYN_SPEAR_OF_LIGHT, 10000);
                break;
            case EVENT_ODYN_RADIANT_TEMPEST:
                sendChatMessage(CHAT_MSG_RAID_BOSS_EMOTE, 0, "Odyn begins to cast Radiant Tempest!");
                castSpellAOE(SPELL_ODYN_RADIANT_TEMPEST);
                scriptEvents.addEvent(EVENT_ODYN_RADIANT_TEMPEST, 48000);
                break;
            case EVENT_ODYN_SHATTER_SPEARS:
                castSpellAOE(SPELL_ODYN_SHATTER_SPEARS);
                scriptEvents.addEvent(EVENT_ODYN_SHATTER_SPEARS, 56000);
                break;
            case EVENT_ODYN_RUNIC_BRAND:
                brandChampions();
                scriptEvents.addEvent(EVENT_ODYN_RUNIC_BRAND, 56000);
                break;
            case EVENT_ODYN_STORMFORGED:
                callAdd(NPC_STORMFORGED_OBLITERATOR, OdynHallCentre);
                scriptEvents.addEvent(EVENT_ODYN_STORMFORGED, 52000);
                break;
            default:
                break;
        }
    }

private:
    // every champion gets a different rune colour
    void brandChampions()
    {
        sendChatMessage(CHAT_MSG_RAID_BOSS_EMOTE, 0, "Odyn brands the champions with runes!");
        castSpellAOE(SPELL_ODYN_RUNIC_BRAND);

        uint32_t brand = 0;
        for (Unit* pPlayer : playersAround(80.0f))
        {
            if (brand >= OdynRunicBrandCount)
                break;

            castSpell(pPlayer, OdynRunicBrands[brand], true);
            ++brand;
        }
    }

    bool mTrialOpen = false;
    uint32_t mWatchMs = 2000;
};

void SetupHallsOfValor(ScriptMgr* mgr)
{
    mgr->register_instance_script(MAP_HALLS_OF_VALOR, &HallsOfValorInstanceScript::Create);

    mgr->register_creature_script(BOSS_HYMDALL, &HallsOfValorHymdallAI::Create);

    mgr->register_creature_script(BOSS_HYRJA, &HallsOfValorHyrjaAI::Create);
    mgr->register_creature_script(NPC_OLMYR_THE_ENLIGHTENED, &HallsOfValorDefenderAI::Create);
    mgr->register_creature_script(NPC_SOLSTEN, &HallsOfValorDefenderAI::Create);

    mgr->register_creature_script(BOSS_FENRYR, &HallsOfValorFenryrAI::Create);
    mgr->register_creature_script(BOSS_FENRYR_DEN, &HallsOfValorFenryrAI::Create);

    for (uint32_t king : OdynKings)
        mgr->register_creature_script(king, &HallsOfValorKingAI::Create);
    mgr->register_creature_script(BOSS_GOD_KING_SKOVALD, &HallsOfValorSkovaldAI::Create);

    mgr->register_creature_script(BOSS_ODYN, &HallsOfValorOdynAI::Create);
}
