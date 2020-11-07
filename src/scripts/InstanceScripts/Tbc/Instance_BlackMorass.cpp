/*
 * Copyright (c) 2014-2020 AscEmu Team <http://www.ascemu.org>
 * Copyright (c) 2008-2015 Sun++ Team <http://www.sunplusplus.info>
 * Copyright (c) 2007-2015 Moon++ Team <http://www.moonplusplus.info>
 * Copyright (C) 2008-2012 ArcEmu Team <http://www.ArcEmu.org/>
 * Copyright (C) 2005-2007 Ascent Team
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "Setup.h"
#include "Instance_BlackMorass.h"

class TheBlackMorassInstanceScript : public InstanceScript
{
public:

    explicit TheBlackMorassInstanceScript(MapMgr* pMapMgr) : InstanceScript(pMapMgr)
    {
    }

    static InstanceScript* Create(MapMgr* pMapMgr) { return new TheBlackMorassInstanceScript(pMapMgr); }


};

// ChronoLordAI
class ChronoLordAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(ChronoLordAI)
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
    ADD_CREATURE_FACTORY_FUNCTION(TemporusAI)
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


//AenusAI
class AenusAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(AenusAI)
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
