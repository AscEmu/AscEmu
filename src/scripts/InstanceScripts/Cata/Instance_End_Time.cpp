/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

// End Time - hand-ported into AscEmu's own CreatureAIScript / InstanceScript framework.
// Echo of Jaina's rotation is simplified (its intro sequence and Blink phase are dropped).
// Murozond's elaborate "rewind time" clone/duplicate mechanic is dropped in favor of a
// straightforward rotation built from his real abilities.

#include "Setup.h"
#include "Instance_End_Time.hpp"

#include "Server/Script/CreatureAIScript.hpp"
#include "Server/Script/InstanceScript.hpp"
#include "Server/Script/ScriptMgr.hpp"

enum EndTimeEvents
{
    EVENT_JAINA_PYROBLAST = 1,
    EVENT_JAINA_FROSTBOLT_VOLLEY,
    EVENT_JAINA_FLARECORE,

    EVENT_MUROZOND_TEMPORAL_BLAST,
    EVENT_MUROZOND_INFINITE_BREATH,

    EVENT_NIGHTSABER_CLAW,
    EVENT_NIGHTSABER_ENRAGE,

    EVENT_GEIST_SHADOW_BOLT,
    EVENT_GEIST_CURSE
};

class EndTimeInstanceScript : public InstanceScript
{
public:
    explicit EndTimeInstanceScript(WorldMap* pMapMgr) : InstanceScript(pMapMgr)
    {
        setBossNumber(EndTimeEncounterCount);
    }

    static InstanceScript* Create(WorldMap* pMapMgr) { return new EndTimeInstanceScript(pMapMgr); }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Echo of Jaina

class EchoOfJainaAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new EchoOfJainaAI(c); }
    explicit EchoOfJainaAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameEngage, getCreature());
        getInstanceScript()->setBossState(DATA_ECHO_OF_JAINA, EncounterStates::InProgress);

        scriptEvents.addEvent(EVENT_JAINA_PYROBLAST, 1);
        scriptEvents.addEvent(EVENT_JAINA_FROSTBOLT_VOLLEY, 8000);
        scriptEvents.addEvent(EVENT_JAINA_FLARECORE, 15000);
    }

    void OnCombatStop(Unit* /*pTarget*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
        getInstanceScript()->setBossState(DATA_ECHO_OF_JAINA, EncounterStates::Failed);
        scriptEvents.resetEvents();
    }

    void OnDied(Unit* /*pKiller*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
        getInstanceScript()->setBossState(DATA_ECHO_OF_JAINA, EncounterStates::Performed);
    }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);

        if (_isCasting())
            return;

        switch (scriptEvents.getFinishedEvent())
        {
            case EVENT_JAINA_PYROBLAST:
                castSpellOnVictim(SPELL_JAINA_PYROBLAST);
                scriptEvents.addEvent(EVENT_JAINA_PYROBLAST, 12000);
                break;
            case EVENT_JAINA_FROSTBOLT_VOLLEY:
                castSpellAOE(SPELL_JAINA_FROSTBOLT_VOLLEY);
                scriptEvents.addEvent(EVENT_JAINA_FROSTBOLT_VOLLEY, 14000);
                break;
            case EVENT_JAINA_FLARECORE:
                castSpellOnSelf(SPELL_JAINA_FLARECORE);
                scriptEvents.addEvent(EVENT_JAINA_FLARECORE, 20000);
                break;
            default:
                break;
        }
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Murozond

class MurozondAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new MurozondAI(c); }
    explicit MurozondAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        sendChatMessage(CHAT_MSG_MONSTER_YELL, 0, "You hope to... what? Stop me, here? Change the fate I worked so tirelessly to weave?");
        getInstanceScript()->sendUnitEncounter(EncounterFrameEngage, getCreature());
        getInstanceScript()->setBossState(DATA_MUROZOND, EncounterStates::InProgress);

        scriptEvents.addEvent(EVENT_MUROZOND_TEMPORAL_BLAST, 5000);
        scriptEvents.addEvent(EVENT_MUROZOND_INFINITE_BREATH, 8500);
    }

    void OnCombatStop(Unit* /*pTarget*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
        getInstanceScript()->setBossState(DATA_MUROZOND, EncounterStates::Failed);
        scriptEvents.resetEvents();
    }

    void OnDied(Unit* /*pKiller*/) override
    {
        sendChatMessage(CHAT_MSG_MONSTER_YELL, 0, "You know not what you have done. Aman'Thul... What I... have... seen...");
        getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
        getInstanceScript()->setBossState(DATA_MUROZOND, EncounterStates::Performed);
    }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);

        if (_isCasting())
            return;

        switch (scriptEvents.getFinishedEvent())
        {
            case EVENT_MUROZOND_TEMPORAL_BLAST:
                if (Unit* pTarget = selectUnitTarget(FilterArgs(TargetFilter_Player)))
                    castSpell(pTarget, SPELL_MUROZOND_TEMPORAL_BLAST);
                scriptEvents.addEvent(EVENT_MUROZOND_TEMPORAL_BLAST, 16000);
                break;
            case EVENT_MUROZOND_INFINITE_BREATH:
                castSpellAOE(SPELL_MUROZOND_INFINITE_BREATH);
                scriptEvents.addEvent(EVENT_MUROZOND_INFINITE_BREATH, 20000);
                break;
            default:
                break;
        }
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Echo of Baine / Sylvanas / Tyrande - encounter tracking only (unscripted upstream too).

class EndTimeUnscriptedBossAI : public CreatureAIScript
{
public:
    EndTimeUnscriptedBossAI(Creature* pCreature, uint32_t dataId) : CreatureAIScript(pCreature), mDataId(dataId) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameEngage, getCreature());
        getInstanceScript()->setBossState(mDataId, EncounterStates::InProgress);
    }

    void OnCombatStop(Unit* /*pTarget*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
        getInstanceScript()->setBossState(mDataId, EncounterStates::Failed);
    }

    void OnDied(Unit* /*pKiller*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
        getInstanceScript()->setBossState(mDataId, EncounterStates::Performed);
    }

private:
    uint32_t mDataId;
};

class EchoOfBaineAI : public EndTimeUnscriptedBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new EchoOfBaineAI(c); }
    explicit EchoOfBaineAI(Creature* pCreature) : EndTimeUnscriptedBossAI(pCreature, DATA_ECHO_OF_BAINE) {}
};

class EchoOfSylvanasAI : public EndTimeUnscriptedBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new EchoOfSylvanasAI(c); }
    explicit EchoOfSylvanasAI(Creature* pCreature) : EndTimeUnscriptedBossAI(pCreature, DATA_ECHO_OF_SYLVANAS) {}
};

class EchoOfTyrandeAI : public EndTimeUnscriptedBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new EchoOfTyrandeAI(c); }
    explicit EchoOfTyrandeAI(Creature* pCreature) : EndTimeUnscriptedBossAI(pCreature, DATA_ECHO_OF_TYRANDE) {}
};

//////////////////////////////////////////////////////////////////////////////////////////
// Trash

class TimeTwistedNightsaberAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new TimeTwistedNightsaberAI(c); }
    explicit TimeTwistedNightsaberAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        scriptEvents.addEvent(EVENT_NIGHTSABER_CLAW, 3000);
        scriptEvents.addEvent(EVENT_NIGHTSABER_ENRAGE, 10000);
    }

    void OnCombatStop(Unit* /*pTarget*/) override
    {
        scriptEvents.resetEvents();
    }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);

        if (_isCasting())
            return;

        switch (scriptEvents.getFinishedEvent())
        {
            case EVENT_NIGHTSABER_CLAW:
                castSpellOnVictim(SPELL_NIGHTSABER_CLAW);
                scriptEvents.addEvent(EVENT_NIGHTSABER_CLAW, 6000);
                break;
            case EVENT_NIGHTSABER_ENRAGE:
                castSpellOnSelf(SPELL_NIGHTSABER_ENRAGE);
                scriptEvents.addEvent(EVENT_NIGHTSABER_ENRAGE, 18000);
                break;
            default:
                break;
        }
    }
};

class TimeTwistedGeistAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new TimeTwistedGeistAI(c); }
    explicit TimeTwistedGeistAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        scriptEvents.addEvent(EVENT_GEIST_SHADOW_BOLT, 2000);
        scriptEvents.addEvent(EVENT_GEIST_CURSE, 7000);
    }

    void OnCombatStop(Unit* /*pTarget*/) override
    {
        scriptEvents.resetEvents();
    }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);

        if (_isCasting())
            return;

        switch (scriptEvents.getFinishedEvent())
        {
            case EVENT_GEIST_SHADOW_BOLT:
                castSpellOnVictim(SPELL_GEIST_SHADOW_BOLT);
                scriptEvents.addEvent(EVENT_GEIST_SHADOW_BOLT, 8000);
                break;
            case EVENT_GEIST_CURSE:
                castSpellOnVictim(SPELL_GEIST_CURSE);
                scriptEvents.addEvent(EVENT_GEIST_CURSE, 16000);
                break;
            default:
                break;
        }
    }
};

void SetupEndTime(ScriptMgr* mgr)
{
    mgr->register_instance_script(MAP_END_TIME, &EndTimeInstanceScript::Create);

    mgr->register_creature_script(BOSS_ECHO_OF_BAINE, &EchoOfBaineAI::Create);
    mgr->register_creature_script(BOSS_ECHO_OF_JAINA, &EchoOfJainaAI::Create);
    mgr->register_creature_script(BOSS_ECHO_OF_SYLVANAS, &EchoOfSylvanasAI::Create);
    mgr->register_creature_script(BOSS_ECHO_OF_TYRANDE, &EchoOfTyrandeAI::Create);
    mgr->register_creature_script(BOSS_MUROZOND, &MurozondAI::Create);

    mgr->register_creature_script(NPC_TIME_TWISTED_NIGHTSABER, &TimeTwistedNightsaberAI::Create);
    mgr->register_creature_script(NPC_TIME_TWISTED_GEIST, &TimeTwistedGeistAI::Create);
}
