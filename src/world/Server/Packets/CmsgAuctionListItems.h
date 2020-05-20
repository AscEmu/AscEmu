/*
Copyright (c) 2014-2020 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once
#include <cstdint>

#include "ManagedPacket.h"
#include "WorldPacket.h"

namespace AscEmu::Packets
{
    class CmsgAuctionListItems : public ManagedPacket
    {
    public:
        WoWGuid guid;

        uint32_t listFrom;
        std::string searchedName;
        uint8_t levelMin;
        uint8_t levelMax;
        uint32_t auctionSlotId;
        uint32_t auctionMainCategory;
        uint32_t auctionSubCategory;
        uint32_t quality;
        uint8_t usable;
        uint8_t getAll;

        CmsgAuctionListItems() : CmsgAuctionListItems(0, 0, "", 0, 0, 0, 0, 0, 0, 0, 0)
        {
        }

        CmsgAuctionListItems(uint64_t guid, uint32_t listFrom, std::string searchedName, uint8_t levelMin, uint8_t levelMax, uint32_t auctionSlotId,
            uint32_t auctionMainCategory, uint32_t auctionSubCategory, uint32_t quality, uint8_t usable, uint8_t getAll) :
            ManagedPacket(CMSG_AUCTION_LIST_ITEMS, 0),
            guid(guid),
            listFrom(listFrom),
            searchedName(searchedName),
            levelMin(levelMin),
            levelMax(levelMax),
            auctionSlotId(auctionSlotId),
            auctionMainCategory(auctionMainCategory),
            auctionSubCategory(auctionSubCategory),
            quality(quality),
            usable(usable),
            getAll(getAll)
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
            packet >> unpacked_guid;
            guid.Init(unpacked_guid);

            packet >> listFrom;
            packet >> searchedName;
            packet >> levelMin;
            packet >> levelMax;
            packet >> auctionSlotId;
            packet >> auctionMainCategory;
            packet >> auctionSubCategory;
            packet >> quality;
            packet >> usable;

            packet >> getAll;

            // sorting is not implemented yet
#if VERSION_STRING < Cata
            uint8_t sortCount;
            packet >> sortCount;
            for (uint8_t i = 0; i < sortCount; ++i)
            {
                packet.read_skip<uint8_t>();
                packet.read_skip<uint8_t>();
            }
#else
            packet.read_skip<uint8_t>();    //sortCount

            for (uint8_t i = 0; i < 15; ++i)
                packet.read_skip<uint8_t>();
#endif

            return true;
        }
    };
}
