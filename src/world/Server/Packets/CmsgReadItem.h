/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once
#include <cstdint>

#include "ManagedPacket.h"

namespace AscEmu { namespace Packets
{
    class CmsgReadItem : public ManagedPacket
    {
    public:
        int8_t srcContainerSlot;
        int8_t srcSlot;

        CmsgReadItem() : CmsgReadItem(0, 0)
        {
        }

        CmsgReadItem(int8_t srcContainerSlot, uint8_t srcSlot) :
            ManagedPacket(CMSG_READ_ITEM, 2),
            srcContainerSlot(srcContainerSlot),
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
            packet >> srcContainerSlot >> srcSlot;
            return true;
        }
    };
}}
