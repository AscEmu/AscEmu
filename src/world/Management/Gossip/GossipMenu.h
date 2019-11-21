/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <cstdint>
#include <string>
#include "GossipDefines.hpp"
#include "Management/Quest.h"
#include "Units/Players/Player.h"

class SERVER_DECL GossipMenu
{
public:

    GossipMenu(uint64_t senderGuid, uint32_t textId, uint32_t sessionLanguage = 0, uint32_t gossipId = 0);

    //MIT starts
    void addItem(uint8_t icon, uint32_t textId, uint32_t id, std::string text = "", uint32_t boxMoney = 0, std::string boxMessage = "", bool isCoded = false);
    void removeItem(uint32_t id);

    void addQuest(QuestProperties const* questProperties, uint8_t icon);
    void removeQuest(uint32_t questId);

    uint32_t getTextID() const { return m_textId; }
    uint32_t getLanguage() const { return m_sessionLanguage; }

    void setTextID(uint32_t textid) { m_textId = textid; }
    void setLanguage(uint32_t language) { m_sessionLanguage = language; }

    void sendGossipPacket(Player* player) const;
    static void sendSimpleMenu(uint64_t guid, uint32_t textId, Player* player);
    static void sendQuickMenu(uint64_t guid, uint32_t textId, Player* player, uint32_t itemId, uint8_t itemIcon, std::string itemText, uint32_t requiredMoney = 0, std::string moneyText = "", bool extra = false);

    static void senGossipComplete(Player* player);

    uint32_t m_textId;
    uint32_t m_sessionLanguage;
    uint32_t m_gossipId;
    uint64_t m_senderGuid;

    std::map<uint32_t, GossipItem> _gossipItemMap;
    std::map<uint32_t, GossipQuestItem> _gossipQuestMap;
};
