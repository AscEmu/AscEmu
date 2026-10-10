/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include <cstdint>

namespace AscEmu::Packets
{
    // the server time the 10.x client asks for on the glue screen
    class SmsgServerTimeOffset : public ManagedPacket
    {
    public:
        int64_t time;

        SmsgServerTimeOffset() : SmsgServerTimeOffset(0) {}

        explicit SmsgServerTimeOffset(int64_t time) :
            ManagedPacket(SMSG_SERVER_TIME_OFFSET, 8),
            time(time)
        {}

    protected:
        size_t expectedSize() const override { return sizeof(time); }

        bool internalSerialise(WorldPacket& packet) override
        {
            if (!m_protocol.isDragonflight())
                return false;

            packet << time;
            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
