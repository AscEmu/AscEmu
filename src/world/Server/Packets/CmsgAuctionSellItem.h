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
            if (m_protocol.expansion < WoW::Expansion::_Cata)
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

                packet >> bidMoney >> buyoutPrice;
                packet >> expireTime;
                return true;
            }
            else if (m_protocol.isCata())
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

                packet >> bidMoney64 >> buyoutPrice64;
                bidMoney = static_cast<uint32_t>(bidMoney64);
                buyoutPrice = static_cast<uint32_t>(buyoutPrice64);
                packet >> expireTime;
                return true;
            }
            else if (m_protocol.isMop())
            {
                packet >> buyoutPrice64 >> bidMoney64;
                bidMoney = static_cast<uint32_t>(bidMoney64);
                buyoutPrice = static_cast<uint32_t>(buyoutPrice64);

                packet >> expireTime;

                auctioneerGuid[3] = packet.readBit();
                itemsCount = static_cast<uint32_t>(packet.readBits(5));

                if (itemsCount > MAX_AUCTION_ITEMS)
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

            return false;
        }
    };
}
