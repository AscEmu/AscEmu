/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include "Management/Gossip/GossipDefines.hpp"
#include "Storage/MySQLDataStore.hpp"

#include <cstdint>
#include <map>
#include <string>

namespace AscEmu::Packets
{
    class SmsgGossipMessage : public ManagedPacket
    {
    public:
        WoWGuid guid;
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
            return 170; //guessed
        }

        bool internalSerialise(WorldPacket& packet) override
        {
            if (m_protocol.isLegion())
            {
                // unit, gossip, friendship faction, text, both counts, then the options and the quests
                packet << guid.toGuid128(m_protocol.realmId, m_receiverMapId);
                packet << id;
                packet << uint32_t(0);
                packet << textId;
                packet << uint32_t(gossipItemList.size());
                packet << uint32_t(gossipQuestList.size());

                for (const auto& itemListItem : gossipItemList)
                {
                    std::string optionText;
                    if (!itemListItem.second.text.empty())
                        optionText = itemListItem.second.text;
                    else
                        optionText = sMySQLStore.getLocaleGossipMenuOptionOrElse(itemListItem.second.textId, locale);

                    packet << uint32_t(itemListItem.first);
                    packet << uint8_t(itemListItem.second.icon);
                    packet << uint8_t(itemListItem.second.isCoded ? 1 : 0);
                    packet << uint32_t(itemListItem.second.boxMoney);
                    packet.writeBits(static_cast<uint32_t>(optionText.length()), 12);
                    packet.writeBits(static_cast<uint32_t>(itemListItem.second.boxMessage.length()), 12);
                    packet.flushBits();
                    packet.writeString(optionText);
                    packet.writeString(itemListItem.second.boxMessage);
                }

                for (const auto& questListItem : gossipQuestList)
                {
                    const std::string questTitle = sMySQLStore.getLocaleGossipTitleOrElse(questListItem.first, locale);

                    packet << uint32_t(questListItem.first);
                    packet << uint32_t(questListItem.second.icon);
                    packet << int32_t(questListItem.second.level);
                    packet << int32_t(0);                               // max scaling level
                    packet << uint32_t(questListItem.second.flags);
                    packet << uint32_t(0);                              // flags 2
                    packet.writeBit(questListItem.second.repeatable != 0);
                    packet.writeBits(static_cast<uint32_t>(questTitle.length()), 9);
                    packet.flushBits();
                    packet.writeString(questTitle);
                }

                return true;
            }

            if (m_protocol.expansion <= WoW::Expansion::_Cata)
            {
                packet << guid.getRawGuid() << id << textId;

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
                    packet << questListItem.first << uint32_t(questListItem.second.icon) << questListItem.second.level;

                    if (m_protocol.expansion >= WoW::Expansion::_WotLK)
                    {
                        packet << questListItem.second.flags << questListItem.second.repeatable;
                    }

                    packet << sMySQLStore.getLocaleGossipTitleOrElse(questListItem.first, locale);
                }
            }
            else // Mop
            {
                packet.writeBits(static_cast<uint8_t>(gossipQuestList.size()), 19);

                for (const auto& questListItem : gossipQuestList)
                {
                    packet.writeBit(questListItem.second.repeatable);

                    std::string questTitle = sMySQLStore.getLocaleGossipTitleOrElse(questListItem.first, locale);
                    packet.writeBits(questTitle.size(), 9);
                }
        
                packet.writeBit(guid[5]);
                packet.writeBit(guid[7]);
                packet.writeBit(guid[4]);
                packet.writeBit(guid[0]);

                packet.writeBits(static_cast<uint32_t>(gossipItemList.size()), 20);

                packet.writeBit(guid[6]);
                packet.writeBit(guid[2]);

                for (const auto& itemListItem : gossipItemList)
                {
                    std::string optionText;
                    if (!itemListItem.second.text.empty())
                        optionText = itemListItem.second.text;
                    else
                        optionText = sMySQLStore.getLocaleGossipMenuOptionOrElse(itemListItem.second.textId, locale);

                    packet.writeBits(itemListItem.second.boxMessage.length(), 12);
                    packet.writeBits(optionText.length(), 12);
                }

                packet.writeBit(guid[3]);
                packet.writeBit(guid[1]);

                packet.flushBits();

                for (const auto& questListItem : gossipQuestList)
                {
                    std::string questTitle = sMySQLStore.getLocaleGossipTitleOrElse(questListItem.first, locale);

                    packet.writeString(questTitle);
                    packet << questListItem.second.flags;
                    packet << questListItem.second.level;
                    packet << uint32_t(questListItem.second.icon);
                    packet << questListItem.first;
                    packet << uint32_t(0); //flags2
                }

                packet.writeByteSeq(guid[1]);
                packet.writeByteSeq(guid[0]);

                for (const auto& itemListItem : gossipItemList)
                {
                    std::string optionText;
                    if (!itemListItem.second.text.empty())
                        optionText = itemListItem.second.text;
                    else
                        optionText = sMySQLStore.getLocaleGossipMenuOptionOrElse(itemListItem.second.textId, locale);

                    packet << itemListItem.second.boxMoney;
                    packet.writeString(itemListItem.second.boxMessage);
                    packet << uint32_t(itemListItem.first);
                    packet << uint8_t(itemListItem.second.isCoded);
                    packet.writeString(optionText);
                    packet << itemListItem.second.icon;
                }

                packet.writeByteSeq(guid[5]);
                packet.writeByteSeq(guid[3]);

                packet << id;

                packet.writeByteSeq(guid[2]);
                packet.writeByteSeq(guid[6]);
                packet.writeByteSeq(guid[4]);
                packet << uint32_t(0);       // ink faction?
                packet.writeByteSeq(guid[7]);
                packet << textId;
            }

            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
