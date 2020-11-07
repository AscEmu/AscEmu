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
#include "Instance_RazorfenDowns.h"

class RazorfenDownsInstanceScript : public InstanceScript
{
public:

    explicit RazorfenDownsInstanceScript(MapMgr* pMapMgr) : InstanceScript(pMapMgr)
    {
    }

    static InstanceScript* Create(MapMgr* pMapMgr) { return new RazorfenDownsInstanceScript(pMapMgr); }


};

class AmnennarTheColdbringerAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(AmnennarTheColdbringerAI)
    explicit AmnennarTheColdbringerAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        auto unknow = addAISpell(10179, 20.0f, TARGET_ATTACKING, 0, 10, false, true);
        unknow->setAttackStopTimer(3000);

        auto frostNova = addAISpell(22645, 20.0f, TARGET_ATTACKING, 0, 10, false, true);
        frostNova->setAttackStopTimer(3000);

        auto amnennarsWhath = addAISpell(13009, 20.0f, TARGET_ATTACKING, 0, 10, false, true);
        amnennarsWhath->setAttackStopTimer(3000);
    }
};


class GluttonAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(GluttonAI)
    explicit GluttonAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        mDiseaseCloud = addAISpell(SP_GLUTTON_DISEASE_CLOUD, 0.0f, TARGET_SELF, 0, 0, false, true);

        auto mFrenzy = addAISpell(SP_GLUTTON_FRENZY, 10, TARGET_ATTACKING, 0, 20);
        mFrenzy->addEmote("Glutton is getting hungry!", CHAT_MSG_MONSTER_YELL);
    }

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        getCreature()->castSpell(getCreature(), mDiseaseCloud->mSpellInfo, true);
    }

    CreatureAISpells* mDiseaseCloud;
};


class MordreshFireEyeAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(MordreshFireEyeAI)
    explicit MordreshFireEyeAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        addAISpell(SP_MORDRESH_FIRE_NOVA, 10.0f, TARGET_SELF, 2, 0);
        addAISpell(SP_MORDRESH_FIREBALL, 10.0f, TARGET_ATTACKING, 3, 0);
    }
};

class PlaguemawTheRottingAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(PlaguemawTheRottingAI)
    explicit PlaguemawTheRottingAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        auto witheredTouch = addAISpell(12947, 20.0f, TARGET_ATTACKING, 0, 10, false, true);
        witheredTouch->setAttackStopTimer(3000);

        auto putridStench = addAISpell(12946, 20.0f, TARGET_ATTACKING, 0, 10, false, true);
        putridStench->setAttackStopTimer(3000);
    }
};

class RagglesnoutAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(RagglesnoutAI)
    explicit RagglesnoutAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        auto unknown = addAISpell(10892, 20.0f, TARGET_ATTACKING, 0, 10, false, true);
        unknown->setAttackStopTimer(3000);

        auto unknown2 = addAISpell(11659, 20.0f, TARGET_ATTACKING, 0, 10, false, true);
        unknown2->setAttackStopTimer(3000);
    }
};


class TutenKashAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(TutenKashAI)
    explicit TutenKashAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        auto curseOfTutenKash = addAISpell(12255, 20.0f, TARGET_ATTACKING, 0, 10, false, true);
        curseOfTutenKash->setAttackStopTimer(3000);

        auto webSpray = addAISpell(12252, 20.0f, TARGET_ATTACKING, 0, 10, false, true);
        webSpray->setAttackStopTimer(3000);
    }
};

void SetupRazorfenDowns(ScriptMgr* mgr)
{
    mgr->register_instance_script(MAP_RAZORFEN_DOWNS, &RazorfenDownsInstanceScript::Create);
    mgr->register_creature_script(CN_AMNENNAR_GOLDBRINGER, &AmnennarTheColdbringerAI::Create);
    mgr->register_creature_script(CN_GLUTTON, &GluttonAI::Create);
    mgr->register_creature_script(CN_MORDRESH_FIRE_EYE, &MordreshFireEyeAI::Create);
    mgr->register_creature_script(CN_PLAGUEMAW_THE_ROTTING, &PlaguemawTheRottingAI::Create);
    mgr->register_creature_script(CN_RAGGLESNOUT, &RagglesnoutAI::Create);
    mgr->register_creature_script(CN_TUTEN_KASH, &TutenKashAI::Create);
}
