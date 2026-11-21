/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include <cstdint>
#include "Management/AuctionHouse.h"

namespace AscEmu::Packets
{
    class CmsgAuctionSellItem : public ManagedPacket
    {
    public:
        WoWGuid auctioneerGuid;

        uint64_t bidMoney = 0;
        uint64_t buyoutPrice = 0;
        uint32_t itemsCount = 0;
        uint32_t expireTime = 0;

        uint64_t itemGuids[MAX_AUCTION_ITEMS];
        uint32_t count[MAX_AUCTION_ITEMS];

        CmsgAuctionSellItem() : CmsgAuctionSellItem(0, 0, 0, 0, 0)
        {}

        CmsgAuctionSellItem(uint64_t auctioneerGuid, uint64_t bidMoney, uint64_t buyoutPrice, uint32_t itemsCount, uint32_t expireTime) :
            ManagedPacket(CMSG_AUCTION_SELL_ITEM, 0),
            auctioneerGuid(auctioneerGuid),
            bidMoney(bidMoney),
            buyoutPrice(buyoutPrice),
            itemsCount(itemsCount),
            expireTime(expireTime)
        {}

    protected:
        bool internalDeserialise(WorldPacket& packet) override
        {
            if (!m_protocol.isValid())
                return false;

            if (m_protocol.isMop())
            {
                packet >> buyoutPrice >> bidMoney;
                packet >> expireTime;

                auctioneerGuid[3] = packet.readBit();
                itemsCount = static_cast<uint32_t>(packet.readBits(5));

                if (!itemsCount || itemsCount > MAX_AUCTION_ITEMS)
                    return false;

                auctioneerGuid[0] = packet.readBit();

                WoWGuid itemGuidsLocal[MAX_AUCTION_ITEMS];
                for (uint32_t i = 0; i < itemsCount; ++i)
                {
                    itemGuidsLocal[i][4] = packet.readBit();
                    itemGuidsLocal[i][6] = packet.readBit();
                    itemGuidsLocal[i][2] = packet.readBit();
                    itemGuidsLocal[i][3] = packet.readBit();
                    itemGuidsLocal[i][5] = packet.readBit();
                    itemGuidsLocal[i][7] = packet.readBit();
                    itemGuidsLocal[i][1] = packet.readBit();
                    itemGuidsLocal[i][0] = packet.readBit();
                }

                auctioneerGuid[6] = packet.readBit();
                auctioneerGuid[2] = packet.readBit();
                auctioneerGuid[1] = packet.readBit();
                auctioneerGuid[4] = packet.readBit();
                auctioneerGuid[5] = packet.readBit();
                auctioneerGuid[7] = packet.readBit();

                for (uint32_t i = 0; i < itemsCount; ++i)
                {
                    packet.readByteSeq(itemGuidsLocal[i][3]);
                    packet.readByteSeq(itemGuidsLocal[i][1]);
                    packet >> count[i];
                    packet.readByteSeq(itemGuidsLocal[i][6]);
                    packet.readByteSeq(itemGuidsLocal[i][4]);
                    packet.readByteSeq(itemGuidsLocal[i][5]);
                    packet.readByteSeq(itemGuidsLocal[i][0]);
                    packet.readByteSeq(itemGuidsLocal[i][2]);
                    packet.readByteSeq(itemGuidsLocal[i][7]);

                    itemGuids[i] = uint64_t(itemGuidsLocal[i]);

                    if (!itemGuids[i] || !count[i] || count[i] > 1000)
                        return false;
                }

                packet.readByteSeq(auctioneerGuid[3]);
                packet.readByteSeq(auctioneerGuid[7]);
                packet.readByteSeq(auctioneerGuid[2]);
                packet.readByteSeq(auctioneerGuid[5]);
                packet.readByteSeq(auctioneerGuid[6]);
                packet.readByteSeq(auctioneerGuid[1]);
                packet.readByteSeq(auctioneerGuid[0]);
                packet.readByteSeq(auctioneerGuid[4]);

                return true;
            }

            uint64_t unpacked_guid = 0;
            packet >> unpacked_guid >> itemsCount;
            auctioneerGuid.init(unpacked_guid);

            if (!itemsCount || itemsCount > MAX_AUCTION_ITEMS)
                return false;

            for (uint32_t i = 0; i < itemsCount; ++i)
            {
                packet >> itemGuids[i];
                packet >> count[i];

                if (!itemGuids[i] || !count[i] || count[i] > 1000)
                    return false;
            }

            if (m_protocol.isContentAtLeast(WoW::Expansion::_Cata))
            {
                packet >> bidMoney >> buyoutPrice;
            }
            else
            {
                uint32_t bidMoney32 = 0;
                uint32_t buyoutPrice32 = 0;
                packet >> bidMoney32 >> buyoutPrice32;
                bidMoney = bidMoney32;
                buyoutPrice = buyoutPrice32;
            }

            packet >> expireTime;
            return true;
        }
    };
}
