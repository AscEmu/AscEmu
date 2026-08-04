/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include "Management/MailMgr.h"
#include <map>

namespace AscEmu::Packets
{
    class SmsgMailListResult : public ManagedPacket
    {
        MessageMap messageMap;

    public:
        SmsgMailListResult(MessageMap& messageMap) :
            ManagedPacket(SMSG_MAIL_LIST_RESULT, 200),
            messageMap(messageMap)
        {
        }

    protected:

        bool internalSerialise(WorldPacket& packet) override
        {
            if (m_protocol.expansion <= WoW::Expansion::_Cata)
            {
                uint32_t realCount = 0;
                uint8_t count = 0;

                if (m_protocol.expansion > WoW::Expansion::_TBC)
                    packet << uint32_t(0);

                packet << uint8_t(0);

                for (auto& message : messageMap)
                {
                    if (message.second.expire_time && static_cast<uint32_t>(UNIXTIME) > message.second.expire_time)
                        continue;

                    if (static_cast<uint32_t>(UNIXTIME) < message.second.delivery_time)
                        continue;

                    if (count >= 50)
                    {
                        ++realCount;
                        continue;
                    }

                    uint8_t guidSize;
                    if (message.second.message_type == 0)
                        guidSize = 8;
                    else
                        guidSize = 4;

                    size_t messageSize = 0;
                    if (m_protocol.expansion <= WoW::Expansion::_TBC)
                    {
                        messageSize = 2 + 4 + 1 + guidSize + 4 * 8 + (message.second.subject.size() + 1) + 1 + (
                            message.second.items.size() * (1 + 4 + 4 + MAX_INSPECTED_ENCHANTMENT_SLOT * 3 * 4 + 4 + 4 + 1 + 4 + 4 + 4));
                    }
                    else if (m_protocol.expansion <= WoW::Expansion::_WotLK)
                    {
                        messageSize = 2 + 4 + 1 + guidSize + 4 * 8 + (message.second.subject.size() + 1) + (message.second.body.size() + 1) + 1 + (
                            message.second.items.size() * (1 + 4 + 4 + MAX_INSPECTED_ENCHANTMENT_SLOT * 3 * 4 + 4 + 4 + 4 + 4 + 4 + 4 + 1));
                    }
                    else if (m_protocol.expansion == WoW::Expansion::_Cata)
                    {
                        messageSize = 2 + 4 + 1 + guidSize + 4 * 8 + (message.second.subject.size() + 1) + (message.second.body.size() + 1) + 1 + (
                            message.second.items.size() * (1 + 4 + 4 + MAX_INSPECTED_ENCHANTMENT_SLOT * 3 * 4 + 4 + 4 + 4 + 4 + 4 + 4 + 1));
                    }

                    packet << uint16_t(messageSize);
                    packet << uint32_t(message.second.message_id);
                    packet << uint8_t(message.second.message_type);

                    switch (message.second.message_type)
                    {
                        case MAIL_TYPE_NORMAL:
                            packet << uint64_t(message.second.sender_guid);
                            break;
                        case MAIL_TYPE_COD:
                        case MAIL_TYPE_AUCTION:
                        case MAIL_TYPE_ITEM:
                            packet << uint32_t(WoWGuid::getGuidLowPartFromUInt64(message.second.sender_guid));
                            break;
                        case MAIL_TYPE_GAMEOBJECT:
                        case MAIL_TYPE_CREATURE:
                            packet << uint32_t(static_cast<uint32_t>(message.second.sender_guid));
                            break;
                    }

                    if (m_protocol.expansion <= WoW::Expansion::_WotLK)
                        packet << uint32_t(message.second.cod);
                    else // Cata
                        packet << uint64_t(message.second.cod);

                    if (m_protocol.expansion <= WoW::Expansion::_TBC)
                    {
                        uint32_t itemPageEntry = 0;
                        if (!message.second.body.empty())
                        {
                            itemPageEntry = sMySQLStore.getItemPageEntryByText(message.second.body);
                            if (itemPageEntry == 0)
                            {
                                itemPageEntry = sObjectMgr.generateItemPageEntry();
                                sMySQLStore.addItemPage(itemPageEntry, message.second.body);
                            }
                        }
                        packet << uint32_t(itemPageEntry);
                    }

                    packet << uint32_t(0);
                    packet << uint32_t(message.second.stationery);

                    if (m_protocol.expansion <= WoW::Expansion::_WotLK)
                        packet << uint32_t(message.second.money);
                    else // Cata
                        packet << uint64_t(message.second.money);

                    packet << uint32_t(message.second.checked_flag);
                    packet << float(float((message.second.expire_time - uint32_t(UNIXTIME)) / DAY));
                    packet << uint32_t(0);

                    packet << message.second.subject;

                    if (m_protocol.expansion > WoW::Expansion::_TBC)
                        packet << message.second.body;

                    packet << uint8_t(message.second.items.size());

                    uint8_t i = 0;
                    if (!message.second.items.empty())
                    {
                        for (auto itemEntry : message.second.items)
                        {
                            const auto item = sObjectMgr.loadItem(itemEntry);
                            if (item == nullptr)
                                continue;

                            packet << uint8_t(i++);
                            packet << uint32_t(item->getGuidLow());
                            packet << uint32_t(item->getEntry());

                            for (uint8_t j = 0; j < MAX_INSPECTED_ENCHANTMENT_SLOT; ++j)
                            {
                                packet << uint32_t(item->getEnchantmentId(j));
                                packet << uint32_t(item->getEnchantmentDuration(j));
                                packet << uint32_t(item->getEnchantmentCharges(j));
                            }

                            packet << uint32_t(item->getRandomPropertiesId());
                            packet << uint32_t(item->getPropertySeed());
                            packet << uint32_t(item->getStackCount());
                            packet << uint32_t(item->getChargesLeft());
                            packet << uint32_t(item->getMaxDurability());
                            packet << uint32_t(item->getDurability());
                            packet << uint8_t(item->m_isLocked ? 1 : 0);
                        }
                    }
                    ++count;
                    ++realCount;
                }

                if (m_protocol.expansion > WoW::Expansion::_TBC)
                {
                    packet.put<uint32_t>(0, realCount);
                    packet.put<uint8_t>(4, count);
                }
                else // TBC and Classic
                {
                    packet.put<uint8_t>(0, count);
                }
            }
            else // Mop
            {
                uint32_t realCount = 0;
                uint8_t count = 0;

                ByteBuffer buffer;

                packet << uint32_t(0);

                size_t countPos = packet.bitwpos();
                packet.writeBits(0, 18);

                for (auto& message : messageMap)
                {
                    if (message.second.expire_time && static_cast<uint32_t>(UNIXTIME) > message.second.expire_time)
                        continue;

                    if (static_cast<uint32_t>(UNIXTIME) < message.second.delivery_time)
                        continue;

                    if (count >= 50)
                    {
                        ++realCount;
                        continue;
                    }

                    uint8_t guidSize;
                    if (message.second.message_type == 0)
                        guidSize = 8;
                    else
                        guidSize = 4;

                    size_t nextMailSize = 6 + 1 + 8 + message.second.items.size() * (4 + 4 + 4 + 4 + 4 + MAX_INSPECTED_ENCHANTMENT_SLOT * (4 + 4 + 4) +
                        4 + 4 + 4 + 4 + 1 + 4) + (message.second.subject.size() + 1) + (message.second.body.size() + 1) + 4 + 4 + 8 + 4 + 8 + 4 + 4 + 1 + 4;

                    if (packet.wpos() + nextMailSize > (sizeof(uint16_t) / 2))
                    {
                        realCount += 1;
                        continue;
                    }

                    packet.writeBit(message.second.message_type != MAIL_TYPE_NORMAL ? 1 : 0);
                    packet.writeBits(message.second.subject.size(), 8);
                    packet.writeBits(message.second.body.size(), 13);
                    packet.writeBit(0);
                    packet.writeBit(0);

                    packet.writeBits(message.second.items.size(), 17);

                    packet.writeBit(1);

                    WoWGuid guid = message.second.message_type == MAIL_TYPE_NORMAL ? message.second.sender_guid : 0;
                    packet.writeBit(guid[2]);
                    packet.writeBit(guid[6]);
                    packet.writeBit(guid[7]);
                    packet.writeBit(guid[0]);
                    packet.writeBit(guid[5]);
                    packet.writeBit(guid[3]);
                    packet.writeBit(guid[1]);
                    packet.writeBit(guid[4]);

                    uint8_t i = 0;
                    for (auto itemEntry : message.second.items)
                    {
                        const auto item = sObjectMgr.loadItem(itemEntry);
                            if (item == nullptr)
                                continue;

                        packet.writeBit(0);

                        buffer << uint32_t(item->getGuidLow());
                        buffer << uint32_t(4);                          // unk
                        buffer << uint32_t(item->getChargesLeft());
                        buffer << uint32_t(item->getDurability());
                        buffer << uint32_t(item->m_isLocked ? 1 : 0);

                        for (uint8_t j = 0; j < MAX_INSPECTED_ENCHANTMENT_SLOT; ++j)
                        {
                            buffer << uint32_t(item->getEnchantmentCharges(j));
                            buffer << uint32_t(item->getEnchantmentDuration(j));
                            buffer << uint32_t(item->getEnchantmentId(j));
                        }

                        buffer << uint32_t(item->getPropertySeed());
                        buffer << int32_t(item->getRandomPropertiesId());
                        buffer << uint32_t(item->getMaxDurability());
                        buffer << uint32_t(item->getStackCount());
                        buffer << uint8_t(i++);
                        buffer << uint32_t(item->getEntry());

                    }

                    buffer.writeString(message.second.body);
                    buffer << uint32_t(message.second.message_id);
                    buffer.writeByteSeq(guid[4]);
                    buffer.writeByteSeq(guid[0]);
                    buffer.writeByteSeq(guid[5]);
                    buffer.writeByteSeq(guid[3]);
                    buffer.writeByteSeq(guid[1]);
                    buffer.writeByteSeq(guid[7]);
                    buffer.writeByteSeq(guid[2]);
                    buffer.writeByteSeq(guid[6]);
                    buffer << uint32_t(0);                      //template
                    buffer << uint64_t(message.second.cod);
                    buffer.writeString(message.second.subject);
                    buffer << uint32_t(message.second.stationery);
                    buffer << float(float((message.second.expire_time - uint32_t(UNIXTIME)) / DAY));
                    buffer << uint64_t(message.second.money);
                    buffer << uint32_t(message.second.checked_flag);

                    if (message.second.message_type != MAIL_TYPE_NORMAL)
                    {
                        switch (message.second.message_type)
                        {
                            case MAIL_TYPE_COD:
                            case MAIL_TYPE_AUCTION:
                            case MAIL_TYPE_ITEM:
                                packet << uint32_t(WoWGuid::getGuidLowPartFromUInt64(message.second.sender_guid));
                                break;
                            case MAIL_TYPE_GAMEOBJECT:
                            case MAIL_TYPE_CREATURE:
                                packet << uint32_t(static_cast<uint32_t>(message.second.sender_guid));
                                break;
                            default:
                                break;
                        }
                    }

                    buffer << uint8_t(message.second.message_type);
                    buffer << uint32_t(0);                          // unknown

                    realCount++;
                    count++;
                }

                packet.flushBits();
                packet.append(buffer);

                packet.put<uint32_t>(0, realCount);
                packet.putBits(countPos, count, 18);
            }

            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
