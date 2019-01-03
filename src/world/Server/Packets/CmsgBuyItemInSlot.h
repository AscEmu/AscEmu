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
    class CmsgBuyItemInSlot : public ManagedPacket
    {
    public:
        WoWGuid srcGuid;
        uint32_t itemId;
        int32_t vendorSlot;
        uint64_t bagGuid;
        int8_t slot;
        uint8_t amount;

        CmsgBuyItemInSlot() : CmsgBuyItemInSlot(0, 0, 0, 0, 0, 0)
        {
        }

        CmsgBuyItemInSlot(uint64_t srcGuid, uint32_t itemId, int32_t vendorSlot, uint64_t bagGuid, int8_t slot, uint8_t amount) :
            ManagedPacket(CMSG_BUY_ITEM_IN_SLOT, 24),
            srcGuid(srcGuid),
            itemId(itemId),
            vendorSlot(vendorSlot),
            bagGuid(bagGuid),
            slot(slot),
            amount(amount)
        {
        }

    protected:
        bool internalSerialise(WorldPacket& /*packet*/) override
        {
            return false;
        }

        bool internalDeserialise(WorldPacket& packet) override
        {
            uint64_t unpackedGuid;
            packet >> unpackedGuid >> itemId >> vendorSlot >> bagGuid >> slot >> amount;
            srcGuid.Init(unpackedGuid);
            return true;
        }
    };
}}
