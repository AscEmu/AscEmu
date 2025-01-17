/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <cstdint>

#include "ManagedPacket.h"

namespace AscEmu::Packets
{
    class SmsgTimeSyncRequest : public ManagedPacket
    {
    public:
        uint32_t counter;
        
        SmsgTimeSyncRequest() : SmsgTimeSyncRequest(0)
        {
        }

        SmsgTimeSyncRequest(uint32_t counter) :
            ManagedPacket(SMSG_TIME_SYNC_REQUEST, 4),
            counter(counter)
        {
        }

    protected:
        size_t expectedSize() const override { return 4; }

        bool internalSerialise(WorldPacket& packet) override
        {
            packet << counter;

            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
