/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
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

    protected:
        bool internalDeserialise(WorldPacket& packet) override
        {
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
