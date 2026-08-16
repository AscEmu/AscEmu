/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include "Management/MailMgr.h"
#include "Utilities/CommonTime.hpp"
#include "WoWGuid.hpp"

namespace AscEmu::Packets
{
    class MsgQueryNextMailTime : public ManagedPacket
    {
        MessageMap messageMap;

    public:
        MsgQueryNextMailTime(MessageMap& messageMap) :
            ManagedPacket(MSG_QUERY_NEXT_MAIL_TIME, 32),
            messageMap(messageMap)
        {
        }

    protected:
        size_t expectedSize() const override { return m_protocol.isMop() ? size_t(96) : size_t(32); }

        bool internalSerialise(WorldPacket& packet) override
        {
            if (m_protocol.isMop())
            {
                bool hasUnreadMail = false;
                for (auto& message : messageMap)
                {
                    if (!(message.second.checked_flag & MAIL_CHECK_MASK_READ))
                    {
                        hasUnreadMail = true;
                        break;
                    }
                }

                if (hasUnreadMail)
                {
                    const auto now = static_cast<uint32_t>(UNIXTIME);
                    const bool hasVirtualRealmAddress = false;    // cross-realm mail addressing is not implemented
                    const bool hasNativeRealmAddress = false;

                    uint8_t count = 0;
                    const size_t countBitPos = packet.bitwpos();
                    packet.writeBits(count, 20);

                    ByteBuffer dataBuffer;

                    for (auto& message : messageMap)
                    {
                        if (message.second.checked_flag & MAIL_CHECK_MASK_READ)
                            continue;

                        if (message.second.deleted_flag != 0 || now < message.second.delivery_time)
                            continue;

                        const WoWGuid senderGuid = message.second.message_type == MAIL_TYPE_NORMAL ? WoWGuid(message.second.sender_guid) : WoWGuid(uint64_t(0));

                        packet.writeBit(senderGuid[3]);
                        packet.writeBit(hasVirtualRealmAddress);
                        packet.writeBit(senderGuid[2]);
                        packet.writeBit(hasNativeRealmAddress);
                        packet.writeBit(senderGuid[6]);
                        packet.writeBit(senderGuid[1]);
                        packet.writeBit(senderGuid[4]);
                        packet.writeBit(senderGuid[0]);
                        packet.writeBit(senderGuid[5]);
                        packet.writeBit(senderGuid[7]);

                        dataBuffer << uint32_t(message.second.message_type != MAIL_TYPE_NORMAL ? message.second.sender_guid : 0);
                        dataBuffer.writeByteSeq(senderGuid[5]);
                        dataBuffer.writeByteSeq(senderGuid[4]);
                        dataBuffer.writeByteSeq(senderGuid[6]);
                        dataBuffer.writeByteSeq(senderGuid[1]);
                        dataBuffer << uint8_t(message.second.message_type);
                        dataBuffer.writeByteSeq(senderGuid[0]);
                        dataBuffer << float(message.second.delivery_time - now);
                        if (hasNativeRealmAddress)
                            dataBuffer << uint32_t(0);     // realm id
                        dataBuffer << uint32_t(message.second.stationery);
                        dataBuffer.writeByteSeq(senderGuid[3]);
                        dataBuffer.writeByteSeq(senderGuid[2]);
                        if (hasVirtualRealmAddress)
                            dataBuffer << uint32_t(0);     // realm id
                        dataBuffer.writeByteSeq(senderGuid[7]);

                        ++count;
                        if (count == 3)     // real client never displays more than 3 mails here
                            break;
                    }

                    packet.flushBits();
                    packet.putBits(countBitPos, count, 20);
                    packet.append(dataBuffer);

                    packet << float(0);
                }
                else
                {
                    packet.writeBits(0, 20);
                    packet.flushBits();
                    packet << float(-1);
                }
            }
            else
            {
                uint32_t unreadMessageCount = 0;
                packet << uint32_t(0);
                packet << uint32_t(0);

                for (auto& message : messageMap)
                {
                    if (message.second.checked_flag & MAIL_CHECK_MASK_READ)
                        continue;

                    if (message.second.deleted_flag == 0 && static_cast<uint32_t>(UNIXTIME) >= message.second.delivery_time)
                    {
                        ++unreadMessageCount;
                        packet << uint64_t(message.second.sender_guid);
                        packet << uint32_t(message.second.message_type != MAIL_TYPE_NORMAL ? message.second.sender_guid : 0);
                        packet << uint32_t(message.second.message_type);
                        packet << uint32_t(message.second.stationery);
                        packet << float(message.second.delivery_time - static_cast<uint32_t>(UNIXTIME));
                    }
                }

                if (unreadMessageCount == 0)
                    packet.put<uint32_t>(0, 0xc7a8c000);
                else
                    packet.put<uint32_t>(4, unreadMessageCount);
            }

            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
