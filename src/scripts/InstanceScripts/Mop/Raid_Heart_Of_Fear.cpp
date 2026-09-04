/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

// Heart of Fear - six bosses, all sourced from wowhead's MoP Classic data (see header comment
// for limitations), hand-ported into AscEmu's own CreatureAIScript / InstanceScript framework.

#include "Setup.h"
#include "Raid_Heart_Of_Fear.hpp"

#include "Server/Script/CreatureAIScript.hpp"
#include "Server/Script/InstanceScript.hpp"
#include "Server/Script/ScriptMgr.hpp"

enum HeartOfFearEvents
{
    EVENT_ZORLOK_INHALE = 1,
    EVENT_ZORLOK_EXHALE,
    EVENT_ZORLOK_ATTENUATION,
    EVENT_ZORLOK_FORCE_AND_VERVE,

    EVENT_TAYAK_TEMPEST_SLASH,
    EVENT_TAYAK_UNSEEN_STRIKE,
    EVENT_TAYAK_WIND_STEP,
    EVENT_TAYAK_STORM_UNLEASHED,

    EVENT_GARALON_FURIOUS_SWIPE,
    EVENT_GARALON_FURY,
    EVENT_GARALON_CRUSH,
    EVENT_GARALON_PHEROMONES,

    EVENT_MELJARAK_WHIRLING_BLADE,
    EVENT_MELJARAK_WIND_BOMB,
    EVENT_MELJARAK_IMPALING_SPEAR,
    EVENT_MELJARAK_AMBER_PRISON,

    EVENT_UNSOK_AMBER_SCALPEL,
    EVENT_UNSOK_BURNING_AMBER,
    EVENT_UNSOK_AMBER_EXPLOSION,
    EVENT_UNSOK_MASSIVE_STOMP,

    EVENT_SHEKZEER_DISSONANCE_FIELD,
    EVENT_SHEKZEER_SONIC_DISCHARGE,
    EVENT_SHEKZEER_DREAD_SCREECH,
    EVENT_SHEKZEER_CRY_OF_TERROR,

    EVENT_ZANTHIK_GENERAL_CLEAVE,
    EVENT_ZANTHIK_GENERAL_ENRAGE,

    EVENT_SWARMBORN_SHADOW_BOLT,
    EVENT_SWARMBORN_POISON
};

class HeartOfFearInstanceScript : public InstanceScript
{
public:
    explicit HeartOfFearInstanceScript(WorldMap* pMapMgr) : InstanceScript(pMapMgr)
    {
        setBossNumber(HeartOfFearEncounterCount);
    }

    static InstanceScript* Create(WorldMap* pMapMgr) { return new HeartOfFearInstanceScript(pMapMgr); }

    void OnGameObjectPushToWorld(GameObject* pGameObject) override
    {
        if (pGameObject->getEntry() == GO_MANTID_AMBER_DOOR)
            mAmberDoorGuids.push_back(pGameObject->getGuidLow());
    }

    void OpenAmberDoor()
    {
        for (uint32_t guid : mAmberDoorGuids)
        {
            if (GameObject* pDoor = GetGameObjectByGuid(guid))
                useDoorOrButton(pDoor);
        }
    }

private:
    std::vector<uint32_t> mAmberDoorGuids;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Imperial Vizier Zor'lok

class ZorlokAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new ZorlokAI(c); }
    explicit ZorlokAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameEngage, getCreature());
        getInstanceScript()->setBossState(DATA_ZORLOK, EncounterStates::InProgress);

        scriptEvents.addEvent(EVENT_ZORLOK_INHALE, 10000);
        scriptEvents.addEvent(EVENT_ZORLOK_EXHALE, 18000);
        scriptEvents.addEvent(EVENT_ZORLOK_ATTENUATION, 6000);
        scriptEvents.addEvent(EVENT_ZORLOK_FORCE_AND_VERVE, 24000);
    }

    void OnCombatStop(Unit* /*pTarget*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
        getInstanceScript()->setBossState(DATA_ZORLOK, EncounterStates::Failed);
        scriptEvents.resetEvents();
    }

    void OnDied(Unit* /*pKiller*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
        getInstanceScript()->setBossState(DATA_ZORLOK, EncounterStates::Performed);
        static_cast<HeartOfFearInstanceScript*>(getInstanceScript())->OpenAmberDoor();
    }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);

        if (_isCasting())
            return;

        switch (scriptEvents.getFinishedEvent())
        {
            case EVENT_ZORLOK_INHALE:
                sendChatMessage(CHAT_MSG_MONSTER_EMOTE, 0, "Imperial Vizier Zor'lok inhales deeply!");
                castSpellOnSelf(SPELL_ZORLOK_INHALE);
                scriptEvents.addEvent(EVENT_ZORLOK_INHALE, 28000);
                break;
            case EVENT_ZORLOK_EXHALE:
                castSpellAOE(SPELL_ZORLOK_EXHALE);
                scriptEvents.addEvent(EVENT_ZORLOK_EXHALE, 30000);
                break;
            case EVENT_ZORLOK_ATTENUATION:
                if (Unit* pTarget = selectUnitTarget(FilterArgs(TargetFilter_Player)))
                    castSpell(pTarget, SPELL_ZORLOK_ATTENUATION);
                scriptEvents.addEvent(EVENT_ZORLOK_ATTENUATION, 20000);
                break;
            case EVENT_ZORLOK_FORCE_AND_VERVE:
                castSpellOnVictim(SPELL_ZORLOK_FORCE_AND_VERVE);
                scriptEvents.addEvent(EVENT_ZORLOK_FORCE_AND_VERVE, 22000);
                break;
            default:
                break;
        }
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Blade Lord Ta'yak

class TayakAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new TayakAI(c); }
    explicit TayakAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameEngage, getCreature());
        getInstanceScript()->setBossState(DATA_TAYAK, EncounterStates::InProgress);

        scriptEvents.addEvent(EVENT_TAYAK_TEMPEST_SLASH, 8000);
        scriptEvents.addEvent(EVENT_TAYAK_UNSEEN_STRIKE, 14000);
        scriptEvents.addEvent(EVENT_TAYAK_WIND_STEP, 20000);
        scriptEvents.addEvent(EVENT_TAYAK_STORM_UNLEASHED, 26000);
    }

    void OnCombatStop(Unit* /*pTarget*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
        getInstanceScript()->setBossState(DATA_TAYAK, EncounterStates::Failed);
        scriptEvents.resetEvents();
    }

    void OnDied(Unit* /*pKiller*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
        getInstanceScript()->setBossState(DATA_TAYAK, EncounterStates::Performed);
    }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);

        if (_isCasting())
            return;

        switch (scriptEvents.getFinishedEvent())
        {
            case EVENT_TAYAK_TEMPEST_SLASH:
                castSpellOnVictim(SPELL_TAYAK_TEMPEST_SLASH);
                scriptEvents.addEvent(EVENT_TAYAK_TEMPEST_SLASH, 16000);
                break;
            case EVENT_TAYAK_UNSEEN_STRIKE:
                if (Unit* pTarget = selectUnitTarget(FilterArgs(TargetFilter_Player)))
                    castSpell(pTarget, SPELL_TAYAK_UNSEEN_STRIKE);
                scriptEvents.addEvent(EVENT_TAYAK_UNSEEN_STRIKE, 22000);
                break;
            case EVENT_TAYAK_WIND_STEP:
                castSpellOnSelf(SPELL_TAYAK_WIND_STEP);
                scriptEvents.addEvent(EVENT_TAYAK_WIND_STEP, 24000);
                break;
            case EVENT_TAYAK_STORM_UNLEASHED:
                castSpellAOE(SPELL_TAYAK_STORM_UNLEASHED);
                scriptEvents.addEvent(EVENT_TAYAK_STORM_UNLEASHED, 28000);
                break;
            default:
                break;
        }
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Garalon

class GaralonAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new GaralonAI(c); }
    explicit GaralonAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameEngage, getCreature());
        getInstanceScript()->setBossState(DATA_GARALON, EncounterStates::InProgress);

        scriptEvents.addEvent(EVENT_GARALON_FURIOUS_SWIPE, 6000);
        scriptEvents.addEvent(EVENT_GARALON_FURY, 16000);
        scriptEvents.addEvent(EVENT_GARALON_CRUSH, 10000);
        scriptEvents.addEvent(EVENT_GARALON_PHEROMONES, 24000);
    }

    void OnCombatStop(Unit* /*pTarget*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
        getInstanceScript()->setBossState(DATA_GARALON, EncounterStates::Failed);
        scriptEvents.resetEvents();
    }

    void OnDied(Unit* /*pKiller*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
        getInstanceScript()->setBossState(DATA_GARALON, EncounterStates::Performed);
    }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);

        if (_isCasting())
            return;

        switch (scriptEvents.getFinishedEvent())
        {
            case EVENT_GARALON_FURIOUS_SWIPE:
                castSpellOnVictim(SPELL_GARALON_FURIOUS_SWIPE);
                scriptEvents.addEvent(EVENT_GARALON_FURIOUS_SWIPE, 16000);
                break;
            case EVENT_GARALON_FURY:
                castSpellOnSelf(SPELL_GARALON_FURY);
                scriptEvents.addEvent(EVENT_GARALON_FURY, 26000);
                break;
            case EVENT_GARALON_CRUSH:
                castSpellOnVictim(SPELL_GARALON_CRUSH);
                scriptEvents.addEvent(EVENT_GARALON_CRUSH, 14000);
                break;
            case EVENT_GARALON_PHEROMONES:
                castSpellAOE(SPELL_GARALON_PHEROMONES);
                scriptEvents.addEvent(EVENT_GARALON_PHEROMONES, 28000);
                break;
            default:
                break;
        }
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Wind Lord Mel'jarak

class MeljarakAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new MeljarakAI(c); }
    explicit MeljarakAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameEngage, getCreature());
        getInstanceScript()->setBossState(DATA_MELJARAK, EncounterStates::InProgress);

        scriptEvents.addEvent(EVENT_MELJARAK_WHIRLING_BLADE, 8000);
        scriptEvents.addEvent(EVENT_MELJARAK_WIND_BOMB, 14000);
        scriptEvents.addEvent(EVENT_MELJARAK_IMPALING_SPEAR, 20000);
        scriptEvents.addEvent(EVENT_MELJARAK_AMBER_PRISON, 26000);
    }

    void OnCombatStop(Unit* /*pTarget*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
        getInstanceScript()->setBossState(DATA_MELJARAK, EncounterStates::Failed);
        scriptEvents.resetEvents();
    }

    void OnDied(Unit* /*pKiller*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
        getInstanceScript()->setBossState(DATA_MELJARAK, EncounterStates::Performed);
    }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);

        if (_isCasting())
            return;

        switch (scriptEvents.getFinishedEvent())
        {
            case EVENT_MELJARAK_WHIRLING_BLADE:
                castSpellOnVictim(SPELL_MELJARAK_WHIRLING_BLADE);
                scriptEvents.addEvent(EVENT_MELJARAK_WHIRLING_BLADE, 18000);
                break;
            case EVENT_MELJARAK_WIND_BOMB:
                if (Unit* pTarget = selectUnitTarget(FilterArgs(TargetFilter_Player)))
                    castSpell(pTarget, SPELL_MELJARAK_WIND_BOMB);
                scriptEvents.addEvent(EVENT_MELJARAK_WIND_BOMB, 24000);
                break;
            case EVENT_MELJARAK_IMPALING_SPEAR:
                castSpellOnVictim(SPELL_MELJARAK_IMPALING_SPEAR);
                scriptEvents.addEvent(EVENT_MELJARAK_IMPALING_SPEAR, 20000);
                break;
            case EVENT_MELJARAK_AMBER_PRISON:
                if (Unit* pTarget = selectUnitTarget(FilterArgs(TargetFilter_Player)))
                    castSpell(pTarget, SPELL_MELJARAK_AMBER_PRISON);
                scriptEvents.addEvent(EVENT_MELJARAK_AMBER_PRISON, 28000);
                break;
            default:
                break;
        }
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Amber-Shaper Un'sok

class UnsokAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new UnsokAI(c); }
    explicit UnsokAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameEngage, getCreature());
        getInstanceScript()->setBossState(DATA_UNSOK, EncounterStates::InProgress);

        scriptEvents.addEvent(EVENT_UNSOK_AMBER_SCALPEL, 5000);
        scriptEvents.addEvent(EVENT_UNSOK_BURNING_AMBER, 12000);
        scriptEvents.addEvent(EVENT_UNSOK_AMBER_EXPLOSION, 18000);
        scriptEvents.addEvent(EVENT_UNSOK_MASSIVE_STOMP, 24000);
    }

    void OnCombatStop(Unit* /*pTarget*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
        getInstanceScript()->setBossState(DATA_UNSOK, EncounterStates::Failed);
        scriptEvents.resetEvents();
    }

    void OnDied(Unit* /*pKiller*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
        getInstanceScript()->setBossState(DATA_UNSOK, EncounterStates::Performed);
    }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);

        if (_isCasting())
            return;

        switch (scriptEvents.getFinishedEvent())
        {
            case EVENT_UNSOK_AMBER_SCALPEL:
                castSpellOnVictim(SPELL_UNSOK_AMBER_SCALPEL);
                scriptEvents.addEvent(EVENT_UNSOK_AMBER_SCALPEL, 16000);
                break;
            case EVENT_UNSOK_BURNING_AMBER:
                if (Unit* pTarget = selectUnitTarget(FilterArgs(TargetFilter_Player)))
                    castSpell(pTarget, SPELL_UNSOK_BURNING_AMBER);
                scriptEvents.addEvent(EVENT_UNSOK_BURNING_AMBER, 22000);
                break;
            case EVENT_UNSOK_AMBER_EXPLOSION:
                castSpellAOE(SPELL_UNSOK_AMBER_EXPLOSION);
                scriptEvents.addEvent(EVENT_UNSOK_AMBER_EXPLOSION, 26000);
                break;
            case EVENT_UNSOK_MASSIVE_STOMP:
                castSpellAOE(SPELL_UNSOK_MASSIVE_STOMP);
                scriptEvents.addEvent(EVENT_UNSOK_MASSIVE_STOMP, 24000);
                break;
            default:
                break;
        }
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Grand Empress Shek'zeer

class ShekzeerAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new ShekzeerAI(c); }
    explicit ShekzeerAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        sendChatMessage(CHAT_MSG_MONSTER_YELL, 0, "The swarm answers to me alone!");
        getInstanceScript()->sendUnitEncounter(EncounterFrameEngage, getCreature());
        getInstanceScript()->setBossState(DATA_SHEKZEER, EncounterStates::InProgress);

        scriptEvents.addEvent(EVENT_SHEKZEER_DISSONANCE_FIELD, 8000);
        scriptEvents.addEvent(EVENT_SHEKZEER_SONIC_DISCHARGE, 6000);
        scriptEvents.addEvent(EVENT_SHEKZEER_DREAD_SCREECH, 20000);
        scriptEvents.addEvent(EVENT_SHEKZEER_CRY_OF_TERROR, 14000);
    }

    void OnCombatStop(Unit* /*pTarget*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
        getInstanceScript()->setBossState(DATA_SHEKZEER, EncounterStates::Failed);
        scriptEvents.resetEvents();
    }

    void OnDied(Unit* /*pKiller*/) override
    {
        sendChatMessage(CHAT_MSG_MONSTER_YELL, 0, "The Empress... falls...");
        getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
        getInstanceScript()->setBossState(DATA_SHEKZEER, EncounterStates::Performed);
    }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);

        if (_isCasting())
            return;

        switch (scriptEvents.getFinishedEvent())
        {
            case EVENT_SHEKZEER_DISSONANCE_FIELD:
                castSpellAOE(SPELL_SHEKZEER_DISSONANCE_FIELD);
                scriptEvents.addEvent(EVENT_SHEKZEER_DISSONANCE_FIELD, 26000);
                break;
            case EVENT_SHEKZEER_SONIC_DISCHARGE:
                castSpellOnVictim(SPELL_SHEKZEER_SONIC_DISCHARGE);
                scriptEvents.addEvent(EVENT_SHEKZEER_SONIC_DISCHARGE, 14000);
                break;
            case EVENT_SHEKZEER_DREAD_SCREECH:
                sendChatMessage(CHAT_MSG_MONSTER_EMOTE, 0, "Grand Empress Shek'zeer lets loose a dread screech!");
                castSpellAOE(SPELL_SHEKZEER_DREAD_SCREECH);
                scriptEvents.addEvent(EVENT_SHEKZEER_DREAD_SCREECH, 32000);
                break;
            case EVENT_SHEKZEER_CRY_OF_TERROR:
                if (Unit* pTarget = selectUnitTarget(FilterArgs(TargetFilter_Player)))
                    castSpell(pTarget, SPELL_SHEKZEER_CRY_OF_TERROR);
                scriptEvents.addEvent(EVENT_SHEKZEER_CRY_OF_TERROR, 24000);
                break;
            default:
                break;
        }
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Trash - Zan'thik General

class ZanthikGeneralAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new ZanthikGeneralAI(c); }
    explicit ZanthikGeneralAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        scriptEvents.addEvent(EVENT_ZANTHIK_GENERAL_CLEAVE, 6000);
        scriptEvents.addEvent(EVENT_ZANTHIK_GENERAL_ENRAGE, 16000);
    }

    void OnCombatStop(Unit* /*pTarget*/) override { scriptEvents.resetEvents(); }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);
        if (_isCasting())
            return;

        switch (scriptEvents.getFinishedEvent())
        {
            case EVENT_ZANTHIK_GENERAL_CLEAVE:
                castSpellOnVictim(SPELL_ZANTHIK_GENERAL_CLEAVE);
                scriptEvents.addEvent(EVENT_ZANTHIK_GENERAL_CLEAVE, 12000);
                break;
            case EVENT_ZANTHIK_GENERAL_ENRAGE:
                castSpellOnSelf(SPELL_ZANTHIK_GENERAL_ENRAGE);
                scriptEvents.addEvent(EVENT_ZANTHIK_GENERAL_ENRAGE, 26000);
                break;
            default:
                break;
        }
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Trash - Shek'zeer Swarmborn

class ShekzeerSwarmbornAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new ShekzeerSwarmbornAI(c); }
    explicit ShekzeerSwarmbornAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        scriptEvents.addEvent(EVENT_SWARMBORN_SHADOW_BOLT, 4000);
        scriptEvents.addEvent(EVENT_SWARMBORN_POISON, 9000);
    }

    void OnCombatStop(Unit* /*pTarget*/) override { scriptEvents.resetEvents(); }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);
        if (_isCasting())
            return;

        switch (scriptEvents.getFinishedEvent())
        {
            case EVENT_SWARMBORN_SHADOW_BOLT:
                if (Unit* pTarget = selectUnitTarget(FilterArgs(TargetFilter_Player)))
                    castSpell(pTarget, SPELL_SWARMBORN_SHADOW_BOLT);
                scriptEvents.addEvent(EVENT_SWARMBORN_SHADOW_BOLT, 14000);
                break;
            case EVENT_SWARMBORN_POISON:
                castSpellOnVictim(SPELL_SWARMBORN_POISON);
                scriptEvents.addEvent(EVENT_SWARMBORN_POISON, 18000);
                break;
            default:
                break;
        }
    }
};

void SetupHeartOfFear(ScriptMgr* mgr)
{
    mgr->register_instance_script(MAP_HEART_OF_FEAR, &HeartOfFearInstanceScript::Create);

    mgr->register_creature_script(BOSS_ZORLOK, &ZorlokAI::Create);
    mgr->register_creature_script(BOSS_TAYAK, &TayakAI::Create);
    mgr->register_creature_script(BOSS_GARALON, &GaralonAI::Create);
    mgr->register_creature_script(BOSS_MELJARAK, &MeljarakAI::Create);
    mgr->register_creature_script(BOSS_UNSOK, &UnsokAI::Create);
    mgr->register_creature_script(BOSS_SHEKZEER, &ShekzeerAI::Create);

    mgr->register_creature_script(NPC_ZANTHIK_GENERAL, &ZanthikGeneralAI::Create);
    mgr->register_creature_script(NPC_SHEKZEER_SWARMBORN, &ShekzeerSwarmbornAI::Create);
}
