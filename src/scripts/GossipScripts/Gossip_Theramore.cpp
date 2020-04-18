/*
Copyright (c) 2014-2020 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "Management/QuestLogEntry.hpp"
#include "Management/TaxiMgr.h"
#include "Storage/MySQLDataStore.hpp"

class CassaCrimsonwing_Gossip : public GossipScript
{
public:

    void onHello(Object* pObject, Player* plr) override
    {
        GossipMenu menu(pObject->getGuid(), 11224);
        if (plr->HasQuest(11142))
            menu.addItem(GOSSIP_ICON_CHAT, GI_THERAMORE_CROMSONWING, 1);

        menu.sendGossipPacket(plr);
    }

    void onSelectOption(Object* /*pObject*/, Player* plr, uint32_t /*Id*/, const char* /*Code*/, uint32_t /*gossipId*/) override
    {
        auto quest_entry = plr->GetQuestLogForEntry(11142);
        if (quest_entry == nullptr)
            return;

        quest_entry->SendQuestComplete();

        plr->TaxiStart(sTaxiMgr.GetTaxiPath(724), 1147, 0);     // Gryph
        GossipMenu::senGossipComplete(plr);
    }

    void destroy() override { delete this; }

};

class CaptainGarranVimes_Gossip : public GossipScript
{
public:

    void onHello(Object* pObject, Player* plr) override
    {
        // Send quests and gossip menu.
        uint32_t Text = sMySQLStore.getGossipTextIdForNpc(pObject->getEntry());
        if (sMySQLStore.getNpcText(Text) == nullptr)
            Text = DefaultGossipTextId;

        GossipMenu menu(pObject->getGuid(), Text, plr->GetSession()->language);
        sQuestMgr.FillQuestMenu(static_cast<Creature*>(pObject), plr, menu);
        if (plr->HasQuest(11123) || plr->GetQuestRewardStatus(11123) == 0)
            menu.addItem(GOSSIP_ICON_CHAT, GI_THERAMORE_SHADY_REST, 0);

        menu.sendGossipPacket(plr);
    }

    void onSelectOption(Object* pObject, Player* plr, uint32_t /*Id*/, const char* /*Code*/, uint32_t /*gossipId*/) override
    {
        GossipMenu::sendSimpleMenu(pObject->getGuid(), 1794, plr);
    }

    void destroy() override { delete this; }

};

void SetupTheramoreGossip(ScriptMgr* mgr)
{
    mgr->register_creature_gossip(23704, new CassaCrimsonwing_Gossip);  // Cassa Crimsonwing
    mgr->register_creature_gossip(4944, new CaptainGarranVimes_Gossip); // Captain Garran Vimes
}
