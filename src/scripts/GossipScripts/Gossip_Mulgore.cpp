/**
 * AscEmu Framework based on ArcEmu MMORPG Server
 * Copyright (C) 2014-2017 AscEmu Team <http://www.ascemu.org>
 * Copyright (C) 2007-2015 Moon++ Team <http://www.moonplusplus.info/>
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
#include "Server/WorldSession.h"
#include "Management/Gossip/Gossip.h"
#include "Units/Players/Player.h"
#include "Server/Script/ScriptMgr.h"

class SkornWhitecloud_Gossip : public Arcemu::Gossip::Script
{
    public:
        void OnHello(Object* pObject, Player* plr) override
        {
            Arcemu::Gossip::Menu::SendQuickMenu(pObject->GetGUID(), 522, plr, 1, GOSSIP_ICON_CHAT, plr->GetSession()->LocalizedGossipOption(GI_MULGORE_STORY_SKORN));
        }

        void OnSelectOption(Object* pObject, Player* plr, uint32 /*Id*/, const char* /*Code*/, uint32 /*gossipId*/) override
        {
            if(!pObject->IsCreature())
                return;
            Arcemu::Gossip::Menu::SendSimpleMenu(pObject->GetGUID(), 523, plr);
        }

        void Destroy() override { delete this; }
};

void SetupMulgoreGossip(ScriptMgr* mgr)
{
    mgr->register_creature_gossip(3052, new SkornWhitecloud_Gossip); // Skorn Whitecloud
}
