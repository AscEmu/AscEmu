/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include <cstdint>

namespace AscEmu::Packets
{
    class CmsgAuctionPlaceBid : public ManagedPacket
    {
    public:
        WoWGuid guid;
        uint32_t auctionId = 0;
        uint64_t price = 0;

        CmsgAuctionPlaceBid() : CmsgAuctionPlaceBid(0, 0, 0)
        {
        }

        CmsgAuctionPlaceBid(uint64_t guid, uint32_t auctionId, uint64_t price) :
            ManagedPacket(CMSG_AUCTION_PLACE_BID, 0),
            guid(guid),
            auctionId(auctionId),
            price(price)
        {}

    protected:
        bool internalDeserialise(WorldPacket& packet) override
        {
            if (!m_protocol.isValid())
                return false;

            if (m_protocol.isMop())
            {
                packet >> auctionId;
                packet >> price;

                guid[1] = packet.readBit();
                guid[6] = packet.readBit();
                guid[3] = packet.readBit();
                guid[7] = packet.readBit();
                guid[2] = packet.readBit();
                guid[4] = packet.readBit();
                guid[0] = packet.readBit();
                guid[5] = packet.readBit();

                packet.readByteSeq(guid[3]);
                packet.readByteSeq(guid[2]);
                packet.readByteSeq(guid[1]);
                packet.readByteSeq(guid[4]);
                packet.readByteSeq(guid[6]);
                packet.readByteSeq(guid[5]);
                packet.readByteSeq(guid[7]);
                packet.readByteSeq(guid[0]);
                return true;
            }

            uint64_t unpacked_guid = 0;
            packet >> unpacked_guid >> auctionId;
            guid.init(unpacked_guid);

            if (m_protocol.isContentAtLeast(WoW::Expansion::_Cata))
            {
                packet >> price;
            }
            else
            {
                uint32_t price32 = 0;
                packet >> price32;
                price = price32;
            }

            return true;
        }
    };
}
