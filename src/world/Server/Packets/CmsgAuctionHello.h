/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include <cstdint>

namespace AscEmu::Packets
{
    // Mop-only: replaces MSG_AUCTION_HELLO's request half with a dedicated opcode
    class CmsgAuctionHello : public ManagedPacket
    {
    public:
        WoWGuid guid;

        CmsgAuctionHello() : CmsgAuctionHello(0)
        {
        }

        CmsgAuctionHello(uint64_t guid) :
            ManagedPacket(CMSG_AUCTION_HELLO, 8),
            guid(guid)
        {
        }

    protected:
        bool internalDeserialise(WorldPacket& packet) override
        {
            if (m_protocol.isMop())
            {
                guid[1] = packet.readBit();
                guid[5] = packet.readBit();
                guid[2] = packet.readBit();
                guid[0] = packet.readBit();
                guid[3] = packet.readBit();
                guid[6] = packet.readBit();
                guid[4] = packet.readBit();
                guid[7] = packet.readBit();

                packet.readByteSeq(guid[2]);
                packet.readByteSeq(guid[7]);
                packet.readByteSeq(guid[1]);
                packet.readByteSeq(guid[3]);
                packet.readByteSeq(guid[5]);
                packet.readByteSeq(guid[0]);
                packet.readByteSeq(guid[4]);
                packet.readByteSeq(guid[6]);
                return true;
            }

            return false;
        }
    };
}
