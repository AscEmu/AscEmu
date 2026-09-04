/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

// Throne of the Four Winds - two encounters (verified against wowhead), hand-ported into
// our own CreatureAIScript / InstanceScript framework. We had no instance
// script for this raid before.
//
// Simplifications versus the original fight design: both encounters revolve heavily around
// vehicle-mounted platform traversal (hurricane vehicles to reach the Conclave's center
// platform, Al'Akir's edge-of-the-world knockback and lightning-rod vehicle phases). None of
// that is ported - each boss instead runs a simplified but functional core rotation.

#include "Setup.h"
#include "Raid_Throne_Of_The_Four_Winds.hpp"

#include "Server/Script/CreatureAIScript.hpp"
#include "Server/Script/InstanceScript.hpp"
#include "Server/Script/ScriptMgr.hpp"

enum ThroneOfTheFourWindsEvents
{
    EVENT_ANSHAL_NURTURE = 1,
    EVENT_ANSHAL_TOXIC_SPORES,
    EVENT_ANSHAL_ZEPHYR,

    EVENT_NEZIR_PERMAFROST,
    EVENT_NEZIR_ICE_PATCH,
    EVENT_NEZIR_SLEET_STORM,

    EVENT_ROHASH_SLICING_GALE,
    EVENT_ROHASH_WIND_BLAST,

    EVENT_ALAKIR_WIND_BURST,
    EVENT_ALAKIR_ICE_STORM,
    EVENT_ALAKIR_ACID_RAIN,
    EVENT_ALAKIR_LIGHTNING,
    EVENT_ALAKIR_SQUALL_LINE,
    EVENT_ALAKIR_STATIC_SHOCK,
    EVENT_ALAKIR_FEEDBACK,
    EVENT_ALAKIR_LIGHTNING_ROD
};

class ThroneOfTheFourWindsInstanceScript : public InstanceScript
{
public:
    explicit ThroneOfTheFourWindsInstanceScript(WorldMap* pMapMgr) : InstanceScript(pMapMgr)
    {
        setBossNumber(ThroneOfTheFourWindsEncounterCount);
    }

    static InstanceScript* Create(WorldMap* pMapMgr) { return new ThroneOfTheFourWindsInstanceScript(pMapMgr); }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Conclave of Wind - Anshal / Nezir / Rohash, fought together as one encounter.

class ConclaveMemberAI : public CreatureAIScript
{
public:
    explicit ConclaveMemberAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameEngage, getCreature());
        getInstanceScript()->setBossState(DATA_CONCLAVE_OF_WIND, EncounterStates::InProgress);
    }

    void OnCombatStop(Unit* /*pTarget*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
        getInstanceScript()->setBossState(DATA_CONCLAVE_OF_WIND, EncounterStates::Failed);
        scriptEvents.resetEvents();
    }

    void OnDied(Unit* /*pKiller*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
    }
};

class AnshalAI : public ConclaveMemberAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new AnshalAI(c); }
    explicit AnshalAI(Creature* pCreature) : ConclaveMemberAI(pCreature) {}

    void OnCombatStart(Unit* pTarget) override
    {
        sendChatMessage(CHAT_MSG_MONSTER_YELL, 0, "It shall be I that earns the favor of our lord by casting out the intruders. My calmest wind shall still prove too much for them!");
        ConclaveMemberAI::OnCombatStart(pTarget);
        scriptEvents.addEvent(EVENT_ANSHAL_NURTURE, 27000);
        scriptEvents.addEvent(EVENT_ANSHAL_TOXIC_SPORES, 20500);
        scriptEvents.addEvent(EVENT_ANSHAL_ZEPHYR, 15000);
    }

    void OnDied(Unit* /*pKiller*/) override
    {
        sendChatMessage(CHAT_MSG_MONSTER_YELL, 0, "Your presence shall no longer defile our home!");
        getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
    }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);

        if (_isCasting())
            return;

        switch (scriptEvents.getFinishedEvent())
        {
            case EVENT_ANSHAL_NURTURE:
                castSpellOnSelf(SPELL_ANSHAL_NURTURE);
                scriptEvents.addEvent(EVENT_ANSHAL_NURTURE, 30000);
                break;
            case EVENT_ANSHAL_TOXIC_SPORES:
                castSpellAOE(SPELL_ANSHAL_TOXIC_SPORES);
                scriptEvents.addEvent(EVENT_ANSHAL_TOXIC_SPORES, 24000);
                break;
            case EVENT_ANSHAL_ZEPHYR:
                sendChatMessage(CHAT_MSG_MONSTER_EMOTE, 0, "Anshal channels Zephyr, healing his allies!");
                castSpellOnSelf(SPELL_ANSHAL_ZEPHYR);
                scriptEvents.addEvent(EVENT_ANSHAL_ZEPHYR, 32000);
                break;
            default:
                break;
        }
    }
};

class NezirAI : public ConclaveMemberAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new NezirAI(c); }
    explicit NezirAI(Creature* pCreature) : ConclaveMemberAI(pCreature) {}

    void OnCombatStart(Unit* pTarget) override
    {
        sendChatMessage(CHAT_MSG_MONSTER_YELL, 0, "The honor of slaying the interlopers shall be mine, brothers! Their feeble bodies will freeze solid from my wind's icy chill!");
        ConclaveMemberAI::OnCombatStart(pTarget);
        scriptEvents.addEvent(EVENT_NEZIR_SLEET_STORM, 11000);
        scriptEvents.addEvent(EVENT_NEZIR_PERMAFROST, 12000);
        scriptEvents.addEvent(EVENT_NEZIR_ICE_PATCH, 14000);
    }

    void OnDied(Unit* /*pKiller*/) override
    {
        sendChatMessage(CHAT_MSG_MONSTER_YELL, 0, "Frozen solid.");
        getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
    }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);

        if (_isCasting())
            return;

        switch (scriptEvents.getFinishedEvent())
        {
            case EVENT_NEZIR_SLEET_STORM:
                castSpellAOE(SPELL_NEZIR_SLEET_STORM);
                scriptEvents.addEvent(EVENT_NEZIR_SLEET_STORM, 20000);
                break;
            case EVENT_NEZIR_PERMAFROST:
                castSpellOnVictim(SPELL_NEZIR_PERMAFROST);
                scriptEvents.addEvent(EVENT_NEZIR_PERMAFROST, 18000);
                break;
            case EVENT_NEZIR_ICE_PATCH:
                if (Unit* pTarget = selectUnitTarget(FilterArgs(TargetFilter_Player)))
                    castSpell(pTarget, SPELL_NEZIR_ICE_PATCH);
                scriptEvents.addEvent(EVENT_NEZIR_ICE_PATCH, 22000);
                break;
            default:
                break;
        }
    }
};

class RohashAI : public ConclaveMemberAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new RohashAI(c); }
    explicit RohashAI(Creature* pCreature) : ConclaveMemberAI(pCreature) {}

    void OnCombatStart(Unit* pTarget) override
    {
        sendChatMessage(CHAT_MSG_MONSTER_YELL, 0, "As I am the strongest wind, it shall be I that tears the invaders apart!");
        ConclaveMemberAI::OnCombatStart(pTarget);
        scriptEvents.addEvent(EVENT_ROHASH_SLICING_GALE, 10000);
        scriptEvents.addEvent(EVENT_ROHASH_WIND_BLAST, 16000);
    }

    void OnDied(Unit* /*pKiller*/) override
    {
        sendChatMessage(CHAT_MSG_MONSTER_YELL, 0, "Blown away!");
        getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
    }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);

        if (_isCasting())
            return;

        switch (scriptEvents.getFinishedEvent())
        {
            case EVENT_ROHASH_SLICING_GALE:
                castSpellAOE(SPELL_ROHASH_SLICING_GALE);
                scriptEvents.addEvent(EVENT_ROHASH_SLICING_GALE, 18000);
                break;
            case EVENT_ROHASH_WIND_BLAST:
                sendChatMessage(CHAT_MSG_MONSTER_EMOTE, 0, "Rohash begins to cast Wind Blast!");
                if (Unit* pTarget = selectUnitTarget(FilterArgs(TargetFilter_Player)))
                    castSpell(pTarget, SPELL_ROHASH_WIND_BLAST);
                scriptEvents.addEvent(EVENT_ROHASH_WIND_BLAST, 20000);
                break;
            default:
                break;
        }
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Al'Akir

class AlakirAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new AlakirAI(c); }
    explicit AlakirAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        sendChatMessage(CHAT_MSG_MONSTER_YELL, 0, "Your challenge is accepted, mortals! Know that you face Al'Akir, Elemental Lord of Air! You have no hope of defeating me!");
        getInstanceScript()->sendUnitEncounter(EncounterFrameEngage, getCreature());
        getInstanceScript()->setBossState(DATA_ALAKIR, EncounterStates::InProgress);

        scriptEvents.addEvent(EVENT_ALAKIR_ICE_STORM, 5000);
        scriptEvents.addEvent(EVENT_ALAKIR_LIGHTNING, 9000);
        scriptEvents.addEvent(EVENT_ALAKIR_WIND_BURST, 23000);
        scriptEvents.addEvent(EVENT_ALAKIR_ACID_RAIN, 15000);
        scriptEvents.addEvent(EVENT_ALAKIR_SQUALL_LINE, 12000);
        scriptEvents.addEvent(EVENT_ALAKIR_STATIC_SHOCK, 7000);
        scriptEvents.addEvent(EVENT_ALAKIR_FEEDBACK, 27000);
        scriptEvents.addEvent(EVENT_ALAKIR_LIGHTNING_ROD, 19000);
    }

    void OnCombatStop(Unit* /*pTarget*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
        getInstanceScript()->setBossState(DATA_ALAKIR, EncounterStates::Failed);
        scriptEvents.resetEvents();
    }

    void OnDied(Unit* /*pKiller*/) override
    {
        sendChatMessage(CHAT_MSG_MONSTER_YELL, 0, "This little one shall vex me no longer.");
        getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
        getInstanceScript()->setBossState(DATA_ALAKIR, EncounterStates::Performed);
    }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);

        if (_isCasting())
            return;

        switch (scriptEvents.getFinishedEvent())
        {
            case EVENT_ALAKIR_ICE_STORM:
                castSpellAOE(SPELL_ALAKIR_ICE_STORM);
                scriptEvents.addEvent(EVENT_ALAKIR_ICE_STORM, 20000);
                break;
            case EVENT_ALAKIR_LIGHTNING:
                if (Unit* pTarget = selectUnitTarget(FilterArgs(TargetFilter_Player)))
                    castSpell(pTarget, SPELL_ALAKIR_LIGHTNING);
                scriptEvents.addEvent(EVENT_ALAKIR_LIGHTNING, 18000);
                break;
            case EVENT_ALAKIR_WIND_BURST:
                sendChatMessage(CHAT_MSG_MONSTER_EMOTE, 0, "Al'Akir begins to cast Wind Burst!");
                castSpellAOE(SPELL_ALAKIR_WIND_BURST);
                scriptEvents.addEvent(EVENT_ALAKIR_WIND_BURST, 25000);
                break;
            case EVENT_ALAKIR_ACID_RAIN:
                castSpellOnVictim(SPELL_ALAKIR_ACID_RAIN);
                scriptEvents.addEvent(EVENT_ALAKIR_ACID_RAIN, 22000);
                break;
            case EVENT_ALAKIR_SQUALL_LINE:
                castSpellAOE(SPELL_ALAKIR_SQUALL_LINE);
                scriptEvents.addEvent(EVENT_ALAKIR_SQUALL_LINE, 24000);
                break;
            case EVENT_ALAKIR_STATIC_SHOCK:
                if (Unit* pTarget = selectUnitTarget(FilterArgs(TargetFilter_Player)))
                    castSpell(pTarget, SPELL_ALAKIR_STATIC_SHOCK);
                scriptEvents.addEvent(EVENT_ALAKIR_STATIC_SHOCK, 16000);
                break;
            case EVENT_ALAKIR_FEEDBACK:
                castSpellOnSelf(SPELL_ALAKIR_FEEDBACK);
                scriptEvents.addEvent(EVENT_ALAKIR_FEEDBACK, 30000);
                break;
            case EVENT_ALAKIR_LIGHTNING_ROD:
                if (Unit* pTarget = selectUnitTarget(FilterArgs(TargetFilter_Player)))
                    castSpell(pTarget, SPELL_ALAKIR_LIGHTNING_ROD);
                scriptEvents.addEvent(EVENT_ALAKIR_LIGHTNING_ROD, 26000);
                break;
            default:
                break;
        }
    }
};

void SetupThroneOfTheFourWinds(ScriptMgr* mgr)
{
    mgr->register_instance_script(MAP_THRONE_OF_FOUR_WINDS, &ThroneOfTheFourWindsInstanceScript::Create);

    mgr->register_creature_script(BOSS_ANSHAL, &AnshalAI::Create);
    mgr->register_creature_script(BOSS_NEZIR, &NezirAI::Create);
    mgr->register_creature_script(BOSS_ROHASH, &RohashAI::Create);
    mgr->register_creature_script(BOSS_ALAKIR, &AlakirAI::Create);
}
