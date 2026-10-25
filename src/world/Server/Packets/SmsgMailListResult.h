/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include "ItemInstanceHelpers.h"
#include "MailPacketHelpers.h"
#include "Management/MailMgr.h"
#include "Objects/Item.hpp"
#include "Management/ObjectMgr.hpp"
#include "Storage/MySQLDataStore.hpp"
#include "Utilities/CommonTime.hpp"
#include "Logging/Log.hpp"

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
            const uint32_t now = static_cast<uint32_t>(UNIXTIME);

            if (m_protocol.expansion >= WoW::Expansion::_WoD)
                return writeModern(packet, now);

            if (m_protocol.expansion <= WoW::Expansion::_Cata)
            {
                uint32_t realCount = 0;
                uint8_t count = 0;

                if (m_protocol.expansion > WoW::Expansion::_TBC)
                    packet << uint32_t(0);

                packet << uint8_t(0);

                for (auto& message : messageMap)
                {
                    if (message.second.expire_time && now > message.second.expire_time)
                        continue;

                    if (now < message.second.delivery_time)
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
                        case MAIL_TYPE_CALENDAR:
                            packet << uint32_t(WoWGuid::getLowGuidFromRaw(message.second.sender_guid));
                            break;
                        case MAIL_TYPE_GAMEOBJECT:
                        case MAIL_TYPE_CREATURE:
                            packet << uint32_t(static_cast<uint32_t>(message.second.sender_guid));
                            break;
                        default:
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
                    packet << float(float(message.second.expire_time - now) / static_cast<float>(TimeVars::Day));
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
                    if (message.second.expire_time && now > message.second.expire_time)
                        continue;

                    if (now < message.second.delivery_time)
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
                        buffer << uint32_t(4); // unk
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
                    buffer << uint32_t(0); // template
                    buffer << uint64_t(message.second.cod);
                    buffer.writeString(message.second.subject);
                    buffer << uint32_t(message.second.stationery);
                    buffer << float(float(message.second.expire_time - now) / static_cast<float>(TimeVars::Day));
                    buffer << uint64_t(message.second.money);
                    buffer << uint32_t(message.second.checked_flag);

                    if (message.second.message_type != MAIL_TYPE_NORMAL)
                    {
                        switch (message.second.message_type)
                        {
                            case MAIL_TYPE_COD:
                            case MAIL_TYPE_AUCTION:
                            case MAIL_TYPE_CALENDAR:
                                packet << uint32_t(WoWGuid::getLowGuidFromRaw(message.second.sender_guid));
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
                    buffer << uint32_t(0); // unknown

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

    private:
        // the sender of a mail: a player for a normal mail, the id of the auction house, creature, game object or event else
        static bool hasSenderGuid(MailMessage const& message) { return message.message_type == MAIL_TYPE_NORMAL; }
        static bool hasAltSender(MailMessage const& message)
        {
            return message.message_type == MAIL_TYPE_AUCTION || message.message_type == MAIL_TYPE_CREATURE
                || message.message_type == MAIL_TYPE_GAMEOBJECT || message.message_type == MAIL_TYPE_CALENDAR;
        }

        void writeSender(WorldPacket& packet, MailMessage const& message) const
        {
            if (hasSenderGuid(message))
                packet << WoWGuid(message.sender_guid).toGuid128(m_protocol.realmId, 0);
            else if (hasAltSender(message))
                packet << int32_t(static_cast<uint32_t>(message.sender_guid));
        }

        void writeAttachment(WorldPacket& packet, Item const* item, uint8_t position) const
        {
            packet << position;

            if (m_protocol.isWoD())
            {
                // 6.x: the item, eight enchantments, then the state of the item
                packet << uint32_t(item->getGuidLow());
                writeItemInstance(packet, m_protocol, item->getEntry(), int32_t(item->getRandomPropertiesId()), int32_t(item->getPropertySeed()));
                for (uint8_t i = 0; i < 8; ++i)
                {
                    const bool known = i < MAX_INSPECTED_ENCHANTMENT_SLOT;
                    packet << int32_t(known ? item->getEnchantmentId(i) : 0);
                    packet << int32_t(known ? item->getEnchantmentDuration(i) : 0);
                    packet << int32_t(known ? item->getEnchantmentCharges(i) : 0);
                }
                packet << int32_t(item->getStackCount()) << int32_t(item->getChargesLeft());
                packet << int32_t(item->getMaxDurability()) << int32_t(item->getDurability());
                packet.writeBit(!item->m_isLocked);
                packet.flushBits();
                return;
            }

            MailWire::writeId(packet, m_protocol, item->getGuidLow());

            // 12.x moves the item in front of its state
            if (m_protocol.isMidnight())
                writeItemInstance(packet, m_protocol, item->getEntry());

            packet << int32_t(item->getStackCount()) << int32_t(item->getChargesLeft());
            packet << uint32_t(item->getMaxDurability()) << int32_t(item->getDurability());

            if (!m_protocol.isMidnight())
                writeItemInstance(packet, m_protocol, item->getEntry(), int32_t(item->getRandomPropertiesId()), int32_t(item->getPropertySeed()));

            packet.writeBits(0, 4);                     // enchantments
            packet.writeBits(0, 2);                     // gems
            packet.writeBit(!item->m_isLocked);
            packet.flushBits();
        }

        static std::vector<std::unique_ptr<Item>> loadAttachments(MailMessage const& message)
        {
            std::vector<std::unique_ptr<Item>> items;
            for (const auto itemLowGuid : message.items)
                if (auto item = sObjectMgr.loadItem(itemLowGuid))
                    items.push_back(std::move(item));

            return items;
        }

        void writeAttachments(WorldPacket& packet, std::vector<std::unique_ptr<Item>> const& items) const
        {
            for (size_t i = 0; i < items.size(); ++i)
                writeAttachment(packet, items[i].get(), static_cast<uint8_t>(i));
        }

        // 6.x and later: count and total, then every mail; the order of sender, attachments and texts changes with 7.x, 10.x and 12.x
        bool writeModern(WorldPacket& packet, uint32_t now) const
        {
            std::vector<MailMessage const*> mails;
            int32_t total = 0;
            for (const auto& [_, message] : messageMap)
            {
                if ((message.expire_time && now > message.expire_time) || now < message.delivery_time)
                    continue;

                ++total;
                if (mails.size() < 50)
                    mails.push_back(&message);
            }

            packet << uint32_t(mails.size()) << total;

            for (const auto* message : mails)
            {
                const auto items = loadAttachments(*message);
                const auto attachments = static_cast<uint32_t>(items.size());
                const float daysLeft = float(message->expire_time - now) / static_cast<float>(TimeVars::Day);

                if (m_protocol.expansion >= WoW::Expansion::_Dragonflight)
                {
                    packet << uint64_t(message->message_id) << uint32_t(message->message_type);
                    packet << uint64_t(message->cod) << int32_t(message->stationery) << uint64_t(message->money);
                    packet << int32_t(message->checked_flag) << daysLeft << int32_t(0) << attachments;
                    writeSender(packet, *message);

                    if (m_protocol.isMidnight())
                        writeAttachments(packet, items);

                    packet.writeBits(message->subject.size(), 8);
                    packet.writeBits(message->body.size(), 13);
                    packet.flushBits();

                    if (!m_protocol.isMidnight())
                        writeAttachments(packet, items);

                    packet.writeString(message->subject);
                    packet.writeString(message->body);
                    continue;
                }

                packet << int32_t(message->message_id) << uint8_t(message->message_type);
                packet << uint64_t(message->cod) << int32_t(message->stationery) << uint64_t(message->money);
                packet << int32_t(message->checked_flag) << daysLeft << int32_t(0) << attachments;

                // 6.x sends the attachments first, 7.x to 9.x after the flags
                if (m_protocol.isWoD())
                    writeAttachments(packet, items);

                packet.writeBit(hasSenderGuid(*message));
                packet.writeBit(hasAltSender(*message));
                packet.writeBits(message->subject.size(), 8);
                packet.writeBits(message->body.size(), 13);
                packet.flushBits();

                if (!m_protocol.isWoD())
                    writeAttachments(packet, items);

                writeSender(packet, *message);
                packet.writeString(message->subject);
                packet.writeString(message->body);
            }

            return true;
        }
    };
}
