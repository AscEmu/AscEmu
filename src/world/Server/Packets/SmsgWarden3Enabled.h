/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"

#include <cstdint>

namespace AscEmu::Packets
{
    class SmsgWarden3Enabled : public ManagedPacket
    {
    public:
        explicit SmsgWarden3Enabled(uint32_t state = 0x00000200U) :
            ManagedPacket(SMSG_WARDEN3_ENABLED, sizeof(uint32_t)), state(state) {}

        uint32_t state;

    protected:
        bool internalSerialise(WorldPacket& packet) override
        {
            if (!m_protocol.isForever())
                return false;
            packet << state;
            return true;
        }

        bool internalDeserialise(WorldPacket&) override { return false; }
    };
}
