/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "Instance_The_Arcway.hpp"

#include "Objects/GameObject.h"
#include "Objects/Units/Creatures/Creature.h"
#include "Server/Script/CreatureAIScript.hpp"
#include "Server/Script/InstanceScript.hpp"
#include "Server/Script/ScriptMgr.hpp"
#include "Spell/Definitions/PowerType.hpp"
#include "Utilities/Random.hpp"

#include <cmath>
#include <vector>

enum TheArcwayEvents
{
    EVENT_ARCWAY_CORSTILAX_PROTOCOL = 1,
    EVENT_ARCWAY_CORSTILAX_PROTOCOL_RELEASE,
    EVENT_ARCWAY_CORSTILAX_QUARANTINE,
    EVENT_ARCWAY_CORSTILAX_CLEANSING_FORCE,
    EVENT_ARCWAY_CORSTILAX_DESTABILIZED_ORB,

    EVENT_ARCWAY_NALTIRA_BLINK_STRIKES,
    EVENT_ARCWAY_NALTIRA_SECOND_BLINK,
    EVENT_ARCWAY_NALTIRA_NETHER_VENOM,
    EVENT_ARCWAY_NALTIRA_TANGLED_WEB,
    EVENT_ARCWAY_NALTIRA_MANAFANG,

    EVENT_ARCWAY_IVANYR_ARCANE_BLAST,
    EVENT_ARCWAY_IVANYR_VOLATILE_MAGIC,
    EVENT_ARCWAY_IVANYR_OVERCHARGE_MANA,
    EVENT_ARCWAY_IVANYR_NETHER_LINK,
    EVENT_ARCWAY_IVANYR_CONSUME_ESSENCE,

    EVENT_ARCWAY_XAKAL_FEL_FISSURE,
    EVENT_ARCWAY_XAKAL_SHADOW_SLASH,
    EVENT_ARCWAY_XAKAL_WICKED_SLAM,
    EVENT_ARCWAY_XAKAL_FELBAT,

    EVENT_ARCWAY_FELBAT_BOMBARDMENT,
    EVENT_ARCWAY_FELBAT_THIRST_FOR_BLOOD,

    EVENT_ARCWAY_VANDROS_ACCELERATING_BLAST,
    EVENT_ARCWAY_VANDROS_CHRONO_SHARDS,
    EVENT_ARCWAY_VANDROS_FORCE_BOMB,
    EVENT_ARCWAY_VANDROS_UNSTABLE_MANA
};

namespace
{
    // a Chrono Shard splits time this long after it formed
    uint32_t const ChronoShardFuse = 8000;

    LocationVector offsetFrom(LocationVector const& origin, float minRange, float maxRange)
    {
        float const angle = Util::getRandomFloat(0.0f, 6.283f);
        float const range = Util::getRandomFloat(minRange, maxRange);
        return LocationVector(origin.x + range * std::cos(angle), origin.y + range * std::sin(angle), origin.z, origin.o);
    }
}

class TheArcwayInstanceScript : public InstanceScript
{
public:
    explicit TheArcwayInstanceScript(WorldMap* pMapMgr) : InstanceScript(pMapMgr)
    {
        setBossNumber(TheArcwayEncounterCount);
    }

    static InstanceScript* Create(WorldMap* pMapMgr) { return new TheArcwayInstanceScript(pMapMgr); }

    void OnGameObjectPushToWorld(GameObject* pGameObject) override
    {
        if (gateKeeper(pGameObject->getEntry()) == TheArcwayEncounterCount)
            return;

        mDoorGuids.push_back(pGameObject->GetNewGUID());
        alignDoor(pGameObject);
    }

    void OnEncounterStateChange(uint32_t /*id*/, uint32_t /*state*/) override
    {
        for (WoWGuid const& guid : mDoorGuids)
        {
            if (GameObject* pDoor = getGameObjectByGuid(guid))
                alignDoor(pDoor);
        }
    }

private:
    // encounter whose defeat opens the passage, encounter count when the object is no door of ours
    static uint32_t gateKeeper(uint32_t entry)
    {
        switch (entry)
        {
            case GO_ARCWAY_DOOR_D_BOTTOM:
            case GO_ARCWAY_DOOR_G:
            case GO_ARCWAY_DOOR_I:
                return DATA_IVANYR;
            case GO_ARCWAY_DOOR_J:
            case GO_ARCWAY_DOOR_L:
                return DATA_CORSTILAX;
            case GO_ARCWAY_DOOR_A:
            case GO_ARCWAY_DOOR_B:
            case GO_ARCWAY_DOOR_K:
                return DATA_NALTIRA;
            default:
                return TheArcwayEncounterCount;
        }
    }

    void alignDoor(GameObject* pDoor)
    {
        bool const open = getBossState(gateKeeper(pDoor->getEntry())) == EncounterStates::Performed;
        pDoor->setState(open ? GO_STATE_OPEN : GO_STATE_CLOSED);
    }

    std::vector<WoWGuid> mDoorGuids;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Shared fighter base: event clock while engaged

class TheArcwayFighterAI : public CreatureAIScript
{
public:
    explicit TheArcwayFighterAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

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

        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);
        onFightTick();

        if (_isCasting())
            return;

        if (uint32_t const eventId = scriptEvents.getFinishedEvent())
            onEvent(eventId);
    }

protected:
    virtual void onPull() {}
    virtual void onFightTick() {}
    virtual void onEvent(uint32_t /*eventId*/) {}

    Unit* anyPlayer() { return selectUnitTarget(FilterArgs(TargetFilter_Player)); }

    // prefers someone other than the tank
    Unit* anyPlayerButTank()
    {
        if (Unit* pTarget = selectUnitTarget(FilterArgs(TargetFilter(TargetFilter_Player | TargetFilter_NotCurrent))))
            return pTarget;

        return anyPlayer();
    }

    void castAtAnyPlayer(uint32_t spellId, bool triggered = false)
    {
        if (Unit* pTarget = anyPlayer())
            castSpell(pTarget, spellId, triggered);
    }

    Creature* bringAdd(uint32_t entry, LocationVector const& spot, CreatureSummonDespawnType despawnType = CORPSE_DESPAWN, uint32_t duration = 0)
    {
        Creature* pAdd = summonCreature(entry, spot, despawnType, duration);
        if (pAdd != nullptr)
            setZoneWideCombat(pAdd);

        return pAdd;
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Encounter base: frame and boss state

class TheArcwayBossAI : public TheArcwayFighterAI
{
public:
    TheArcwayBossAI(Creature* pCreature, uint32_t encounterId) : TheArcwayFighterAI(pCreature), mEncounterId(encounterId) {}

    void OnCombatStart(Unit* pTarget) override
    {
        if (InstanceScript* pInstance = getInstanceScript())
        {
            pInstance->sendUnitEncounter(EncounterFrameEngage, getCreature());
            pInstance->setBossState(mEncounterId, EncounterStates::InProgress);
        }

        TheArcwayFighterAI::OnCombatStart(pTarget);
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
        TheArcwayFighterAI::OnCombatStop(pTarget);
    }

    void OnDied(Unit* /*pKiller*/) override
    {
        if (InstanceScript* pInstance = getInstanceScript())
        {
            pInstance->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
            pInstance->setBossState(mEncounterId, EncounterStates::Performed);
        }

        onDefeated();
    }

protected:
    virtual void onReset() {}
    virtual void onDefeated() {}

    uint32_t mEncounterId;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Corstilax

class TheArcwayCorstilaxAI : public TheArcwayBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new TheArcwayCorstilaxAI(c); }
    explicit TheArcwayCorstilaxAI(Creature* pCreature) : TheArcwayBossAI(pCreature, DATA_CORSTILAX) {}

protected:
    void onPull() override
    {
        scriptEvents.addEvent(EVENT_ARCWAY_CORSTILAX_PROTOCOL, 5000);
        scriptEvents.addEvent(EVENT_ARCWAY_CORSTILAX_DESTABILIZED_ORB, 13000);
        scriptEvents.addEvent(EVENT_ARCWAY_CORSTILAX_QUARANTINE, 20000);
        scriptEvents.addEvent(EVENT_ARCWAY_CORSTILAX_CLEANSING_FORCE, 28000);
    }

    void onReset() override
    {
        setRooted(false);
        _removeAuraOnPlayers(SPELL_CORSTILAX_NIGHTWELL_ENERGY);
    }

    void onDefeated() override { _removeAuraOnPlayers(SPELL_CORSTILAX_NIGHTWELL_ENERGY); }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_ARCWAY_CORSTILAX_PROTOCOL:
                runSuppressionProtocol();
                scriptEvents.addEvent(EVENT_ARCWAY_CORSTILAX_PROTOCOL, 36000);
                break;
            case EVENT_ARCWAY_CORSTILAX_PROTOCOL_RELEASE:
                setRooted(false);
                break;
            case EVENT_ARCWAY_CORSTILAX_QUARANTINE:
                castSpellAOE(SPELL_CORSTILAX_QUARANTINE);
                scriptEvents.addEvent(EVENT_ARCWAY_CORSTILAX_QUARANTINE, 32000);
                break;
            case EVENT_ARCWAY_CORSTILAX_CLEANSING_FORCE:
                sendChatMessage(CHAT_MSG_RAID_BOSS_EMOTE, 0, "Corstilax begins to channel Cleansing Force!");
                castSpellOnSelf(SPELL_CORSTILAX_CLEANSING_FORCE);
                scriptEvents.addEvent(EVENT_ARCWAY_CORSTILAX_CLEANSING_FORCE, 32000);
                break;
            case EVENT_ARCWAY_CORSTILAX_DESTABILIZED_ORB:
                if (Unit* pTarget = anyPlayer())
                {
                    castSpell(pTarget, SPELL_CORSTILAX_DESTABILIZED_ORB);
                    summonCreature(NPC_DESTABILIZED_ORB, pTarget->GetPosition(), TIMED_DESPAWN, 60000);
                }
                scriptEvents.addEvent(EVENT_ARCWAY_CORSTILAX_DESTABILIZED_ORB, 32000);
                break;
            default:
                break;
        }
    }

private:
    // a protocol drone hunts one player while Corstilax holds still and powers it
    void runSuppressionProtocol()
    {
        Unit* pTarget = anyPlayerButTank();
        if (pTarget == nullptr)
            return;

        castSpell(pTarget, SPELL_CORSTILAX_SUPPRESSION_PROTOCOL);

        Creature* pProtocol = summonCreature(NPC_SUPPRESSION_PROTOCOL, pTarget->GetPosition(), TIMED_DESPAWN, 12000);
        if (pProtocol == nullptr)
            return;

        pProtocol->castSpell(pTarget, SPELL_CORSTILAX_PROTOCOL_FIXATE, true);
        addThreat(pTarget, 1000000.0f, pProtocol);
        if (CreatureAIScript* pDrone = pProtocol->GetScript())
            pDrone->attackStart(pTarget);

        setRooted(true);
        castSpell(pProtocol, SPELL_CORSTILAX_PROTOCOL_BEAM, true);
        scriptEvents.addEvent(EVENT_ARCWAY_CORSTILAX_PROTOCOL_RELEASE, 6000);
    }
};

class TheArcwaySuppressionProtocolAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new TheArcwaySuppressionProtocolAI(c); }
    explicit TheArcwaySuppressionProtocolAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnLoad() override { _setMeleeDisabled(true); }

    // the drone cannot be destroyed
    void DamageTaken(Unit* /*pAttacker*/, uint32_t* damage) override { *damage = 0; }
};

class TheArcwayDestabilizedOrbAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new TheArcwayDestabilizedOrbAI(c); }
    explicit TheArcwayDestabilizedOrbAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnLoad() override
    {
        setReactState(REACT_PASSIVE);
        setRooted(true);
        _setMeleeDisabled(true);
        addUnitFlags(UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
        castSpellOnSelf(SPELL_DESTABILIZED_ORB_AURA, true);
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Nal'tira

class TheArcwayNaltiraAI : public TheArcwayBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new TheArcwayNaltiraAI(c); }
    explicit TheArcwayNaltiraAI(Creature* pCreature) : TheArcwayBossAI(pCreature, DATA_NALTIRA) {}

protected:
    void onPull() override
    {
        scriptEvents.addEvent(EVENT_ARCWAY_NALTIRA_BLINK_STRIKES, 15000);
        scriptEvents.addEvent(EVENT_ARCWAY_NALTIRA_NETHER_VENOM, 26000);
        scriptEvents.addEvent(EVENT_ARCWAY_NALTIRA_TANGLED_WEB, 32000);

        if (isHeroic())
            scriptEvents.addEvent(EVENT_ARCWAY_NALTIRA_MANAFANG, 25000);
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_ARCWAY_NALTIRA_BLINK_STRIKES:
                castSpellOnSelf(SPELL_NALTIRA_BLINK_STRIKES_VISUAL, true);
                blinkBehind(anyPlayerButTank());
                scriptEvents.addEvent(EVENT_ARCWAY_NALTIRA_SECOND_BLINK, 3000);
                scriptEvents.addEvent(EVENT_ARCWAY_NALTIRA_BLINK_STRIKES, 30000);
                break;
            case EVENT_ARCWAY_NALTIRA_SECOND_BLINK:
                blinkBehind(anyPlayerButTank());
                break;
            case EVENT_ARCWAY_NALTIRA_NETHER_VENOM:
                castSpellAOE(SPELL_NALTIRA_NETHER_VENOM);
                scriptEvents.addEvent(EVENT_ARCWAY_NALTIRA_NETHER_VENOM, 30000);
                break;
            case EVENT_ARCWAY_NALTIRA_TANGLED_WEB:
                sendChatMessage(CHAT_MSG_RAID_BOSS_EMOTE, 0, "Nal'tira binds her prey with a Tangled Web!");
                castSpellAOE(SPELL_NALTIRA_TANGLED_WEB);
                scriptEvents.addEvent(EVENT_ARCWAY_NALTIRA_TANGLED_WEB, 22000);
                break;
            case EVENT_ARCWAY_NALTIRA_MANAFANG:
                bringAdd(NPC_VICIOUS_MANAFANG, offsetFrom(getCreature()->GetPosition(), 10.0f, 30.0f));
                scriptEvents.addEvent(EVENT_ARCWAY_NALTIRA_MANAFANG, 22000);
                break;
            default:
                break;
        }
    }

private:
    void blinkBehind(Unit* pTarget)
    {
        if (pTarget == nullptr)
            return;

        castSpell(pTarget, SPELL_NALTIRA_BLINK_STRIKES_JUMP, true);
        castSpellOnSelf(SPELL_NALTIRA_BLINK_STRIKES_HIT, true);
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Ivanyr

class TheArcwayIvanyrAI : public TheArcwayBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new TheArcwayIvanyrAI(c); }
    explicit TheArcwayIvanyrAI(Creature* pCreature) : TheArcwayBossAI(pCreature, DATA_IVANYR) {}

protected:
    void onPull() override
    {
        mStarving = false;
        scriptEvents.addEvent(EVENT_ARCWAY_IVANYR_ARCANE_BLAST, 2000);
        scriptEvents.addEvent(EVENT_ARCWAY_IVANYR_VOLATILE_MAGIC, 8000);
        scriptEvents.addEvent(EVENT_ARCWAY_IVANYR_OVERCHARGE_MANA, 28000);
        scriptEvents.addEvent(EVENT_ARCWAY_IVANYR_NETHER_LINK, 56000);
    }

    void onFightTick() override
    {
        // drained of mana he turns on the players' essence
        if (mStarving)
            return;

        Creature* pSelf = getCreature();
        if (pSelf->getMaxPower(POWER_TYPE_MANA) == 0 || pSelf->getPower(POWER_TYPE_MANA) > 0)
            return;

        mStarving = true;
        scriptEvents.resetEvents();
        sendChatMessage(CHAT_MSG_RAID_BOSS_EMOTE, 0, "Ivanyr is starved of mana and begins Withering Consumption!");
        castSpellOnSelf(SPELL_IVANYR_WITHERING_CONSUMPTION, true);
        scriptEvents.addEvent(EVENT_ARCWAY_IVANYR_CONSUME_ESSENCE, 8000);
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_ARCWAY_IVANYR_ARCANE_BLAST:
                castSpellOnVictim(SPELL_IVANYR_ARCANE_BLAST);
                scriptEvents.addEvent(EVENT_ARCWAY_IVANYR_ARCANE_BLAST, 2500);
                break;
            case EVENT_ARCWAY_IVANYR_VOLATILE_MAGIC:
                sendChatMessage(CHAT_MSG_RAID_BOSS_EMOTE, 0, "Ivanyr charges his foes with Volatile Magic!");
                castSpellAOE(SPELL_IVANYR_VOLATILE_MAGIC);
                scriptEvents.addEvent(EVENT_ARCWAY_IVANYR_VOLATILE_MAGIC, 24000);
                break;
            case EVENT_ARCWAY_IVANYR_OVERCHARGE_MANA:
                if (Creature* pCrystal = findNearestCreature(NPC_NIGHT_CRYSTAL, 100.0f))
                    castSpell(pCrystal, SPELL_IVANYR_OVERCHARGE_MANA);
                else
                    castSpellAOE(SPELL_IVANYR_OVERCHARGE_MANA);
                scriptEvents.addEvent(EVENT_ARCWAY_IVANYR_OVERCHARGE_MANA, 40000);
                break;
            case EVENT_ARCWAY_IVANYR_NETHER_LINK:
                castSpellAOE(SPELL_IVANYR_NETHER_LINK);
                scriptEvents.addEvent(EVENT_ARCWAY_IVANYR_NETHER_LINK, 56000);
                break;
            case EVENT_ARCWAY_IVANYR_CONSUME_ESSENCE:
                castSpellOnSelf(SPELL_IVANYR_CONSUME_ESSENCE);
                scriptEvents.addEvent(EVENT_ARCWAY_IVANYR_CONSUME_ESSENCE, 16000);
                break;
            default:
                break;
        }
    }

private:
    bool mStarving = false;
};

//////////////////////////////////////////////////////////////////////////////////////////
// General Xakal

class TheArcwayXakalAI : public TheArcwayBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new TheArcwayXakalAI(c); }
    explicit TheArcwayXakalAI(Creature* pCreature) : TheArcwayBossAI(pCreature, DATA_GENERAL_XAKAL) {}

protected:
    void onPull() override
    {
        scriptEvents.addEvent(EVENT_ARCWAY_XAKAL_FEL_FISSURE, 6000);
        scriptEvents.addEvent(EVENT_ARCWAY_XAKAL_FELBAT, 11000);
        scriptEvents.addEvent(EVENT_ARCWAY_XAKAL_SHADOW_SLASH, 13000);
        scriptEvents.addEvent(EVENT_ARCWAY_XAKAL_WICKED_SLAM, 40000);
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_ARCWAY_XAKAL_FEL_FISSURE:
                castSpellAOE(SPELL_XAKAL_FEL_FISSURE);
                if (Unit* pTarget = anyPlayer())
                    summonCreature(NPC_FEL_FISSURE, pTarget->GetPosition());
                scriptEvents.addEvent(EVENT_ARCWAY_XAKAL_FEL_FISSURE, 14000);
                break;
            case EVENT_ARCWAY_XAKAL_SHADOW_SLASH:
                castSpellOnVictim(SPELL_XAKAL_SHADOW_SLASH);
                scriptEvents.addEvent(EVENT_ARCWAY_XAKAL_SHADOW_SLASH, 14000);
                break;
            case EVENT_ARCWAY_XAKAL_WICKED_SLAM:
                sendChatMessage(CHAT_MSG_RAID_BOSS_EMOTE, 0, "General Xakal prepares a Wicked Slam!");
                castSpellAOE(SPELL_XAKAL_WICKED_SLAM);
                scriptEvents.addEvent(EVENT_ARCWAY_XAKAL_WICKED_SLAM, 40000);
                break;
            case EVENT_ARCWAY_XAKAL_FELBAT:
                bringAdd(NPC_DREAD_FELBAT, offsetFrom(getCreature()->GetPosition(), 10.0f, 30.0f));
                scriptEvents.addEvent(EVENT_ARCWAY_XAKAL_FELBAT, 22000);
                break;
            default:
                break;
        }
    }
};

class TheArcwayFelFissureAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new TheArcwayFelFissureAI(c); }
    explicit TheArcwayFelFissureAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnLoad() override
    {
        setReactState(REACT_PASSIVE);
        setRooted(true);
        _setMeleeDisabled(true);
        addUnitFlags(UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
        castSpellOnSelf(SPELL_FEL_FISSURE_AREA, true);
    }
};

class TheArcwayDreadFelbatAI : public TheArcwayFighterAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new TheArcwayDreadFelbatAI(c); }
    explicit TheArcwayDreadFelbatAI(Creature* pCreature) : TheArcwayFighterAI(pCreature) {}

protected:
    void onPull() override
    {
        scriptEvents.addEvent(EVENT_ARCWAY_FELBAT_BOMBARDMENT, 2000);
        scriptEvents.addEvent(EVENT_ARCWAY_FELBAT_THIRST_FOR_BLOOD, 7000);
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_ARCWAY_FELBAT_BOMBARDMENT:
                castAtAnyPlayer(SPELL_FELBAT_BOMBARDMENT, true);
                scriptEvents.addEvent(EVENT_ARCWAY_FELBAT_BOMBARDMENT, 3000);
                break;
            case EVENT_ARCWAY_FELBAT_THIRST_FOR_BLOOD:
                castSpellOnVictim(SPELL_FELBAT_THIRST_FOR_BLOOD);
                scriptEvents.addEvent(EVENT_ARCWAY_FELBAT_THIRST_FOR_BLOOD, 5000);
                break;
            default:
                break;
        }
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Advisor Vandros

class TheArcwayVandrosAI : public TheArcwayBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new TheArcwayVandrosAI(c); }
    explicit TheArcwayVandrosAI(Creature* pCreature) : TheArcwayBossAI(pCreature, DATA_ADVISOR_VANDROS) {}

protected:
    void onPull() override
    {
        mTimeUnravelled = false;
        scriptEvents.addEvent(EVENT_ARCWAY_VANDROS_ACCELERATING_BLAST, 3000);
        scriptEvents.addEvent(EVENT_ARCWAY_VANDROS_CHRONO_SHARDS, 6000);
        scriptEvents.addEvent(EVENT_ARCWAY_VANDROS_FORCE_BOMB, 16000);
    }

    void onFightTick() override
    {
        // second half adds Unstable Mana
        if (mTimeUnravelled || _getHealthPercent() > 50)
            return;

        mTimeUnravelled = true;
        sendChatMessage(CHAT_MSG_RAID_BOSS_EMOTE, 0, "Advisor Vandros tears at the flow of time!");
        scriptEvents.addEvent(EVENT_ARCWAY_VANDROS_UNSTABLE_MANA, 3000);
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_ARCWAY_VANDROS_ACCELERATING_BLAST:
                castSpellOnVictim(SPELL_VANDROS_ACCELERATING_BLAST);
                scriptEvents.addEvent(EVENT_ARCWAY_VANDROS_ACCELERATING_BLAST, 6000);
                break;
            case EVENT_ARCWAY_VANDROS_CHRONO_SHARDS:
                castSpellAOE(SPELL_VANDROS_CHRONO_SHARDS);
                scriptEvents.addEvent(EVENT_ARCWAY_VANDROS_CHRONO_SHARDS, 6000);
                break;
            case EVENT_ARCWAY_VANDROS_FORCE_BOMB:
                sendChatMessage(CHAT_MSG_RAID_BOSS_EMOTE, 0, "Advisor Vandros conjures a Force Bomb!");
                castSpellAOE(SPELL_VANDROS_FORCE_BOMB);
                scriptEvents.addEvent(EVENT_ARCWAY_VANDROS_FORCE_BOMB, 16000);
                break;
            case EVENT_ARCWAY_VANDROS_UNSTABLE_MANA:
                castAtAnyPlayer(SPELL_VANDROS_UNSTABLE_MANA);
                scriptEvents.addEvent(EVENT_ARCWAY_VANDROS_UNSTABLE_MANA, 30000);
                break;
            default:
                break;
        }
    }

private:
    bool mTimeUnravelled = false;
};

class TheArcwayChronoShardAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new TheArcwayChronoShardAI(c); }
    explicit TheArcwayChronoShardAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnLoad() override
    {
        setReactState(REACT_PASSIVE);
        _setMeleeDisabled(true);
        castSpellOnSelf(SPELL_CHRONO_SHARD_PULSE_VISUAL, true);

        LocationVector const drift = offsetFrom(getCreature()->GetPosition(), 3.0f, 10.0f);
        moveTo(drift.x, drift.y, drift.z, false);
    }

    void AIUpdate(unsigned long time_passed) override
    {
        if (mSplit)
            return;

        mAge += static_cast<uint32_t>(time_passed);
        if (mAge < ChronoShardFuse)
            return;

        mSplit = true;
        castSpellAOE(SPELL_CHRONO_SHARD_TIME_SPLIT, true);
        despawn(1000, 0);
    }

private:
    uint32_t mAge = 0;
    bool mSplit = false;
};

void SetupTheArcway(ScriptMgr* mgr)
{
    mgr->register_instance_script(MAP_THE_ARCWAY, &TheArcwayInstanceScript::Create);

    mgr->register_creature_script(BOSS_CORSTILAX, &TheArcwayCorstilaxAI::Create);
    mgr->register_creature_script(NPC_SUPPRESSION_PROTOCOL, &TheArcwaySuppressionProtocolAI::Create);
    mgr->register_creature_script(NPC_DESTABILIZED_ORB, &TheArcwayDestabilizedOrbAI::Create);

    mgr->register_creature_script(BOSS_NALTIRA, &TheArcwayNaltiraAI::Create);

    mgr->register_creature_script(BOSS_IVANYR, &TheArcwayIvanyrAI::Create);

    mgr->register_creature_script(BOSS_GENERAL_XAKAL, &TheArcwayXakalAI::Create);
    mgr->register_creature_script(NPC_FEL_FISSURE, &TheArcwayFelFissureAI::Create);
    mgr->register_creature_script(NPC_DREAD_FELBAT, &TheArcwayDreadFelbatAI::Create);

    mgr->register_creature_script(BOSS_ADVISOR_VANDROS, &TheArcwayVandrosAI::Create);
    mgr->register_creature_script(NPC_CHRONO_SHARD, &TheArcwayChronoShardAI::Create);
}
