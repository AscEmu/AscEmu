/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include "MailPacketHelpers.h"
#include <cstdint>

namespace AscEmu::Packets
{
    class CmsgMailMarkAsRead : public ManagedPacket
    {
    public:
        WoWGuid gobjGuid;
        uint32_t messageId;

        CmsgMailMarkAsRead() : CmsgMailMarkAsRead(0, 0)
        {
        }

        CmsgMailMarkAsRead(uint64_t gobjGuid, uint32_t messageId) :
            ManagedPacket(CMSG_MAIL_MARK_AS_READ, 12),
            gobjGuid(gobjGuid),
            messageId(messageId)
        {
        }

        bool deserialise(WorldPacket& packet) override
        {
            if (packet.remaining() < expectedSize())
                return false;

            return internalDeserialise(packet);
        }

    protected:
        size_t expectedSize() const override
        {
            if (m_protocol.expansion <= WoW::Expansion::_Cata)
                return m_minimum_size;
            else if (m_protocol.isMop())
                return 6; // uint32 messageId + 2 bytes holding the guid bits
            return 0;
        }

        bool internalDeserialise(WorldPacket& packet) override
        {
            // 6.x and later: the mailbox, then the mail
            if (m_protocol.expansion >= WoW::Expansion::_WoD)
            {
                gobjGuid = WoWGuid(MailWire::readGuid(packet));
                messageId = MailWire::readId(packet, m_protocol);
                return true;
            }

            if (m_protocol.isMop())
            {
                packet >> messageId;

                gobjGuid[0] = packet.readBit();
                gobjGuid[2] = packet.readBit();
                gobjGuid[3] = packet.readBit();
                packet.readBit();   // unused
                gobjGuid[4] = packet.readBit();
                gobjGuid[6] = packet.readBit();
                gobjGuid[7] = packet.readBit();
                gobjGuid[1] = packet.readBit();
                gobjGuid[5] = packet.readBit();
                packet.flushBits();

                packet.readByteSeq(gobjGuid[1]);
                packet.readByteSeq(gobjGuid[7]);
                packet.readByteSeq(gobjGuid[2]);
                packet.readByteSeq(gobjGuid[5]);
                packet.readByteSeq(gobjGuid[6]);
                packet.readByteSeq(gobjGuid[3]);
                packet.readByteSeq(gobjGuid[4]);
                packet.readByteSeq(gobjGuid[0]);

                return true;
            }
            else if (m_protocol.expansion < WoW::Expansion::_Mop)
            {
                uint64_t unpackedGuid;
                packet >> unpackedGuid >> messageId;
                gobjGuid = WoWGuid(unpackedGuid);

                return true;
            }

            return false;
        }
    };
}
