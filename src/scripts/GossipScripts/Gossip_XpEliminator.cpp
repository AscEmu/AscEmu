/**
 * AscEmu Framework based on ArcEmu MMORPG Server
 * Copyright (c) 2014-2018 AscEmu Team <http://www.ascemu.org>
 * Copyright (C) 2008-2012 ArcEmu Team <http://www.ArcEmu.org/> <http://www.arcemu.org>
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

// XpEleminatorGossip
//  GossipScript subclass for turning on/off Player experience gain
class XpEliminatorGossip : public Arcemu::Gossip::Script
{
    public:

        void OnHello(Object* pObject, Player* plr) override
        {
            Arcemu::Gossip::Menu menu(pObject->GetGUID(), 14736);
            if (plr->CanGainXp())
                menu.AddItem(GOSSIP_ICON_CHAT, plr->GetSession()->LocalizedGossipOption(GI_DISABLE_XP_GAIN), 1, 100000, plr->GetSession()->LocalizedGossipOption(GI_BOXMSG_DISABLE_XP_GAIN));
            else
                menu.AddItem(GOSSIP_ICON_CHAT, plr->GetSession()->LocalizedGossipOption(GI_ENABLE_XP_GAIN), 1, 100000, plr->GetSession()->LocalizedGossipOption(GI_BOXMSG_ENABLE_XP_GAIN));

            menu.Send(plr);
        }

        void OnSelectOption(Object* /*pObject*/, Player* plr, uint32 /*Id*/, const char* /*Code*/, uint32 /*gossipId*/) override
        {
            // turning xp gains on/off costs 10g each time
            if (plr->HasGold(100000))
            {
                plr->ModGold(-100000);
                plr->ToggleXpGain();
            }
            Arcemu::Gossip::Menu::Complete(plr);
        }

        void Destroy() override { delete this; }

};

void SetupXpEliminatorGossip(ScriptMgr* mgr)
{
    mgr->register_creature_gossip(35364, new XpEliminatorGossip);    // Slahtzt the Horde NPC
    mgr->register_creature_gossip(35365, new XpEliminatorGossip);    // Besten the Alliance NPC
}
