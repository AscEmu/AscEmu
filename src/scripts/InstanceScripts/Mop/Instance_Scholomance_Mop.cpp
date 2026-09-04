/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

// Scholomance (Mists revamp) - five bosses, all sourced from wowhead's MoP Classic data (see
// header comment for the entry-id history), hand-ported into AscEmu's own CreatureAIScript /
// InstanceScript framework.

#include "Setup.h"
#include "Instance_Scholomance_Mop.hpp"

#include "Server/Script/CreatureAIScript.hpp"
#include "Server/Script/InstanceScript.hpp"
#include "Server/Script/ScriptMgr.hpp"

enum ScholomanceMopEvents
{
    EVENT_CHILLHEART_WRACK_SOUL = 1,
    EVENT_CHILLHEART_ICE_WRATH,
    EVENT_CHILLHEART_TOUCH_OF_THE_GRAVE,
    EVENT_CHILLHEART_FRIGID_GRASP,

    EVENT_JANDICE_WONDROUS_RAPIDITY,
    EVENT_JANDICE_GRAVITY_FLUX,
    EVENT_JANDICE_WHIRL_OF_ILLUSION,

    EVENT_RATTLEGORE_BONE_ARMOR,
    EVENT_RATTLEGORE_BONE_SPIKE,
    EVENT_RATTLEGORE_RUSTING,
    EVENT_RATTLEGORE_SOULFLAME,

    EVENT_LILIAN_DARK_SIMULACRUM,
    EVENT_LILIAN_ARCANE_BOMB,
    EVENT_LILIAN_POLYFORMIC_ACID_POTION,

    EVENT_GANDLING_DEATHS_GRASP,
    EVENT_GANDLING_DARK_BLAZE,
    EVENT_GANDLING_BLAZING_SOUL,
    EVENT_GANDLING_UNLEASHED_ANGUISH,

    EVENT_CARVER_CLEAVE,
    EVENT_CARVER_MORTAL_STRIKE,

    EVENT_ACOLYTE_SHADOW_BOLT,
    EVENT_ACOLYTE_CURSE,

    EVENT_CANDLESTICK_FIREBALL,
    EVENT_CANDLESTICK_FROSTBOLT
};

// Mists redesigned Scholomance into its own instance with a new map id
// (MAP_SCHOLOMANCE_MOP, 1007) - unrelated to the Classic instance registered on
// MAP_SCHOLOMANCE (289) by SetupScholomance(), which stays untouched.
class ScholomanceMopInstanceScript : public InstanceScript
{
public:
    explicit ScholomanceMopInstanceScript(WorldMap* pMapMgr) : InstanceScript(pMapMgr)
    {
        setBossNumber(ScholomanceMopEncounterCount);
    }

    static InstanceScript* Create(WorldMap* pMapMgr) { return new ScholomanceMopInstanceScript(pMapMgr); }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Instructor Chillheart

class ChillheartAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new ChillheartAI(c); }
    explicit ChillheartAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameEngage, getCreature());
        getInstanceScript()->setBossState(DATA_CHILLHEART, EncounterStates::InProgress);

        scriptEvents.addEvent(EVENT_CHILLHEART_WRACK_SOUL, 6000);
        scriptEvents.addEvent(EVENT_CHILLHEART_ICE_WRATH, 12000);
        scriptEvents.addEvent(EVENT_CHILLHEART_TOUCH_OF_THE_GRAVE, 9000);
        scriptEvents.addEvent(EVENT_CHILLHEART_FRIGID_GRASP, 18000);
    }

    void OnCombatStop(Unit* /*pTarget*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
        getInstanceScript()->setBossState(DATA_CHILLHEART, EncounterStates::Failed);
        scriptEvents.resetEvents();
    }

    void OnDied(Unit* /*pKiller*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
        getInstanceScript()->setBossState(DATA_CHILLHEART, EncounterStates::Performed);
    }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);

        if (_isCasting())
            return;

        switch (scriptEvents.getFinishedEvent())
        {
            case EVENT_CHILLHEART_WRACK_SOUL:
                if (Unit* pTarget = selectUnitTarget(FilterArgs(TargetFilter_Player)))
                    castSpell(pTarget, SPELL_CHILLHEART_WRACK_SOUL);
                scriptEvents.addEvent(EVENT_CHILLHEART_WRACK_SOUL, 16000);
                break;
            case EVENT_CHILLHEART_ICE_WRATH:
                if (Unit* pTarget = selectUnitTarget(FilterArgs(TargetFilter_Player)))
                    castSpell(pTarget, SPELL_CHILLHEART_ICE_WRATH);
                scriptEvents.addEvent(EVENT_CHILLHEART_ICE_WRATH, 20000);
                break;
            case EVENT_CHILLHEART_TOUCH_OF_THE_GRAVE:
                castSpellOnSelf(SPELL_CHILLHEART_TOUCH_OF_THE_GRAVE);
                scriptEvents.addEvent(EVENT_CHILLHEART_TOUCH_OF_THE_GRAVE, 22000);
                break;
            case EVENT_CHILLHEART_FRIGID_GRASP:
                if (Unit* pTarget = selectUnitTarget(FilterArgs(TargetFilter_Player)))
                    castSpell(pTarget, SPELL_CHILLHEART_FRIGID_GRASP);
                scriptEvents.addEvent(EVENT_CHILLHEART_FRIGID_GRASP, 24000);
                break;
            default:
                break;
        }
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Jandice Barov

class JandiceBarovAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new JandiceBarovAI(c); }
    explicit JandiceBarovAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameEngage, getCreature());
        getInstanceScript()->setBossState(DATA_JANDICE_BAROV, EncounterStates::InProgress);

        scriptEvents.addEvent(EVENT_JANDICE_WONDROUS_RAPIDITY, 10000);
        scriptEvents.addEvent(EVENT_JANDICE_GRAVITY_FLUX, 16000);
        scriptEvents.addEvent(EVENT_JANDICE_WHIRL_OF_ILLUSION, 22000);
    }

    void OnCombatStop(Unit* /*pTarget*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
        getInstanceScript()->setBossState(DATA_JANDICE_BAROV, EncounterStates::Failed);
        scriptEvents.resetEvents();
    }

    void OnDied(Unit* /*pKiller*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
        getInstanceScript()->setBossState(DATA_JANDICE_BAROV, EncounterStates::Performed);
    }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);

        if (_isCasting())
            return;

        switch (scriptEvents.getFinishedEvent())
        {
            case EVENT_JANDICE_WONDROUS_RAPIDITY:
                castSpellOnSelf(SPELL_JANDICE_WONDROUS_RAPIDITY);
                scriptEvents.addEvent(EVENT_JANDICE_WONDROUS_RAPIDITY, 26000);
                break;
            case EVENT_JANDICE_GRAVITY_FLUX:
                if (Unit* pTarget = selectUnitTarget(FilterArgs(TargetFilter_Player)))
                    castSpell(pTarget, SPELL_JANDICE_GRAVITY_FLUX);
                scriptEvents.addEvent(EVENT_JANDICE_GRAVITY_FLUX, 20000);
                break;
            case EVENT_JANDICE_WHIRL_OF_ILLUSION:
                sendChatMessage(CHAT_MSG_MONSTER_EMOTE, 0, "Jandice Barov spins into a whirl of illusion!");
                castSpellAOE(SPELL_JANDICE_WHIRL_OF_ILLUSION);
                scriptEvents.addEvent(EVENT_JANDICE_WHIRL_OF_ILLUSION, 30000);
                break;
            default:
                break;
        }
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Rattlegore

class RattlegoreAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new RattlegoreAI(c); }
    explicit RattlegoreAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameEngage, getCreature());
        getInstanceScript()->setBossState(DATA_RATTLEGORE, EncounterStates::InProgress);

        scriptEvents.addEvent(EVENT_RATTLEGORE_BONE_ARMOR, 8000);
        scriptEvents.addEvent(EVENT_RATTLEGORE_BONE_SPIKE, 14000);
        scriptEvents.addEvent(EVENT_RATTLEGORE_RUSTING, 20000);
        scriptEvents.addEvent(EVENT_RATTLEGORE_SOULFLAME, 26000);
    }

    void OnCombatStop(Unit* /*pTarget*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
        getInstanceScript()->setBossState(DATA_RATTLEGORE, EncounterStates::Failed);
        scriptEvents.resetEvents();
    }

    void OnDied(Unit* /*pKiller*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
        getInstanceScript()->setBossState(DATA_RATTLEGORE, EncounterStates::Performed);
    }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);

        if (_isCasting())
            return;

        switch (scriptEvents.getFinishedEvent())
        {
            case EVENT_RATTLEGORE_BONE_ARMOR:
                castSpellOnSelf(SPELL_RATTLEGORE_BONE_ARMOR);
                scriptEvents.addEvent(EVENT_RATTLEGORE_BONE_ARMOR, 26000);
                break;
            case EVENT_RATTLEGORE_BONE_SPIKE:
                castSpellOnVictim(SPELL_RATTLEGORE_BONE_SPIKE);
                scriptEvents.addEvent(EVENT_RATTLEGORE_BONE_SPIKE, 18000);
                break;
            case EVENT_RATTLEGORE_RUSTING:
                if (Unit* pTarget = selectUnitTarget(FilterArgs(TargetFilter_Player)))
                    castSpell(pTarget, SPELL_RATTLEGORE_RUSTING);
                scriptEvents.addEvent(EVENT_RATTLEGORE_RUSTING, 20000);
                break;
            case EVENT_RATTLEGORE_SOULFLAME:
                castSpellAOE(SPELL_RATTLEGORE_SOULFLAME);
                scriptEvents.addEvent(EVENT_RATTLEGORE_SOULFLAME, 24000);
                break;
            default:
                break;
        }
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Lilian Voss

class LilianVossAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new LilianVossAI(c); }
    explicit LilianVossAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameEngage, getCreature());
        getInstanceScript()->setBossState(DATA_LILIAN_VOSS, EncounterStates::InProgress);

        scriptEvents.addEvent(EVENT_LILIAN_DARK_SIMULACRUM, 5000);
        scriptEvents.addEvent(EVENT_LILIAN_ARCANE_BOMB, 12000);
        scriptEvents.addEvent(EVENT_LILIAN_POLYFORMIC_ACID_POTION, 20000);
    }

    void OnCombatStop(Unit* /*pTarget*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
        getInstanceScript()->setBossState(DATA_LILIAN_VOSS, EncounterStates::Failed);
        scriptEvents.resetEvents();
    }

    void OnDied(Unit* /*pKiller*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
        getInstanceScript()->setBossState(DATA_LILIAN_VOSS, EncounterStates::Performed);
    }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);

        if (_isCasting())
            return;

        switch (scriptEvents.getFinishedEvent())
        {
            case EVENT_LILIAN_DARK_SIMULACRUM:
                if (Unit* pTarget = selectUnitTarget(FilterArgs(TargetFilter_Player)))
                    castSpell(pTarget, SPELL_LILIAN_DARK_SIMULACRUM);
                scriptEvents.addEvent(EVENT_LILIAN_DARK_SIMULACRUM, 22000);
                break;
            case EVENT_LILIAN_ARCANE_BOMB:
                castSpellAOE(SPELL_LILIAN_ARCANE_BOMB);
                scriptEvents.addEvent(EVENT_LILIAN_ARCANE_BOMB, 18000);
                break;
            case EVENT_LILIAN_POLYFORMIC_ACID_POTION:
                castSpellOnVictim(SPELL_LILIAN_POLYFORMIC_ACID_POTION);
                scriptEvents.addEvent(EVENT_LILIAN_POLYFORMIC_ACID_POTION, 24000);
                break;
            default:
                break;
        }
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Darkmaster Gandling

class GandlingAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new GandlingAI(c); }
    explicit GandlingAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        sendChatMessage(CHAT_MSG_MONSTER_YELL, 0, "Class is now in session. The lesson? Suffering.");
        getInstanceScript()->sendUnitEncounter(EncounterFrameEngage, getCreature());
        getInstanceScript()->setBossState(DATA_GANDLING, EncounterStates::InProgress);

        scriptEvents.addEvent(EVENT_GANDLING_DEATHS_GRASP, 6000);
        scriptEvents.addEvent(EVENT_GANDLING_DARK_BLAZE, 12000);
        scriptEvents.addEvent(EVENT_GANDLING_BLAZING_SOUL, 18000);
        scriptEvents.addEvent(EVENT_GANDLING_UNLEASHED_ANGUISH, 24000);
    }

    void OnCombatStop(Unit* /*pTarget*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
        getInstanceScript()->setBossState(DATA_GANDLING, EncounterStates::Failed);
        scriptEvents.resetEvents();
    }

    void OnDied(Unit* /*pKiller*/) override
    {
        sendChatMessage(CHAT_MSG_MONSTER_YELL, 0, "The curse... lifts... at last...");
        getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
        getInstanceScript()->setBossState(DATA_GANDLING, EncounterStates::Performed);
    }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);

        if (_isCasting())
            return;

        switch (scriptEvents.getFinishedEvent())
        {
            case EVENT_GANDLING_DEATHS_GRASP:
                if (Unit* pTarget = selectUnitTarget(FilterArgs(TargetFilter_Player)))
                    castSpell(pTarget, SPELL_GANDLING_DEATHS_GRASP);
                scriptEvents.addEvent(EVENT_GANDLING_DEATHS_GRASP, 18000);
                break;
            case EVENT_GANDLING_DARK_BLAZE:
                castSpellOnVictim(SPELL_GANDLING_DARK_BLAZE);
                scriptEvents.addEvent(EVENT_GANDLING_DARK_BLAZE, 20000);
                break;
            case EVENT_GANDLING_BLAZING_SOUL:
                castSpellAOE(SPELL_GANDLING_BLAZING_SOUL);
                scriptEvents.addEvent(EVENT_GANDLING_BLAZING_SOUL, 26000);
                break;
            case EVENT_GANDLING_UNLEASHED_ANGUISH:
                castSpellOnSelf(SPELL_GANDLING_UNLEASHED_ANGUISH);
                scriptEvents.addEvent(EVENT_GANDLING_UNLEASHED_ANGUISH, 30000);
                break;
            default:
                break;
        }
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Trash - Krastinovian Carver

class KrastinovianCarverAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new KrastinovianCarverAI(c); }
    explicit KrastinovianCarverAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        scriptEvents.addEvent(EVENT_CARVER_CLEAVE, 5000);
        scriptEvents.addEvent(EVENT_CARVER_MORTAL_STRIKE, 10000);
    }

    void OnCombatStop(Unit* /*pTarget*/) override { scriptEvents.resetEvents(); }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);
        if (_isCasting())
            return;

        switch (scriptEvents.getFinishedEvent())
        {
            case EVENT_CARVER_CLEAVE:
                castSpellOnVictim(SPELL_CARVER_CLEAVE);
                scriptEvents.addEvent(EVENT_CARVER_CLEAVE, 12000);
                break;
            case EVENT_CARVER_MORTAL_STRIKE:
                castSpellOnVictim(SPELL_CARVER_MORTAL_STRIKE);
                scriptEvents.addEvent(EVENT_CARVER_MORTAL_STRIKE, 16000);
                break;
            default:
                break;
        }
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Trash - Scholomance Acolyte

class ScholomanceAcolyteAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new ScholomanceAcolyteAI(c); }
    explicit ScholomanceAcolyteAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        scriptEvents.addEvent(EVENT_ACOLYTE_SHADOW_BOLT, 4000);
        scriptEvents.addEvent(EVENT_ACOLYTE_CURSE, 9000);
    }

    void OnCombatStop(Unit* /*pTarget*/) override { scriptEvents.resetEvents(); }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);
        if (_isCasting())
            return;

        switch (scriptEvents.getFinishedEvent())
        {
            case EVENT_ACOLYTE_SHADOW_BOLT:
                if (Unit* pTarget = selectUnitTarget(FilterArgs(TargetFilter_Player)))
                    castSpell(pTarget, SPELL_ACOLYTE_SHADOW_BOLT);
                scriptEvents.addEvent(EVENT_ACOLYTE_SHADOW_BOLT, 12000);
                break;
            case EVENT_ACOLYTE_CURSE:
                castSpellOnVictim(SPELL_ACOLYTE_CURSE);
                scriptEvents.addEvent(EVENT_ACOLYTE_CURSE, 20000);
                break;
            default:
                break;
        }
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Trash - Candlestick Mage

class CandlestickMageAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new CandlestickMageAI(c); }
    explicit CandlestickMageAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        scriptEvents.addEvent(EVENT_CANDLESTICK_FIREBALL, 4000);
        scriptEvents.addEvent(EVENT_CANDLESTICK_FROSTBOLT, 9000);
    }

    void OnCombatStop(Unit* /*pTarget*/) override { scriptEvents.resetEvents(); }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);
        if (_isCasting())
            return;

        switch (scriptEvents.getFinishedEvent())
        {
            case EVENT_CANDLESTICK_FIREBALL:
                castSpellOnVictim(SPELL_CANDLESTICK_FIREBALL);
                scriptEvents.addEvent(EVENT_CANDLESTICK_FIREBALL, 10000);
                break;
            case EVENT_CANDLESTICK_FROSTBOLT:
                if (Unit* pTarget = selectUnitTarget(FilterArgs(TargetFilter_Player)))
                    castSpell(pTarget, SPELL_CANDLESTICK_FROSTBOLT);
                scriptEvents.addEvent(EVENT_CANDLESTICK_FROSTBOLT, 14000);
                break;
            default:
                break;
        }
    }
};

void SetupScholomanceMop(ScriptMgr* mgr)
{
    mgr->register_instance_script(MAP_SCHOLOMANCE_MOP, &ScholomanceMopInstanceScript::Create);

    mgr->register_creature_script(BOSS_CHILLHEART, &ChillheartAI::Create);
    mgr->register_creature_script(BOSS_JANDICE_BAROV, &JandiceBarovAI::Create);
    mgr->register_creature_script(BOSS_RATTLEGORE, &RattlegoreAI::Create);
    mgr->register_creature_script(BOSS_LILIAN_VOSS, &LilianVossAI::Create);
    mgr->register_creature_script(BOSS_GANDLING, &GandlingAI::Create);

    mgr->register_creature_script(NPC_KRASTINOVIAN_CARVER, &KrastinovianCarverAI::Create);
    mgr->register_creature_script(NPC_SCHOLOMANCE_ACOLYTE, &ScholomanceAcolyteAI::Create);
    mgr->register_creature_script(NPC_CANDLESTICK_MAGE, &CandlestickMageAI::Create);
}
