/*
Copyright (c) 2014-2018 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <cstdint>

#include "ManagedPacket.h"
#include "WorldPacket.h"

namespace AscEmu { namespace Packets
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
        bool internalSerialise(WorldPacket& /*packet*/) override
        {
            return false;
        }

        bool internalDeserialise(WorldPacket& packet) override
        {
            packet >> destInventorySlot >> destSlot >> srcInventorySlot >> srcSlot;
            return true;
        }
    };
}}
