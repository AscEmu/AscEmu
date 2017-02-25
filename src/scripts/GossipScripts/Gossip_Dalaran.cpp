/**
 * AscEmu Framework based on ArcEmu MMORPG Server
 * Copyright (C) 2014-2017 AscEmu Team <http://www.ascemu.org>
 * Copyright (C) 2009-2012 ArcEmu Team <http://www.arcemu.org/>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */
 
#include "Setup.h"
#include "Objects/GameObject.h"
#include "Server/Packets/Opcode.h"
#include "Server/WorldSession.h"

enum UnorderedEntrys
{
    GO_DEDICATION_OF_HONOR  = 202443,
    GT_DEDICATION_OF_HONOR  = 15921,    // "Dedicated to those that fell to the Scourge during the war in the frozen wastes."
    GI_SEE_FALL_LICH_KING   = 351       // "See the fall of the Lich King."

};

 class DedicationOfHonorAI : public GameObjectAIScript
{
    public:
        ADD_GAMEOBJECT_FACTORY_FUNCTION(DedicationOfHonorAI)
        DedicationOfHonorAI(GameObject* go) : GameObjectAIScript(go){}
        ~DedicationOfHonorAI() {}

        void OnActivate(Player* player)
        {
            Arcemu::Gossip::Menu::SendQuickMenu(_gameobject->GetGUID(), GT_DEDICATION_OF_HONOR, player, 1, GOSSIP_ICON_CHAT, player->GetSession()->LocalizedGossipOption(GI_SEE_FALL_LICH_KING));
        }
};

class DedicationOfHonorGossip : public GossipScript
{
    public:

        DedicationOfHonorGossip() : GossipScript(){}

        void OnSelectOption(Object* object, Player* player, uint32 Id, const char* enteredcode)
        {
#if VERSION_STRING > TBC
            uint32 video_id = 16;
            player->GetSession()->OutPacket(SMSG_TRIGGER_MOVIE, sizeof(uint32), &video_id);
            Arcemu::Gossip::Menu::Complete(player);
#endif
        }
};

void SetupDalaranGossip(ScriptMgr* mgr)
{
    mgr->register_gameobject_script(GO_DEDICATION_OF_HONOR, &DedicationOfHonorAI::Create);
    mgr->register_go_gossip_script(GO_DEDICATION_OF_HONOR, new DedicationOfHonorGossip);
}
