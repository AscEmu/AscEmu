/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <cstdint>

#include "ManagedPacket.h"

namespace AscEmu { namespace Packets
{
    class SmsgReadItemFailed : public ManagedPacket
    {
    public:
        uint64_t itemGuid;
        uint8_t error;

        SmsgReadItemFailed() : SmsgReadItemFailed(0, 0)
        {
        }

        SmsgReadItemFailed(uint64_t itemGuid, uint8_t error) :
            ManagedPacket(SMSG_READ_ITEM_FAILED, 0),
            itemGuid(itemGuid),
            error(error)
        {
        }

    protected:

        size_t expectedSize() const override { return 8 + 1; }

        bool internalSerialise(WorldPacket& packet) override
        {
            packet << itemGuid << error;
            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}}
