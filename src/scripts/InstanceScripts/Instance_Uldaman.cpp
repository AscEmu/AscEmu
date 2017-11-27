/*
 * ArcScripts for ArcEmu MMORPG Server
 * Copyright (C) 2008-2012 ArcEmu Team <http://www.ArcEmu.org/>
 * Copyright (C) 2008-2015 Sun++ Team <http://www.sunplusplus.info/>
 * Copyright (C) 2005-2007 Ascent Team
 * Copyright (C) 2007-2015 Moon++ Team <http://www.moonplusplus.info/>
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
#include "Instance_Uldaman.h"


class Archaedas : public CreatureAIScript
{
        ADD_CREATURE_FACTORY_FUNCTION(Archaedas);
        Archaedas(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            enableCreatureAISpellSystem = true;

            auto groundTremor = addAISpell(6524, 20.0f, TARGET_ATTACKING, 0, 10, false, true);
            groundTremor->setAttackStopTimer(3000);
        }

        void OnCombatStop(Unit* /*mTarget*/) override
        {
            setAIAgent(AGENT_NULL);
            getCreature()->GetAIInterface()->setAiState(AI_STATE_IDLE);
        }
};


class Revelosh : public CreatureAIScript
{
        ADD_CREATURE_FACTORY_FUNCTION(Revelosh);
        Revelosh(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            enableCreatureAISpellSystem = true;

            auto unknown = addAISpell(10392, 20.0f, TARGET_ATTACKING, 0, 10, false, true);
            unknown->setAttackStopTimer(3000);

            auto unknown2 = addAISpell(2860, 20.0f, TARGET_ATTACKING, 0, 10, false, true);
            unknown2->setAttackStopTimer(3000);
        }

        void OnCombatStop(Unit* /*mTarget*/) override
        {
            setAIAgent(AGENT_NULL);
            getCreature()->GetAIInterface()->setAiState(AI_STATE_IDLE);
        }
};


class Grimlok : public CreatureAIScript
{
        ADD_CREATURE_FACTORY_FUNCTION(Grimlok);
        Grimlok(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            enableCreatureAISpellSystem = true;

            auto chainBolt = addAISpell(8292, 20.0f, TARGET_ATTACKING, 0, 10, false, true);
            chainBolt->setAttackStopTimer(3000);

            auto unknown = addAISpell(10392, 20.0f, TARGET_ATTACKING, 0, 10, false, true);
            unknown->setAttackStopTimer(3000);

            auto shrink = addAISpell(8066, 20.0f, TARGET_ATTACKING, 0, 10, false, true);
            shrink->setAttackStopTimer(3000);
        }

        void OnCombatStop(Unit* /*mTarget*/) override
        {
            setAIAgent(AGENT_NULL);
            getCreature()->GetAIInterface()->setAiState(AI_STATE_IDLE);
        }
};


class Baelog : public CreatureAIScript
{
        ADD_CREATURE_FACTORY_FUNCTION(Baelog);
        Baelog(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            enableCreatureAISpellSystem = true;

            auto unknown = addAISpell(15613, 20.0f, TARGET_ATTACKING, 0, 10, false, true);
            unknown->setAttackStopTimer(3000);

            auto shieldSlam = addAISpell(15655, 20.0f, TARGET_ATTACKING, 0, 10, false, true);
            shieldSlam->setAttackStopTimer(3000);
        }

        void OnCombatStop(Unit* /*mTarget*/) override
        {
            setAIAgent(AGENT_NULL);
            getCreature()->GetAIInterface()->setAiState(AI_STATE_IDLE);
        }
};


class GalgannFirehammer : public CreatureAIScript
{
        ADD_CREATURE_FACTORY_FUNCTION(GalgannFirehammer);
        GalgannFirehammer(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            enableCreatureAISpellSystem = true;

            auto unknown = addAISpell(10448, 20.0f, TARGET_ATTACKING, 0, 10, false, true);
            unknown->setAttackStopTimer(3000);

            auto flameLash = addAISpell(18958, 20.0f, TARGET_ATTACKING, 0, 10, false, true);
            flameLash->setAttackStopTimer(3000);

            auto fireNova = addAISpell(12470, 20.0f, TARGET_ATTACKING, 0, 10, false, true);
            fireNova->setAttackStopTimer(3000);
        }

        void OnCombatStop(Unit* /*mTarget*/) override
        {
            setAIAgent(AGENT_NULL);
            getCreature()->GetAIInterface()->setAiState(AI_STATE_IDLE);
        }
};


class Ironaya : public CreatureAIScript
{
        ADD_CREATURE_FACTORY_FUNCTION(Ironaya);
        Ironaya(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            enableCreatureAISpellSystem = true;

            auto arcingSmash = addAISpell(16169, 20.0f, TARGET_ATTACKING, 0, 10, false, true);
            arcingSmash->setAttackStopTimer(3000);

            auto warstomp = addAISpell(24375, 20.0f, TARGET_ATTACKING, 0, 10, false, true);
            warstomp->setAttackStopTimer(3000);
        }

        void OnCombatStop(Unit* /*mTarget*/) override
        {
            setAIAgent(AGENT_NULL);
            getCreature()->GetAIInterface()->setAiState(AI_STATE_IDLE);
        }
};

void SetupUldaman(ScriptMgr* mgr)
{
    //Creatures
    mgr->register_creature_script(CN_ARCHAEDAS, &Archaedas::Create);
    mgr->register_creature_script(CN_REVELOSH, &Revelosh::Create);
    mgr->register_creature_script(CN_GRIMLOK, &Grimlok::Create);
    mgr->register_creature_script(CN_BAELOG, &Baelog::Create);
    mgr->register_creature_script(CN_GALGAN_FIREHAMMER, &GalgannFirehammer::Create);
    mgr->register_creature_script(CN_IRONAYA, &Ironaya::Create);
}
