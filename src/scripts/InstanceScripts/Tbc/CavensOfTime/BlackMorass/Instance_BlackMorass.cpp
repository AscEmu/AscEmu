/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Instance_BlackMorass.h"

#include "Setup.h"
#include "Server/Script/CreatureAIScript.hpp"
#include "Server/Script/InstanceScript.hpp"

class TheBlackMorassInstanceScript : public InstanceScript
{
public:
    explicit TheBlackMorassInstanceScript(WorldMap* pMapMgr) : InstanceScript(pMapMgr){}
    static InstanceScript* Create(WorldMap* pMapMgr) { return new TheBlackMorassInstanceScript(pMapMgr); }
};

// ChronoLordAI
class ChronoLordAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new ChronoLordAI(c); }
    explicit ChronoLordAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        auto arcaneBlast = addAISpell(ARCANE_BLAST, 0.0f, TARGET_ATTACKING, 0, 10);
        arcaneBlast->setAttackStopTimer(1000);

        auto timeLapse = addAISpell(TIME_LAPSE, 0.0f, TARGET_ATTACKING, 0, 8);
        timeLapse->setAttackStopTimer(1000);

        addEmoteForEvent(Event_OnCombatStart, SAY_CHRONOLORD_01);
        addEmoteForEvent(Event_OnTargetDied, SAY_CHRONOLORD_02);
        addEmoteForEvent(Event_OnTargetDied, SAY_CHRONOLORD_03);
        addEmoteForEvent(Event_OnDied, SAY_CHRONOLORD_04);
    }
};

// TemporusAI
class TemporusAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new TemporusAI(c); }
    explicit TemporusAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        auto haste = addAISpell(HASTEN, 0.0f, TARGET_SELF, 0, 10);
        haste->setAttackStopTimer(1000);

        auto mortalWound = addAISpell(MORTAL_WOUND, 0.0f, TARGET_ATTACKING, 0, 5);
        mortalWound->setAttackStopTimer(1000);

        addEmoteForEvent(Event_OnCombatStart, SAY_TEMPORUS_01);
        addEmoteForEvent(Event_OnTargetDied, SAY_TEMPORUS_02);
        addEmoteForEvent(Event_OnTargetDied, SAY_TEMPORUS_03);
        addEmoteForEvent(Event_OnDied, SAY_TEMPORUS_04);
    }
};

// AenusAI
class AenusAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new AenusAI(c); }
    explicit AenusAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        auto sandBreath = addAISpell(SAND_BREATH, 0.0f, TARGET_DESTINATION, 0, 15, false, true);
        sandBreath->setAttackStopTimer(1000);

        auto timeStop = addAISpell(TIME_STOP, 0.0f, TARGET_VARIOUS, 0, 15, false, true);
        timeStop->setAttackStopTimer(1000);

        auto frenzy = addAISpell(FRENZY, 0.0f, TARGET_SELF, 0, 8, false, true);
        frenzy->setAttackStopTimer(1000);

        addEmoteForEvent(Event_OnCombatStart, SAY_AENUS_01);
        addEmoteForEvent(Event_OnTargetDied, SAY_AENUS_02);
        addEmoteForEvent(Event_OnTargetDied, SAY_AENUS_03);
        addEmoteForEvent(Event_OnDied, SAY_AENUS_04);
    }
};

void SetupTheBlackMorass(ScriptMgr* mgr)
{
    mgr->register_instance_script(MAP_COT_BLACK_MORASS, &TheBlackMorassInstanceScript::Create);

    mgr->register_creature_script(CN_CHRONO_LORD_DEJA, &ChronoLordAI::Create);
    mgr->register_creature_script(CN_TEMPORUS, &TemporusAI::Create);
    mgr->register_creature_script(CN_AEONUS, &AenusAI::Create);
}
