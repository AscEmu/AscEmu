/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

// Gate of the Setting Sun - four bosses, all sourced from wowhead's MoP Classic data, hand-
// ported into AscEmu's own CreatureAIScript / InstanceScript framework.

#include "Setup.h"
#include "Instance_Gate_Of_The_Setting_Sun.hpp"

#include "Server/Script/CreatureAIScript.hpp"
#include "Server/Script/InstanceScript.hpp"
#include "Server/Script/ScriptMgr.hpp"

enum GateOfTheSettingSunEvents
{
    EVENT_KIPTILAK_THROW_EXPLOSIVES = 1,
    EVENT_KIPTILAK_SABOTAGE,

    EVENT_GADOK_PREY_TIME,
    EVENT_GADOK_IMPALING_STRIKE,
    EVENT_GADOK_STRAFING_RUN,
    EVENT_GADOK_ACID_BOMB,

    EVENT_RIMOK_VISCOUS_FLUID,
    EVENT_RIMOK_FRENZIED_ASSAULT,
    EVENT_RIMOK_BOMBARD,

    EVENT_RAIGONN_BATTERING_HEADBUTT,
    EVENT_RAIGONN_ENGULFING_WINDS,
    EVENT_RAIGONN_STOMP,

    EVENT_COURTYARD_DEFENDER_CLEAVE,
    EVENT_COURTYARD_DEFENDER_ENRAGE
};

class GateOfTheSettingSunInstanceScript : public InstanceScript
{
public:
    explicit GateOfTheSettingSunInstanceScript(WorldMap* pMapMgr) : InstanceScript(pMapMgr)
    {
        setBossNumber(GateOfTheSettingSunEncounterCount);
    }

    static InstanceScript* Create(WorldMap* pMapMgr) { return new GateOfTheSettingSunInstanceScript(pMapMgr); }

    void OnGameObjectPushToWorld(GameObject* pGameObject) override
    {
        if (pGameObject->getEntry() == GO_GATE_OF_THE_SETTING_SUN)
            mGateGuids.push_back(pGameObject->getGuidLow());
    }

    void OpenGate()
    {
        for (uint32_t guid : mGateGuids)
        {
            if (GameObject* pGate = GetGameObjectByGuid(guid))
                useDoorOrButton(pGate);
        }
    }

private:
    std::vector<uint32_t> mGateGuids;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Saboteur Kip'tilak

class KiptilakAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new KiptilakAI(c); }
    explicit KiptilakAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameEngage, getCreature());
        getInstanceScript()->setBossState(DATA_KIPTILAK, EncounterStates::InProgress);

        mWorldInFlamesPhase = 0;
        scriptEvents.addEvent(EVENT_KIPTILAK_THROW_EXPLOSIVES, 4000);
        scriptEvents.addEvent(EVENT_KIPTILAK_SABOTAGE, 12000);
    }

    void OnCombatStop(Unit* /*pTarget*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
        getInstanceScript()->setBossState(DATA_KIPTILAK, EncounterStates::Failed);
        scriptEvents.resetEvents();
    }

    void OnDied(Unit* /*pKiller*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
        getInstanceScript()->setBossState(DATA_KIPTILAK, EncounterStates::Performed);
        static_cast<GateOfTheSettingSunInstanceScript*>(getInstanceScript())->OpenGate();
    }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);

        if (mWorldInFlamesPhase < 2 && _getHealthPercent() <= (mWorldInFlamesPhase == 0 ? 70 : 30))
        {
            ++mWorldInFlamesPhase;
            sendChatMessage(CHAT_MSG_MONSTER_YELL, 0, "World in flames!");
            castSpellAOE(SPELL_KIPTILAK_MANTID_MUNITION_EXPLOSION);
        }

        if (_isCasting())
            return;

        switch (scriptEvents.getFinishedEvent())
        {
            case EVENT_KIPTILAK_THROW_EXPLOSIVES:
                castSpellAOE(SPELL_KIPTILAK_THROW_EXPLOSIVES);
                scriptEvents.addEvent(EVENT_KIPTILAK_THROW_EXPLOSIVES, 6000);
                break;
            case EVENT_KIPTILAK_SABOTAGE:
                if (Unit* pTarget = selectUnitTarget(FilterArgs(TargetFilter_Player)))
                    castSpell(pTarget, SPELL_KIPTILAK_SABOTAGE);
                scriptEvents.addEvent(EVENT_KIPTILAK_SABOTAGE, 18000);
                break;
            default:
                break;
        }
    }

private:
    int mWorldInFlamesPhase = 0;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Striker Ga'dok

class GadokAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new GadokAI(c); }
    explicit GadokAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameEngage, getCreature());
        getInstanceScript()->setBossState(DATA_GADOK, EncounterStates::InProgress);

        scriptEvents.addEvent(EVENT_GADOK_PREY_TIME, 10000);
        scriptEvents.addEvent(EVENT_GADOK_IMPALING_STRIKE, 6000);
        scriptEvents.addEvent(EVENT_GADOK_STRAFING_RUN, 16000);
        scriptEvents.addEvent(EVENT_GADOK_ACID_BOMB, 20000);
    }

    void OnCombatStop(Unit* /*pTarget*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
        getInstanceScript()->setBossState(DATA_GADOK, EncounterStates::Failed);
        scriptEvents.resetEvents();
    }

    void OnDied(Unit* /*pKiller*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
        getInstanceScript()->setBossState(DATA_GADOK, EncounterStates::Performed);
    }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);

        if (_isCasting())
            return;

        switch (scriptEvents.getFinishedEvent())
        {
            case EVENT_GADOK_PREY_TIME:
                if (Unit* pTarget = selectUnitTarget(FilterArgs(TargetFilter_Player)))
                    castSpell(pTarget, SPELL_GADOK_PREY_TIME);
                scriptEvents.addEvent(EVENT_GADOK_PREY_TIME, 24000);
                break;
            case EVENT_GADOK_IMPALING_STRIKE:
                castSpellOnVictim(SPELL_GADOK_IMPALING_STRIKE);
                scriptEvents.addEvent(EVENT_GADOK_IMPALING_STRIKE, 14000);
                break;
            case EVENT_GADOK_STRAFING_RUN:
                sendChatMessage(CHAT_MSG_MONSTER_EMOTE, 0, "Striker Ga'dok strafes the battlefield!");
                castSpellAOE(SPELL_GADOK_STRAFING_RUN);
                scriptEvents.addEvent(EVENT_GADOK_STRAFING_RUN, 22000);
                break;
            case EVENT_GADOK_ACID_BOMB:
                if (Unit* pTarget = selectUnitTarget(FilterArgs(TargetFilter_Player)))
                    castSpell(pTarget, SPELL_GADOK_ACID_BOMB);
                scriptEvents.addEvent(EVENT_GADOK_ACID_BOMB, 18000);
                break;
            default:
                break;
        }
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Commander Ri'mok

class RimokAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new RimokAI(c); }
    explicit RimokAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameEngage, getCreature());
        getInstanceScript()->setBossState(DATA_RIMOK, EncounterStates::InProgress);

        scriptEvents.addEvent(EVENT_RIMOK_VISCOUS_FLUID, 8000);
        scriptEvents.addEvent(EVENT_RIMOK_FRENZIED_ASSAULT, 16000);
        scriptEvents.addEvent(EVENT_RIMOK_BOMBARD, 12000);
    }

    void OnCombatStop(Unit* /*pTarget*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
        getInstanceScript()->setBossState(DATA_RIMOK, EncounterStates::Failed);
        scriptEvents.resetEvents();
    }

    void OnDied(Unit* /*pKiller*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
        getInstanceScript()->setBossState(DATA_RIMOK, EncounterStates::Performed);
    }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);

        if (_isCasting())
            return;

        switch (scriptEvents.getFinishedEvent())
        {
            case EVENT_RIMOK_VISCOUS_FLUID:
                castSpellAOE(SPELL_RIMOK_VISCOUS_FLUID);
                scriptEvents.addEvent(EVENT_RIMOK_VISCOUS_FLUID, 20000);
                break;
            case EVENT_RIMOK_FRENZIED_ASSAULT:
                castSpellOnSelf(SPELL_RIMOK_FRENZIED_ASSAULT);
                scriptEvents.addEvent(EVENT_RIMOK_FRENZIED_ASSAULT, 28000);
                break;
            case EVENT_RIMOK_BOMBARD:
                if (Unit* pTarget = selectUnitTarget(FilterArgs(TargetFilter_Player)))
                    castSpell(pTarget, SPELL_RIMOK_BOMBARD);
                scriptEvents.addEvent(EVENT_RIMOK_BOMBARD, 16000);
                break;
            default:
                break;
        }
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Raigonn

class RaigonnAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new RaigonnAI(c); }
    explicit RaigonnAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameEngage, getCreature());
        getInstanceScript()->setBossState(DATA_RAIGONN, EncounterStates::InProgress);

        mHardenCast = false;
        scriptEvents.addEvent(EVENT_RAIGONN_BATTERING_HEADBUTT, 9000);
        scriptEvents.addEvent(EVENT_RAIGONN_ENGULFING_WINDS, 5000);
        scriptEvents.addEvent(EVENT_RAIGONN_STOMP, 16000);
    }

    void OnCombatStop(Unit* /*pTarget*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
        getInstanceScript()->setBossState(DATA_RAIGONN, EncounterStates::Failed);
        scriptEvents.resetEvents();
    }

    void OnDied(Unit* /*pKiller*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
        getInstanceScript()->setBossState(DATA_RAIGONN, EncounterStates::Performed);
    }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);

        if (!mHardenCast && _getHealthPercent() <= 50)
        {
            mHardenCast = true;
            sendChatMessage(CHAT_MSG_MONSTER_EMOTE, 0, "Raigonn's carapace hardens!");
            castSpellOnSelf(SPELL_RAIGONN_IMPERVIOUS_CARAPACE);
        }

        if (_isCasting())
            return;

        switch (scriptEvents.getFinishedEvent())
        {
            case EVENT_RAIGONN_BATTERING_HEADBUTT:
                castSpellOnVictim(SPELL_RAIGONN_BATTERING_HEADBUTT);
                scriptEvents.addEvent(EVENT_RAIGONN_BATTERING_HEADBUTT, 16000);
                break;
            case EVENT_RAIGONN_ENGULFING_WINDS:
                castSpellAOE(SPELL_RAIGONN_ENGULFING_WINDS);
                scriptEvents.addEvent(EVENT_RAIGONN_ENGULFING_WINDS, 22000);
                break;
            case EVENT_RAIGONN_STOMP:
                if (Unit* pTarget = selectUnitTarget(FilterArgs(TargetFilter_Player)))
                    castSpell(pTarget, SPELL_RAIGONN_STOMP);
                scriptEvents.addEvent(EVENT_RAIGONN_STOMP, 18000);
                break;
            default:
                break;
        }
    }

private:
    bool mHardenCast = false;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Trash - Courtyard Defender

class CourtyardDefenderAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new CourtyardDefenderAI(c); }
    explicit CourtyardDefenderAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        scriptEvents.addEvent(EVENT_COURTYARD_DEFENDER_CLEAVE, 6000);
        scriptEvents.addEvent(EVENT_COURTYARD_DEFENDER_ENRAGE, 16000);
    }

    void OnCombatStop(Unit* /*pTarget*/) override { scriptEvents.resetEvents(); }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);
        if (_isCasting())
            return;

        switch (scriptEvents.getFinishedEvent())
        {
            case EVENT_COURTYARD_DEFENDER_CLEAVE:
                castSpellOnVictim(SPELL_COURTYARD_DEFENDER_CLEAVE);
                scriptEvents.addEvent(EVENT_COURTYARD_DEFENDER_CLEAVE, 12000);
                break;
            case EVENT_COURTYARD_DEFENDER_ENRAGE:
                castSpellOnSelf(SPELL_COURTYARD_DEFENDER_ENRAGE);
                scriptEvents.addEvent(EVENT_COURTYARD_DEFENDER_ENRAGE, 26000);
                break;
            default:
                break;
        }
    }
};

void SetupGateOfTheSettingSun(ScriptMgr* mgr)
{
    mgr->register_instance_script(MAP_GATE_OF_THE_SETTING_SUN, &GateOfTheSettingSunInstanceScript::Create);

    mgr->register_creature_script(BOSS_KIPTILAK, &KiptilakAI::Create);
    mgr->register_creature_script(BOSS_GADOK, &GadokAI::Create);
    mgr->register_creature_script(BOSS_RIMOK, &RimokAI::Create);
    mgr->register_creature_script(BOSS_RAIGONN, &RaigonnAI::Create);

    mgr->register_creature_script(NPC_COURTYARD_DEFENDER, &CourtyardDefenderAI::Create);
}
