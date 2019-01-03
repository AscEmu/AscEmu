/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <cstdint>

#include "ManagedPacket.h"

namespace AscEmu { namespace Packets
{
    class SmsgReadItemOk : public ManagedPacket
    {
    public:
        uint64_t itemGuid;

        SmsgReadItemOk() : SmsgReadItemOk(0)
        {
        }

        SmsgReadItemOk(uint64_t itemGuid) :
            ManagedPacket(SMSG_READ_ITEM_OK, 0),
            itemGuid(itemGuid)
        {
        }

    protected:

        size_t expectedSize() const override { return 8; }

        bool internalSerialise(WorldPacket& packet) override
        {
            packet << itemGuid;
            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}}
