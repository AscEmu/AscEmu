/**
 * AscEmu Framework based on ArcEmu MMORPG Server
 * Copyright (C) 2014-2015 AscEmu Team <http://www.ascemu.org>
 * Copyright (C) 2008-2009 FrozenThrone Shard <http://www.dowlee.it/ft>
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

//    Wyrmrest Temple Flighter
// Top to Bottom & Top to Middle
#define CN_TOP 26949
#define NPCTEXT_TOP 12714
// Middle to Top & Middle to Bottom
#define CN_MIDDLE 27575
#define NPCTEXT_MIDDLE 12887
// Bottom to Top & Bottom to Middle
#define CN_BOTTOM 26443
#define NPCTEXT_BOTTOM 12713

class SCRIPT_DECL WyrmrestTemple_FlightGossip : public Arcemu::Gossip::Script
{
    public:
        void OnHello(Object* pObject, Player* plr)
        {
            Arcemu::Gossip::Menu menu(pObject->GetGUID(), 0);
            switch(pObject->GetEntry())
            {
                case CN_TOP:
                    {
                        menu.setTextID(NPCTEXT_TOP);
                        menu.AddItem(ICON_CHAT, plr->GetSession()->LocalizedGossipOption(GI_WYMREST_TOP_TO_BOTTOM), 1);
                        menu.AddItem(ICON_CHAT, plr->GetSession()->LocalizedGossipOption(GI_WYMREST_TOP_TO_MIDDLE), 2);
                    }
                    break;
                case CN_MIDDLE:
                    {
                        menu.setTextID(NPCTEXT_MIDDLE);
                        menu.AddItem(ICON_CHAT, plr->GetSession()->LocalizedGossipOption(GI_WYMREST_MIDDLE_TO_TOP), 3);
                        menu.AddItem(ICON_CHAT, plr->GetSession()->LocalizedGossipOption(GI_WYMREST_MIDDLE_TO_BOTTOM), 4);
                    }
                    break;
                case CN_BOTTOM:
                    {
                        menu.setTextID(NPCTEXT_BOTTOM);
                        menu.AddItem(ICON_CHAT, plr->GetSession()->LocalizedGossipOption(GI_WYMREST_BOTTOM_TO_TOP), 5);
                        menu.AddItem(ICON_CHAT, plr->GetSession()->LocalizedGossipOption(GI_WYMREST_BOTTOM_TO_MIDDLE), 6);
                    }
                    break;
            }

            menu.Send(plr);
        }

        void OnSelectOption(Object* pObject, Player* plr, uint32 Id, const char* Code)
        {
            Arcemu::Gossip::Menu::Complete(plr);
            switch(Id)
            {
                case 1:
                    plr->TaxiStart(sTaxiMgr.GetTaxiPath(879), 6371, 0);     // Drake
                    break;
                case 2:
                    plr->TaxiStart(sTaxiMgr.GetTaxiPath(880), 6371, 0);     // Drake
                    break;
                case 3:
                    plr->TaxiStart(sTaxiMgr.GetTaxiPath(881), 6371, 0);     // Drake
                    break;
                case 4:
                    plr->TaxiStart(sTaxiMgr.GetTaxiPath(882), 6371, 0);     // Drake
                    break;
                case 5:
                    plr->TaxiStart(sTaxiMgr.GetTaxiPath(878), 6371, 0);     // Drake
                    break;
                case 6:
                    plr->TaxiStart(sTaxiMgr.GetTaxiPath(883), 6371, 0);     // Drake
                    break;
                default:
                    break;
            }
        }

};

void SetupWyrmrestTempleGossip(ScriptMgr* mgr)
{
    Arcemu::Gossip::Script* WyrmrestTempleFlightGossip = new WyrmrestTemple_FlightGossip;

    mgr->register_creature_gossip(CN_TOP, WyrmrestTempleFlightGossip);    // Torastrasza <Majordomo to the Ruling Council>
    mgr->register_creature_gossip(CN_MIDDLE, WyrmrestTempleFlightGossip);    // Lord Afrasastrasz <Commander of Wyrmrest Temple Defenses>
    mgr->register_creature_gossip(CN_BOTTOM, WyrmrestTempleFlightGossip);    // Tariolstrasz <Steward of Wyrmrest Temple>
}
