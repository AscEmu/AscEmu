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

// \todo move most defines to enum, text to db (use SendScriptTextChatMessage(ID))
#include "Setup.h"
#include "Instance_Maraudon.h"


class CelebrasTheCursedAI : public CreatureAIScript
{
        ADD_CREATURE_FACTORY_FUNCTION(CelebrasTheCursedAI);
        CelebrasTheCursedAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            enableCreatureAISpellSystem = true;

            auto wrath = addAISpell(21667, 20.0f, TARGET_ATTACKING, 0, 10, false, true);
            wrath->setAttackStopTimer(3000);

            auto entanglingRoots = addAISpell(21331, 20.0f, TARGET_ATTACKING, 0, 10, false, true);
            entanglingRoots->setAttackStopTimer(3000);

            auto twistedTranquility = addAISpell(21793, 20.0f, TARGET_ATTACKING, 0, 10, false, true);
            twistedTranquility->setAttackStopTimer(3000);
        }

        void OnCombatStop(Unit* /*mTarget*/) override
        {
            setAIAgent(AGENT_NULL);
            getCreature()->GetAIInterface()->setAiState(AI_STATE_IDLE);
        }
};


class LordVyletongueAI : public CreatureAIScript
{
        ADD_CREATURE_FACTORY_FUNCTION(LordVyletongueAI);
        LordVyletongueAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            enableCreatureAISpellSystem = true;

            auto putridBreath = addAISpell(21080, 20.0f, TARGET_ATTACKING, 0, 10, false, true);
            putridBreath->setAttackStopTimer(3000);

            auto smokeBomb = addAISpell(8817, 20.0f, TARGET_ATTACKING, 0, 10, false, true);
            smokeBomb->setAttackStopTimer(3000);
        }

        void OnCombatStop(Unit* /*mTarget*/) override
        {
            setAIAgent(AGENT_NULL);
            getCreature()->GetAIInterface()->setAiState(AI_STATE_IDLE);
        }
};


class MeshlokTheHarvesterAI : public CreatureAIScript
{
        ADD_CREATURE_FACTORY_FUNCTION(MeshlokTheHarvesterAI);
        MeshlokTheHarvesterAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            enableCreatureAISpellSystem = true;

            auto warStomp = addAISpell(24375, 20.0f, TARGET_ATTACKING, 0, 10, false, true);
            warStomp->setAttackStopTimer(3000);

            auto strike = addAISpell(15580, 20.0f, TARGET_ATTACKING, 0, 10, false, true);
            strike->setAttackStopTimer(3000);
        }

        void OnCombatStop(Unit* /*mTarget*/) override
        {
            setAIAgent(AGENT_NULL);
            getCreature()->GetAIInterface()->setAiState(AI_STATE_IDLE);
        }
};


class PrincessTheradrasAI : public CreatureAIScript
{
        ADD_CREATURE_FACTORY_FUNCTION(PrincessTheradrasAI);
        PrincessTheradrasAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            enableCreatureAISpellSystem = true;

            auto duelField = addAISpell(21909, 20.0f, TARGET_ATTACKING, 0, 10, false, true);
            duelField->setAttackStopTimer(3000);

            auto boulder = addAISpell(21832, 20.0f, TARGET_ATTACKING, 0, 10, false, true);
            boulder->setAttackStopTimer(3000);

            auto knockdown = addAISpell(19128, 20.0f, TARGET_ATTACKING, 0, 10, false, true);
            knockdown->setAttackStopTimer(3000);

            auto repulsiveGaze = addAISpell(21869, 20.0f, TARGET_ATTACKING, 0, 10, false, true);
            repulsiveGaze->setAttackStopTimer(3000);
        }

        void OnCombatStop(Unit* /*mTarget*/) override
        {
            setAIAgent(AGENT_NULL);
            getCreature()->GetAIInterface()->setAiState(AI_STATE_IDLE);
        }
};


class RazorlashAI : public CreatureAIScript
{
        ADD_CREATURE_FACTORY_FUNCTION(RazorlashAI);
        RazorlashAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            enableCreatureAISpellSystem = true;

            auto puncture = addAISpell(21911, 20.0f, TARGET_ATTACKING, 0, 10, false, true);
            puncture->setAttackStopTimer(3000);

            auto unknown = addAISpell(15584, 20.0f, TARGET_ATTACKING, 0, 10, false, true);
            unknown->setAttackStopTimer(3000);

            auto unknown2 = addAISpell(21749, 20.0f, TARGET_ATTACKING, 0, 10, false, true);
            unknown2->setAttackStopTimer(3000);
        }

        void OnCombatStop(Unit* /*mTarget*/) override
        {
            setAIAgent(AGENT_NULL);
            getCreature()->GetAIInterface()->setAiState(AI_STATE_IDLE);
        }
};


class TinkererGizlockAI : public CreatureAIScript
{
        ADD_CREATURE_FACTORY_FUNCTION(TinkererGizlockAI);
        TinkererGizlockAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            enableCreatureAISpellSystem = true;

            auto goblinDragonGun = addAISpell(21833, 20.0f, TARGET_ATTACKING, 0, 10, false, true);
            goblinDragonGun->setAttackStopTimer(3000);

            auto bomb = addAISpell(22334, 20.0f, TARGET_ATTACKING, 0, 10, false, true);
            bomb->setAttackStopTimer(3000);
        }

        void OnCombatStop(Unit* /*mTarget*/) override
        {
            setAIAgent(AGENT_NULL);
            getCreature()->GetAIInterface()->setAiState(AI_STATE_IDLE);
        }
};


class NoxxionAI : public CreatureAIScript
{
        ADD_CREATURE_FACTORY_FUNCTION(NoxxionAI);
        NoxxionAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            enableCreatureAISpellSystem = true;

            auto toxicVolley = addAISpell(21687, 20.0f, TARGET_ATTACKING, 0, 10, false, true);
            toxicVolley->setAttackStopTimer(3000);

            auto sporeCloud = addAISpell(21547, 20.0f, TARGET_ATTACKING, 0, 10, false, true);
            sporeCloud->setAttackStopTimer(3000);
        }

        void OnCombatStop(Unit* /*mTarget*/) override
        {
            setAIAgent(AGENT_NULL);
            getCreature()->GetAIInterface()->setAiState(AI_STATE_IDLE);
        }
};

void SetupMaraudon(ScriptMgr* mgr)
{
    mgr->register_creature_script(CN_CELEBRAS_THE_CURESE, &CelebrasTheCursedAI::Create);
    mgr->register_creature_script(CN_LORD_VYLETONGUE, &LordVyletongueAI::Create);
    mgr->register_creature_script(CN_MESHLOCK_THE_HARVESTER, &MeshlokTheHarvesterAI::Create);
    mgr->register_creature_script(CN_PRINCESS_THERADRAS, &PrincessTheradrasAI::Create);
    mgr->register_creature_script(CN_RAZORLASH, &RazorlashAI::Create);
    mgr->register_creature_script(CN_TRINKERER_GIZLOCK, &TinkererGizlockAI::Create);
    mgr->register_creature_script(CN_NOXXION, &NoxxionAI::Create);
}
