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
#include "Raid_Ulduar.h"

//////////////////////////////////////////////////////////////////////////////////////////
//Ulduar Instance
class UlduarScript : public MoonInstanceScript
{
    public:

        MOONSCRIPT_INSTANCE_FACTORY_FUNCTION(UlduarScript, MoonInstanceScript);
        UlduarScript(MapMgr* pMapMgr) : MoonInstanceScript(pMapMgr)
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

        }
};

//////////////////////////////////////////////////////////////////////////////////////////
//Ulduar Teleporter
class UlduarTeleporterAI : public GameObjectAIScript
{
    public:

        UlduarTeleporterAI(GameObject* go) : GameObjectAIScript(go)
        {
        }

        ~UlduarTeleporterAI() {}

        static GameObjectAIScript* Create(GameObject* go) { return new UlduarTeleporterAI(go); }

        void OnActivate(Player* player)
        {
            UlduarScript* pInstance = (UlduarScript*)player->GetMapMgr()->GetScript();
            if (!pInstance)
                return;

            GossipMenu* menu = NULL;
            objmgr.CreateGossipMenuForPlayer(&menu, _gameobject->GetGUID(), 14424, player);
            menu->AddItem(ICON_CHAT, player->GetSession()->LocalizedGossipOption(521), 0);      // Expedition Base Camp.

            // Unlock after engaging Flame Leviathan
            menu->AddItem(ICON_CHAT, player->GetSession()->LocalizedGossipOption(522), 1);      // Formation Grounds

            if (pInstance->GetInstanceData(Data_EncounterState, CN_FLAME_LEVIATHAN) == State_Finished)
                menu->AddItem(ICON_CHAT, player->GetSession()->LocalizedGossipOption(523), 2);      // Colossal Forge

            if (pInstance->GetInstanceData(Data_EncounterState, CN_XT_002_DECONSTRUCTOR) == State_Finished)
            {
                menu->AddItem(ICON_CHAT, player->GetSession()->LocalizedGossipOption(524), 3);      // Scrapyard
                menu->AddItem(ICON_CHAT, player->GetSession()->LocalizedGossipOption(525), 4);      // Antechamber of Ulduar
            }

            if (pInstance->GetInstanceData(Data_EncounterState, CN_KOLOGARN) == State_Finished)
                menu->AddItem(ICON_CHAT, player->GetSession()->LocalizedGossipOption(526), 5);      // Shattered Walkway

            if (pInstance->GetInstanceData(Data_EncounterState, CN_AURIAYA) == State_Finished)
                menu->AddItem(ICON_CHAT, player->GetSession()->LocalizedGossipOption(527), 6);      // Conservatory of Life

            if (pInstance->GetInstanceData(Data_EncounterState, CN_MIMIRON) == State_Finished)
                menu->AddItem(ICON_CHAT, player->GetSession()->LocalizedGossipOption(528), 7);      // Spark of Imagination

            if (pInstance->GetInstanceData(Data_EncounterState, CN_GENERAL_VEZAX) == State_Finished)
                menu->AddItem(ICON_CHAT, player->GetSession()->LocalizedGossipOption(529), 8);      // Prison of Yogg-Saron

            menu->SendTo(player);
        }
};

class UlduarTeleporterGossip : public GossipScript
{
    public:

        UlduarTeleporterGossip() : GossipScript(){}

        void OnSelectOption(Object* object, Player* player, uint32 Id, const char* enteredcode)
        {
            Arcemu::Gossip::Menu::Complete(player);

            if (Id >= 9)
                return;
            
            switch (Id)
            {
                case 0:
                    player->CastSpell(player, 64014, true);     // Expedition Base Camp
                    break;
                case 1:
                    player->CastSpell(player, 64032, true);     // Formation Grounds
                    break;
                case 2:
                    player->CastSpell(player, 64028, true);     // Colossal Forge
                    break;
                case 3:
                    player->CastSpell(player, 64031, true);     // Scrapyard
                    break;
                case 4:
                    player->CastSpell(player, 64030, true);     // Antechamber of Ulduar
                    break;
                case 5:
                    player->CastSpell(player, 64029, true);     // Shattered Walkway
                    break;
                case 6:
                    player->CastSpell(player, 64024, true);     // Conservatory of Life
                    break;
                case 7:
                    player->CastSpell(player, 64025, true);     // Spark of Imagination
                    break;
                case 8:
                    player->CastSpell(player, 65042, true);     // Prison of Yogg-Saron
                    break;
            }
        }
};

void SetupUlduar(ScriptMgr* mgr)
{
    //Instance
    mgr->register_instance_script(MAP_ULDUAR, &UlduarScript::Create);

    //Teleporter
    mgr->register_gameobject_script(194569, &UlduarTeleporterAI::Create);
    mgr->register_go_gossip_script(194569, new UlduarTeleporterGossip());
};
