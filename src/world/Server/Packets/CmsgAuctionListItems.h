/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include <cstdint>

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
        uint8_t exactMatch = 0;      //Mop

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
        bool internalDeserialise(WorldPacket& packet) override
        {
            if (m_protocol.expansion < WoW::Expansion::_Cata)
            {
                uint64_t unpacked_guid;
                packet >> unpacked_guid;
                guid.init(unpacked_guid);

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
                uint8_t sortCount;
                packet >> sortCount;
                for (uint8_t i = 0; i < sortCount; ++i)
                {
                    packet.readSkip<uint8_t>();
                    packet.readSkip<uint8_t>();
                }

                return true;
            }
            else if (m_protocol.isCata())
            {
                uint64_t unpacked_guid;
                packet >> unpacked_guid;
                guid.init(unpacked_guid);

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
                packet.readSkip<uint8_t>();    // unk

                uint8_t sortCount;
                packet >> sortCount;
                for (uint8_t i = 0; i < sortCount; ++i)
                {
                    packet.readSkip<uint8_t>();
                    packet.readSkip<uint8_t>();
                }

                return true;
            }
            else if (m_protocol.isMop())
            {
                packet >> auctionSlotId;
                packet >> listFrom;
                packet >> auctionMainCategory;
                packet.readSkip<uint8_t>();    // unk

                packet >> levelMax;
                packet >> levelMin;

                packet >> quality;
                packet >> auctionSubCategory;

                // sorting is not implemented yet
                uint8_t sortCount;
                packet >> sortCount;
                for (uint8_t i = 0; i < sortCount; ++i)
                    packet.readSkip<uint8_t>();

                guid[3] = packet.readBit();
                guid[4] = packet.readBit();
                guid[5] = packet.readBit();
                guid[2] = packet.readBit();

                exactMatch = packet.readBit();
                usable = packet.readBit();

                guid[7] = packet.readBit();
                guid[0] = packet.readBit();

                const uint32_t searchStringLen = packet.readBits(8);

                guid[1] = packet.readBit();
                guid[6] = packet.readBit();

                packet.readByteSeq(guid[6]);
                packet.readByteSeq(guid[3]);
                packet.readByteSeq(guid[4]);
                packet.readByteSeq(guid[0]);
                packet.readByteSeq(guid[7]);
                packet.readByteSeq(guid[2]);

                searchedName = packet.readString(searchStringLen);

                packet.readByteSeq(guid[1]);
                packet.readByteSeq(guid[5]);

                return true;
            }

            return false;
        }
    };
}
