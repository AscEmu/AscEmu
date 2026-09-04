/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

// Shado-Pan Monastery - three bosses (verified against wowhead's MoP Classic data), hand-
// ported into AscEmu's own CreatureAIScript / InstanceScript framework.

#include "Setup.h"
#include "Instance_Shado_Pan_Monastery.hpp"

#include "Server/Script/CreatureAIScript.hpp"
#include "Server/Script/InstanceScript.hpp"
#include "Server/Script/ScriptMgr.hpp"

enum ShadoPanMonasteryEvents
{
    EVENT_SNOWDRIFT_FISTS_OF_FURY = 1,
    EVENT_SNOWDRIFT_TORNADO_KICK,
    EVENT_SNOWDRIFT_QUIVERING_PALM,
    EVENT_SNOWDRIFT_FLYING_KICK,

    EVENT_VIOLENCE_DISORIENTING_SMASH,
    EVENT_VIOLENCE_SMOKE_BLADES,
    EVENT_VIOLENCE_SHA_SPIKE,

    EVENT_CLOUDSTRIKE_INVOKE_LIGHTNING,
    EVENT_CLOUDSTRIKE_STATIC_FIELD,
    EVENT_CLOUDSTRIKE_LIGHTNING_BREATH,

    EVENT_AMBUSHER_FLIP_OUT,
    EVENT_AMBUSHER_ICE_TRAP,
    EVENT_AMBUSHER_SHADOWSTEP,

    EVENT_DISCIPLE_CLEAVE,
    EVENT_DISCIPLE_ENRAGE
};

class ShadoPanMonasteryInstanceScript : public InstanceScript
{
public:
    explicit ShadoPanMonasteryInstanceScript(WorldMap* pMapMgr) : InstanceScript(pMapMgr)
    {
        setBossNumber(ShadoPanMonasteryEncounterCount);
    }

    static InstanceScript* Create(WorldMap* pMapMgr) { return new ShadoPanMonasteryInstanceScript(pMapMgr); }

    void OnGameObjectPushToWorld(GameObject* pGameObject) override
    {
        if (pGameObject->getEntry() == GO_OUTER_DOORS)
            mOuterDoorGuids.push_back(pGameObject->getGuidLow());
    }

    void OpenOuterDoors()
    {
        for (uint32_t guid : mOuterDoorGuids)
        {
            if (GameObject* pDoor = GetGameObjectByGuid(guid))
                useDoorOrButton(pDoor);
        }
    }

private:
    std::vector<uint32_t> mOuterDoorGuids;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Master Snowdrift

class MasterSnowdriftAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new MasterSnowdriftAI(c); }
    explicit MasterSnowdriftAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameEngage, getCreature());
        getInstanceScript()->setBossState(DATA_MASTER_SNOWDRIFT, EncounterStates::InProgress);

        scriptEvents.addEvent(EVENT_SNOWDRIFT_FISTS_OF_FURY, 10000);
        scriptEvents.addEvent(EVENT_SNOWDRIFT_TORNADO_KICK, 16000);
        scriptEvents.addEvent(EVENT_SNOWDRIFT_QUIVERING_PALM, 22000);
        scriptEvents.addEvent(EVENT_SNOWDRIFT_FLYING_KICK, 8000);
    }

    void OnCombatStop(Unit* /*pTarget*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
        getInstanceScript()->setBossState(DATA_MASTER_SNOWDRIFT, EncounterStates::Failed);
        scriptEvents.resetEvents();
    }

    void OnDied(Unit* /*pKiller*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
        getInstanceScript()->setBossState(DATA_MASTER_SNOWDRIFT, EncounterStates::Performed);
        static_cast<ShadoPanMonasteryInstanceScript*>(getInstanceScript())->OpenOuterDoors();
    }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);

        if (_isCasting())
            return;

        switch (scriptEvents.getFinishedEvent())
        {
            case EVENT_SNOWDRIFT_FISTS_OF_FURY:
                castSpellOnSelf(SPELL_SNOWDRIFT_FISTS_OF_FURY);
                scriptEvents.addEvent(EVENT_SNOWDRIFT_FISTS_OF_FURY, 24000);
                break;
            case EVENT_SNOWDRIFT_TORNADO_KICK:
                castSpellOnVictim(SPELL_SNOWDRIFT_TORNADO_KICK);
                scriptEvents.addEvent(EVENT_SNOWDRIFT_TORNADO_KICK, 18000);
                break;
            case EVENT_SNOWDRIFT_QUIVERING_PALM:
                if (Unit* pTarget = selectUnitTarget(FilterArgs(TargetFilter_Player)))
                    castSpell(pTarget, SPELL_SNOWDRIFT_QUIVERING_PALM);
                scriptEvents.addEvent(EVENT_SNOWDRIFT_QUIVERING_PALM, 26000);
                break;
            case EVENT_SNOWDRIFT_FLYING_KICK:
                if (Unit* pTarget = selectUnitTarget(FilterArgs(TargetFilter_Player)))
                    castSpell(pTarget, SPELL_SNOWDRIFT_FLYING_KICK);
                scriptEvents.addEvent(EVENT_SNOWDRIFT_FLYING_KICK, 14000);
                break;
            default:
                break;
        }
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Sha of Violence

class ShaOfViolenceAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new ShaOfViolenceAI(c); }
    explicit ShaOfViolenceAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        sendChatMessage(CHAT_MSG_MONSTER_YELL, 0, "Rage consumes you already!");
        getInstanceScript()->sendUnitEncounter(EncounterFrameEngage, getCreature());
        getInstanceScript()->setBossState(DATA_SHA_OF_VIOLENCE, EncounterStates::InProgress);

        scriptEvents.addEvent(EVENT_VIOLENCE_DISORIENTING_SMASH, 9000);
        scriptEvents.addEvent(EVENT_VIOLENCE_SMOKE_BLADES, 15000);
        scriptEvents.addEvent(EVENT_VIOLENCE_SHA_SPIKE, 6000);
    }

    void OnCombatStop(Unit* /*pTarget*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
        getInstanceScript()->setBossState(DATA_SHA_OF_VIOLENCE, EncounterStates::Failed);
        scriptEvents.resetEvents();
    }

    void OnDied(Unit* /*pKiller*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
        getInstanceScript()->setBossState(DATA_SHA_OF_VIOLENCE, EncounterStates::Performed);
    }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);

        if (_isCasting())
            return;

        switch (scriptEvents.getFinishedEvent())
        {
            case EVENT_VIOLENCE_DISORIENTING_SMASH:
                castSpellOnVictim(SPELL_VIOLENCE_DISORIENTING_SMASH);
                scriptEvents.addEvent(EVENT_VIOLENCE_DISORIENTING_SMASH, 20000);
                break;
            case EVENT_VIOLENCE_SMOKE_BLADES:
                castSpellAOE(SPELL_VIOLENCE_SMOKE_BLADES);
                scriptEvents.addEvent(EVENT_VIOLENCE_SMOKE_BLADES, 22000);
                break;
            case EVENT_VIOLENCE_SHA_SPIKE:
                if (Unit* pTarget = selectUnitTarget(FilterArgs(TargetFilter_Player)))
                    castSpell(pTarget, SPELL_VIOLENCE_SHA_SPIKE);
                scriptEvents.addEvent(EVENT_VIOLENCE_SHA_SPIKE, 12000);
                break;
            default:
                break;
        }
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Gu Cloudstrike

class GuCloudstrikeAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new GuCloudstrikeAI(c); }
    explicit GuCloudstrikeAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameEngage, getCreature());
        getInstanceScript()->setBossState(DATA_GU_CLOUDSTRIKE, EncounterStates::InProgress);

        scriptEvents.addEvent(EVENT_CLOUDSTRIKE_INVOKE_LIGHTNING, 12000);
        scriptEvents.addEvent(EVENT_CLOUDSTRIKE_STATIC_FIELD, 7000);
        scriptEvents.addEvent(EVENT_CLOUDSTRIKE_LIGHTNING_BREATH, 18000);
    }

    void OnCombatStop(Unit* /*pTarget*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
        getInstanceScript()->setBossState(DATA_GU_CLOUDSTRIKE, EncounterStates::Failed);
        scriptEvents.resetEvents();
    }

    void OnDied(Unit* /*pKiller*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
        getInstanceScript()->setBossState(DATA_GU_CLOUDSTRIKE, EncounterStates::Performed);
    }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);

        if (_isCasting())
            return;

        switch (scriptEvents.getFinishedEvent())
        {
            case EVENT_CLOUDSTRIKE_INVOKE_LIGHTNING:
                sendChatMessage(CHAT_MSG_MONSTER_EMOTE, 0, "Gu Cloudstrike invokes the fury of lightning!");
                castSpellAOE(SPELL_CLOUDSTRIKE_INVOKE_LIGHTNING);
                scriptEvents.addEvent(EVENT_CLOUDSTRIKE_INVOKE_LIGHTNING, 26000);
                break;
            case EVENT_CLOUDSTRIKE_STATIC_FIELD:
                if (Unit* pTarget = selectUnitTarget(FilterArgs(TargetFilter_Player)))
                    castSpell(pTarget, SPELL_CLOUDSTRIKE_STATIC_FIELD);
                scriptEvents.addEvent(EVENT_CLOUDSTRIKE_STATIC_FIELD, 15000);
                break;
            case EVENT_CLOUDSTRIKE_LIGHTNING_BREATH:
                castSpellOnVictim(SPELL_CLOUDSTRIKE_LIGHTNING_BREATH);
                scriptEvents.addEvent(EVENT_CLOUDSTRIKE_LIGHTNING_BREATH, 20000);
                break;
            default:
                break;
        }
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Trash - Shado-Pan Ambusher

class ShadopanAmbusherAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new ShadopanAmbusherAI(c); }
    explicit ShadopanAmbusherAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        scriptEvents.addEvent(EVENT_AMBUSHER_FLIP_OUT, 6000);
        scriptEvents.addEvent(EVENT_AMBUSHER_ICE_TRAP, 4000);
        scriptEvents.addEvent(EVENT_AMBUSHER_SHADOWSTEP, 12000);
    }

    void OnCombatStop(Unit* /*pTarget*/) override { scriptEvents.resetEvents(); }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);
        if (_isCasting())
            return;

        switch (scriptEvents.getFinishedEvent())
        {
            case EVENT_AMBUSHER_FLIP_OUT:
                castSpellOnSelf(SPELL_AMBUSHER_FLIP_OUT);
                scriptEvents.addEvent(EVENT_AMBUSHER_FLIP_OUT, 18000);
                break;
            case EVENT_AMBUSHER_ICE_TRAP:
                castSpellAOE(SPELL_AMBUSHER_ICE_TRAP);
                scriptEvents.addEvent(EVENT_AMBUSHER_ICE_TRAP, 16000);
                break;
            case EVENT_AMBUSHER_SHADOWSTEP:
                if (Unit* pTarget = selectUnitTarget(FilterArgs(TargetFilter_Player)))
                    castSpell(pTarget, SPELL_AMBUSHER_SHADOWSTEP);
                scriptEvents.addEvent(EVENT_AMBUSHER_SHADOWSTEP, 20000);
                break;
            default:
                break;
        }
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Trash - Shado-Pan Disciple

class ShadopanDiscipleAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new ShadopanDiscipleAI(c); }
    explicit ShadopanDiscipleAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        scriptEvents.addEvent(EVENT_DISCIPLE_CLEAVE, 6000);
        scriptEvents.addEvent(EVENT_DISCIPLE_ENRAGE, 16000);
    }

    void OnCombatStop(Unit* /*pTarget*/) override { scriptEvents.resetEvents(); }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);
        if (_isCasting())
            return;

        switch (scriptEvents.getFinishedEvent())
        {
            case EVENT_DISCIPLE_CLEAVE:
                castSpellOnVictim(SPELL_DISCIPLE_CLEAVE);
                scriptEvents.addEvent(EVENT_DISCIPLE_CLEAVE, 12000);
                break;
            case EVENT_DISCIPLE_ENRAGE:
                castSpellOnSelf(SPELL_DISCIPLE_ENRAGE);
                scriptEvents.addEvent(EVENT_DISCIPLE_ENRAGE, 26000);
                break;
            default:
                break;
        }
    }
};

void SetupShadoPanMonastery(ScriptMgr* mgr)
{
    mgr->register_instance_script(MAP_SHADO_PAN_MONASTERY, &ShadoPanMonasteryInstanceScript::Create);

    mgr->register_creature_script(BOSS_MASTER_SNOWDRIFT, &MasterSnowdriftAI::Create);
    mgr->register_creature_script(BOSS_SHA_OF_VIOLENCE, &ShaOfViolenceAI::Create);
    mgr->register_creature_script(BOSS_GU_CLOUDSTRIKE, &GuCloudstrikeAI::Create);

    mgr->register_creature_script(NPC_SHADOPAN_AMBUSHER, &ShadopanAmbusherAI::Create);
    mgr->register_creature_script(NPC_SHADOPAN_DISCIPLE, &ShadopanDiscipleAI::Create);
}
