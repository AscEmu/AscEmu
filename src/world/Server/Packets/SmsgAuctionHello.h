/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include <cstdint>

namespace AscEmu::Packets
{
    // Mop-only: replaces MSG_AUCTION_HELLO's reply half with a dedicated opcode
    class SmsgAuctionHello : public ManagedPacket
    {
    public:
        WoWGuid guid;
        uint32_t auctionHouseId;
        uint8_t isAuctionHouseEnabled;

        SmsgAuctionHello() : SmsgAuctionHello(0, 0, 0)
        {
        }

        SmsgAuctionHello(uint64_t guid, uint32_t auctionHouseId, uint8_t isAuctionHouseEnabled) :
            ManagedPacket(SMSG_AUCTION_HELLO, 1 + 1 + 8 + 4),
            guid(guid),
            auctionHouseId(auctionHouseId),
            isAuctionHouseEnabled(isAuctionHouseEnabled)
        {
        }

    protected:
        bool internalSerialise(WorldPacket& packet) override
        {
            if (m_protocol.isMop())
            {
                packet.writeBit(guid[6]);
                packet.writeBit(guid[7]);
                packet.writeBit(guid[3]);
                packet.writeBit(isAuctionHouseEnabled);
                packet.writeBit(guid[4]);
                packet.writeBit(guid[2]);
                packet.writeBit(guid[5]);
                packet.writeBit(guid[0]);
                packet.writeBit(guid[1]);
                packet.flushBits();

                packet.writeByteSeq(guid[3]);
                packet << auctionHouseId;
                packet.writeByteSeq(guid[4]);
                packet.writeByteSeq(guid[2]);
                packet.writeByteSeq(guid[7]);
                packet.writeByteSeq(guid[1]);
                packet.writeByteSeq(guid[0]);
                packet.writeByteSeq(guid[6]);
                packet.writeByteSeq(guid[5]);
                return true;
            }

            return false;
        }
    };
}
