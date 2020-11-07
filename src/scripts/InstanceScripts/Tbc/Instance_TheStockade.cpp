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
#include "Instance_TheStockade.h"

class InstanceStormwindStockadeScript : public InstanceScript
{
public:

    explicit InstanceStormwindStockadeScript(MapMgr* pMapMgr) : InstanceScript(pMapMgr)
    {}

    static InstanceScript* Create(MapMgr* pMapMgr) { return new InstanceStormwindStockadeScript(pMapMgr); }
};

class DeepfuryAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(DeepfuryAI)
    explicit DeepfuryAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        addAISpell(SHIELD_SLAM, 100.0f, TARGET_ATTACKING, 0, 8);
        addAISpell(IMPROVED_BLOCKING, 100.0f, TARGET_SELF, 0, 20);
    }

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        getCreature()->castSpell(getCreature(), 7164, false); // Defensive Stance
    }

    void AIUpdate() override
    {
        if (_getHealthPercent() <= 15 && getAIAgent() != AGENT_FLEE)
        {
            setAIAgent(AGENT_FLEE);
            _setMeleeDisabled(false);
            _setRangedDisabled(true);
            _setCastDisabled(true);
            moveTo(float(105.693390), float(-58.426674), float(-34.856178), true);
        }
    }
};

class HamhockAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(HamhockAI)
    explicit HamhockAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        addAISpell(BLOODLUST, 100.0f, TARGET_RANDOM_FRIEND, 0, 60);
        addAISpell(CHAINLIGHT, 50.0f, TARGET_ATTACKING, 2, 7);

        addEmoteForEvent(Event_OnCombatStart, 8759);
    }
};

class BazilAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(BazilAI)
    explicit BazilAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        addAISpell(SMOKE_BOMB, 100.0f, TARGET_ATTACKING, 9, 15);
        addAISpell(BATTLE_SHOUT, 100.0f, TARGET_SELF, 3, 30);

        addEmoteForEvent(Event_OnCombatStart, 8760);
    }

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        getCreature()->castSpell(getCreature(), 674, false); // Dual Wield
    }
};

class DextrenAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(DextrenAI)
    explicit DextrenAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        addAISpell(FRIGHTENING_SHOUT, 33.0f, TARGET_ATTACKING, 8, 30);
        addAISpell(STRIKE, 33.0f, TARGET_SELF, 0, 10);
    }

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        getCreature()->castSpell(getCreature(), 7165, false); // Battle Stance
    }
};

class TargorrTheDreadAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(TargorrTheDreadAI)

    bool Enrage;
    CreatureAISpells *Enraged;

    explicit TargorrTheDreadAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        Enraged = addAISpell(ENRAGE, 0.0f, TARGET_SELF);
        addAISpell(THRASH, 50.0f, TARGET_SELF, 0, 8);
        Enrage = false;
    }

    void AIUpdate() override
    {
        if (_getHealthPercent() < 50 && !Enrage)
        {
            Enrage = true;
            _castAISpell(Enraged);
        }
    }
};

class InmateAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(InmateAI)
    explicit InmateAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        addAISpell(REND, 100.0f, TARGET_ATTACKING, 5, 16);
    }

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        getCreature()->castSpell(getCreature(), 7165, false); // Battle Stance
    }
};

class InsurgentAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(InsurgentAI)
    explicit InsurgentAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        addAISpell(DEMORALIZING_SHOUT, 100.0f, TARGET_SELF, 7, 25);
    }

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        getCreature()->castSpell(getCreature(), 9128, false); // Battle Shout
    }
};

class PrisonerAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(PrisonerAI)
    explicit PrisonerAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        addAISpell(KICK, 100.0f, TARGET_ATTACKING, 5, 16);
        addAISpell(DISARM, 100.0f, TARGET_ATTACKING, 10, 14);
    }

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        getCreature()->castSpell(getCreature(), 7165, false); // Battle Stance
    }
};

class ConvictAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(ConvictAI)
    explicit ConvictAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        addAISpell(BACKHAND, 100.0f, TARGET_ATTACKING, 5, 12);
    }

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        getCreature()->castSpell(getCreature(), 674, false); // Dual Wield
    }
};

void SetupTheStockade(ScriptMgr* mgr)
{
    //Instance
    mgr->register_instance_script(MAP_THE_STOCKADE, &InstanceStormwindStockadeScript::Create);

    //Creatures
    mgr->register_creature_script(CN_KAMDEEPFURY, &DeepfuryAI::Create);
    mgr->register_creature_script(CN_HAMHOCK, &HamhockAI::Create);
    mgr->register_creature_script(CN_BAZILTHREDD, &BazilAI::Create);
    mgr->register_creature_script(CN_DEXTRENWARD, &DextrenAI::Create);
    mgr->register_creature_script(CN_TARGORR_THE_DREAD, &TargorrTheDreadAI::Create);
    mgr->register_creature_script(CN_DEFINMATE, &InmateAI::Create);
    mgr->register_creature_script(CN_DEFINSURGENT, &InsurgentAI::Create);
    mgr->register_creature_script(CN_DEFPRISONER, &PrisonerAI::Create);
    mgr->register_creature_script(CN_DEFCONVICT, &ConvictAI::Create);
}
