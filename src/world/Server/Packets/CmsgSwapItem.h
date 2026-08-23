/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
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

            return false;
        }
    };
}
