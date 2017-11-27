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
#include "Instance_RazorfenDowns.h"


class AmnennarTheColdbringerAI : public CreatureAIScript
{

        ADD_CREATURE_FACTORY_FUNCTION(AmnennarTheColdbringerAI);
        AmnennarTheColdbringerAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            enableCreatureAISpellSystem = true;

            auto unknow = addAISpell(10179, 20.0f, TARGET_ATTACKING, 0, 10, false, true);
            unknow->setAttackStopTimer(3000);

            auto frostNova = addAISpell(22645, 20.0f, TARGET_ATTACKING, 0, 10, false, true);
            frostNova->setAttackStopTimer(3000);

            auto amnennarsWhath = addAISpell(13009, 20.0f, TARGET_ATTACKING, 0, 10, false, true);
            amnennarsWhath->setAttackStopTimer(3000);
        }

        void OnCombatStop(Unit* /*mTarget*/) override
        {
            setAIAgent(AGENT_NULL);
            getCreature()->GetAIInterface()->setAiState(AI_STATE_IDLE);
        }
};


class GluttonAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(GluttonAI);
    GluttonAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        enableCreatureAISpellSystem = true;

        mDiseaseCloud = addAISpell(SP_GLUTTON_DISEASE_CLOUD, 0.0f, TARGET_SELF, 0, 0, false, true);

        auto mFrenzy = addAISpell(SP_GLUTTON_FRENZY, 10, TARGET_ATTACKING, 0, 20);
        mFrenzy->addEmote("Glutton is getting hungry!", CHAT_MSG_MONSTER_YELL);
    }

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        getCreature()->CastSpell(getCreature(), mDiseaseCloud->mSpellInfo, true);
    }

    CreatureAISpells* mDiseaseCloud;
};


class MordreshFireEyeAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(MordreshFireEyeAI);
    MordreshFireEyeAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        //spells
        AddSpell(SP_MORDRESH_FIRE_NOVA, Target_Self, 10, 2, 0);
        AddSpell(SP_MORDRESH_FIREBALL, Target_Current, 10, 3, 0, 0, 40);
    }
};

class PlaguemawTheRottingAI : public CreatureAIScript
{
        ADD_CREATURE_FACTORY_FUNCTION(PlaguemawTheRottingAI);
        PlaguemawTheRottingAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            enableCreatureAISpellSystem = true;

            auto witheredTouch = addAISpell(12947, 20.0f, TARGET_ATTACKING, 0, 10, false, true);
            witheredTouch->setAttackStopTimer(3000);

            auto putridStench = addAISpell(12946, 20.0f, TARGET_ATTACKING, 0, 10, false, true);
            putridStench->setAttackStopTimer(3000);
        }

        void OnCombatStop(Unit* /*mTarget*/) override
        {
            setAIAgent(AGENT_NULL);
            getCreature()->GetAIInterface()->setAiState(AI_STATE_IDLE);
        }
};

class RagglesnoutAI : public CreatureAIScript
{
        ADD_CREATURE_FACTORY_FUNCTION(RagglesnoutAI);
        RagglesnoutAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            enableCreatureAISpellSystem = true;

            auto unknown = addAISpell(10892, 20.0f, TARGET_ATTACKING, 0, 10, false, true);
            unknown->setAttackStopTimer(3000);

            auto unknown2 = addAISpell(11659, 20.0f, TARGET_ATTACKING, 0, 10, false, true);
            unknown2->setAttackStopTimer(3000);
        }

        void OnCombatStop(Unit* /*mTarget*/) override
        {
            setAIAgent(AGENT_NULL);
            getCreature()->GetAIInterface()->setAiState(AI_STATE_IDLE);
        }
};


class TutenKashAI : public CreatureAIScript
{
        ADD_CREATURE_FACTORY_FUNCTION(TutenKashAI);
        TutenKashAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            enableCreatureAISpellSystem = true;

            auto curseOfTutenKash = addAISpell(12255, 20.0f, TARGET_ATTACKING, 0, 10, false, true);
            curseOfTutenKash->setAttackStopTimer(3000);

            auto webSpray = addAISpell(12252, 20.0f, TARGET_ATTACKING, 0, 10, false, true);
            webSpray->setAttackStopTimer(3000);
        }

        void OnCombatStop(Unit* /*mTarget*/) override
        {
            setAIAgent(AGENT_NULL);
            getCreature()->GetAIInterface()->setAiState(AI_STATE_IDLE);
        }
};

void SetupRazorfenDowns(ScriptMgr* mgr)
{
    mgr->register_creature_script(CN_AMNENNAR_GOLDBRINGER, &AmnennarTheColdbringerAI::Create);
    mgr->register_creature_script(CN_GLUTTON, &GluttonAI::Create);
    mgr->register_creature_script(CN_MORDRESH_FIRE_EYE, &MordreshFireEyeAI::Create);
    mgr->register_creature_script(CN_PLAGUEMAW_THE_ROTTING, &PlaguemawTheRottingAI::Create);
    mgr->register_creature_script(CN_RAGGLESNOUT, &RagglesnoutAI::Create);
    mgr->register_creature_script(CN_TUTEN_KASH, &TutenKashAI::Create);
}
