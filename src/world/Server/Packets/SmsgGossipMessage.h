/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once
#include <cstdint>

#include "ManagedPacket.h"
#include "Management/Gossip/GossipDefines.hpp"
#include "Storage/MySQLDataStore.hpp"

namespace AscEmu::Packets
{
    class SmsgGossipMessage : public ManagedPacket
    {
    public:
        uint64_t guid;
        uint32_t id;
        uint32_t textId;
        uint32_t locale;

        std::map<uint32_t, GossipItem> gossipItemList;
        std::map<uint32_t, GossipQuestItem> gossipQuestList;

        SmsgGossipMessage() : SmsgGossipMessage(0, 0, 0, 0, {}, {})
        {
        }

        SmsgGossipMessage(uint64_t guid, uint32_t id, uint32_t textId, uint32_t locale, std::map<uint32_t, GossipItem> gossipItemList, std::map<uint32_t, GossipQuestItem> gossipQuestList) :
            ManagedPacket(SMSG_GOSSIP_MESSAGE, 0),
            guid(guid),
            id(id),
            textId(textId),
            locale(locale),
            gossipItemList(gossipItemList),
            gossipQuestList(gossipQuestList)
        {
        }

    protected:
        size_t expectedSize() const override
        {
            return 512; //guessed
        }

        bool internalSerialise(WorldPacket& packet) override
        {
            packet << guid << id << textId;

            packet << uint32_t(gossipItemList.size());
            for (const auto& itemListItem : gossipItemList)
            {
                packet << uint32_t(itemListItem.first);
                packet << itemListItem.second.icon;
                packet << itemListItem.second.isCoded;
                packet << itemListItem.second.boxMoney;

                if (!itemListItem.second.text.empty())
                    packet << itemListItem.second.text;
                else
                    packet << sMySQLStore.getLocaleGossipMenuOptionOrElse(itemListItem.second.textId, locale);

                packet << itemListItem.second.boxMessage;
            }

            packet << uint32_t(gossipQuestList.size());
            for (const auto& questListItem : gossipQuestList)
            {
                packet << questListItem.first << uint32_t(questListItem.second.icon);
#if VERSION_STRING < WotLK
                switch (questListItem.second.icon)
                {
                    case QuestStatus::NotFinished:
                    case QuestStatus::AvailableChat:
                        packet << uint32_t(0);
                        break;
                    case QuestStatus::Finished:
                        packet << uint32_t(1);
                        break;
                    default:
                        packet << uint32_t(0);
                        break;
                }
#else
                packet << questListItem.second.level << questListItem.second.flags << uint8_t(0);
#endif
                packet << sMySQLStore.getLocaleGossipTitleOrElse(questListItem.first, locale);
            }

            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
