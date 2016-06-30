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
#include <Management/QuestLogEntry.hpp>

class CassaCrimsonwing_Gossip : public Arcemu::Gossip::Script
{
    public:

        void OnHello(Object* pObject, Player* plr)
        {
            GossipMenu* Menu;
            objmgr.CreateGossipMenuForPlayer(&Menu, pObject->GetGUID(), 11224, plr);

            Arcemu::Gossip::Menu menu(pObject->GetGUID(), 11224);
            if (plr->HasQuest(11142))
                menu.AddItem(GOSSIP_ICON_CHAT, plr->GetSession()->LocalizedGossipOption(GI_THERAMORE_CROMSONWING), 1);

            menu.Send(plr);
        }

        void OnSelectOption(Object* pObject, Player* plr, uint32 Id, const char* Code)
        {
            auto quest_entry = plr->GetQuestLogForEntry(11142);
            if (quest_entry == nullptr)
                return;
            quest_entry->SendQuestComplete();

            plr->TaxiStart(sTaxiMgr.GetTaxiPath(724), 1147, 0);     // Gryph
        }

        void Destroy() { delete this; }

};

class CaptainGarranVimes_Gossip : public Arcemu::Gossip::Script
{
    public:

        void OnHello(Object* pObject, Player* plr)
        {
            //Send quests and gossip menu.
            uint32 Text = objmgr.GetGossipTextForNpc(pObject->GetEntry());
            if (sMySQLStore.GetNpcText(Text) == nullptr)
                Text = DefaultGossipTextId;

            Arcemu::Gossip::Menu menu(pObject->GetGUID(), Text, plr->GetSession()->language);
            sQuestMgr.FillQuestMenu(static_cast<Creature*>(pObject), plr, menu);
            if (plr->HasQuest(11123) || (plr->GetQuestRewardStatus(11123) == 0))
                menu.AddItem(GOSSIP_ICON_CHAT, plr->GetSession()->LocalizedGossipOption(GI_THERAMORE_SHADY_REST), 0);

            menu.Send(plr);
        }

        void OnSelectOption(Object* pObject, Player* plr, uint32 Id, const char* Code)
        {
            Arcemu::Gossip::Menu::SendSimpleMenu(pObject->GetGUID(), 1794, plr);
        }

        void Destroy() { delete this; }

};

void SetupTheramoreGossip(ScriptMgr* mgr)
{
    mgr->register_creature_gossip(23704, new CassaCrimsonwing_Gossip); // Cassa Crimsonwing
    mgr->register_creature_gossip(4944, new CaptainGarranVimes_Gossip); // Captain Garran Vimes
}
