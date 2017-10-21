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
class InstanceStormwindStockadeScript : public MoonInstanceScript
{
    public:

        MOONSCRIPT_INSTANCE_FACTORY_FUNCTION(InstanceStormwindStockadeScript, MoonInstanceScript);
        InstanceStormwindStockadeScript(MapMgr* pMapMgr) : MoonInstanceScript(pMapMgr)
        {
            // Way to select bosses
            BuildEncounterMap();
            if (mEncounters.size() == 0)
                return;

            for (EncounterMap::iterator Iter = mEncounters.begin(); Iter != mEncounters.end(); ++Iter)
            {
                if ((*Iter).second.mState != State_Finished)
                    continue;
            }
        }

        void OnGameObjectPushToWorld(GameObject* pGameObject) { }

        void SetInstanceData(uint32 pType, uint32 pIndex, uint32 pData)
        {
            if (pType != Data_EncounterState || pIndex == 0)
                return;

            EncounterMap::iterator Iter = mEncounters.find(pIndex);
            if (Iter == mEncounters.end())
                return;

            (*Iter).second.mState = (EncounterState)pData;
        }

        uint32 GetInstanceData(uint32 pType, uint32 pIndex)
        {
            if (pType != Data_EncounterState || pIndex == 0)
                return 0;

            EncounterMap::iterator Iter = mEncounters.find(pIndex);
            if (Iter == mEncounters.end())
                return 0;

            return (*Iter).second.mState;
        }

        void OnCreatureDeath(Creature* pCreature, Unit* pUnit)
        {
            EncounterMap::iterator Iter = mEncounters.find(pCreature->GetEntry());
            if (Iter == mEncounters.end())
                return;

            (*Iter).second.mState = State_Finished;

            return;
        }
};

// DeepfuryAI
class DeepfuryAI : public MoonScriptBossAI
{
    MOONSCRIPT_FACTORY_FUNCTION(DeepfuryAI, MoonScriptBossAI);
    DeepfuryAI(Creature* pCreature) : MoonScriptBossAI(pCreature)
    {
        AddSpell(SHIELD_SLAM, Target_Current, 100, 0, 8);
        AddSpell(IMPROVED_BLOCKING, Target_Self, 100, 0, 20);
    }

    void OnCombatStart(Unit* pTarget)
    {
        _unit->CastSpell(_unit, 7164, false); // Defensive Stance
        ParentClass::OnCombatStart(pTarget);
    }

    void AIUpdate()
    {
        if (GetHealthPercent() <= 15 && GetBehavior() != Behavior_Flee)
        {
            SetBehavior(Behavior_Flee);
            SetAllowMelee(false);
            SetAllowRanged(false);
            SetAllowSpell(false);
            moveTo(float(105.693390), float(-58.426674), float(-34.856178), true);
        }
        ParentClass::AIUpdate();
    }
};

// HamhockAI
class HamhockAI : public MoonScriptCreatureAI
{
    MOONSCRIPT_FACTORY_FUNCTION(HamhockAI, MoonScriptCreatureAI);
    HamhockAI(Creature* pCreature) : MoonScriptCreatureAI(pCreature)
    {
        AddSpell(BLOODLUST, Target_RandomFriendly, 100, 0, 60);
        AddSpell(CHAINLIGHT, Target_Current, 50, 2, 7);
    }

    void OnCombatStart(Unit* pTarget)
    {
        sendDBChatMessage(8759);
        ParentClass::OnCombatStart(pTarget);
    }
};

// BazilAI
class BazilAI : public MoonScriptCreatureAI
{
    MOONSCRIPT_FACTORY_FUNCTION(BazilAI, MoonScriptCreatureAI);
    BazilAI(Creature* pCreature) : MoonScriptCreatureAI(pCreature)
    {
        AddSpell(SMOKE_BOMB, Target_Current, 100, 9, 15);
        AddSpell(BATTLE_SHOUT, Target_Self, 100, 3, 30);
    }

    void OnCombatStart(Unit* pTarget)
    {
        _unit->CastSpell(_unit, 674, false); // Dual Wield
        sendDBChatMessage(8760);
        ParentClass::OnCombatStart(pTarget);
    }
};

// DextrenAI
class DextrenAI : public MoonScriptCreatureAI
{
    MOONSCRIPT_FACTORY_FUNCTION(DextrenAI, MoonScriptCreatureAI);
    DextrenAI(Creature* pCreature) : MoonScriptCreatureAI(pCreature)
    {
        AddSpell(FRIGHTENING_SHOUT, Target_Current, 33, 8, 30);
        AddSpell(STRIKE, Target_Self, 33, 0, 10);
    }

    void OnCombatStart(Unit* pTarget)
    {
        _unit->CastSpell(_unit, 7165, false); // Battle Stance
        ParentClass::OnCombatStart(pTarget);
    }
};

// TargorrTheDreadAI
class TargorrTheDreadAI : public MoonScriptCreatureAI
{
    bool Enrage = false;
    SpellDesc *Enraged;

    MOONSCRIPT_FACTORY_FUNCTION(TargorrTheDreadAI, MoonScriptCreatureAI);
    TargorrTheDreadAI(Creature* pCreature) : MoonScriptCreatureAI(pCreature)
    {
        Enraged = AddSpell(ENRAGE, Target_Self, 0, 0, 0);
        AddSpell(THRASH, Target_Self, 50, 0, 8);
        Enrage = false;
    }

    void AIUpdate()
    {
        if (GetHealthPercent() < 50 && !Enrage)
        {
            Enrage = true;
            CastSpellNowNoScheduling(Enraged);
        }
        ParentClass::AIUpdate();
    }
};

// InmateAI
class InmateAI : public MoonScriptCreatureAI
{
    MOONSCRIPT_FACTORY_FUNCTION(InmateAI, MoonScriptCreatureAI);
    InmateAI(Creature* pCreature) : MoonScriptCreatureAI(pCreature)
    {
        AddSpell(REND, Target_Current, 100, 5, 16);
    }

    void OnCombatStart(Unit* pTarget)
    {
        _unit->CastSpell(_unit, 7165, false); // Battle Stance
        ParentClass::OnCombatStart(pTarget);
    }
};

// InsurgentAI
class InsurgentAI : public MoonScriptCreatureAI
{
    MOONSCRIPT_FACTORY_FUNCTION(InsurgentAI, MoonScriptCreatureAI);
    InsurgentAI(Creature* pCreature) : MoonScriptCreatureAI(pCreature)
    {
        AddSpell(DEMORALIZING_SHOUT, Target_Self, 100, 7, 25);
    }

    void OnCombatStart(Unit* pTarget)
    {
        _unit->CastSpell(_unit, 9128, false); // Battle Shout
        ParentClass::OnCombatStart(pTarget);
    }
};

// PrisonerAI
class PrisonerAI : public MoonScriptCreatureAI
{
    MOONSCRIPT_FACTORY_FUNCTION(PrisonerAI, MoonScriptCreatureAI);
    PrisonerAI(Creature* pCreature) : MoonScriptCreatureAI(pCreature)
    {
        AddSpell(KICK, Target_Current, 100, 5, 16);
        AddSpell(DISARM, Target_Current, 100, 10, 14);
    }

    void OnCombatStart(Unit* pTarget)
    {
        _unit->CastSpell(_unit, 7165, false); // Battle Stance
        ParentClass::OnCombatStart(pTarget);
    }
};

// ConvictAI
class ConvictAI : public MoonScriptCreatureAI
{
    MOONSCRIPT_FACTORY_FUNCTION(ConvictAI, MoonScriptCreatureAI);
    ConvictAI(Creature* pCreature) : MoonScriptCreatureAI(pCreature)
    {
        AddSpell(BACKHAND, Target_Current, 100, 5, 12);
    }

    void OnCombatStart(Unit* pTarget)
    {
        _unit->CastSpell(_unit, 674, false); // Dual Wield
        ParentClass::OnCombatStart(pTarget);
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
