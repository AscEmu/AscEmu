/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <cstdint>

#include "ManagedPacket.h"

namespace AscEmu::Packets
{
    class SmsgInstanceReset : public ManagedPacket
    {
    public:
        uint32_t mapId;

        SmsgInstanceReset() : SmsgInstanceReset(0)
        {
        }

        SmsgInstanceReset(uint32_t mapId) :
            ManagedPacket(SMSG_INSTANCE_RESET, 0),
            mapId(mapId)
        {
        }

    protected:
        size_t expectedSize() const override
        {
            return 4;
        }

        bool internalSerialise(WorldPacket& packet) override
        {
            packet << mapId;
            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
