/*
Copyright (c) 2014-2020 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <cstdint>

#include "ManagedPacket.h"

namespace AscEmu::Packets
{
    class SmsgPong : public ManagedPacket
    {
    public:
        uint32_t ping;

        SmsgPong() : SmsgPong(0)
        {
        }

        SmsgPong(uint32_t ping) :
            ManagedPacket(SMSG_PONG, 4),
            ping(ping)
        {
        }

    protected:
        size_t expectedSize() const override { return m_minimum_size; }

        bool internalSerialise(WorldPacket& packet) override
        {
            packet << ping;

            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
