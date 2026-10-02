/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include "world/Data/InventoryLayout.hpp"
#include <cstdint>

namespace AscEmu::Packets
{
    class CmsgSplitItem : public ManagedPacket
    {
    public:
        int8_t destInventorySlot;
        int8_t destSlot;
        int8_t srcInventorySlot;
        int8_t srcSlot;
        int32_t itemCount;

        CmsgSplitItem() : CmsgSplitItem(0, 0, 0, 0, 0)
        {
        }

        CmsgSplitItem(int8_t srcInventorySlot, int8_t srcSlot, int8_t destInventorySlot, int8_t destSlot, uint32_t itemCount) :
            ManagedPacket(CMSG_SPLIT_ITEM, 8),
            destInventorySlot(destInventorySlot),
            destSlot(destSlot),
            srcInventorySlot(srcInventorySlot),
            srcSlot(srcSlot),
            itemCount(itemCount)
        {
        }

    protected:
        bool internalDeserialise(WorldPacket& packet) override
        {
            if (m_protocol.expansion < WoW::Expansion::_Mop)
            {
                packet >> srcInventorySlot >> srcSlot >> destInventorySlot >> destSlot >> itemCount;
                return true;
            }
            else if (m_protocol.isMop())
            {
                packet >> srcInventorySlot;
                packet >> itemCount;
                packet >> destInventorySlot >> srcSlot >> destSlot;
                return true;
            }
            else if (m_protocol.isForever())
            {
                // Forever 1.60.1.70124: empty InvUpdate (2-bit item count),
                // source container/slot, destination container/slot, quantity.
                if (packet.readBits(2) != 0)
                    return false;

                packet >> srcInventorySlot >> srcSlot >> destInventorySlot >> destSlot >> itemCount;

                if (srcInventorySlot == InventoryLayout::SlotNotSet)
                {
                    const int16_t logicalSlot = InventoryLayout::Forever::logicalSlot(static_cast<uint8_t>(srcSlot));
                    if (logicalSlot == InventoryLayout::NoSlotAvailable)
                        return false;
                    srcSlot = static_cast<int8_t>(logicalSlot);
                }
                else
                {
                    const int16_t logicalContainerSlot = InventoryLayout::Forever::logicalSlot(static_cast<uint8_t>(srcInventorySlot));
                    if (logicalContainerSlot == InventoryLayout::NoSlotAvailable)
                        return false;
                    srcInventorySlot = static_cast<int8_t>(logicalContainerSlot);
                }

                if (destInventorySlot == InventoryLayout::SlotNotSet)
                {
                    const int16_t logicalSlot = InventoryLayout::Forever::logicalSlot(static_cast<uint8_t>(destSlot));
                    if (logicalSlot == InventoryLayout::NoSlotAvailable)
                        return false;
                    destSlot = static_cast<int8_t>(logicalSlot);
                }
                else
                {
                    const int16_t logicalContainerSlot = InventoryLayout::Forever::logicalSlot(static_cast<uint8_t>(destInventorySlot));
                    if (logicalContainerSlot == InventoryLayout::NoSlotAvailable)
                        return false;
                    destInventorySlot = static_cast<int8_t>(logicalContainerSlot);
                }

                return true;
            }

            return false;
        }
    };
}
