/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

// Temple of the Jade Serpent - four bosses (verified against wowhead's MoP Classic data),
// hand-ported into AscEmu's own CreatureAIScript / InstanceScript framework. See the header
// comment for what was simplified.

#include "Setup.h"
#include "Instance_Temple_Of_The_Jade_Serpent.hpp"

#include "Server/Script/CreatureAIScript.hpp"
#include "Server/Script/InstanceScript.hpp"
#include "Server/Script/ScriptMgr.hpp"

enum TempleOfTheJadeSerpentEvents
{
    EVENT_MARI_HYDROLANCE = 1,
    EVENT_MARI_CALL_WATER,
    EVENT_MARI_BUBBLE_BURST,
    EVENT_MARI_WASH_AWAY,

    EVENT_STONESTEP_AGONY,
    EVENT_STONESTEP_SUNFIRE_RAYS,
    EVENT_STONESTEP_HAUNTING_GAZE,
    EVENT_STONESTEP_HELLFIRE_ARROWS,

    EVENT_FLAMEHEART_SERPENT_STRIKE,
    EVENT_FLAMEHEART_SERPENT_KICK,
    EVENT_FLAMEHEART_SERPENT_WAVE,
    EVENT_FLAMEHEART_JADE_FIRE,

    EVENT_SHA_WITHER_WILL,
    EVENT_SHA_TOUCH_OF_NOTHINGNESS,
    EVENT_SHA_GATHERING_DOUBT,

    EVENT_LESSER_SHA_MIND_FLAY,
    EVENT_LESSER_SHA_SHADOW_BOLT,

    EVENT_YULON_PRIEST_SMITE,
    EVENT_YULON_PRIEST_HEAL
};

class TempleOfTheJadeSerpentInstanceScript : public InstanceScript
{
public:
    explicit TempleOfTheJadeSerpentInstanceScript(WorldMap* pMapMgr) : InstanceScript(pMapMgr)
    {
        setBossNumber(TempleOfTheJadeSerpentEncounterCount);
    }

    static InstanceScript* Create(WorldMap* pMapMgr) { return new TempleOfTheJadeSerpentInstanceScript(pMapMgr); }

    void OnGameObjectPushToWorld(GameObject* pGameObject) override
    {
        if (pGameObject->getEntry() == GO_TEMPLE_DOOR_1 || pGameObject->getEntry() == GO_TEMPLE_DOOR_2)
            mTempleDoorGuids.push_back(pGameObject->getGuidLow());
    }

    void OpenTempleDoor()
    {
        for (uint32_t guid : mTempleDoorGuids)
        {
            if (GameObject* pDoor = GetGameObjectByGuid(guid))
                useDoorOrButton(pDoor);
        }
    }

private:
    std::vector<uint32_t> mTempleDoorGuids;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Wise Mari

class WiseMariAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new WiseMariAI(c); }
    explicit WiseMariAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        sendChatMessage(CHAT_MSG_MONSTER_YELL, 0, "The pool will swallow you whole!");
        getInstanceScript()->sendUnitEncounter(EncounterFrameEngage, getCreature());
        getInstanceScript()->setBossState(DATA_WISE_MARI, EncounterStates::InProgress);

        scriptEvents.addEvent(EVENT_MARI_HYDROLANCE, 5000);
        scriptEvents.addEvent(EVENT_MARI_CALL_WATER, 14000);
        scriptEvents.addEvent(EVENT_MARI_BUBBLE_BURST, 9000);
        scriptEvents.addEvent(EVENT_MARI_WASH_AWAY, 20000);
    }

    void OnCombatStop(Unit* /*pTarget*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
        getInstanceScript()->setBossState(DATA_WISE_MARI, EncounterStates::Failed);
        scriptEvents.resetEvents();
    }

    void OnDied(Unit* /*pKiller*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
        getInstanceScript()->setBossState(DATA_WISE_MARI, EncounterStates::Performed);
        static_cast<TempleOfTheJadeSerpentInstanceScript*>(getInstanceScript())->OpenTempleDoor();
    }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);

        if (_isCasting())
            return;

        switch (scriptEvents.getFinishedEvent())
        {
            case EVENT_MARI_HYDROLANCE:
                castSpellOnVictim(SPELL_MARI_HYDROLANCE);
                scriptEvents.addEvent(EVENT_MARI_HYDROLANCE, 9000);
                break;
            case EVENT_MARI_CALL_WATER:
                sendChatMessage(CHAT_MSG_MONSTER_EMOTE, 0, "Wise Mari calls forth water elementals!");
                castSpellOnSelf(SPELL_MARI_CALL_WATER);
                scriptEvents.addEvent(EVENT_MARI_CALL_WATER, 30000);
                break;
            case EVENT_MARI_BUBBLE_BURST:
                if (Unit* pTarget = selectUnitTarget(FilterArgs(TargetFilter_Player)))
                    castSpell(pTarget, SPELL_MARI_BUBBLE_BURST);
                scriptEvents.addEvent(EVENT_MARI_BUBBLE_BURST, 16000);
                break;
            case EVENT_MARI_WASH_AWAY:
                castSpellAOE(SPELL_MARI_WASH_AWAY);
                scriptEvents.addEvent(EVENT_MARI_WASH_AWAY, 24000);
                break;
            default:
                break;
        }
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Lorewalker Stonestep

class StonestepAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new StonestepAI(c); }
    explicit StonestepAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameEngage, getCreature());
        getInstanceScript()->setBossState(DATA_STONESTEP, EncounterStates::InProgress);

        scriptEvents.addEvent(EVENT_STONESTEP_AGONY, 6000);
        scriptEvents.addEvent(EVENT_STONESTEP_SUNFIRE_RAYS, 12000);
        scriptEvents.addEvent(EVENT_STONESTEP_HAUNTING_GAZE, 18000);
        scriptEvents.addEvent(EVENT_STONESTEP_HELLFIRE_ARROWS, 9000);
    }

    void OnCombatStop(Unit* /*pTarget*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
        getInstanceScript()->setBossState(DATA_STONESTEP, EncounterStates::Failed);
        scriptEvents.resetEvents();
    }

    void OnDied(Unit* /*pKiller*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
        getInstanceScript()->setBossState(DATA_STONESTEP, EncounterStates::Performed);
    }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);

        if (_isCasting())
            return;

        switch (scriptEvents.getFinishedEvent())
        {
            case EVENT_STONESTEP_AGONY:
                castSpellOnVictim(SPELL_STONESTEP_AGONY);
                scriptEvents.addEvent(EVENT_STONESTEP_AGONY, 14000);
                break;
            case EVENT_STONESTEP_SUNFIRE_RAYS:
                castSpellAOE(SPELL_STONESTEP_SUNFIRE_RAYS);
                scriptEvents.addEvent(EVENT_STONESTEP_SUNFIRE_RAYS, 20000);
                break;
            case EVENT_STONESTEP_HAUNTING_GAZE:
                if (Unit* pTarget = selectUnitTarget(FilterArgs(TargetFilter_Player)))
                    castSpell(pTarget, SPELL_STONESTEP_HAUNTING_GAZE);
                scriptEvents.addEvent(EVENT_STONESTEP_HAUNTING_GAZE, 22000);
                break;
            case EVENT_STONESTEP_HELLFIRE_ARROWS:
                if (Unit* pTarget = selectUnitTarget(FilterArgs(TargetFilter_Player)))
                    castSpell(pTarget, SPELL_STONESTEP_HELLFIRE_ARROWS);
                scriptEvents.addEvent(EVENT_STONESTEP_HELLFIRE_ARROWS, 13000);
                break;
            default:
                break;
        }
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Liu Flameheart

class FlameheartAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new FlameheartAI(c); }
    explicit FlameheartAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        sendChatMessage(CHAT_MSG_MONSTER_YELL, 0, "The Jade Serpent's power is mine to command!");
        getInstanceScript()->sendUnitEncounter(EncounterFrameEngage, getCreature());
        getInstanceScript()->setBossState(DATA_FLAMEHEART, EncounterStates::InProgress);

        scriptEvents.addEvent(EVENT_FLAMEHEART_SERPENT_STRIKE, 5000);
        scriptEvents.addEvent(EVENT_FLAMEHEART_SERPENT_KICK, 11000);
        scriptEvents.addEvent(EVENT_FLAMEHEART_SERPENT_WAVE, 17000);
        scriptEvents.addEvent(EVENT_FLAMEHEART_JADE_FIRE, 8000);
    }

    void OnCombatStop(Unit* /*pTarget*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
        getInstanceScript()->setBossState(DATA_FLAMEHEART, EncounterStates::Failed);
        scriptEvents.resetEvents();
    }

    void OnDied(Unit* /*pKiller*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
        getInstanceScript()->setBossState(DATA_FLAMEHEART, EncounterStates::Performed);
    }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);

        if (_isCasting())
            return;

        switch (scriptEvents.getFinishedEvent())
        {
            case EVENT_FLAMEHEART_SERPENT_STRIKE:
                castSpellOnVictim(SPELL_FLAMEHEART_JADE_SERPENT_STRIKE);
                scriptEvents.addEvent(EVENT_FLAMEHEART_SERPENT_STRIKE, 12000);
                break;
            case EVENT_FLAMEHEART_SERPENT_KICK:
                castSpellOnVictim(SPELL_FLAMEHEART_JADE_SERPENT_KICK);
                scriptEvents.addEvent(EVENT_FLAMEHEART_SERPENT_KICK, 16000);
                break;
            case EVENT_FLAMEHEART_SERPENT_WAVE:
                castSpellAOE(SPELL_FLAMEHEART_JADE_SERPENT_WAVE);
                scriptEvents.addEvent(EVENT_FLAMEHEART_SERPENT_WAVE, 22000);
                break;
            case EVENT_FLAMEHEART_JADE_FIRE:
                if (Unit* pTarget = selectUnitTarget(FilterArgs(TargetFilter_Player)))
                    castSpell(pTarget, SPELL_FLAMEHEART_JADE_FIRE);
                scriptEvents.addEvent(EVENT_FLAMEHEART_JADE_FIRE, 14000);
                break;
            default:
                break;
        }
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Sha of Doubt

class ShaOfDoubtAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new ShaOfDoubtAI(c); }
    explicit ShaOfDoubtAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        sendChatMessage(CHAT_MSG_MONSTER_YELL, 0, "You doubt yourselves already...");
        getInstanceScript()->sendUnitEncounter(EncounterFrameEngage, getCreature());
        getInstanceScript()->setBossState(DATA_SHA_OF_DOUBT, EncounterStates::InProgress);

        scriptEvents.addEvent(EVENT_SHA_WITHER_WILL, 7000);
        scriptEvents.addEvent(EVENT_SHA_TOUCH_OF_NOTHINGNESS, 13000);
        scriptEvents.addEvent(EVENT_SHA_GATHERING_DOUBT, 20000);
    }

    void OnCombatStop(Unit* /*pTarget*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
        getInstanceScript()->setBossState(DATA_SHA_OF_DOUBT, EncounterStates::Failed);
        scriptEvents.resetEvents();
    }

    void OnDied(Unit* /*pKiller*/) override
    {
        sendChatMessage(CHAT_MSG_MONSTER_YELL, 0, "Doubt... consumes... all...");
        getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
        getInstanceScript()->setBossState(DATA_SHA_OF_DOUBT, EncounterStates::Performed);
    }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);

        if (_isCasting())
            return;

        switch (scriptEvents.getFinishedEvent())
        {
            case EVENT_SHA_WITHER_WILL:
                if (Unit* pTarget = selectUnitTarget(FilterArgs(TargetFilter_Player)))
                    castSpell(pTarget, SPELL_SHA_WITHER_WILL);
                scriptEvents.addEvent(EVENT_SHA_WITHER_WILL, 18000);
                break;
            case EVENT_SHA_TOUCH_OF_NOTHINGNESS:
                castSpellOnVictim(SPELL_SHA_TOUCH_OF_NOTHINGNESS);
                scriptEvents.addEvent(EVENT_SHA_TOUCH_OF_NOTHINGNESS, 15000);
                break;
            case EVENT_SHA_GATHERING_DOUBT:
                sendChatMessage(CHAT_MSG_MONSTER_EMOTE, 0, "The Sha of Doubt gathers a pool of doubt!");
                castSpellAOE(SPELL_SHA_GATHERING_DOUBT);
                scriptEvents.addEvent(EVENT_SHA_GATHERING_DOUBT, 28000);
                break;
            default:
                break;
        }
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Trash - Lesser Sha

class LesserShaAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new LesserShaAI(c); }
    explicit LesserShaAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        scriptEvents.addEvent(EVENT_LESSER_SHA_MIND_FLAY, 5000);
        scriptEvents.addEvent(EVENT_LESSER_SHA_SHADOW_BOLT, 10000);
    }

    void OnCombatStop(Unit* /*pTarget*/) override { scriptEvents.resetEvents(); }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);
        if (_isCasting())
            return;

        switch (scriptEvents.getFinishedEvent())
        {
            case EVENT_LESSER_SHA_MIND_FLAY:
                castSpellOnVictim(SPELL_LESSER_SHA_MIND_FLAY);
                scriptEvents.addEvent(EVENT_LESSER_SHA_MIND_FLAY, 14000);
                break;
            case EVENT_LESSER_SHA_SHADOW_BOLT:
                if (Unit* pTarget = selectUnitTarget(FilterArgs(TargetFilter_Player)))
                    castSpell(pTarget, SPELL_LESSER_SHA_SHADOW_BOLT);
                scriptEvents.addEvent(EVENT_LESSER_SHA_SHADOW_BOLT, 16000);
                break;
            default:
                break;
        }
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Trash - Yu'lon Priest

class YulonPriestAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new YulonPriestAI(c); }
    explicit YulonPriestAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        scriptEvents.addEvent(EVENT_YULON_PRIEST_SMITE, 4000);
        scriptEvents.addEvent(EVENT_YULON_PRIEST_HEAL, 9000);
    }

    void OnCombatStop(Unit* /*pTarget*/) override { scriptEvents.resetEvents(); }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);
        if (_isCasting())
            return;

        switch (scriptEvents.getFinishedEvent())
        {
            case EVENT_YULON_PRIEST_SMITE:
                castSpellOnVictim(SPELL_YULON_PRIEST_SMITE);
                scriptEvents.addEvent(EVENT_YULON_PRIEST_SMITE, 10000);
                break;
            case EVENT_YULON_PRIEST_HEAL:
                castSpellOnSelf(SPELL_YULON_PRIEST_HEAL);
                scriptEvents.addEvent(EVENT_YULON_PRIEST_HEAL, 18000);
                break;
            default:
                break;
        }
    }
};

void SetupTempleOfTheJadeSerpent(ScriptMgr* mgr)
{
    mgr->register_instance_script(MAP_TEMPLE_OF_THE_JADE_SERPENT, &TempleOfTheJadeSerpentInstanceScript::Create);

    mgr->register_creature_script(BOSS_WISE_MARI, &WiseMariAI::Create);
    mgr->register_creature_script(BOSS_STONESTEP, &StonestepAI::Create);
    mgr->register_creature_script(BOSS_FLAMEHEART, &FlameheartAI::Create);
    mgr->register_creature_script(BOSS_SHA_OF_DOUBT, &ShaOfDoubtAI::Create);

    mgr->register_creature_script(NPC_LESSER_SHA, &LesserShaAI::Create);
    mgr->register_creature_script(NPC_YULON_PRIEST, &YulonPriestAI::Create);
}
