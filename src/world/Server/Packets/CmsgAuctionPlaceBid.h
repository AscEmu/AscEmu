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
        uint32_t auctionId;
        uint32_t price;

        // used since Cata
        uint64_t price64 = 0;

        CmsgAuctionPlaceBid() : CmsgAuctionPlaceBid(0, 0, 0)
        {
        }

        CmsgAuctionPlaceBid(uint64_t guid, uint32_t auctionId, uint32_t price) :
            ManagedPacket(CMSG_AUCTION_PLACE_BID, 0),
            guid(guid),
            auctionId(auctionId),
            price(price)
        {
        }

    protected:
        bool internalDeserialise(WorldPacket& packet) override
        {
            if (m_protocol.expansion < WoW::Expansion::_Cata)
            {
                uint64_t unpacked_guid;
                packet >> unpacked_guid >> auctionId;
                packet >> price;
                guid.init(unpacked_guid);
                return true;
            }
            else if (m_protocol.isCata())
            {
                uint64_t unpacked_guid;
                packet >> unpacked_guid >> auctionId;
                packet >> price64;
                price = static_cast<uint32_t>(price64);
                guid.init(unpacked_guid);
                return true;
            }
            else if (m_protocol.isMop())
            {
                packet >> auctionId;
                packet >> price64;
                price = static_cast<uint32_t>(price64);

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

            return false;
        }
    };
}
