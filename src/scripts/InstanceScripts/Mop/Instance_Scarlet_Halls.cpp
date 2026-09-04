/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

// Scarlet Halls - three bosses (verified against wowhead's MoP Classic data), hand-ported
// into AscEmu's own CreatureAIScript / InstanceScript framework.

#include "Setup.h"
#include "Instance_Scarlet_Halls.hpp"

#include "Server/Script/CreatureAIScript.hpp"
#include "Server/Script/InstanceScript.hpp"
#include "Server/Script/ScriptMgr.hpp"

enum ScarletHallsEvents
{
    EVENT_BRAUN_PIERCING_THROW = 1,
    EVENT_BRAUN_DEATH_BLOSSOM,
    EVENT_BRAUN_CALL_DOG,

    EVENT_HARLAN_HEROIC_LEAP,
    EVENT_HARLAN_BLADES_OF_LIGHT,
    EVENT_HARLAN_DRAGONS_REACH,

    EVENT_KOEGLER_BOOK_BURNER,
    EVENT_KOEGLER_GREATER_DRAGONS_BREATH,
    EVENT_KOEGLER_FIREBALL_VOLLEY,
    EVENT_KOEGLER_PYROBLAST,

    EVENT_MASTER_ARCHER_MULTI_SHOT,
    EVENT_MASTER_ARCHER_CLEAVE,

    EVENT_EVANGELIST_SMITE,
    EVENT_EVANGELIST_HEAL,

    EVENT_STARVING_HOUND_CLEAVE,
    EVENT_STARVING_HOUND_ENRAGE
};

class ScarletHallsInstanceScript : public InstanceScript
{
public:
    explicit ScarletHallsInstanceScript(WorldMap* pMapMgr) : InstanceScript(pMapMgr)
    {
        setBossNumber(ScarletHallsEncounterCount);
    }

    static InstanceScript* Create(WorldMap* pMapMgr) { return new ScarletHallsInstanceScript(pMapMgr); }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Houndmaster Braun

class HoundmasterBraunAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new HoundmasterBraunAI(c); }
    explicit HoundmasterBraunAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameEngage, getCreature());
        getInstanceScript()->setBossState(DATA_HOUNDMASTER_BRAUN, EncounterStates::InProgress);

        mBloodyRageCast = false;
        scriptEvents.addEvent(EVENT_BRAUN_PIERCING_THROW, 5000);
        scriptEvents.addEvent(EVENT_BRAUN_DEATH_BLOSSOM, 12000);
        scriptEvents.addEvent(EVENT_BRAUN_CALL_DOG, 15000);
    }

    void OnCombatStop(Unit* /*pTarget*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
        getInstanceScript()->setBossState(DATA_HOUNDMASTER_BRAUN, EncounterStates::Failed);
        scriptEvents.resetEvents();
    }

    void OnDied(Unit* /*pKiller*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
        getInstanceScript()->setBossState(DATA_HOUNDMASTER_BRAUN, EncounterStates::Performed);
    }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);

        if (!mBloodyRageCast && _getHealthPercent() <= 50)
        {
            mBloodyRageCast = true;
            sendChatMessage(CHAT_MSG_MONSTER_YELL, 0, "The hounds won't listen... I'll do this myself!");
            castSpellOnSelf(SPELL_BRAUN_BLOODY_RAGE);
        }

        if (_isCasting())
            return;

        switch (scriptEvents.getFinishedEvent())
        {
            case EVENT_BRAUN_PIERCING_THROW:
                if (Unit* pTarget = selectUnitTarget(FilterArgs(TargetFilter_Player)))
                    castSpell(pTarget, SPELL_BRAUN_PIERCING_THROW);
                scriptEvents.addEvent(EVENT_BRAUN_PIERCING_THROW, 10000);
                break;
            case EVENT_BRAUN_DEATH_BLOSSOM:
                castSpellOnVictim(SPELL_BRAUN_DEATH_BLOSSOM);
                scriptEvents.addEvent(EVENT_BRAUN_DEATH_BLOSSOM, 18000);
                break;
            case EVENT_BRAUN_CALL_DOG:
                sendChatMessage(CHAT_MSG_MONSTER_EMOTE, 0, "Houndmaster Braun calls for another hound!");
                castSpellOnSelf(SPELL_BRAUN_CALL_DOG);
                scriptEvents.addEvent(EVENT_BRAUN_CALL_DOG, 20000);
                break;
            default:
                break;
        }
    }

private:
    bool mBloodyRageCast = false;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Armsmaster Harlan

class ArmsmasterHarlanAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new ArmsmasterHarlanAI(c); }
    explicit ArmsmasterHarlanAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    // Reference dialogue data for entry 58632 also has an "On your guard!" line (sound 29426)
    // with no separate cast point of its own - his real fight opens by calling on two allies
    // to join him (EMOTE, no sound), fired here alongside the aggro line since this port
    // doesn't summon separate add NPCs for that moment.
    void OnCombatStart(Unit* /*pTarget*/) override
    {
        sendChatMessage(CHAT_MSG_MONSTER_YELL, 29428, "Ah-hah! Another chance to test my might.");
        sendChatMessage(CHAT_MSG_MONSTER_EMOTE, 0, "Armsmaster Harlan calls on two of his allies to join the fight!");
        getInstanceScript()->sendUnitEncounter(EncounterFrameEngage, getCreature());
        getInstanceScript()->setBossState(DATA_ARMSMASTER_HARLAN, EncounterStates::InProgress);

        scriptEvents.addEvent(EVENT_HARLAN_HEROIC_LEAP, 14000);
        scriptEvents.addEvent(EVENT_HARLAN_BLADES_OF_LIGHT, 20000);
        scriptEvents.addEvent(EVENT_HARLAN_DRAGONS_REACH, 8000);
    }

    void OnCombatStop(Unit* /*pTarget*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
        getInstanceScript()->setBossState(DATA_ARMSMASTER_HARLAN, EncounterStates::Failed);
        scriptEvents.resetEvents();
    }

    void OnDied(Unit* /*pKiller*/) override
    {
        sendChatMessage(CHAT_MSG_MONSTER_YELL, 29427, "Bested... by the likes of...");
        getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
        getInstanceScript()->setBossState(DATA_ARMSMASTER_HARLAN, EncounterStates::Performed);
    }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);

        if (_isCasting())
            return;

        switch (scriptEvents.getFinishedEvent())
        {
            case EVENT_HARLAN_HEROIC_LEAP:
                if (Unit* pTarget = selectUnitTarget(FilterArgs(TargetFilter_Player)))
                    castSpell(pTarget, SPELL_HARLAN_HEROIC_LEAP);
                scriptEvents.addEvent(EVENT_HARLAN_HEROIC_LEAP, 24000);
                break;
            case EVENT_HARLAN_BLADES_OF_LIGHT:
                sendChatMessage(CHAT_MSG_MONSTER_EMOTE, 0, "Armsmaster Harlan casts Blades of Light!");
                castSpellAOE(SPELL_HARLAN_BLADES_OF_LIGHT);
                scriptEvents.addEvent(EVENT_HARLAN_BLADES_OF_LIGHT, 30000);
                break;
            case EVENT_HARLAN_DRAGONS_REACH:
                castSpellOnVictim(SPELL_HARLAN_DRAGONS_REACH);
                scriptEvents.addEvent(EVENT_HARLAN_DRAGONS_REACH, 16000);
                break;
            default:
                break;
        }
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Flameweaver Koegler

class FlameweaverKoeglerAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new FlameweaverKoeglerAI(c); }
    explicit FlameweaverKoeglerAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        sendChatMessage(CHAT_MSG_MONSTER_YELL, 0, "The flames of the Light will cleanse you!");
        getInstanceScript()->sendUnitEncounter(EncounterFrameEngage, getCreature());
        getInstanceScript()->setBossState(DATA_FLAMEWEAVER_KOEGLER, EncounterStates::InProgress);

        scriptEvents.addEvent(EVENT_KOEGLER_BOOK_BURNER, 10000);
        scriptEvents.addEvent(EVENT_KOEGLER_GREATER_DRAGONS_BREATH, 16000);
        scriptEvents.addEvent(EVENT_KOEGLER_FIREBALL_VOLLEY, 6000);
        scriptEvents.addEvent(EVENT_KOEGLER_PYROBLAST, 22000);
    }

    void OnCombatStop(Unit* /*pTarget*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
        getInstanceScript()->setBossState(DATA_FLAMEWEAVER_KOEGLER, EncounterStates::Failed);
        scriptEvents.resetEvents();
    }

    void OnDied(Unit* /*pKiller*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
        getInstanceScript()->setBossState(DATA_FLAMEWEAVER_KOEGLER, EncounterStates::Performed);
    }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);

        if (_isCasting())
            return;

        switch (scriptEvents.getFinishedEvent())
        {
            case EVENT_KOEGLER_BOOK_BURNER:
                castSpellAOE(SPELL_KOEGLER_BOOK_BURNER);
                scriptEvents.addEvent(EVENT_KOEGLER_BOOK_BURNER, 24000);
                break;
            case EVENT_KOEGLER_GREATER_DRAGONS_BREATH:
                castSpellOnVictim(SPELL_KOEGLER_GREATER_DRAGONS_BREATH);
                scriptEvents.addEvent(EVENT_KOEGLER_GREATER_DRAGONS_BREATH, 20000);
                break;
            case EVENT_KOEGLER_FIREBALL_VOLLEY:
                if (Unit* pTarget = selectUnitTarget(FilterArgs(TargetFilter_Player)))
                    castSpell(pTarget, SPELL_KOEGLER_FIREBALL_VOLLEY);
                scriptEvents.addEvent(EVENT_KOEGLER_FIREBALL_VOLLEY, 14000);
                break;
            case EVENT_KOEGLER_PYROBLAST:
                if (Unit* pTarget = selectUnitTarget(FilterArgs(TargetFilter_Player)))
                    castSpell(pTarget, SPELL_KOEGLER_PYROBLAST);
                scriptEvents.addEvent(EVENT_KOEGLER_PYROBLAST, 26000);
                break;
            default:
                break;
        }
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Trash - Master Archer

class MasterArcherAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new MasterArcherAI(c); }
    explicit MasterArcherAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        scriptEvents.addEvent(EVENT_MASTER_ARCHER_MULTI_SHOT, 5000);
        scriptEvents.addEvent(EVENT_MASTER_ARCHER_CLEAVE, 10000);
    }

    void OnCombatStop(Unit* /*pTarget*/) override { scriptEvents.resetEvents(); }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);
        if (_isCasting())
            return;

        switch (scriptEvents.getFinishedEvent())
        {
            case EVENT_MASTER_ARCHER_MULTI_SHOT:
                castSpellOnVictim(SPELL_MASTER_ARCHER_MULTI_SHOT);
                scriptEvents.addEvent(EVENT_MASTER_ARCHER_MULTI_SHOT, 14000);
                break;
            case EVENT_MASTER_ARCHER_CLEAVE:
                castSpellOnVictim(SPELL_MASTER_ARCHER_CLEAVE);
                scriptEvents.addEvent(EVENT_MASTER_ARCHER_CLEAVE, 12000);
                break;
            default:
                break;
        }
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Trash - Scarlet Evangelist

class ScarletEvangelistAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new ScarletEvangelistAI(c); }
    explicit ScarletEvangelistAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        scriptEvents.addEvent(EVENT_EVANGELIST_SMITE, 4000);
        scriptEvents.addEvent(EVENT_EVANGELIST_HEAL, 9000);
    }

    void OnCombatStop(Unit* /*pTarget*/) override { scriptEvents.resetEvents(); }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);
        if (_isCasting())
            return;

        switch (scriptEvents.getFinishedEvent())
        {
            case EVENT_EVANGELIST_SMITE:
                castSpellOnVictim(SPELL_EVANGELIST_SMITE);
                scriptEvents.addEvent(EVENT_EVANGELIST_SMITE, 10000);
                break;
            case EVENT_EVANGELIST_HEAL:
                castSpellOnSelf(SPELL_EVANGELIST_HEAL);
                scriptEvents.addEvent(EVENT_EVANGELIST_HEAL, 18000);
                break;
            default:
                break;
        }
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Trash - Starving Hound

class StarvingHoundAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new StarvingHoundAI(c); }
    explicit StarvingHoundAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        scriptEvents.addEvent(EVENT_STARVING_HOUND_CLEAVE, 6000);
        scriptEvents.addEvent(EVENT_STARVING_HOUND_ENRAGE, 16000);
    }

    void OnCombatStop(Unit* /*pTarget*/) override { scriptEvents.resetEvents(); }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);
        if (_isCasting())
            return;

        switch (scriptEvents.getFinishedEvent())
        {
            case EVENT_STARVING_HOUND_CLEAVE:
                castSpellOnVictim(SPELL_STARVING_HOUND_CLEAVE);
                scriptEvents.addEvent(EVENT_STARVING_HOUND_CLEAVE, 12000);
                break;
            case EVENT_STARVING_HOUND_ENRAGE:
                castSpellOnSelf(SPELL_STARVING_HOUND_ENRAGE);
                scriptEvents.addEvent(EVENT_STARVING_HOUND_ENRAGE, 26000);
                break;
            default:
                break;
        }
    }
};

void SetupScarletHalls(ScriptMgr* mgr)
{
    mgr->register_instance_script(MAP_SCARLET_HALLS, &ScarletHallsInstanceScript::Create);

    mgr->register_creature_script(BOSS_HOUNDMASTER_BRAUN, &HoundmasterBraunAI::Create);
    mgr->register_creature_script(BOSS_ARMSMASTER_HARLAN, &ArmsmasterHarlanAI::Create);
    mgr->register_creature_script(BOSS_FLAMEWEAVER_KOEGLER, &FlameweaverKoeglerAI::Create);

    mgr->register_creature_script(NPC_MASTER_ARCHER, &MasterArcherAI::Create);
    mgr->register_creature_script(NPC_SCARLET_EVANGELIST, &ScarletEvangelistAI::Create);
    mgr->register_creature_script(NPC_STARVING_HOUND, &StarvingHoundAI::Create);
}
