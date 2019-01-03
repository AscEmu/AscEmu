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
    class CmsgWrapItem : public ManagedPacket
    {
    public:
        int8_t srcBagSlot;
        int8_t srcSlot;
        int8_t destBagSlot;
        int8_t destSlot;

        CmsgWrapItem() : CmsgWrapItem(0, 0, 0, 0)
        {
        }

        CmsgWrapItem(int8_t srcBagSlot, int8_t srcSlot, int8_t destBagSlot, int8_t destSlot) :
            ManagedPacket(CMSG_WRAP_ITEM, 4),
            srcBagSlot(srcBagSlot),
            srcSlot(srcSlot),
            destBagSlot(destBagSlot),
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
            packet >> srcBagSlot >> srcSlot >> destBagSlot >> destSlot;
            return true;
        }
    };
}}
