/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "GossipMenu.hpp"
#include "Management/QuestDefines.hpp"
#include "Management/QuestProperties.hpp"
#include "Objects/Units/Players/Player.hpp"
#include "Server/WorldSession.h"
#include "Server/Packets/SmsgGossipMessage.h"
#include "Server/Packets/SmsgGossipComplete.h"

using namespace AscEmu::Packets;

GossipMenu::GossipMenu(uint64_t senderGuid, uint32_t textId, uint32_t sessionLanguage, uint32_t gossipId) : 
m_textId(textId), m_sessionLanguage(sessionLanguage), m_gossipId(gossipId), m_senderGuid(senderGuid)
{}

void GossipMenu::addItem(uint8_t icon, uint32_t textId, uint32_t id, std::string text /*= ""*/, uint32_t boxMoney /*= 0*/, std::string boxMessage /*= ""*/, bool isCoded /*= false*/)
{
    const GossipItem item(icon, text, textId, isCoded, boxMoney, boxMessage);
    this->_gossipItemMap.insert(std::make_pair(id, item));
}

void GossipMenu::removeItem(uint32_t id)
{
    const auto itr = _gossipItemMap.find(id);
    if (itr != _gossipItemMap.end())
        _gossipItemMap.erase(itr);
}

void GossipMenu::addQuest(QuestProperties const* questProperties, uint8_t icon)
{
    const auto isRepeatable = questProperties->is_repeatable > 0 && !questProperties->HasFlag(QUEST_FLAGS_DAILY) && !questProperties->HasFlag(QUEST_FLAGS_WEEKLY);
    const GossipQuestItem questItem(icon, questProperties->questlevel, questProperties->quest_flags, isRepeatable);
    this->_gossipQuestMap.insert(std::make_pair(questProperties->id, questItem));
}

void GossipMenu::removeQuest(uint32_t questId)
{
    const auto itr = _gossipQuestMap.find(questId);
    if (itr != _gossipQuestMap.end())
        _gossipQuestMap.erase(itr);
}

void GossipMenu::sendGossipPacket(Player* player) const
{
    player->getSession()->SendPacket(SmsgGossipMessage(m_senderGuid, m_gossipId, m_textId, m_sessionLanguage, _gossipItemMap, _gossipQuestMap).serialise().get());
}

void GossipMenu::sendSimpleMenu(uint64_t guid, uint32_t textId, Player* player)
{
    player->getSession()->SendPacket(SmsgGossipMessage(guid, 0, textId, 0, {}, {}).serialise().get());
}

void GossipMenu::sendQuickMenu(uint64_t guid, uint32_t textId, Player* player, uint32_t itemId, uint8_t itemIcon, std::string itemText, uint32_t requiredMoney/*=0*/, std::string moneyText/*=""*/, bool extra/*=false*/)
{
    std::map<uint32_t, GossipItem> tempItemList;
    const GossipItem tempItem(itemIcon, itemText, 0, extra, requiredMoney, moneyText);
    tempItemList.insert(std::make_pair(itemId, tempItem));

    player->getSession()->SendPacket(SmsgGossipMessage(guid, 0, textId, 0, tempItemList, {}).serialise().get());
}

void GossipMenu::senGossipComplete(Player* player)
{
    player->sendPacket(SmsgGossipComplete().serialise().get());
}
