/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

// Scarlet Monastery (Mists revamp) - three encounters (verified against wowhead's MoP
// Classic data), hand-ported into AscEmu's own CreatureAIScript / InstanceScript framework.

#include "Setup.h"
#include "Instance_Scarlet_Monastery_Mop.hpp"

#include "Server/Script/CreatureAIScript.hpp"
#include "Server/Script/InstanceScript.hpp"
#include "Server/Script/ScriptMgr.hpp"

enum ScarletMonasteryMopEvents
{
    EVENT_THALNOS_SPIRIT_GALE = 1,
    EVENT_THALNOS_EVICT_SOUL,
    EVENT_THALNOS_RAISE_FALLEN_CRUSADER,

    EVENT_KORLOFF_BLAZING_FISTS,
    EVENT_KORLOFF_FIRESTORM_KICK,
    EVENT_KORLOFF_RISING_FLAME,
    EVENT_KORLOFF_SCORCHED_EARTH,

    EVENT_DURAND_FLASH_OF_STEEL,
    EVENT_DURAND_DASHING_STRIKE,

    EVENT_WHITEMANE_HOLY_SMITE,
    EVENT_WHITEMANE_POWER_WORD_SHIELD,
    EVENT_WHITEMANE_MASS_RESURRECTION,

    EVENT_FLAMETHROWER_FIREBALL,
    EVENT_FLAMETHROWER_FLAMESTRIKE,

    EVENT_FRENZIED_SPIRIT_SHADOW_BOLT,
    EVENT_FRENZIED_SPIRIT_ENRAGE,

    EVENT_JUDICATOR_HOLY_SMITE,
    EVENT_JUDICATOR_HAMMER_OF_JUSTICE
};

// Mists redesigned Scarlet Monastery into its own instance with a new map id
// (MAP_SCARLET_MONASTERY_MOP, 1004) - unrelated to the Classic instance registered on
// MAP_SCARLET_MONASTERY (189) by SetupScarletMonastery(), which stays untouched.
class ScarletMonasteryMopInstanceScript : public InstanceScript
{
public:
    explicit ScarletMonasteryMopInstanceScript(WorldMap* pMapMgr) : InstanceScript(pMapMgr)
    {
        setBossNumber(ScarletMonasteryMopEncounterCount);
    }

    static InstanceScript* Create(WorldMap* pMapMgr) { return new ScarletMonasteryMopInstanceScript(pMapMgr); }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Thalnos the Soulrender

class ThalnosAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new ThalnosAI(c); }
    explicit ThalnosAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameEngage, getCreature());
        getInstanceScript()->setBossState(DATA_THALNOS, EncounterStates::InProgress);

        scriptEvents.addEvent(EVENT_THALNOS_SPIRIT_GALE, 8000);
        scriptEvents.addEvent(EVENT_THALNOS_EVICT_SOUL, 14000);
        scriptEvents.addEvent(EVENT_THALNOS_RAISE_FALLEN_CRUSADER, 20000);
    }

    void OnCombatStop(Unit* /*pTarget*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
        getInstanceScript()->setBossState(DATA_THALNOS, EncounterStates::Failed);
        scriptEvents.resetEvents();
    }

    void OnDied(Unit* /*pKiller*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
        getInstanceScript()->setBossState(DATA_THALNOS, EncounterStates::Performed);
    }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);

        if (_isCasting())
            return;

        switch (scriptEvents.getFinishedEvent())
        {
            case EVENT_THALNOS_SPIRIT_GALE:
                castSpellOnVictim(SPELL_THALNOS_SPIRIT_GALE);
                scriptEvents.addEvent(EVENT_THALNOS_SPIRIT_GALE, 16000);
                break;
            case EVENT_THALNOS_EVICT_SOUL:
                if (Unit* pTarget = selectUnitTarget(FilterArgs(TargetFilter_Player)))
                    castSpell(pTarget, SPELL_THALNOS_EVICT_SOUL);
                scriptEvents.addEvent(EVENT_THALNOS_EVICT_SOUL, 20000);
                break;
            case EVENT_THALNOS_RAISE_FALLEN_CRUSADER:
                sendChatMessage(CHAT_MSG_MONSTER_EMOTE, 0, "Thalnos the Soulrender raises the fallen!");
                castSpellOnSelf(SPELL_THALNOS_RAISE_FALLEN_CRUSADER);
                scriptEvents.addEvent(EVENT_THALNOS_RAISE_FALLEN_CRUSADER, 30000);
                break;
            default:
                break;
        }
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Brother Korloff

class KorloffAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new KorloffAI(c); }
    explicit KorloffAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameEngage, getCreature());
        getInstanceScript()->setBossState(DATA_KORLOFF, EncounterStates::InProgress);

        scriptEvents.addEvent(EVENT_KORLOFF_BLAZING_FISTS, 6000);
        scriptEvents.addEvent(EVENT_KORLOFF_FIRESTORM_KICK, 12000);
        scriptEvents.addEvent(EVENT_KORLOFF_RISING_FLAME, 18000);
        scriptEvents.addEvent(EVENT_KORLOFF_SCORCHED_EARTH, 9000);
    }

    void OnCombatStop(Unit* /*pTarget*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
        getInstanceScript()->setBossState(DATA_KORLOFF, EncounterStates::Failed);
        scriptEvents.resetEvents();
    }

    void OnDied(Unit* /*pKiller*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
        getInstanceScript()->setBossState(DATA_KORLOFF, EncounterStates::Performed);
    }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);

        if (_isCasting())
            return;

        switch (scriptEvents.getFinishedEvent())
        {
            case EVENT_KORLOFF_BLAZING_FISTS:
                castSpellOnVictim(SPELL_KORLOFF_BLAZING_FISTS);
                scriptEvents.addEvent(EVENT_KORLOFF_BLAZING_FISTS, 14000);
                break;
            case EVENT_KORLOFF_FIRESTORM_KICK:
                castSpellOnVictim(SPELL_KORLOFF_FIRESTORM_KICK);
                scriptEvents.addEvent(EVENT_KORLOFF_FIRESTORM_KICK, 20000);
                break;
            case EVENT_KORLOFF_RISING_FLAME:
                if (Unit* pTarget = selectUnitTarget(FilterArgs(TargetFilter_Player)))
                    castSpell(pTarget, SPELL_KORLOFF_RISING_FLAME);
                scriptEvents.addEvent(EVENT_KORLOFF_RISING_FLAME, 22000);
                break;
            case EVENT_KORLOFF_SCORCHED_EARTH:
                castSpellAOE(SPELL_KORLOFF_SCORCHED_EARTH);
                scriptEvents.addEvent(EVENT_KORLOFF_SCORCHED_EARTH, 24000);
                break;
            default:
                break;
        }
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Commander Durand & High Inquisitor Whitemane

class DurandWhitemaneMemberAI : public CreatureAIScript
{
public:
    explicit DurandWhitemaneMemberAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameEngage, getCreature());
        getInstanceScript()->setBossState(DATA_DURAND_AND_WHITEMANE, EncounterStates::InProgress);
    }

    void OnCombatStop(Unit* /*pTarget*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
        getInstanceScript()->setBossState(DATA_DURAND_AND_WHITEMANE, EncounterStates::Failed);
        scriptEvents.resetEvents();
    }

    void OnDied(Unit* /*pKiller*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
        getInstanceScript()->setBossState(DATA_DURAND_AND_WHITEMANE, EncounterStates::Performed);
    }
};

class DurandAI : public DurandWhitemaneMemberAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new DurandAI(c); }
    explicit DurandAI(Creature* pCreature) : DurandWhitemaneMemberAI(pCreature) {}

    void OnCombatStart(Unit* pTarget) override
    {
        sendChatMessage(CHAT_MSG_MONSTER_YELL, 0, "For the Crusade!");
        DurandWhitemaneMemberAI::OnCombatStart(pTarget);
        scriptEvents.addEvent(EVENT_DURAND_FLASH_OF_STEEL, 7000);
        scriptEvents.addEvent(EVENT_DURAND_DASHING_STRIKE, 13000);
    }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);

        if (_isCasting())
            return;

        switch (scriptEvents.getFinishedEvent())
        {
            case EVENT_DURAND_FLASH_OF_STEEL:
                castSpellAOE(SPELL_DURAND_FLASH_OF_STEEL);
                scriptEvents.addEvent(EVENT_DURAND_FLASH_OF_STEEL, 18000);
                break;
            case EVENT_DURAND_DASHING_STRIKE:
                if (Unit* pTarget = selectUnitTarget(FilterArgs(TargetFilter_Player)))
                    castSpell(pTarget, SPELL_DURAND_DASHING_STRIKE);
                scriptEvents.addEvent(EVENT_DURAND_DASHING_STRIKE, 16000);
                break;
            default:
                break;
        }
    }
};

// Reference dialogue data for entry 3977 has SAY_WH_INTRO ("Mograine has fallen! You shall pay
// for this treachery! Arise, my champion! Arise!", sound 5838), SAY_WH_KILL ("The Light has
// spoken!", sound 5839), and SAY_WH_RESSURECT ("Arise, my champion!", sound 5840) - no death
// line exists for her in the reference, so OnDied keeps its own invented line.
class WhitemaneAI : public DurandWhitemaneMemberAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new WhitemaneAI(c); }
    explicit WhitemaneAI(Creature* pCreature) : DurandWhitemaneMemberAI(pCreature) {}

    void OnCombatStart(Unit* pTarget) override
    {
        sendChatMessage(CHAT_MSG_MONSTER_YELL, 5838, "Mograine has fallen! You shall pay for this treachery! Arise, my champion! Arise!");
        DurandWhitemaneMemberAI::OnCombatStart(pTarget);
        scriptEvents.addEvent(EVENT_WHITEMANE_HOLY_SMITE, 4000);
        scriptEvents.addEvent(EVENT_WHITEMANE_POWER_WORD_SHIELD, 10000);
        scriptEvents.addEvent(EVENT_WHITEMANE_MASS_RESURRECTION, 26000);
    }

    void OnTargetDied(Unit* /*pTarget*/) override
    {
        sendChatMessage(CHAT_MSG_MONSTER_YELL, 5839, "The Light has spoken!");
    }

    void OnDied(Unit* pKiller) override
    {
        sendChatMessage(CHAT_MSG_MONSTER_YELL, 0, "I... failed... the Crusade...");
        DurandWhitemaneMemberAI::OnDied(pKiller);
    }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);

        if (_isCasting())
            return;

        switch (scriptEvents.getFinishedEvent())
        {
            case EVENT_WHITEMANE_HOLY_SMITE:
                castSpellOnVictim(SPELL_WHITEMANE_HOLY_SMITE);
                scriptEvents.addEvent(EVENT_WHITEMANE_HOLY_SMITE, 8000);
                break;
            case EVENT_WHITEMANE_POWER_WORD_SHIELD:
                castSpellOnSelf(SPELL_WHITEMANE_POWER_WORD_SHIELD);
                scriptEvents.addEvent(EVENT_WHITEMANE_POWER_WORD_SHIELD, 24000);
                break;
            case EVENT_WHITEMANE_MASS_RESURRECTION:
                sendChatMessage(CHAT_MSG_MONSTER_YELL, 5840, "Arise, my champion!");
                sendChatMessage(CHAT_MSG_MONSTER_EMOTE, 0, "High Inquisitor Whitemane calls upon the Light to resurrect her allies!");
                castSpellOnSelf(SPELL_WHITEMANE_MASS_RESURRECTION);
                scriptEvents.addEvent(EVENT_WHITEMANE_MASS_RESURRECTION, 40000);
                break;
            default:
                break;
        }
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Trash - Scarlet Flamethrower

class ScarletFlamethrowerAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new ScarletFlamethrowerAI(c); }
    explicit ScarletFlamethrowerAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        scriptEvents.addEvent(EVENT_FLAMETHROWER_FIREBALL, 4000);
        scriptEvents.addEvent(EVENT_FLAMETHROWER_FLAMESTRIKE, 12000);
    }

    void OnCombatStop(Unit* /*pTarget*/) override { scriptEvents.resetEvents(); }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);
        if (_isCasting())
            return;

        switch (scriptEvents.getFinishedEvent())
        {
            case EVENT_FLAMETHROWER_FIREBALL:
                castSpellOnVictim(SPELL_FLAMETHROWER_FIREBALL);
                scriptEvents.addEvent(EVENT_FLAMETHROWER_FIREBALL, 10000);
                break;
            case EVENT_FLAMETHROWER_FLAMESTRIKE:
                castSpellAOE(SPELL_FLAMETHROWER_FLAMESTRIKE);
                scriptEvents.addEvent(EVENT_FLAMETHROWER_FLAMESTRIKE, 20000);
                break;
            default:
                break;
        }
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Trash - Frenzied Spirit

class FrenziedSpiritAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new FrenziedSpiritAI(c); }
    explicit FrenziedSpiritAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        scriptEvents.addEvent(EVENT_FRENZIED_SPIRIT_SHADOW_BOLT, 5000);
        scriptEvents.addEvent(EVENT_FRENZIED_SPIRIT_ENRAGE, 16000);
    }

    void OnCombatStop(Unit* /*pTarget*/) override { scriptEvents.resetEvents(); }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);
        if (_isCasting())
            return;

        switch (scriptEvents.getFinishedEvent())
        {
            case EVENT_FRENZIED_SPIRIT_SHADOW_BOLT:
                if (Unit* pTarget = selectUnitTarget(FilterArgs(TargetFilter_Player)))
                    castSpell(pTarget, SPELL_FRENZIED_SPIRIT_SHADOW_BOLT);
                scriptEvents.addEvent(EVENT_FRENZIED_SPIRIT_SHADOW_BOLT, 14000);
                break;
            case EVENT_FRENZIED_SPIRIT_ENRAGE:
                castSpellOnSelf(SPELL_FRENZIED_SPIRIT_ENRAGE);
                scriptEvents.addEvent(EVENT_FRENZIED_SPIRIT_ENRAGE, 26000);
                break;
            default:
                break;
        }
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Trash - Scarlet Judicator

class ScarletJudicatorAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new ScarletJudicatorAI(c); }
    explicit ScarletJudicatorAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        scriptEvents.addEvent(EVENT_JUDICATOR_HOLY_SMITE, 4000);
        scriptEvents.addEvent(EVENT_JUDICATOR_HAMMER_OF_JUSTICE, 10000);
    }

    void OnCombatStop(Unit* /*pTarget*/) override { scriptEvents.resetEvents(); }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);
        if (_isCasting())
            return;

        switch (scriptEvents.getFinishedEvent())
        {
            case EVENT_JUDICATOR_HOLY_SMITE:
                castSpellOnVictim(SPELL_JUDICATOR_HOLY_SMITE);
                scriptEvents.addEvent(EVENT_JUDICATOR_HOLY_SMITE, 10000);
                break;
            case EVENT_JUDICATOR_HAMMER_OF_JUSTICE:
                if (Unit* pTarget = selectUnitTarget(FilterArgs(TargetFilter_Player)))
                    castSpell(pTarget, SPELL_JUDICATOR_HAMMER_OF_JUSTICE);
                scriptEvents.addEvent(EVENT_JUDICATOR_HAMMER_OF_JUSTICE, 22000);
                break;
            default:
                break;
        }
    }
};

void SetupScarletMonasteryMop(ScriptMgr* mgr)
{
    mgr->register_instance_script(MAP_SCARLET_MONASTERY_MOP, &ScarletMonasteryMopInstanceScript::Create);

    mgr->register_creature_script(BOSS_THALNOS, &ThalnosAI::Create);
    mgr->register_creature_script(BOSS_KORLOFF, &KorloffAI::Create);
    mgr->register_creature_script(BOSS_DURAND, &DurandAI::Create);

    // BOSS_WHITEMANE (3977) reuses the Classic Scarlet Monastery's own Whitemane entry, which
    // already has a WhitemaneAI registered in Classic/ScarletMonestary - our creature
    // script registry is global by entry id (not per-map), so registering WhitemaneAI::Create
    // here too would silently lose to whichever Setup*() runs first and leave one of the two
    // dungeons' Whitemane running the wrong kit. Flagged for Zyres - see framework-change list.
    // mgr->register_creature_script(BOSS_WHITEMANE, &WhitemaneAI::Create);

    mgr->register_creature_script(NPC_SCARLET_FLAMETHROWER, &ScarletFlamethrowerAI::Create);
    mgr->register_creature_script(NPC_FRENZIED_SPIRIT, &FrenziedSpiritAI::Create);
    mgr->register_creature_script(NPC_SCARLET_JUDICATOR, &ScarletJudicatorAI::Create);
}
