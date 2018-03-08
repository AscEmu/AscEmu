/**
 * AscEmu Framework based on ArcEmu MMORPG Server
 * Copyright (c) 2014-2018 AscEmu Team <http://www.ascemu.org>
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

class CurgleCranklehop_Gossip : public Arcemu::Gossip::Script
{
    public:

        void OnHello(Object* pObject, Player* plr) override
        {
            Arcemu::Gossip::Menu menu(pObject->getGuid(), 1519);
            menu.AddItem(GOSSIP_ICON_CHAT, plr->GetSession()->LocalizedGossipOption(GI_TANARIS_CRANK_HIPPO), 1);
            menu.AddItem(GOSSIP_ICON_CHAT, plr->GetSession()->LocalizedGossipOption(GI_TANARIS_CRANK_GORDUNNI), 2);
            menu.Send(plr);
        }

        void OnSelectOption(Object* pObject, Player* plr, uint32 Id, const char* /*Code*/, uint32 /*gossipId*/) override
        {
            Arcemu::Gossip::Menu menu(pObject->getGuid(), 0);
            if (1 == Id)
                menu.setTextID(1521);
            else
                menu.setTextID(1646);
            menu.Send(plr);
        }

        void Destroy() override { delete this; }

};

class TrentonLighthammer_Gossip : public Arcemu::Gossip::Script
{
    public:

        void OnHello(Object* pObject, Player* plr) override
        {
            Arcemu::Gossip::Menu::SendQuickMenu(pObject->getGuid(), 1758, plr, 1, GOSSIP_ICON_CHAT, plr->GetSession()->LocalizedGossipOption(GI_TANARIS_TELL_TRENTON));
        }

        void OnSelectOption(Object* pObject, Player* plr, uint32 /*Id*/, const char* /*Code*/, uint32 /*gossipId*/) override
        {
            Arcemu::Gossip::Menu menu(pObject->getGuid(), 0);
            menu.setTextID(1759);
            menu.Send(plr);
        }

        void Destroy() override { delete this; }
};

void SetupTanarisGossip(ScriptMgr* mgr)
{
    mgr->register_creature_gossip(7763, new CurgleCranklehop_Gossip);        // Curgle Cranklehop
    mgr->register_creature_gossip(7804, new TrentonLighthammer_Gossip);    // Trenton Lighthammer
}
