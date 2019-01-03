/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once
#include <cstdint>

#include "ManagedPacket.h"
#include "WorldPacket.h"
#include "Management/AuctionHouse.h"

namespace AscEmu { namespace Packets
{
    class CmsgAuctionSellItem : public ManagedPacket
    {
    public:
        WoWGuid auctioneerGuid;
        uint64_t bidMoney;
        uint64_t buyoutPrice;
        uint32_t itemsCount;
        uint32_t expireTime;

        uint64_t itemGuids[MAX_AUCTION_ITEMS];
        uint32_t count[MAX_AUCTION_ITEMS];

        CmsgAuctionSellItem() : CmsgAuctionSellItem(0, 0, 0, 0, 0)
        {
        }

        CmsgAuctionSellItem(uint64_t auctioneerGuid, uint64_t bidMoney, uint64_t buyoutPrice, uint32_t itemsCount, uint32_t expireTime) :
            ManagedPacket(CMSG_AUCTION_SELL_ITEM, 0),
            auctioneerGuid(auctioneerGuid),
            bidMoney(bidMoney),
            buyoutPrice(buyoutPrice),
            itemsCount(itemsCount),
            expireTime(expireTime)
        {
        }

    protected:
        bool internalSerialise(WorldPacket& /*packet*/) override
        {
            return false;
        }

        bool internalDeserialise(WorldPacket& packet) override
        {
            uint64_t unpacked_guid;
            packet >> unpacked_guid >> itemsCount;

            for (uint32_t i = 0; i < itemsCount; ++i)
            {
                packet >> itemGuids[i];
                packet >> count[i];

                if (!itemGuids[i] || !count[i] || count[i] > 1000)
                    return false;
            }

            packet >> bidMoney >> buyoutPrice >> expireTime;

            auctioneerGuid.Init(unpacked_guid);
            return true;
        }
    };
}}
