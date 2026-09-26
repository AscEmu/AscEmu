/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include "Management/Gossip/GossipDefines.hpp"
#include "Storage/MySQLDataStore.hpp"
#include "version/Forever/World/BroadcastTextId.hpp"
#include "WoWGuid.hpp"

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
        uint16_t mapId;

        std::map<uint32_t, GossipItem> gossipItemList;
        std::map<uint32_t, GossipQuestItem> gossipQuestList;

        SmsgGossipMessage() : SmsgGossipMessage(0, 0, 0, 0, {}, {}, 0)
        {
        }

        SmsgGossipMessage(uint64_t guid, uint32_t id, uint32_t textId, uint32_t locale, std::map<uint32_t, GossipItem> gossipItemList, std::map<uint32_t, GossipQuestItem> gossipQuestList, uint16_t mapId = 0) :
            ManagedPacket(SMSG_GOSSIP_MESSAGE, 0),
            guid(guid),
            id(id),
            textId(textId),
            locale(locale),
            mapId(mapId),
            gossipItemList(std::move(gossipItemList)),
            gossipQuestList(std::move(gossipQuestList))
        {
        }

    protected:
        size_t expectedSize() const override
        {
            return 170; //guessed
        }

        bool internalSerialise(WorldPacket& packet) override
        {
            if (m_protocol.isForever())
            {
                // Forever 1.60.1.70009 follows the modern GossipMessage layout:
                // GUID, GossipID, LfgDungeonsID, FriendshipFactionID,
                // GossipOptions[], GossipText[] (quest entries), then the two
                // optional text references RandomTextID/BroadcastTextID.
                const WoWGuid modernGuid = WoWGuid::createModernFromLegacy(
                    guid.getRawGuid(), m_protocol.realmId, mapId, 0);
                const auto packedGuid = modernGuid.packModern();
                packet.append(packedGuid.data(), packedGuid.size());

                packet << static_cast<int32_t>(id); // GossipID
                packet << static_cast<int32_t>(0);  // LfgDungeonsID
                packet << static_cast<int32_t>(0);  // FriendshipFactionID
                packet << static_cast<uint32_t>(gossipItemList.size());
                packet << static_cast<uint32_t>(gossipQuestList.size());

                int32_t orderIndex = 0;
                for (const auto& [optionId, item] : gossipItemList)
                {
                    std::string optionText;
                    if (!item.text.empty())
                        optionText = item.text;
                    else
                        optionText = sMySQLStore.getLocaleGossipMenuOptionOrElse(item.textId, locale);

                    // ClientGossipOptions
                    packet << static_cast<int32_t>(optionId);
                    packet << static_cast<uint32_t>(item.icon); // OptionNPC
                    packet << static_cast<int8_t>(0);           // OptionFlags
                    packet << static_cast<uint64_t>(item.boxMoney);
                    packet << static_cast<uint32_t>(0);         // OptionLanguage
                    packet << static_cast<uint32_t>(0);         // Treasure.Items count
                    packet << static_cast<int32_t>(0);          // GossipOptionFlags
                    packet << orderIndex++;

                    packet.writeBits(static_cast<uint32_t>(optionText.size()), 12);
                    packet.writeBits(static_cast<uint32_t>(item.boxMessage.size()), 12);
                    packet.writeBits(0U, 2);                    // GossipOptionStatus
                    packet.writeBit(false);                     // SpellID absent
                    packet.writeBit(false);                     // OverrideIconID absent
                    packet.writeBits(0U, 8);                    // FailureDescription length
                    packet.flushBits();

                    packet.writeString(optionText);
                    packet.writeString(item.boxMessage);
                }

                for (const auto& [questId, quest] : gossipQuestList)
                {
                    const std::string title = sMySQLStore.getLocaleGossipTitleOrElse(questId, locale);

                    packet << static_cast<int32_t>(questId);
                    packet << static_cast<int32_t>(0);           // ContentTuningID
                    packet << static_cast<int32_t>(quest.icon);  // QuestType
                    packet << static_cast<int32_t>(0);           // QuestInfoID
                    packet << static_cast<int32_t>(quest.flags); // QuestFlags[0]
                    packet << static_cast<int32_t>(0);           // QuestFlags[1]
                    packet << static_cast<int32_t>(0);           // QuestFlags[2]
                    packet << static_cast<int32_t>(0);           // QuestFlags[3]

                    packet.writeBit(quest.repeatable != 0);
                    packet.writeBit(false);                      // ResetByScheduler
                    packet.writeBit(false);                      // Important
                    packet.writeBit(false);                      // Meta
                    packet.writeBits(static_cast<uint32_t>(title.size()), 9);
                    packet.flushBits();
                    packet.writeString(title);
                }

                // Forever uses a 32-bit BroadcastText reference. AscEmu reserves
                // the upper 6 bits as a source namespace and keeps the lower
                // 26 bits as the original source id. Native Blizzard ids use
                // namespace 0 and are sent unchanged. Normal AscEmu gossip
                // text ids use the Gossip namespace.
                uint32_t foreverRandomTextId = 0;

                if (textId != 0)
                {
                    using namespace AscEmu::Version::Forever;

                    if (BroadcastTextId::isValidSourceId(textId))
                    {
                        foreverRandomTextId = BroadcastTextId::encode(
                            BroadcastTextId::Type::Gossip, textId);
                    }
                }

                packet.writeBit(foreverRandomTextId != 0); // RandomTextID present
                packet.writeBit(false);                    // BroadcastTextID absent
                packet.flushBits();

                if (foreverRandomTextId != 0)
                    packet << static_cast<int32_t>(foreverRandomTextId);

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
