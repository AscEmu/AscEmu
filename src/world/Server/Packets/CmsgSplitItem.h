/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <cstdint>

#include "ManagedPacket.h"
#include "WorldPacket.h"

namespace AscEmu { namespace Packets
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
            srcInventorySlot(srcInventorySlot),
            srcSlot(srcSlot),
            destInventorySlot(destInventorySlot),
            destSlot(destSlot),
            itemCount(itemCount)
        {
        }

    protected:
        bool internalSerialise(WorldPacket& /*packet*/) override
        {
            return false;
        }

        bool internalDeserialise(WorldPacket& packet) override
        {
            packet >> srcInventorySlot >> srcSlot >> destInventorySlot >> destSlot >> itemCount;
            return true;
        }
    };
}}
