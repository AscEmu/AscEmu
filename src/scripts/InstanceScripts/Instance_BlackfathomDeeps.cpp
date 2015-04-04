/*
 * ArcScripts for ArcEmu MMORPG Server
 * Copyright (C) 2008-2012 ArcEmu Team <http://www.ArcEmu.org/>
 * Copyright (C) 2008-2015 Sun++ Team <http://www.sunplusplus.info/>
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
#include "Instance_BlackfathomDeeps.h"

//////////////////////////////////////////////////////////////////////////////////////////
//Blackfathom Deeps
class InstanceBlackfathomDeepsScript : public MoonInstanceScript
{
    public:

        MOONSCRIPT_INSTANCE_FACTORY_FUNCTION(InstanceBlackfathomDeepsScript, MoonInstanceScript);
        InstanceBlackfathomDeepsScript(MapMgr* pMapMgr) : MoonInstanceScript(pMapMgr)
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

// LadySarevessAI
class LadySarevessAI : public MoonScriptCreatureAI
{
    public:
        MOONSCRIPT_FACTORY_FUNCTION(LadySarevessAI, MoonScriptCreatureAI);
        LadySarevessAI(Creature* pCreature) : MoonScriptCreatureAI(pCreature)
        {
            AddSpell(8435, Target_Current, 10, 2, 0);    // Forked Lightning
            AddSpell(865, Target_Self, 15, 0, 25);        // Frost Nova
            AddSpell(246, Target_Current, 15, 0, 10);    // Slow
        }

        void OnCombatStart(Unit* pTarget)
        {
            _unit->SendScriptTextChatMessage(7912);     // You should not be here! Slay them!
        }
};

// BaronAquanisAI
class BaronAquanisAI : public MoonScriptCreatureAI
{
    public:
        MOONSCRIPT_FACTORY_FUNCTION(BaronAquanisAI, MoonScriptCreatureAI);
        BaronAquanisAI(Creature* pCreature) : MoonScriptCreatureAI(pCreature)
        {
            // Frostbolt
            AddSpell(15043, Target_Current, 20, 3, 0);
            AggroNearestPlayer();
            SetDespawnWhenInactive(true);
        }
};

// FathomStone
class FathomStone : public GameObjectAIScript
{
    public:

        FathomStone(GameObject*  goinstance) : GameObjectAIScript(goinstance)
        {
            SpawnBaronAquanis = true;
        }

        static GameObjectAIScript* Create(GameObject*  GO) { return new FathomStone(GO); }

        void OnActivate(Player*  pPlayer)
        {
            if (pPlayer->IsTeamHorde() && SpawnBaronAquanis == true) // Horde
            {
                // Spawn Baron Aquanis
                _gameobject->GetMapMgr()->GetInterface()->SpawnCreature(CN_BARON_AQUANIS, -782.021f, -63.5876f, -45.0935f, -2.44346f, true, false, 0, 0);
                SpawnBaronAquanis = false;
            }
        }

    protected:

        bool SpawnBaronAquanis;
};

// KelrisAI
class KelrisAI : public MoonScriptCreatureAI
{
    public:
        MOONSCRIPT_FACTORY_FUNCTION(KelrisAI, MoonScriptCreatureAI);
        KelrisAI(Creature* pCreature) : MoonScriptCreatureAI(pCreature)
        {
            AddSpell(8399, Target_RandomPlayer, 12, 1.3f, 0, 0, 0, false, "Sleep...", Text_Yell, 5804);    // Sleep
            AddSpell(15587, Target_Current, 16, 1.5f, 0);    // Mind Blast
        }

        void OnCombatStart(Unit* pTarget)
        {
            _unit->SendScriptTextChatMessage(3966);     // Who dares disturb my meditation?
        }

        void OnTargetDied(Unit* pTarget)
        {
            _unit->SendScriptTextChatMessage(3968);     // Dust to dust.
        }
};

// AkumaiAI
class AkumaiAI : public MoonScriptCreatureAI
{
    public:
        MOONSCRIPT_FACTORY_FUNCTION(AkumaiAI, MoonScriptCreatureAI);
        AkumaiAI(Creature* pCreature) : MoonScriptCreatureAI(pCreature)
        {
            AddSpell(3490, Target_Self, 12, 0, 0);    // Frenzied Rage
            AddSpell(3815, Target_Self, 16, 0, 45);    // Poison Cloud
        }
};

// MorriduneGossip
class MorriduneGossip : public GossipScript
{
    public:

        void GossipHello(Object* pObject, Player* pPlayer)
        {
            GossipMenu* menu;
            objmgr.CreateGossipMenuForPlayer(&menu, pObject->GetGUID(), MORRIDUNE_ON_HELLO, pPlayer);

            if (pPlayer->IsTeamAlliance())
                menu->AddItem(ICON_CHAT, pPlayer->GetSession()->LocalizedGossipOption(MORRIDUNE_OPTION_1), 1);
            else
                menu->AddItem(ICON_CHAT, pPlayer->GetSession()->LocalizedGossipOption(MORRIDUNE_OPTION_2), 2);

            menu->SendTo(pPlayer);
        }

        void GossipSelectOption(Object* pObject, Player* pPlayer, uint32 Id, uint32 IntId, const char* Code)
        {
            switch (IntId)
            {
                case 0:
                    GossipHello(pObject, pPlayer);
                    break;
                case 1:
                    pPlayer->SafeTeleport(1, 0, 9951.52f, 2280.32f, 1341.39f, 0);
                    break;
                case 2:
                    pPlayer->SafeTeleport(1, 0, 4247.74f, 745.879f, -24.2967f, 4.36996f);
                    break;
            }
        }
};

void SetupBlackfathomDeeps(ScriptMgr* mgr)
{
    //Instance
    mgr->register_instance_script(MAP_BLACKFATHOM_DEEPS, &InstanceBlackfathomDeepsScript::Create);

    mgr->register_creature_script(CN_LADY_SAREVESS, &LadySarevessAI::Create);
    mgr->register_creature_script(CN_BARON_AQUANIS, &BaronAquanisAI::Create);
    mgr->register_creature_script(CN_LORD_KELRIS, &KelrisAI::Create);
    mgr->register_creature_script(CN_AKUMAI, &AkumaiAI::Create);

    mgr->register_creature_gossip(CN_MORRIDUNE, new MorriduneGossip);

    mgr->register_gameobject_script(GO_FATHOM_STONE, &FathomStone::Create);
}
