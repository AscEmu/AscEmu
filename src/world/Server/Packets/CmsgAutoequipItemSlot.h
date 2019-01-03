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
    class CmsgAutoequipItemSlot : public ManagedPacket
    {
    public:
        uint64_t itemGuid;
        int8_t destSlot;

        CmsgAutoequipItemSlot() : CmsgAutoequipItemSlot(0, 0)
        {
        }

        CmsgAutoequipItemSlot(uint64_t itemGuid, int8_t destSlot) :
            ManagedPacket(CMSG_AUTOEQUIP_ITEM_SLOT, 9),
            itemGuid(itemGuid),
            destSlot(destSlot)
        {
        }

    protected:
        bool internalSerialise(WorldPacket& /*packet*/) override
        {
            return false;
        }

        bool internalDeserialise(WorldPacket& packet) override
        {
            packet >> itemGuid >> destSlot;
            return true;
        }
    };
}}
