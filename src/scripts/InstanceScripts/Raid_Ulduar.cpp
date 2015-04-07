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

static float UlduarTeleCoords[9][4] =
{
    { -706.122f, -92.6024f, 429.876f, 0.0f },
    { 131.248f, -35.3802f, 409.804f, 0.0f },
    { 553.233f, -12.3247f, 409.679f, 0.0f },
    { 926.292f, -11.4635f, 418.595f, -0.017452f },
    { 1498.05f, -24.3509f, 420.966f, 0.034906f },
    { 1859.65f, -24.9121f, 448.811f, 0.0f },
    { 2086.26f, -23.9948f, 421.316f, 0.0f },
    { 2518.16f, 2569.03f, 412.299f, 0.0f },
    { 1854.82f, -11.5608f, 334.175f, 0.0f }
};

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
            GossipMenu* menu = NULL;
            objmgr.CreateGossipMenuForPlayer(&menu, _gameobject->GetGUID(), 31024, player);

            menu->AddItem(ICON_CHAT, player->GetSession()->LocalizedGossipOption(521), 0);      // Expedition Base Camp.
            menu->AddItem(ICON_CHAT, player->GetSession()->LocalizedGossipOption(522), 1);      // Formation Grounds
            menu->AddItem(ICON_CHAT, player->GetSession()->LocalizedGossipOption(523), 2);      // Colossal Forge
            menu->AddItem(ICON_CHAT, player->GetSession()->LocalizedGossipOption(524), 3);      // Scrapyard
            menu->AddItem(ICON_CHAT, player->GetSession()->LocalizedGossipOption(525), 4);      // Antechamber of Ulduar
            menu->AddItem(ICON_CHAT, player->GetSession()->LocalizedGossipOption(526), 5);      // Shattered Walkway
            menu->AddItem(ICON_CHAT, player->GetSession()->LocalizedGossipOption(527), 6);      // Conservatory of Life
            menu->AddItem(ICON_CHAT, player->GetSession()->LocalizedGossipOption(528), 7);      // Spark of Imagination
            menu->AddItem(ICON_CHAT, player->GetSession()->LocalizedGossipOption(529), 8);      // Prison of Yogg-Saron

            menu->SendTo(player);
        }
};

class UlduarTeleporterGossip : public GossipScript
{
    public:

        UlduarTeleporterGossip() : GossipScript()
        {
        }

        void OnSelectOption(Object* object, Player* player, uint32 Id, const char* enteredcode)
        {
            Arcemu::Gossip::Menu::Complete(player);

            if (Id >= 9)
                return;
            else
                player->SafeTeleport(603, player->GetInstanceID(), UlduarTeleCoords[Id][0], UlduarTeleCoords[Id][1],  UlduarTeleCoords[Id][2], UlduarTeleCoords[Id][3]);
        }
};

void SetupUlduar(ScriptMgr* mgr)
{
    mgr->register_gameobject_script(194569, &UlduarTeleporterAI::Create);
    mgr->register_go_gossip_script(194569, new UlduarTeleporterGossip());
};