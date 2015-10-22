/**
 * AscEmu Framework based on ArcEmu MMORPG Server
 * Copyright (C) 2014-2015 AscEmu Team <http://www.ascemu.org>
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

class CurgleCranklehop_Gossip : public Arcemu::Gossip::Script
{
    public:

        void OnHello(Object* pObject, Player* plr)
        {
            Arcemu::Gossip::Menu menu(pObject->GetGUID(), 1519);
            menu.AddItem(ICON_CHAT, plr->GetSession()->LocalizedGossipOption(GI_TANARIS_CRANK_HIPPO), 1);
            menu.AddItem(ICON_CHAT, plr->GetSession()->LocalizedGossipOption(GI_TANARIS_CRANK_GORDUNNI), 2);
            menu.Send(plr);
        }

        void OnSelectOption(Object* pObject, Player* plr, uint32 Id, const char* Code)
        {
            Arcemu::Gossip::Menu menu(pObject->GetGUID(), 0);
            if (1 == Id)
                menu.setTextID(1521);
            else
                menu.setTextID(1646);
            menu.Send(plr);
        }

        void Destroy() { delete this; }

};

class TrentonLighthammer_Gossip : public Arcemu::Gossip::Script
{
    public:

        void OnHello(Object* pObject, Player* plr)
        {
            Arcemu::Gossip::Menu::SendQuickMenu(pObject->GetGUID(), 1758, plr, 1, ICON_CHAT, plr->GetSession()->LocalizedGossipOption(GI_TANARIS_TELL_TRENTON));
        }

        void GossipSelectOption(Object* pObject, Player* plr, uint32 Id, const char* Code)
        {
            Arcemu::Gossip::Menu::SendSimpleMenu(pObject->GetGUID(), 1759, plr);
        }

        void Destroy() { delete this; }
};

void SetupTanarisGossip(ScriptMgr* mgr)
{
    mgr->register_creature_gossip(7763, new CurgleCranklehop_Gossip);        // Curgle Cranklehop
    mgr->register_creature_gossip(7804, new TrentonLighthammer_Gossip);    // Trenton Lighthammer
}
