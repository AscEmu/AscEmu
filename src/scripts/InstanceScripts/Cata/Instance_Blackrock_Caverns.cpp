/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

// Blackrock Caverns - five bosses (verified against wowhead), hand-ported into AscEmu's own
// CreatureAIScript / InstanceScript framework.
//
// Simplifications versus the original fight design: Karsh Steelbender's signature "copy a
// party member's weapon" transform mechanic and Corla's out-of-combat "Evolution" pre-pull
// buildup are dropped; both bosses instead run a simplified but functional core rotation.

#include "Setup.h"
#include "Instance_Blackrock_Caverns.hpp"

#include "Server/Script/CreatureAIScript.hpp"
#include "Server/Script/InstanceScript.hpp"
#include "Server/Script/ScriptMgr.hpp"

enum BlackrockCavernsEvents
{
    EVENT_ROMOGG_CHAINS_OF_WOE = 1,
    EVENT_ROMOGG_QUAKE,
    EVENT_ROMOGG_WOUNDING_STRIKE,

    EVENT_CORLA_EVOLUTION,
    EVENT_CORLA_DARK_COMMAND,

    EVENT_KARSH_CLEAVE,
    EVENT_KARSH_HEAT_WAVE,

    EVENT_BEAUTY_MAGMA_SPIT,
    EVENT_BEAUTY_BERSERKER_CHARGE,
    EVENT_BEAUTY_FLAMEBREAK,
    EVENT_BEAUTY_TERRIFYING_ROAR,

    EVENT_OBSIDIUS_STONE_BLOW,
    EVENT_OBSIDIUS_TWILIGHT_CORRUPTION,
    EVENT_OBSIDIUS_THUNDERCLAP,

    EVENT_FLAME_CALLER_BLAST_WAVE,
    EVENT_FLAME_CALLER_CALL_FLAMES,

    EVENT_TORTURER_RED_HOT_POKER,
    EVENT_TORTURER_WILD_BEATDOWN,

    EVENT_SADIST_HEAT_SEEKER_BLADE,
    EVENT_SADIST_SINISTER_STRIKE,

    EVENT_PRISONER_HEAD_CRACK,
    EVENT_PRISONER_INFECTED_WOUND
};

class BlackrockCavernsInstanceScript : public InstanceScript
{
public:
    explicit BlackrockCavernsInstanceScript(WorldMap* pMapMgr) : InstanceScript(pMapMgr)
    {
        setBossNumber(BlackrockCavernsEncounterCount);
    }

    static InstanceScript* Create(WorldMap* pMapMgr) { return new BlackrockCavernsInstanceScript(pMapMgr); }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Rom'ogg Bonecrusher

class RomoggBonecrusherAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new RomoggBonecrusherAI(c); }
    explicit RomoggBonecrusherAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    // Reference dialogue data for entry 39665 also has an "%s calls for help!" emote (no sound)
    // with no adjacent-pack-alert mechanic modeled for this boss to attach it to.
    void OnCombatStart(Unit* /*pTarget*/) override
    {
        sendChatMessage(CHAT_MSG_MONSTER_YELL, 18925, "Boss Cho'gall not gonna be happy 'bout dis!");
        getInstanceScript()->sendUnitEncounter(EncounterFrameEngage, getCreature());
        getInstanceScript()->setBossState(DATA_ROMOGG_BONECRUSHER, EncounterStates::InProgress);

        scriptEvents.addEvent(EVENT_ROMOGG_CHAINS_OF_WOE, 30000);
        scriptEvents.addEvent(EVENT_ROMOGG_WOUNDING_STRIKE, 16000);
        scriptEvents.addEvent(EVENT_ROMOGG_QUAKE, 24000);
    }

    void OnCombatStop(Unit* /*pTarget*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
        getInstanceScript()->setBossState(DATA_ROMOGG_BONECRUSHER, EncounterStates::Failed);
        scriptEvents.resetEvents();
    }

    void OnTargetDied(Unit* /*pTarget*/) override
    {
        sendChatMessage(CHAT_MSG_MONSTER_YELL, 18926, "Dat's what you get! Noting!");
    }

    void OnDied(Unit* /*pKiller*/) override
    {
        sendChatMessage(CHAT_MSG_MONSTER_YELL, 18928, "Rom'ogg sorry...");
        getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
        getInstanceScript()->setBossState(DATA_ROMOGG_BONECRUSHER, EncounterStates::Performed);
    }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);

        if (_isCasting())
            return;

        switch (scriptEvents.getFinishedEvent())
        {
            case EVENT_ROMOGG_CHAINS_OF_WOE:
                sendChatMessage(CHAT_MSG_MONSTER_YELL, 18927, "Stand still! Rom'ogg crack your skulls!");
                sendChatMessage(CHAT_MSG_MONSTER_EMOTE, 0, "Rom'ogg prepares to unleash The Skullcracker on nearby enemies!");
                if (Unit* pTarget = selectUnitTarget(FilterArgs(TargetFilter_Player)))
                    castSpell(pTarget, SPELL_ROMOGG_CHAINS_OF_WOE);
                scriptEvents.addEvent(EVENT_ROMOGG_CHAINS_OF_WOE, 30000);
                break;
            case EVENT_ROMOGG_QUAKE:
                castSpellAOE(SPELL_ROMOGG_QUAKE);
                scriptEvents.addEvent(EVENT_ROMOGG_QUAKE, 24000);
                break;
            case EVENT_ROMOGG_WOUNDING_STRIKE:
                castSpellOnVictim(SPELL_ROMOGG_WOUNDING_STRIKE);
                scriptEvents.addEvent(EVENT_ROMOGG_WOUNDING_STRIKE, 13500);
                break;
            default:
                break;
        }
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Corla, Herald of Twilight

class CorlaAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new CorlaAI(c); }
    explicit CorlaAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        sendChatMessage(CHAT_MSG_MONSTER_YELL, 18589, "HERETICS! You will suffer for this interruption!");
        getInstanceScript()->sendUnitEncounter(EncounterFrameEngage, getCreature());
        getInstanceScript()->setBossState(DATA_CORLA_HERALD_OF_TWILIGHT, EncounterStates::InProgress);

        scriptEvents.addEvent(EVENT_CORLA_EVOLUTION, 17000);
        scriptEvents.addEvent(EVENT_CORLA_DARK_COMMAND, 24000);
    }

    void OnCombatStop(Unit* /*pTarget*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
        getInstanceScript()->setBossState(DATA_CORLA_HERALD_OF_TWILIGHT, EncounterStates::Failed);
        scriptEvents.resetEvents();
    }

    void OnTargetDied(Unit* /*pTarget*/) override
    {
        sendChatMessage(CHAT_MSG_MONSTER_YELL, 18590, "There is only one true path of enlightenment! DEATH!");
    }

    void OnDied(Unit* /*pKiller*/) override
    {
        sendChatMessage(CHAT_MSG_MONSTER_YELL, 18594, "For the master I'd die a thousand times... A thousan...");
        getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
        getInstanceScript()->setBossState(DATA_CORLA_HERALD_OF_TWILIGHT, EncounterStates::Performed);
    }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);

        if (_isCasting())
            return;

        switch (scriptEvents.getFinishedEvent())
        {
            case EVENT_CORLA_EVOLUTION:
                sendChatMessage(CHAT_MSG_MONSTER_YELL, 18592, "Bask in his power! Rise as an agent of the master's rage!");
                sendChatMessage(CHAT_MSG_MONSTER_EMOTE, 0, "A Twilight Zealot has evolved!");
                castSpellOnSelf(SPELL_CORLA_EVOLUTION);
                scriptEvents.addEvent(EVENT_CORLA_EVOLUTION, 25000);
                break;
            case EVENT_CORLA_DARK_COMMAND:
                if (Unit* pTarget = selectUnitTarget(FilterArgs(TargetFilter_Player)))
                    castSpell(pTarget, getRaidModeValue(SPELL_CORLA_DARK_COMMAND_10, SPELL_CORLA_DARK_COMMAND_25, SPELL_CORLA_DARK_COMMAND_10, SPELL_CORLA_DARK_COMMAND_25));
                scriptEvents.addEvent(EVENT_CORLA_DARK_COMMAND, 24000);
                break;
            default:
                break;
        }
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Karsh Steelbender

class KarshSteelbenderAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new KarshSteelbenderAI(c); }
    explicit KarshSteelbenderAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        sendChatMessage(CHAT_MSG_MONSTER_YELL, 18852, "Bodies to test my armaments upon!");
        getInstanceScript()->sendUnitEncounter(EncounterFrameEngage, getCreature());
        getInstanceScript()->setBossState(DATA_KARSH_STEELBENDER, EncounterStates::InProgress);

        scriptEvents.addEvent(EVENT_KARSH_CLEAVE, 10000);
        scriptEvents.addEvent(EVENT_KARSH_HEAT_WAVE, 18000);
    }

    void OnCombatStop(Unit* /*pTarget*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
        getInstanceScript()->setBossState(DATA_KARSH_STEELBENDER, EncounterStates::Failed);
        scriptEvents.resetEvents();
    }

    void OnTargetDied(Unit* /*pTarget*/) override
    {
        sendChatMessage(CHAT_MSG_MONSTER_YELL, 18853, "Merely an impurity in the compound...");
    }

    void OnDied(Unit* /*pKiller*/) override
    {
        sendChatMessage(CHAT_MSG_MONSTER_YELL, 18855, "We number in the millions! Your efforts are wasted...");
        getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
        getInstanceScript()->setBossState(DATA_KARSH_STEELBENDER, EncounterStates::Performed);
    }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);

        if (_isCasting())
            return;

        switch (scriptEvents.getFinishedEvent())
        {
            case EVENT_KARSH_CLEAVE:
                castSpellAOE(SPELL_KARSH_CLEAVE);
                scriptEvents.addEvent(EVENT_KARSH_CLEAVE, 11000);
                break;
            case EVENT_KARSH_HEAT_WAVE:
                sendChatMessage(CHAT_MSG_MONSTER_YELL, 18854, "Feel the burn!");
                sendChatMessage(CHAT_MSG_MONSTER_EMOTE, 0, "Karsh Steelbender's armor shimmers with heat. Strike now!");
                if (Unit* pTarget = selectUnitTarget(FilterArgs(TargetFilter_Player)))
                    castSpell(pTarget, SPELL_KARSH_HEAT_WAVE);
                scriptEvents.addEvent(EVENT_KARSH_HEAT_WAVE, 20000);
                break;
            default:
                break;
        }
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Beauty

class BeautyAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new BeautyAI(c); }
    explicit BeautyAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameEngage, getCreature());
        getInstanceScript()->setBossState(DATA_BEAUTY, EncounterStates::InProgress);

        scriptEvents.addEvent(EVENT_BEAUTY_MAGMA_SPIT, 10000);
        scriptEvents.addEvent(EVENT_BEAUTY_BERSERKER_CHARGE, 15000);
        scriptEvents.addEvent(EVENT_BEAUTY_FLAMEBREAK, 21000);
        scriptEvents.addEvent(EVENT_BEAUTY_TERRIFYING_ROAR, 36000);
    }

    void OnCombatStop(Unit* /*pTarget*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
        getInstanceScript()->setBossState(DATA_BEAUTY, EncounterStates::Failed);
        scriptEvents.resetEvents();
    }

    void OnDied(Unit* /*pKiller*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
        getInstanceScript()->setBossState(DATA_BEAUTY, EncounterStates::Performed);
    }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);

        if (_isCasting())
            return;

        switch (scriptEvents.getFinishedEvent())
        {
            case EVENT_BEAUTY_MAGMA_SPIT:
                if (Unit* pTarget = selectUnitTarget(FilterArgs(TargetFilter_Player)))
                    castSpell(pTarget, SPELL_BEAUTY_MAGMA_SPIT);
                scriptEvents.addEvent(EVENT_BEAUTY_MAGMA_SPIT, 14000);
                break;
            case EVENT_BEAUTY_BERSERKER_CHARGE:
                if (Unit* pTarget = selectUnitTarget(FilterArgs(TargetFilter_Player)))
                    castSpell(pTarget, SPELL_BEAUTY_BERSERKER_CHARGE);
                scriptEvents.addEvent(EVENT_BEAUTY_BERSERKER_CHARGE, 20000);
                break;
            case EVENT_BEAUTY_FLAMEBREAK:
                castSpellAOE(SPELL_BEAUTY_FLAMEBREAK);
                scriptEvents.addEvent(EVENT_BEAUTY_FLAMEBREAK, 22000);
                break;
            case EVENT_BEAUTY_TERRIFYING_ROAR:
                castSpellAOE(SPELL_BEAUTY_TERRIFYING_ROAR);
                scriptEvents.addEvent(EVENT_BEAUTY_TERRIFYING_ROAR, 36000);
                break;
            default:
                break;
        }
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Ascendant Lord Obsidius

class AscendantLordObsidiusAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new AscendantLordObsidiusAI(c); }
    explicit AscendantLordObsidiusAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        sendChatMessage(CHAT_MSG_MONSTER_YELL, 18899, "You come seeking answers? Then have them! Look upon your answer to living!");
        getInstanceScript()->sendUnitEncounter(EncounterFrameEngage, getCreature());
        getInstanceScript()->setBossState(DATA_ASCENDANT_LORD_OBSIDIUS, EncounterStates::InProgress);

        scriptEvents.addEvent(EVENT_OBSIDIUS_STONE_BLOW, 8000);
        scriptEvents.addEvent(EVENT_OBSIDIUS_TWILIGHT_CORRUPTION, 12000);
        if (isHeroic())
            scriptEvents.addEvent(EVENT_OBSIDIUS_THUNDERCLAP, 15600);
    }

    void OnCombatStop(Unit* /*pTarget*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
        getInstanceScript()->setBossState(DATA_ASCENDANT_LORD_OBSIDIUS, EncounterStates::Failed);
        scriptEvents.resetEvents();
    }

    void OnTargetDied(Unit* /*pTarget*/) override
    {
        sendChatMessage(CHAT_MSG_MONSTER_YELL, 18900, "Your kind has no place in the master's world.");
    }

    void OnDied(Unit* /*pKiller*/) override
    {
        sendChatMessage(CHAT_MSG_MONSTER_YELL, 18902, "I cannot be destroyed... Only... de... layed...");
        getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
        getInstanceScript()->setBossState(DATA_ASCENDANT_LORD_OBSIDIUS, EncounterStates::Performed);
    }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);

        if (_isCasting())
            return;

        switch (scriptEvents.getFinishedEvent())
        {
            case EVENT_OBSIDIUS_STONE_BLOW:
                castSpellOnVictim(SPELL_OBSIDIUS_STONE_BLOW);
                scriptEvents.addEvent(EVENT_OBSIDIUS_STONE_BLOW, 16000);
                break;
            case EVENT_OBSIDIUS_TWILIGHT_CORRUPTION:
                sendChatMessage(CHAT_MSG_MONSTER_YELL, 18901, "Earth can be shaped, molded... You cannot! You are useless!");
                sendChatMessage(CHAT_MSG_MONSTER_EMOTE, 0, "Ascendant Lord Obsidius prepares to trade places with one of his shadows!");
                if (Unit* pTarget = selectUnitTarget(FilterArgs(TargetFilter_Player)))
                    castSpell(pTarget, SPELL_OBSIDIUS_TWILIGHT_CORRUPTION);
                scriptEvents.addEvent(EVENT_OBSIDIUS_TWILIGHT_CORRUPTION, 18000);
                break;
            case EVENT_OBSIDIUS_THUNDERCLAP:
                castSpellAOE(SPELL_OBSIDIUS_THUNDERCLAP);
                scriptEvents.addEvent(EVENT_OBSIDIUS_THUNDERCLAP, 20000);
                break;
            default:
                break;
        }
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Trash

class TwilightFlameCallerAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new TwilightFlameCallerAI(c); }
    explicit TwilightFlameCallerAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        scriptEvents.addEvent(EVENT_FLAME_CALLER_BLAST_WAVE, 9000);
        scriptEvents.addEvent(EVENT_FLAME_CALLER_CALL_FLAMES, 12000);
    }

    void OnCombatStop(Unit* /*pTarget*/) override { scriptEvents.resetEvents(); }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);
        if (_isCasting())
            return;

        switch (scriptEvents.getFinishedEvent())
        {
            case EVENT_FLAME_CALLER_BLAST_WAVE:
                castSpellOnSelf(SPELL_FLAME_CALLER_BLAST_WAVE);
                scriptEvents.addEvent(EVENT_FLAME_CALLER_BLAST_WAVE, 18000);
                break;
            case EVENT_FLAME_CALLER_CALL_FLAMES:
                castSpellOnSelf(SPELL_FLAME_CALLER_CALL_FLAMES);
                scriptEvents.addEvent(EVENT_FLAME_CALLER_CALL_FLAMES, 13000);
                break;
            default:
                break;
        }
    }
};

class TwilightTorturerAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new TwilightTorturerAI(c); }
    explicit TwilightTorturerAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        scriptEvents.addEvent(EVENT_TORTURER_RED_HOT_POKER, 9000);
        scriptEvents.addEvent(EVENT_TORTURER_WILD_BEATDOWN, 17000);
    }

    void OnCombatStop(Unit* /*pTarget*/) override { scriptEvents.resetEvents(); }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);
        if (_isCasting())
            return;

        switch (scriptEvents.getFinishedEvent())
        {
            case EVENT_TORTURER_RED_HOT_POKER:
                castSpellOnSelf(SPELL_TORTURER_RED_HOT_POKER);
                scriptEvents.addEvent(EVENT_TORTURER_RED_HOT_POKER, 18000);
                break;
            case EVENT_TORTURER_WILD_BEATDOWN:
                castSpellOnSelf(SPELL_TORTURER_WILD_BEATDOWN);
                scriptEvents.addEvent(EVENT_TORTURER_WILD_BEATDOWN, 13000);
                break;
            default:
                break;
        }
    }
};

class TwilightSadistAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new TwilightSadistAI(c); }
    explicit TwilightSadistAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        scriptEvents.addEvent(EVENT_SADIST_HEAT_SEEKER_BLADE, 13000);
        scriptEvents.addEvent(EVENT_SADIST_SINISTER_STRIKE, 17000);
    }

    void OnCombatStop(Unit* /*pTarget*/) override { scriptEvents.resetEvents(); }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);
        if (_isCasting())
            return;

        switch (scriptEvents.getFinishedEvent())
        {
            case EVENT_SADIST_HEAT_SEEKER_BLADE:
                if (Unit* pTarget = selectUnitTarget(FilterArgs(TargetFilter_Player)))
                    castSpell(pTarget, SPELL_SADIST_HEAT_SEEKER_BLADE);
                scriptEvents.addEvent(EVENT_SADIST_HEAT_SEEKER_BLADE, 16000);
                break;
            case EVENT_SADIST_SINISTER_STRIKE:
                castSpellOnVictim(SPELL_SADIST_SINISTER_STRIKE);
                scriptEvents.addEvent(EVENT_SADIST_SINISTER_STRIKE, 14000);
                break;
            default:
                break;
        }
    }
};

// Mad Prisoner and Crazed Mage share the exact same kit in the reference implementation.
// Reference dialogue data for entry 39985 also has a plain "%s becomes enraged!" emote (no
// sound); the reference doesn't record what triggers it (no health-percent or timer noted),
// so it isn't wired up here rather than guessing a threshold.
class MadPrisonerAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new MadPrisonerAI(c); }
    explicit MadPrisonerAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        scriptEvents.addEvent(EVENT_PRISONER_HEAD_CRACK, 9000);
        scriptEvents.addEvent(EVENT_PRISONER_INFECTED_WOUND, 13000);
    }

    void OnCombatStop(Unit* /*pTarget*/) override { scriptEvents.resetEvents(); }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);
        if (_isCasting())
            return;

        switch (scriptEvents.getFinishedEvent())
        {
            case EVENT_PRISONER_HEAD_CRACK:
                castSpellOnSelf(SPELL_PRISONER_HEAD_CRACK);
                scriptEvents.addEvent(EVENT_PRISONER_HEAD_CRACK, 18000);
                break;
            case EVENT_PRISONER_INFECTED_WOUND:
                castSpellOnSelf(SPELL_PRISONER_INFECTED_WOUND);
                scriptEvents.addEvent(EVENT_PRISONER_INFECTED_WOUND, 13000);
                break;
            default:
                break;
        }
    }
};

// Conflagration and Twilight Element Warden are passive environmental hazards (out-of-combat
// only in the reference) that apply their effect once on load rather than fighting back.
class ConflagrationAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new ConflagrationAI(c); }
    explicit ConflagrationAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnLoad() override
    {
        castSpellOnSelf(SPELL_CONFLAGRATION_BURNING_HEAT, true);
    }
};

class TwilightElementWardenAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new TwilightElementWardenAI(c); }
    explicit TwilightElementWardenAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnLoad() override
    {
        castSpellOnSelf(SPELL_ELEMENT_WARDEN_BORE, true);
    }
};

void SetupBlackrockCaverns(ScriptMgr* mgr)
{
    mgr->register_instance_script(MAP_BLACKROCK_CAVERNS, &BlackrockCavernsInstanceScript::Create);

    mgr->register_creature_script(BOSS_ROMOGG_BONECRUSHER, &RomoggBonecrusherAI::Create);
    mgr->register_creature_script(BOSS_CORLA_HERALD_OF_TWILIGHT, &CorlaAI::Create);
    mgr->register_creature_script(BOSS_KARSH_STEELBENDER, &KarshSteelbenderAI::Create);
    mgr->register_creature_script(BOSS_BEAUTY, &BeautyAI::Create);
    mgr->register_creature_script(BOSS_ASCENDANT_LORD_OBSIDIUS, &AscendantLordObsidiusAI::Create);

    mgr->register_creature_script(NPC_TWILIGHT_FLAME_CALLER, &TwilightFlameCallerAI::Create);
    mgr->register_creature_script(NPC_TWILIGHT_TORTURER, &TwilightTorturerAI::Create);
    mgr->register_creature_script(NPC_TWILIGHT_SADIST, &TwilightSadistAI::Create);
    mgr->register_creature_script(NPC_MAD_PRISONER, &MadPrisonerAI::Create);
    mgr->register_creature_script(NPC_CRAZED_MAGE, &MadPrisonerAI::Create);

    mgr->register_creature_script(NPC_CONFLAGRATION, &ConflagrationAI::Create);
    mgr->register_creature_script(NPC_TWILIGHT_ELEMENT_WARDEN, &TwilightElementWardenAI::Create);
}
