/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include <cstdint>
#include <vector>

namespace AscEmu::Packets
{
    struct VendorListItem
    {
        uint32_t itemId = 0;
        uint32_t displayId = 0;
        uint32_t availableAmount = 0;  // 0xFFFFFFFF == unlimited
        uint32_t price = 0;
        uint32_t maxDurability = 0;    // only used pre-Cata
        uint32_t buyCount = 0;
        uint32_t extendedCostId = 0;   // 0 == none
        uint32_t type = 1;             // 1 == item, 2 == currency
        uint32_t slot = 0;             // 1-based slot index, as sent to the client
    };

    class SmsgListInventory : public ManagedPacket
    {
    public:
        WoWGuid vendorGuid;
        uint32_t rawItemCount = 0;     // unfiltered vendor item count (not just what's visible to this player)
        bool vendorIsArmorer = false;
        std::vector<VendorListItem> items;

        SmsgListInventory(uint64_t vendorGuid, uint32_t rawItemCount, bool vendorIsArmorer, std::vector<VendorListItem> items) :
            ManagedPacket(SMSG_LIST_INVENTORY, 0),
            vendorGuid(vendorGuid),
            rawItemCount(rawItemCount),
            vendorIsArmorer(vendorIsArmorer),
            items(std::move(items))
        {
        }

    protected:
        bool internalSerialise(WorldPacket& packet) override
        {
            // If there are no items visible to this player and this byte is 1, the client opens an
            // empty vendor window; otherwise it silently ignores the packet. Retail sends the raw
            // (unfiltered) vendor item count here, falling back to the armorer flag only when the
            // vendor truly has none configured.
            const uint8_t emptyListByte = rawItemCount ? static_cast<uint8_t>(rawItemCount) : static_cast<uint8_t>(vendorIsArmorer);

            if (m_protocol.isMop())
            {
                packet.writeBit(vendorGuid[5]);
                packet.writeBit(vendorGuid[7]);
                packet.writeBit(vendorGuid[1]);
                packet.writeBit(vendorGuid[3]);
                packet.writeBit(vendorGuid[6]);

                packet.writeBits(items.size(), 18); // item count

                for (const auto& item : items)
                {
                    packet.writeBit(0);
                    packet.writeBit(item.extendedCostId == 0);
                    packet.writeBit(1);
                }

                packet.writeBit(vendorGuid[4]);
                packet.writeBit(vendorGuid[0]);
                packet.writeBit(vendorGuid[2]);

                packet << emptyListByte;

                ByteBuffer itemsData(32 * items.size());
                for (const auto& item : items)
                {
                    itemsData << int32_t(item.availableAmount);
                    itemsData << uint32_t(item.price);
                    itemsData << uint32_t(item.type);
                    itemsData << int32_t(-1);
                    itemsData << uint32_t(item.displayId);
                    itemsData << uint32_t(item.buyCount);
                    itemsData << uint32_t(item.itemId);

                    if (item.extendedCostId != 0)
                        itemsData << uint32_t(item.extendedCostId);

                    itemsData << uint32_t(0);
                    itemsData << uint32_t(item.slot);
                }
                packet.append(itemsData);

                packet.writeByteSeq(vendorGuid[3]);
                packet.writeByteSeq(vendorGuid[7]);
                packet.writeByteSeq(vendorGuid[0]);
                packet.writeByteSeq(vendorGuid[6]);
                packet.writeByteSeq(vendorGuid[2]);
                packet.writeByteSeq(vendorGuid[1]);
                packet.writeByteSeq(vendorGuid[4]);
                packet.writeByteSeq(vendorGuid[5]);

                return true;
            }
            else if (m_protocol.isCata())
            {
                packet.writeBit(vendorGuid[1]);
                packet.writeBit(vendorGuid[0]);

                packet.writeBits(items.size(), 21); // item count

                packet.writeBit(vendorGuid[3]);
                packet.writeBit(vendorGuid[6]);
                packet.writeBit(vendorGuid[5]);
                packet.writeBit(vendorGuid[2]);
                packet.writeBit(vendorGuid[7]);

                for (const auto& item : items)
                    packet.writeBit(item.extendedCostId == 0);

                packet.writeBit(vendorGuid[4]);
                packet.flushBits();

                ByteBuffer itemsData(32 * items.size());
                for (const auto& item : items)
                {
                    itemsData << uint32_t(item.slot);
                    itemsData << uint32_t(item.maxDurability);

                    if (item.extendedCostId != 0)
                        itemsData << uint32_t(item.extendedCostId);

                    itemsData << uint32_t(item.itemId);
                    itemsData << uint32_t(item.type);
                    itemsData << uint32_t(item.price);
                    itemsData << uint32_t(item.displayId);
                    itemsData << int32_t(item.availableAmount);
                    itemsData << uint32_t(item.buyCount);
                }
                packet.append(itemsData);

                packet.writeByteSeq(vendorGuid[5]);
                packet.writeByteSeq(vendorGuid[4]);
                packet.writeByteSeq(vendorGuid[1]);
                packet.writeByteSeq(vendorGuid[0]);
                packet.writeByteSeq(vendorGuid[6]);

                // Unlike Mop, the Cata client's "Reason" byte here is never set to anything but 0,
                // even for an empty list.
                packet << uint8_t(0);

                packet.writeByteSeq(vendorGuid[2]);
                packet.writeByteSeq(vendorGuid[3]);
                packet.writeByteSeq(vendorGuid[7]);

                return true;
            }
            else if (m_protocol.expansion < WoW::Expansion::_Cata)
            {
                packet << uint64_t(vendorGuid);
                packet << uint8_t(items.size());

                for (const auto& item : items)
                {
                    packet << uint32_t(item.slot);
                    packet << uint32_t(item.itemId);
                    packet << uint32_t(item.displayId);
                    packet << uint32_t(item.availableAmount);
                    packet << uint32_t(item.price);
                    packet << uint32_t(item.maxDurability);
                    packet << uint32_t(item.buyCount);
                    packet << uint32_t(item.extendedCostId);
                }

                // If every configured item got filtered out for this player, the client expects a
                // trailing reason byte (0 == no specific reason) after the (still-zero) count byte.
                if (items.size() == 0)
                    packet << uint8_t(0);

                return true;
            }

            return false;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
