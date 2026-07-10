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

        uint64_t bidMoney64 = 0;        //Cata
        uint64_t buyoutPrice64 = 0;     //Cata

        uint32_t bidMoney;
        uint32_t buyoutPrice;

        uint32_t itemsCount;
        uint32_t expireTime;

        uint64_t itemGuids[MAX_AUCTION_ITEMS];
        uint32_t count[MAX_AUCTION_ITEMS];

        CmsgAuctionSellItem() : CmsgAuctionSellItem(0, 0, 0, 0, 0)
        {
        }

        CmsgAuctionSellItem(uint64_t auctioneerGuid, uint32_t bidMoney, uint32_t buyoutPrice, uint32_t itemsCount, uint32_t expireTime) :
            ManagedPacket(CMSG_AUCTION_SELL_ITEM, 0),
            auctioneerGuid(auctioneerGuid),
            bidMoney(bidMoney),
            buyoutPrice(buyoutPrice),
            itemsCount(itemsCount),
            expireTime(expireTime)
        {
        }

    protected:
        bool internalDeserialise(WorldPacket& packet) override
        {
            uint64_t unpacked_guid;
            packet >> unpacked_guid >> itemsCount;
            auctioneerGuid.init(unpacked_guid);

            for (uint32_t i = 0; i < itemsCount; ++i)
            {
                packet >> itemGuids[i];
                packet >> count[i];

                if (!itemGuids[i] || !count[i] || count[i] > 1000)
                    return false;
            }

            if (m_protocol.expansion < WoW::Expansion::_Cata)
            {
                packet >> bidMoney >> buyoutPrice;
            }
            else
            {
                packet >> bidMoney64 >> buyoutPrice64;
                bidMoney = static_cast<uint32_t>(bidMoney64);
                buyoutPrice = static_cast<uint32_t>(buyoutPrice64);
            }

            packet >> expireTime;

            return true;
        }
    };
}
