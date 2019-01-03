/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once
#include <cstdint>

#include "ManagedPacket.h"

namespace AscEmu { namespace Packets
{
    class CmsgOpenItem : public ManagedPacket
    {
    public:
        uint8_t containerSlot;
        uint8_t slot;

        CmsgOpenItem() : CmsgOpenItem(0, 0)
        {
        }

        CmsgOpenItem(uint8_t containerSlot, uint8_t slot) :
            ManagedPacket(CMSG_OPEN_ITEM, 2),
            containerSlot(containerSlot),
            slot(slot)
        {
        }

    protected:
        bool internalSerialise(WorldPacket& /*packet*/) override
        {
            return false;
        }

        bool internalDeserialise(WorldPacket& packet) override
        {
            packet >> containerSlot >> slot;
            return true;
        }
    };
}}
