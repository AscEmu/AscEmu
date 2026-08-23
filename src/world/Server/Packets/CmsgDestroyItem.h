/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include <cstdint>

namespace AscEmu::Packets
{
    class CmsgDestroyItem : public ManagedPacket
    {
    public:
        int8_t srcInventorySlot;
        int8_t srcSlot;

        CmsgDestroyItem() : CmsgDestroyItem(0, 0)
        {
        }

        CmsgDestroyItem(int8_t srcInventorySlot, int8_t srcSlot) :
            ManagedPacket(CMSG_DESTROY_ITEM, 2),
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
                packet.readSkip<int32_t>();     // count, not used - whole stack is always destroyed
                packet >> srcSlot >> srcInventorySlot;
                return true;
            }

            return false;
        }
    };
}
