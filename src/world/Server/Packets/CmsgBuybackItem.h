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
    class CmsgBuybackItem : public ManagedPacket
    {
    public:
        uint64_t itemGuid;
        int32_t buybackSlot;

        CmsgBuybackItem() : CmsgBuybackItem(0, 0)
        {
        }

        CmsgBuybackItem(uint64_t itemGuid, int32_t buybackSlot) :
            ManagedPacket(CMSG_BUYBACK_ITEM, 8),
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
}}
