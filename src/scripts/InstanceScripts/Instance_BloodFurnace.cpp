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
#include "Instance_BloodFurnace.h"

//////////////////////////////////////////////////////////////////////////////////////////
//Hellfire Citadel: The Blood Furnace
class InstanceTheBloodFurnaceScript : public MoonInstanceScript
{
    public:

        MOONSCRIPT_INSTANCE_FACTORY_FUNCTION(InstanceTheBloodFurnaceScript, MoonInstanceScript);
        InstanceTheBloodFurnaceScript(MapMgr* pMapMgr) : MoonInstanceScript(pMapMgr)
        {
            // Way to select bosses
            BuildEncounterMap();
            if (mEncounters.size() == 0)
                return;

            for (auto Iter = mEncounters.begin(); Iter != mEncounters.end(); ++Iter)
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

// Keli'dan the BreakerAI
class KelidanTheBreakerAI : public MoonScriptBossAI
{
    MOONSCRIPT_FACTORY_FUNCTION(KelidanTheBreakerAI, MoonScriptBossAI);
    KelidanTheBreakerAI(Creature* pCreature) : MoonScriptBossAI(pCreature)
    {
        //spells
        if (IsHeroic())
        {
            mShadowBoltVolley = AddSpell(KELIDAN_SHADOW_BOLT_VOLLEY_H, Target_Self, 25, 0, 6);
            mFireNova = AddSpell(KELIDAN_FIRE_NOVA_H, Target_Current, 15, 0, 12);
        }
        else
        {
            mShadowBoltVolley = AddSpell(KELIDAN_SHADOW_BOLT_VOLLEY, Target_Self, 25, 0, 6);
            mFireNova = AddSpell(KELIDAN_FIRE_NOVA, Target_Self, 15, 0, 12);
        }

        mBurningNova = AddSpell(KELIDAN_BURNING_NOVA, Target_Self, 0, 0, 0);
        mBurningNova->AddEmote("Closer! Come closer... and burn!", Text_Yell);
        mVortex = AddSpell(KELIDAN_FIRE_NOVA, Target_Self, 0, 0, 0);
        AddSpell(KELIDAN_CORRUPTION, Target_Current, 15, 0, 10);

        mBurningNovaTimer = INVALIDATE_TIMER;
        SetAIUpdateFreq(800);
    }

    void OnCombatStart(Unit* pTarget)
    {
        _unit->SendScriptTextChatMessage(4841);     // Who dares interrupt--What is this; what have you done? You'll ruin everything!

        mBurningNovaTimer = AddTimer(15000);
        ParentClass::OnCombatStart(pTarget);
    }

    void AIUpdate()
    {
        if (!IsCasting())
        {
            if (mBurningNovaTimer == INVALIDATE_TIMER || IsTimerFinished(mBurningNovaTimer))
            {
                if (IsHeroic())
                    CastSpell(mVortex);
                CastSpell(mBurningNova);

                ResetTimer(mBurningNovaTimer, 30000);

                ParentClass::AIUpdate();
            }
        }

        ParentClass::AIUpdate();
    }

    void OnTargetDied(Unit* pTarget)
    {
        switch (RandomUInt(1))
        {
            case 0:
                _unit->SendScriptTextChatMessage(4845);     // Just as you deserve!
                break;
            case 1:
                _unit->SendScriptTextChatMessage(4846);     // Your friends will soon be joining you!
                break;
        }
    }

    void OnDied(Unit* pTarget)
    {
        _unit->SendScriptTextChatMessage(4848);     // Good...luck. You'll need it.
    }

    SpellDesc* mShadowBoltVolley;
    SpellDesc* mFireNova;
    SpellDesc* mBurningNova;
    SpellDesc* mVortex;
    int32 mBurningNovaTimer;
};


// Broggok
class BroggokAI : public MoonScriptCreatureAI
{
    public:

        ADD_CREATURE_FACTORY_FUNCTION(BroggokAI);
        BroggokAI(Creature* pCreature) : MoonScriptCreatureAI(pCreature)
        {
            AddSpell(POISON_BOLT, Target_Self, 12.0f, 0, 15);
            AddSpell(POISON_CLOUD, Target_RandomPlayerDestination, 8.0f, 0, 40, 0, 40);
            AddSpell(SLIME_SPRAY, Target_Self, 10.0f, 0, 25);
        }

        void OnDied(Unit* pKiller)
        {
            GameObject* pDoor = NULL;
            pDoor = _unit->GetMapMgr()->GetInterface()->GetGameObjectNearestCoords(456.157349f, 34.248005f, 9.559463f, GO_BROGGOK);
            if (pDoor)
                pDoor->SetState(GO_STATE_OPEN);

            MoonScriptCreatureAI::OnDied(pKiller);
        }
};


// The Maker
class TheMakerAI : public MoonScriptCreatureAI
{
    public:

        ADD_CREATURE_FACTORY_FUNCTION(TheMakerAI);
        TheMakerAI(Creature* pCreature) : MoonScriptCreatureAI(pCreature)
        {
            AddSpell(DOMINATION, Target_RandomPlayer, 8.0f, 0, 30);
            AddSpell(ACID_SPRAY, Target_Self, 10.0f, 0, 20);
            AddSpell(THROW_BEAKER, Target_RandomPlayerDestination, 20.0f, 0, 0, 0, 40);
        }

        void OnCombatStart(Unit* pTarget)
        {
            switch (RandomUInt(2))
            {
                case 0:
                    _unit->SendScriptTextChatMessage(4849);     // My work must not be interrupted!
                    break;
                case 1:
                    _unit->SendScriptTextChatMessage(4850);     // Perhaps I can find a use for you...
                    break;
                case 2:
                    _unit->SendScriptTextChatMessage(4851);     // Anger...hate... These are tools I can use.
                    break;
            }
        }

        void OnTargetDied(Unit* pTarget)
        {
            switch (RandomUInt(1))
            {
                case 0:
                    _unit->SendScriptTextChatMessage(4852);     // Let's see what I can make of you!
                    break;
                case 1:
                    _unit->SendScriptTextChatMessage(4853);     // It is pointless to resist.
                    break;
            }
        }

        void OnDied(Unit* pKiller)
        {
            _unit->SendScriptTextChatMessage(4854);     // Stay away from... Me!

            GameObject* pDoor = NULL;
            pDoor = _unit->GetMapMgr()->GetInterface()->GetGameObjectNearestCoords(327.155487f, 149.753418f, 9.559869f, GO_THE_MAKER);
            if (pDoor)
                pDoor->SetState(GO_STATE_OPEN);

            MoonScriptCreatureAI::OnDied(pKiller);
        }
};

void SetupBloodFurnace(ScriptMgr* mgr)
{
    //Instance
    mgr->register_instance_script(MAP_HC_BLOOD_FURNANCE, &InstanceTheBloodFurnaceScript::Create);

    mgr->register_creature_script(CN_KELIDAN_THE_BREAKER, &KelidanTheBreakerAI::Create);
    mgr->register_creature_script(CN_BROGGOK, &BroggokAI::Create);
    mgr->register_creature_script(CN_THE_MAKER, &TheMakerAI::Create);
}
