/**
 * AscEmu Framework based on ArcEmu MMORPG Server
 * Copyright (c) 2014-2018 AscEmu Team <http://www.ascemu.org>
 * Copyright (C) 2007-2015 Moon++ Team <http://www.moonplusplus.info/>
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
#include "Management/TaxiMgr.h"
#include "Server/WorldSession.h"
#include "Units/Creatures/Creature.h"
#include "Management/Gossip/Gossip.h"
#include "Server/Script/ScriptMgr.h"

class SilvaFilnaveth_Gossip : public Arcemu::Gossip::Script
{
    public:

        void OnHello(Object* pObject, Player* plr) override
        {
            Arcemu::Gossip::Menu menu(pObject->GetGUID(), 0);
            if (plr->getClass() == DRUID && plr->getRace() == RACE_NIGHTELF)
            {
                menu.setTextID(4914);
                menu.AddItem(GOSSIP_ICON_CHAT, plr->GetSession()->LocalizedGossipOption(GI_MG_FLY_VILLAGE), 1);
            }
            else if (plr->getClass() == DRUID && plr->getRace() == RACE_TAUREN)
                menu.setTextID(4915);
            else
                menu.setTextID(4913);

            menu.Send(plr);
        }

        void OnSelectOption(Object* pObject, Player* plr, uint32 /*Id*/, const char* /*Code*/, uint32 /*gossipId*/) override
        {
            Creature* pCreature = (pObject->IsCreature()) ? (static_cast<Creature*>(pObject)) : NULL;
            if (pCreature == NULL)
                return;
            plr->TaxiStart(sTaxiMgr.GetTaxiPath(315), 479, 0);     // Hippogryph
        }

        void Destroy() override { delete this; }

};

class BunthenPlainswind_Gossip : public Arcemu::Gossip::Script
{
    public:

        void OnHello(Object* pObject, Player* plr) override
        {
            Arcemu::Gossip::Menu menu(pObject->GetGUID(), 0);
            if (plr->getClass() == DRUID && plr->getRace() == RACE_TAUREN)
            {
                menu.setTextID(4918);
                menu.AddItem(GOSSIP_ICON_CHAT, plr->GetSession()->LocalizedGossipOption(GI_MG_FLY_THUNDER_BLUFF), 1);
            }
            else if (plr->getClass() == DRUID && plr->getRace() == RACE_NIGHTELF)
                menu.setTextID(4917);
            else
                menu.setTextID(4916);

            menu.Send(plr);
        }

        void OnSelectOption(Object* pObject, Player* plr, uint32 /*Id*/, const char* /*Code*/, uint32 /*gossipId*/) override
        {
            Creature* pCreature = (pObject->IsCreature()) ? (static_cast<Creature*>(pObject)) : NULL;
            if (pCreature == NULL)
                return;
            plr->TaxiStart(sTaxiMgr.GetTaxiPath(316), 295, 0);     // Wyvern
        }

        void Destroy() { delete this; }

};

void SetupMoongladeGossip(ScriptMgr* mgr)
{
    mgr->register_creature_gossip(11800, new SilvaFilnaveth_Gossip); // Silva Fil'naveth
    mgr->register_creature_gossip(11798, new BunthenPlainswind_Gossip); // Bunthen Plainswind

}
