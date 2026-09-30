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
    class CmsgSwapInvItem : public ManagedPacket
    {
    public:
        int8_t destSlot;
        int8_t srcSlot;

        CmsgSwapInvItem() : CmsgSwapInvItem(0, 0)
        {
        }

        CmsgSwapInvItem(int8_t destSlot, int8_t srcSlot) :
            ManagedPacket(CMSG_SWAP_INV_ITEM, 2),
            destSlot(destSlot),
            srcSlot(srcSlot)
        {
        }

    protected:
        bool internalDeserialise(WorldPacket& packet) override
        {
            if (m_protocol.expansion <= WoW::Expansion::_TBC)
            {
                packet >> srcSlot >> destSlot;
                return true;
            }
            else if (m_protocol.expansion <= WoW::Expansion::_Cata)
            {
                packet >> destSlot >> srcSlot;
                return true;
            }
            else if (m_protocol.isMop())
            {
                packet >> srcSlot >> destSlot;
                return true;
            }
            else if (m_protocol.expansion == WoW::Expansion::Forever)
            {
                // Forever 1.60.1.70124: InvUpdate count (2 bits), two
                // container/slot entries, then destination/source slots.
                const uint32_t count = packet.readBits(2);
                if (count != 2)
                    return false;

                packet.readSkip<int8_t>(); // destination container
                packet.readSkip<int8_t>(); // destination slot in InvUpdate
                packet.readSkip<int8_t>(); // source container
                packet.readSkip<int8_t>(); // source slot in InvUpdate

                uint8_t wireDestSlot = 0;
                uint8_t wireSrcSlot = 0;
                packet >> wireDestSlot >> wireSrcSlot;

                const int16_t logicalDestSlot = InventoryLayout::Forever::logicalSlot(wireDestSlot);
                const int16_t logicalSrcSlot = InventoryLayout::Forever::logicalSlot(wireSrcSlot);
                if (logicalDestSlot == InventoryLayout::NoSlotAvailable || logicalSrcSlot == InventoryLayout::NoSlotAvailable) return false;

                destSlot = static_cast<int8_t>(logicalDestSlot);
                srcSlot = static_cast<int8_t>(logicalSrcSlot);
                return true;
            }

            return false;
        }
    };
}
