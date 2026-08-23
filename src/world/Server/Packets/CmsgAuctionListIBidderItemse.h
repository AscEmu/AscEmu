/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include <cstdint>

namespace AscEmu::Packets
{
    class CmsgAuctionListIBidderItemse : public ManagedPacket
    {
    public:
        WoWGuid guid;
        uint32_t listForm;
        uint32_t outbiddedCount;

        CmsgAuctionListIBidderItemse() : CmsgAuctionListIBidderItemse(0, 0, 0)
        {
        }

        CmsgAuctionListIBidderItemse(uint64_t guid, uint32_t listForm, uint32_t outbiddedCount) :
            ManagedPacket(CMSG_AUCTION_LIST_BIDDER_ITEMS, 0),
            guid(guid),
            listForm(listForm),
            outbiddedCount(outbiddedCount)
        {
        }

    protected:
        bool internalDeserialise(WorldPacket& packet) override
        {
            if (m_protocol.expansion < WoW::Expansion::_Mop)
            {
                uint64_t unpacked_guid;
                packet >> unpacked_guid >> listForm >> outbiddedCount;
                guid.init(unpacked_guid);
                return true;
            }
            else if (m_protocol.isMop())
            {
                packet >> listForm;

                guid[3] = packet.readBit();
                guid[4] = packet.readBit();
                guid[1] = packet.readBit();
                guid[5] = packet.readBit();
                guid[6] = packet.readBit();
                guid[2] = packet.readBit();

                outbiddedCount = static_cast<uint32_t>(packet.readBits(7));

                guid[7] = packet.readBit();
                guid[0] = packet.readBit();

                packet.readByteSeq(guid[3]);
                packet.readByteSeq(guid[4]);
                packet.readByteSeq(guid[1]);
                packet.readByteSeq(guid[0]);
                packet.readByteSeq(guid[2]);
                packet.readByteSeq(guid[5]);

                for (uint32_t i = 0; i < outbiddedCount; ++i)
                    packet.readSkip<uint32_t>();

                packet.readByteSeq(guid[7]);
                packet.readByteSeq(guid[6]);
                return true;
            }

            return false;
        }
    };
}
