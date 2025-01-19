/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <cstdint>

#include "ManagedPacket.h"
#include "WorldPacket.h"

namespace AscEmu::Packets
{
    class CmsgBuyBackItem : public ManagedPacket
    {
    public:
        uint64_t itemGuid;
        int32_t buybackSlot;

        CmsgBuyBackItem() : CmsgBuyBackItem(0, 0)
        {
        }

        CmsgBuyBackItem(uint64_t itemGuid, int32_t buybackSlot) :
            ManagedPacket(CMSG_BUY_BACK_ITEM, 8),
            itemGuid(itemGuid),
            buybackSlot(buybackSlot)
        {
        }

    protected:
        bool internalSerialise(WorldPacket& /*packet*/) override
        {
            return false;
        }

        bool internalDeserialise(WorldPacket& packet) override
        {
            packet >> itemGuid >> buybackSlot;
            return true;
        }
    };
}
