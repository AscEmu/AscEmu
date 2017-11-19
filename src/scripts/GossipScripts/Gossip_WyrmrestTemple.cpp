/**
 * AscEmu Framework based on ArcEmu MMORPG Server
 * Copyright (C) 2014-2017 AscEmu Team <http://www.ascemu.org>
 * Copyright (C) 2008-2009 FrozenThrone Shard
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
#include "Management/TaxiMgr.h"
#include "Objects/ObjectMgr.h"

enum CreatureEntry
{
    CN_TORASTRASZA = 26949,
    CN_AFRASASTRASZ = 27575,
    CN_TARIOLSTRASZ = 26443
};

enum eGossipText
{
    GT_TORASTRASZA = 12714,
    GT_AFRASASTRASZ = 12887,
    GT_TARIOLSTRASZ = 12713
};

enum eGossipItems
{
    GI_WYMREST_TOP_TO_BOTTOM = 413,  // "Yes, please, I would like to return to the ground level of the temple."
    GI_WYMREST_TOP_TO_MIDDLE = 414,  // "I would like to go Lord Afrasastrasz in the middle of the temple."
    GI_WYMREST_MIDDLE_TO_TOP = 415,  // "My lord, I need to get to the top of the temple."
    GI_WYMREST_MIDDLE_TO_BOTTOM = 416,  // "Can I get a ride back to ground level, Lord Afrasastrasz?"
    GI_WYMREST_BOTTOM_TO_TOP = 417,  // "Steward, please allow me to ride one of the drakes to the queen's chamber at the top of the temple."
    GI_WYMREST_BOTTOM_TO_MIDDLE = 418   // "Can you spare a drake to take me to Lord Afrasastrasz in the middle of the temple?"
};

class WyrmrestTemple_Gossip : public Arcemu::Gossip::Script
{
public:

    void OnHello(Object* pObject, Player* plr) override
    {
        switch (pObject->GetEntry())
        {
            case CN_TORASTRASZA:
            {
                Arcemu::Gossip::Menu menu(pObject->GetGUID(), GT_TORASTRASZA, 0);
                menu.AddItem(GOSSIP_ICON_CHAT, plr->GetSession()->LocalizedGossipOption(GI_WYMREST_TOP_TO_BOTTOM), 1);
                menu.AddItem(GOSSIP_ICON_CHAT, plr->GetSession()->LocalizedGossipOption(GI_WYMREST_TOP_TO_MIDDLE), 2);
                menu.Send(plr);
            }
            break;
            case CN_AFRASASTRASZ:
            {
                Arcemu::Gossip::Menu menu(pObject->GetGUID(), GT_AFRASASTRASZ, 0);
                menu.AddItem(GOSSIP_ICON_CHAT, plr->GetSession()->LocalizedGossipOption(GI_WYMREST_MIDDLE_TO_TOP), 3);
                menu.AddItem(GOSSIP_ICON_CHAT, plr->GetSession()->LocalizedGossipOption(GI_WYMREST_MIDDLE_TO_BOTTOM), 4);
                menu.Send(plr);
            }
            break;
            case CN_TARIOLSTRASZ:
            {
                Arcemu::Gossip::Menu menu(pObject->GetGUID(), GT_TARIOLSTRASZ, 0);
                menu.AddItem(GOSSIP_ICON_CHAT, plr->GetSession()->LocalizedGossipOption(GI_WYMREST_BOTTOM_TO_TOP), 5);
                menu.AddItem(GOSSIP_ICON_CHAT, plr->GetSession()->LocalizedGossipOption(GI_WYMREST_BOTTOM_TO_MIDDLE), 6);
                menu.Send(plr);
            }
            break;
        }

    };

    void OnSelectOption(Object* /*pObject*/, Player* plr, uint32 Id, const char* /*Code*/, uint32 /*gossipId*/) override
    {
        switch (Id)
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
        Arcemu::Gossip::Menu::Complete(plr);
    };
};


void SetupWyrmrestTempleGossip(ScriptMgr* mgr)
{
    Arcemu::Gossip::Script* gs = new WyrmrestTemple_Gossip();
    mgr->register_creature_gossip(CN_TORASTRASZA, gs);
    mgr->register_creature_gossip(CN_AFRASASTRASZ, gs);
    mgr->register_creature_gossip(CN_TARIOLSTRASZ, gs);
}
