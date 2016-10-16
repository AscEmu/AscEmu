/**
 * AscEmu Framework based on ArcEmu MMORPG Server
 * Copyright (C) 2014-2016 AscEmu Team <http://www.ascemu.org>
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

class ArchmageMalin_Gossip : public Arcemu::Gossip::Script
{
    public:

        void OnHello(Object* pObject, Player* plr)
        {
            Arcemu::Gossip::Menu menu(pObject->GetGUID(), 11469);

            if (plr->HasQuest(11223))
                menu.AddItem(GOSSIP_ICON_CHAT, plr->GetSession()->LocalizedGossipOption(GI_SW_ARCHMAGE_JAINA), 1);

            menu.Send(plr);
        }

        void OnSelectOption(Object* pObject, Player* plr, uint32 Id, const char* Code)
        {
            static_cast<Creature*>(pObject)->CastSpell(plr, sSpellCustomizations.GetSpellInfo(42711), true);
            Arcemu::Gossip::Menu::Complete(plr);
        }

        void Destroy() { delete this; }
};


//This is when you talk to Thargold Ironwing...He will fly you through Stormwind Harbor to check it out.
class SWHarborFlyAround : public Arcemu::Gossip::Script
{
    public:

        void OnHello(Object* pObject, Player* Plr)
        {
            Arcemu::Gossip::Menu menu(pObject->GetGUID(), 13454);
            menu.AddItem(GOSSIP_ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(GI_SW_HARBOR_FLY_YES), 1);
            menu.AddItem(GOSSIP_ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(GI_SW_HARBOR_FLY_NO), 2);

            menu.Send(Plr);
        }
        void OnSelectOption(Object* pObject, Player* Plr, uint32 Id, const char* Code)
        {
            Arcemu::Gossip::Menu::Complete(Plr);
            if (1 == Id)
                Plr->TaxiStart(sTaxiMgr.GetTaxiPath(1041), 25679, 0);
        }

        void Destroy() { delete this; }
};

void SetupStormwindGossip(ScriptMgr* mgr)
{
    mgr->register_creature_gossip(2708, new ArchmageMalin_Gossip); // Archmage Malin
    mgr->register_creature_gossip(29154, new SWHarborFlyAround);
}
