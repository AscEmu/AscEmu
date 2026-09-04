/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

// Siege of Niuzao Temple - four bosses (verified against wowhead's MoP Classic data where
// reachable), hand-ported into AscEmu's own CreatureAIScript / InstanceScript framework.

#include "Setup.h"
#include "Instance_Siege_Of_Niuzao_Temple.hpp"

#include "Server/Script/CreatureAIScript.hpp"
#include "Server/Script/InstanceScript.hpp"
#include "Server/Script/ScriptMgr.hpp"

enum SiegeOfNiuzaoTempleEvents
{
    EVENT_JINBAK_SAP_PUDDLE = 1,
    EVENT_JINBAK_SAP_RESIDUE,
    EVENT_JINBAK_DETONATE,

    EVENT_VOJAK_CAUSTIC_TAR,
    EVENT_VOJAK_BOMBARD,
    EVENT_VOJAK_DASHING_STRIKE,
    EVENT_VOJAK_THOUSAND_BLADES,

    EVENT_PAVALAK_BLADE_RUSH,
    EVENT_PAVALAK_TEMPEST,
    EVENT_PAVALAK_SIEGE_EXPLOSIVE,
    EVENT_PAVALAK_SERRATED_BLADE,

    EVENT_NERONOK_HURL_BRICK,
    EVENT_NERONOK_CAUSTIC_PITCH,
    EVENT_NERONOK_QUICK_DRY_RESIN,
    EVENT_NERONOK_GUSTING_WINDS,

    EVENT_WARCALLER_SHADOW_BOLT,
    EVENT_WARCALLER_CURSE,

    EVENT_FLESHRENDER_CLEAVE,
    EVENT_FLESHRENDER_ENRAGE
};

class SiegeOfNiuzaoTempleInstanceScript : public InstanceScript
{
public:
    explicit SiegeOfNiuzaoTempleInstanceScript(WorldMap* pMapMgr) : InstanceScript(pMapMgr)
    {
        setBossNumber(SiegeOfNiuzaoTempleEncounterCount);
    }

    static InstanceScript* Create(WorldMap* pMapMgr) { return new SiegeOfNiuzaoTempleInstanceScript(pMapMgr); }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Vizier Jin'bak

class JinbakAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new JinbakAI(c); }
    explicit JinbakAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameEngage, getCreature());
        getInstanceScript()->setBossState(DATA_JINBAK, EncounterStates::InProgress);

        scriptEvents.addEvent(EVENT_JINBAK_SAP_PUDDLE, 6000);
        scriptEvents.addEvent(EVENT_JINBAK_SAP_RESIDUE, 4000);
        scriptEvents.addEvent(EVENT_JINBAK_DETONATE, 26000);
    }

    void OnCombatStop(Unit* /*pTarget*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
        getInstanceScript()->setBossState(DATA_JINBAK, EncounterStates::Failed);
        scriptEvents.resetEvents();
    }

    void OnDied(Unit* /*pKiller*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
        getInstanceScript()->setBossState(DATA_JINBAK, EncounterStates::Performed);
    }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);

        if (_isCasting())
            return;

        switch (scriptEvents.getFinishedEvent())
        {
            case EVENT_JINBAK_SAP_PUDDLE:
                castSpellOnSelf(SPELL_JINBAK_SAP_PUDDLE);
                scriptEvents.addEvent(EVENT_JINBAK_SAP_PUDDLE, 30000);
                break;
            case EVENT_JINBAK_SAP_RESIDUE:
                castSpellOnVictim(SPELL_JINBAK_SAP_RESIDUE);
                scriptEvents.addEvent(EVENT_JINBAK_SAP_RESIDUE, 14000);
                break;
            case EVENT_JINBAK_DETONATE:
                sendChatMessage(CHAT_MSG_MONSTER_YELL, 0, "Let the sap consume you!");
                castSpellAOE(SPELL_JINBAK_DETONATE);
                scriptEvents.addEvent(EVENT_JINBAK_DETONATE, 32000);
                break;
            default:
                break;
        }
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Commander Vo'jak

class VojakAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new VojakAI(c); }
    explicit VojakAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameEngage, getCreature());
        getInstanceScript()->setBossState(DATA_VOJAK, EncounterStates::InProgress);

        scriptEvents.addEvent(EVENT_VOJAK_CAUSTIC_TAR, 8000);
        scriptEvents.addEvent(EVENT_VOJAK_BOMBARD, 14000);
        scriptEvents.addEvent(EVENT_VOJAK_DASHING_STRIKE, 6000);
        scriptEvents.addEvent(EVENT_VOJAK_THOUSAND_BLADES, 20000);
    }

    void OnCombatStop(Unit* /*pTarget*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
        getInstanceScript()->setBossState(DATA_VOJAK, EncounterStates::Failed);
        scriptEvents.resetEvents();
    }

    void OnDied(Unit* /*pKiller*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
        getInstanceScript()->setBossState(DATA_VOJAK, EncounterStates::Performed);
    }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);

        if (_isCasting())
            return;

        switch (scriptEvents.getFinishedEvent())
        {
            case EVENT_VOJAK_CAUSTIC_TAR:
                castSpellAOE(SPELL_VOJAK_CAUSTIC_TAR);
                scriptEvents.addEvent(EVENT_VOJAK_CAUSTIC_TAR, 22000);
                break;
            case EVENT_VOJAK_BOMBARD:
                if (Unit* pTarget = selectUnitTarget(FilterArgs(TargetFilter_Player)))
                    castSpell(pTarget, SPELL_VOJAK_BOMBARD);
                scriptEvents.addEvent(EVENT_VOJAK_BOMBARD, 18000);
                break;
            case EVENT_VOJAK_DASHING_STRIKE:
                castSpellOnVictim(SPELL_VOJAK_DASHING_STRIKE);
                scriptEvents.addEvent(EVENT_VOJAK_DASHING_STRIKE, 16000);
                break;
            case EVENT_VOJAK_THOUSAND_BLADES:
                sendChatMessage(CHAT_MSG_MONSTER_EMOTE, 0, "Commander Vo'jak spins into a flurry of blades!");
                castSpellAOE(SPELL_VOJAK_THOUSAND_BLADES);
                scriptEvents.addEvent(EVENT_VOJAK_THOUSAND_BLADES, 26000);
                break;
            default:
                break;
        }
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// General Pa'valak

class PavalakAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new PavalakAI(c); }
    explicit PavalakAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameEngage, getCreature());
        getInstanceScript()->setBossState(DATA_PAVALAK, EncounterStates::InProgress);

        scriptEvents.addEvent(EVENT_PAVALAK_BLADE_RUSH, 10000);
        scriptEvents.addEvent(EVENT_PAVALAK_TEMPEST, 16000);
        scriptEvents.addEvent(EVENT_PAVALAK_SIEGE_EXPLOSIVE, 6000);
        scriptEvents.addEvent(EVENT_PAVALAK_SERRATED_BLADE, 12000);
    }

    void OnCombatStop(Unit* /*pTarget*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
        getInstanceScript()->setBossState(DATA_PAVALAK, EncounterStates::Failed);
        scriptEvents.resetEvents();
    }

    void OnDied(Unit* /*pKiller*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
        getInstanceScript()->setBossState(DATA_PAVALAK, EncounterStates::Performed);
    }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);

        if (_isCasting())
            return;

        switch (scriptEvents.getFinishedEvent())
        {
            case EVENT_PAVALAK_BLADE_RUSH:
                castSpellOnVictim(SPELL_PAVALAK_BLADE_RUSH);
                scriptEvents.addEvent(EVENT_PAVALAK_BLADE_RUSH, 20000);
                break;
            case EVENT_PAVALAK_TEMPEST:
                castSpellAOE(SPELL_PAVALAK_TEMPEST);
                scriptEvents.addEvent(EVENT_PAVALAK_TEMPEST, 24000);
                break;
            case EVENT_PAVALAK_SIEGE_EXPLOSIVE:
                if (Unit* pTarget = selectUnitTarget(FilterArgs(TargetFilter_Player)))
                    castSpell(pTarget, SPELL_PAVALAK_SIEGE_EXPLOSIVE);
                scriptEvents.addEvent(EVENT_PAVALAK_SIEGE_EXPLOSIVE, 18000);
                break;
            case EVENT_PAVALAK_SERRATED_BLADE:
                castSpellOnVictim(SPELL_PAVALAK_SERRATED_BLADE);
                scriptEvents.addEvent(EVENT_PAVALAK_SERRATED_BLADE, 14000);
                break;
            default:
                break;
        }
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Wing Leader Ner'onok

class NeronokAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new NeronokAI(c); }
    explicit NeronokAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        sendChatMessage(CHAT_MSG_MONSTER_YELL, 0, "None shall pass while I still draw breath!");
        getInstanceScript()->sendUnitEncounter(EncounterFrameEngage, getCreature());
        getInstanceScript()->setBossState(DATA_NERONOK, EncounterStates::InProgress);

        scriptEvents.addEvent(EVENT_NERONOK_HURL_BRICK, 6000);
        scriptEvents.addEvent(EVENT_NERONOK_CAUSTIC_PITCH, 12000);
        scriptEvents.addEvent(EVENT_NERONOK_QUICK_DRY_RESIN, 20000);
        scriptEvents.addEvent(EVENT_NERONOK_GUSTING_WINDS, 26000);
    }

    void OnCombatStop(Unit* /*pTarget*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
        getInstanceScript()->setBossState(DATA_NERONOK, EncounterStates::Failed);
        scriptEvents.resetEvents();
    }

    void OnDied(Unit* /*pKiller*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
        getInstanceScript()->setBossState(DATA_NERONOK, EncounterStates::Performed);
    }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);

        if (_isCasting())
            return;

        switch (scriptEvents.getFinishedEvent())
        {
            case EVENT_NERONOK_HURL_BRICK:
                if (Unit* pTarget = selectUnitTarget(FilterArgs(TargetFilter_Player)))
                    castSpell(pTarget, SPELL_NERONOK_HURL_BRICK);
                scriptEvents.addEvent(EVENT_NERONOK_HURL_BRICK, 18000);
                break;
            case EVENT_NERONOK_CAUSTIC_PITCH:
                castSpellAOE(SPELL_NERONOK_CAUSTIC_PITCH);
                scriptEvents.addEvent(EVENT_NERONOK_CAUSTIC_PITCH, 24000);
                break;
            case EVENT_NERONOK_QUICK_DRY_RESIN:
                castSpellOnVictim(SPELL_NERONOK_QUICK_DRY_RESIN);
                scriptEvents.addEvent(EVENT_NERONOK_QUICK_DRY_RESIN, 22000);
                break;
            case EVENT_NERONOK_GUSTING_WINDS:
                castSpellAOE(SPELL_NERONOK_GUSTING_WINDS);
                scriptEvents.addEvent(EVENT_NERONOK_GUSTING_WINDS, 28000);
                break;
            default:
                break;
        }
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Trash - Sra'thik Warcaller

class SrathikWarcallerAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new SrathikWarcallerAI(c); }
    explicit SrathikWarcallerAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        scriptEvents.addEvent(EVENT_WARCALLER_SHADOW_BOLT, 4000);
        scriptEvents.addEvent(EVENT_WARCALLER_CURSE, 9000);
    }

    void OnCombatStop(Unit* /*pTarget*/) override { scriptEvents.resetEvents(); }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);
        if (_isCasting())
            return;

        switch (scriptEvents.getFinishedEvent())
        {
            case EVENT_WARCALLER_SHADOW_BOLT:
                if (Unit* pTarget = selectUnitTarget(FilterArgs(TargetFilter_Player)))
                    castSpell(pTarget, SPELL_WARCALLER_SHADOW_BOLT);
                scriptEvents.addEvent(EVENT_WARCALLER_SHADOW_BOLT, 14000);
                break;
            case EVENT_WARCALLER_CURSE:
                castSpellOnVictim(SPELL_WARCALLER_CURSE);
                scriptEvents.addEvent(EVENT_WARCALLER_CURSE, 20000);
                break;
            default:
                break;
        }
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Trash - Sra'thik Fleshrender

class SrathikFleshrenderAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new SrathikFleshrenderAI(c); }
    explicit SrathikFleshrenderAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        scriptEvents.addEvent(EVENT_FLESHRENDER_CLEAVE, 6000);
        scriptEvents.addEvent(EVENT_FLESHRENDER_ENRAGE, 16000);
    }

    void OnCombatStop(Unit* /*pTarget*/) override { scriptEvents.resetEvents(); }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);
        if (_isCasting())
            return;

        switch (scriptEvents.getFinishedEvent())
        {
            case EVENT_FLESHRENDER_CLEAVE:
                castSpellOnVictim(SPELL_FLESHRENDER_CLEAVE);
                scriptEvents.addEvent(EVENT_FLESHRENDER_CLEAVE, 12000);
                break;
            case EVENT_FLESHRENDER_ENRAGE:
                castSpellOnSelf(SPELL_FLESHRENDER_ENRAGE);
                scriptEvents.addEvent(EVENT_FLESHRENDER_ENRAGE, 26000);
                break;
            default:
                break;
        }
    }
};

void SetupSiegeOfNiuzaoTemple(ScriptMgr* mgr)
{
    mgr->register_instance_script(MAP_SIEGE_OF_NIUZAO_TEMPLE, &SiegeOfNiuzaoTempleInstanceScript::Create);

    mgr->register_creature_script(BOSS_JINBAK, &JinbakAI::Create);
    mgr->register_creature_script(BOSS_VOJAK, &VojakAI::Create);
    mgr->register_creature_script(BOSS_PAVALAK, &PavalakAI::Create);
    mgr->register_creature_script(BOSS_NERONOK, &NeronokAI::Create);

    mgr->register_creature_script(NPC_SRATHIK_WARCALLER, &SrathikWarcallerAI::Create);
    mgr->register_creature_script(NPC_SRATHIK_FLESHRENDER, &SrathikFleshrenderAI::Create);
}
