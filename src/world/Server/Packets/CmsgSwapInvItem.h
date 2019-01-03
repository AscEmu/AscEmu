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
        bool internalSerialise(WorldPacket& /*packet*/) override
        {
            return false;
        }

        bool internalDeserialise(WorldPacket& packet) override
        {
            packet >> destSlot >> srcSlot;
            return true;
        }
    };
}}
