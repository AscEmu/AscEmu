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
#include "Instance_TheStockade.h"

//////////////////////////////////////////////////////////////////////////////////////////
//Stormwind Stockade
class InstanceStormwindStockadeScript : public InstanceScript
{
    public:

        InstanceStormwindStockadeScript(MapMgr* pMapMgr) : InstanceScript(pMapMgr)
        {}

        static InstanceScript* Create(MapMgr* pMapMgr) { return new InstanceStormwindStockadeScript(pMapMgr); }
};

// DeepfuryAI
class DeepfuryAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(DeepfuryAI);
    DeepfuryAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        AddSpell(SHIELD_SLAM, Target_Current, 100, 0, 8);
        AddSpell(IMPROVED_BLOCKING, Target_Self, 100, 0, 20);
    }

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        getCreature()->CastSpell(getCreature(), 7164, false); // Defensive Stance
        
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

// HamhockAI
class HamhockAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(HamhockAI);
    HamhockAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        AddSpell(BLOODLUST, Target_RandomFriendly, 100, 0, 60);
        AddSpell(CHAINLIGHT, Target_Current, 50, 2, 7);

        // new
        addEmoteForEvent(Event_OnCombatStart, 8759);
    }
};

// BazilAI
class BazilAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(BazilAI);
    BazilAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        AddSpell(SMOKE_BOMB, Target_Current, 100, 9, 15);
        AddSpell(BATTLE_SHOUT, Target_Self, 100, 3, 30);

        // new
        addEmoteForEvent(Event_OnCombatStart, 8760);
    }

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        getCreature()->CastSpell(getCreature(), 674, false); // Dual Wield
    }
};

// DextrenAI
class DextrenAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(DextrenAI);
    DextrenAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        AddSpell(FRIGHTENING_SHOUT, Target_Current, 33, 8, 30);
        AddSpell(STRIKE, Target_Self, 33, 0, 10);
    }

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        getCreature()->CastSpell(getCreature(), 7165, false); // Battle Stance
    }
};

// TargorrTheDreadAI
class TargorrTheDreadAI : public CreatureAIScript
{
    bool Enrage = false;
    SpellDesc *Enraged;

    ADD_CREATURE_FACTORY_FUNCTION(TargorrTheDreadAI);
    TargorrTheDreadAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        Enraged = AddSpell(ENRAGE, Target_Self, 0, 0, 0);
        AddSpell(THRASH, Target_Self, 50, 0, 8);
        Enrage = false;
    }

    void AIUpdate() override
    {
        if (_getHealthPercent() < 50 && !Enrage)
        {
            Enrage = true;
            CastSpellNowNoScheduling(Enraged);
        }
    }
};

// InmateAI
class InmateAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(InmateAI);
    InmateAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        AddSpell(REND, Target_Current, 100, 5, 16);
    }

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        getCreature()->CastSpell(getCreature(), 7165, false); // Battle Stance
    }
};

// InsurgentAI
class InsurgentAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(InsurgentAI);
    InsurgentAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        AddSpell(DEMORALIZING_SHOUT, Target_Self, 100, 7, 25);
    }

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        getCreature()->CastSpell(getCreature(), 9128, false); // Battle Shout
    }
};

// PrisonerAI
class PrisonerAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(PrisonerAI);
    PrisonerAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        AddSpell(KICK, Target_Current, 100, 5, 16);
        AddSpell(DISARM, Target_Current, 100, 10, 14);
    }

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        getCreature()->CastSpell(getCreature(), 7165, false); // Battle Stance
    }
};

// ConvictAI
class ConvictAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(ConvictAI);
    ConvictAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        AddSpell(BACKHAND, Target_Current, 100, 5, 12);
    }

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        getCreature()->CastSpell(getCreature(), 674, false); // Dual Wield
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
