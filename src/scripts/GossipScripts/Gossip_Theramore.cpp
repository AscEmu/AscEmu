/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "Management/QuestLogEntry.hpp"
#include "Management/TaxiMgr.h"
#include "Storage/MySQLDataStore.hpp"

class CassaCrimsonwing_Gossip : public Arcemu::Gossip::Script
{
public:

    void OnHello(Object* pObject, Player* plr) override
    {
        Arcemu::Gossip::Menu menu(pObject->getGuid(), 11224);
        if (plr->HasQuest(11142))
            menu.AddItem(GOSSIP_ICON_CHAT, plr->GetSession()->LocalizedGossipOption(GI_THERAMORE_CROMSONWING), 1);

        menu.Send(plr);
    }

    void OnSelectOption(Object* /*pObject*/, Player* plr, uint32 /*Id*/, const char* /*Code*/, uint32 /*gossipId*/) override
    {
        auto quest_entry = plr->GetQuestLogForEntry(11142);
        if (quest_entry == nullptr)
            return;

        quest_entry->SendQuestComplete();

        plr->TaxiStart(sTaxiMgr.GetTaxiPath(724), 1147, 0);     // Gryph
        Arcemu::Gossip::Menu::Complete(plr);
    }

    void Destroy() override { delete this; }

};

class CaptainGarranVimes_Gossip : public Arcemu::Gossip::Script
{
public:

    void OnHello(Object* pObject, Player* plr) override
    {
        // Send quests and gossip menu.
        uint32 Text = sMySQLStore.getGossipTextIdForNpc(pObject->getEntry());
        if (sMySQLStore.getNpcText(Text) == nullptr)
            Text = DefaultGossipTextId;

        Arcemu::Gossip::Menu menu(pObject->getGuid(), Text, plr->GetSession()->language);
        sQuestMgr.FillQuestMenu(static_cast<Creature*>(pObject), plr, menu);
        if (plr->HasQuest(11123) || (plr->GetQuestRewardStatus(11123) == 0))
            menu.AddItem(GOSSIP_ICON_CHAT, plr->GetSession()->LocalizedGossipOption(GI_THERAMORE_SHADY_REST), 0);

        menu.Send(plr);
    }

    void OnSelectOption(Object* pObject, Player* plr, uint32 /*Id*/, const char* /*Code*/, uint32 /*gossipId*/) override
    {
        Arcemu::Gossip::Menu::SendSimpleMenu(pObject->getGuid(), 1794, plr);
    }

    void Destroy() override { delete this; }

};

void SetupTheramoreGossip(ScriptMgr* mgr)
{
    mgr->register_creature_gossip(23704, new CassaCrimsonwing_Gossip);  // Cassa Crimsonwing
    mgr->register_creature_gossip(4944, new CaptainGarranVimes_Gossip); // Captain Garran Vimes
}
