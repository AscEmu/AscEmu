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
    class CmsgAutoequipItem : public ManagedPacket
    {
    public:
        int8_t srcInventorySlot;
        int8_t srcSlot;

        CmsgAutoequipItem() : CmsgAutoequipItem(0, 0)
        {
        }

        CmsgAutoequipItem(int8_t srcInventorySlot, int8_t srcSlot) :
            ManagedPacket(CMSG_AUTOEQUIP_ITEM, 2),
            srcInventorySlot(srcInventorySlot),
            srcSlot(srcSlot)
        {
        }

    protected:
        bool internalDeserialise(WorldPacket& packet) override
        {
            if (m_protocol.expansion < WoW::Expansion::_Mop)
            {
                packet >> srcInventorySlot >> srcSlot;
                return true;
            }
            else if (m_protocol.isMop())
            {
                packet >> srcSlot >> srcInventorySlot;
                return true;
            }
            else if (m_protocol.expansion == WoW::Expansion::_Forever)
            {
                // Forever 1.60.1.70124: InvUpdate count (2 bits), one
                // container/slot entry, followed by the source location.
                const uint32_t count = packet.readBits(2);
                if (count != 1)
                    return false;

                packet.readSkip<int8_t>(); // container in InvUpdate
                packet.readSkip<int8_t>(); // slot in InvUpdate

                packet >> srcInventorySlot >> srcSlot;

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
