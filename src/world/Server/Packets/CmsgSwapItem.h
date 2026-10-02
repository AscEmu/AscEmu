/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include "Data/InventoryLayout.hpp"
#include <cstdint>

namespace AscEmu::Packets
{
    class CmsgSwapItem : public ManagedPacket
    {
    public:
        int8_t destInventorySlot;
        int8_t destSlot;
        int8_t srcInventorySlot;
        int8_t srcSlot;

        CmsgSwapItem() : CmsgSwapItem(0, 0, 0, 0)
        {
        }

        CmsgSwapItem(int8_t destInventorySlot, int8_t destSlot, int8_t srcInventorySlot, int8_t srcSlot) :
            ManagedPacket(CMSG_SWAP_ITEM, 4),
            destInventorySlot(destInventorySlot),
            destSlot(destSlot),
            srcInventorySlot(srcInventorySlot),
            srcSlot(srcSlot)
        {
        }

    protected:
        bool internalDeserialise(WorldPacket& packet) override
        {
            if (m_protocol.expansion < WoW::Expansion::_Mop)
            {
                packet >> destInventorySlot >> destSlot >> srcInventorySlot >> srcSlot;
                return true;
            }
            else if (m_protocol.isMop())
            {
                int8_t srcSlotAlt = 0, srcInventorySlotAlt = 0, destInventorySlotAlt = 0, destSlotAlt = 0;
                packet >> srcSlotAlt >> srcInventorySlotAlt >> destInventorySlotAlt >> destSlotAlt;

                const uint32_t count = packet.readBits(2);
                if (count != 2)
                    return false;

                bool hasSlot[2] = {};
                bool hasBag[2] = {};
                for (uint8_t i = 0; i < 2; ++i)
                {
                    hasSlot[i] = !packet.readBit();
                    hasBag[i] = !packet.readBit();
                }

                destInventorySlot = hasBag[0] ? packet.read<int8_t>() : destInventorySlotAlt;
                destSlot = hasSlot[0] ? packet.read<int8_t>() : destSlotAlt;
                srcInventorySlot = hasBag[1] ? packet.read<int8_t>() : srcInventorySlotAlt;
                srcSlot = hasSlot[1] ? packet.read<int8_t>() : srcSlotAlt;
                return true;
            }
            else if (m_protocol.isForever())
            {
                // Forever 1.60.1.70124: InvUpdate count (2 bits), two
                // container/slot entries, then container A/container B/slot A/slot B.
                const uint32_t count = packet.readBits(2);
                if (count != 2)
                    return false;

                packet.readSkip<int8_t>(); // destination container in InvUpdate
                packet.readSkip<int8_t>(); // destination slot in InvUpdate
                packet.readSkip<int8_t>(); // source container in InvUpdate
                packet.readSkip<int8_t>(); // source slot in InvUpdate

                packet >> destInventorySlot >> srcInventorySlot >> destSlot >> srcSlot;

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

                return true;
            }

            return false;
        }
    };
}
