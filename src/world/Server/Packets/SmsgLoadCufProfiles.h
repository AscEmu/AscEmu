/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include <cstdint>

namespace AscEmu::Packets
{
    class SmsgLoadCufProfiles : public ManagedPacket
    {
    public:
        SmsgLoadCufProfiles() : ManagedPacket(SMSG_LOAD_CUF_PROFILES, 1)
        {
        }

    protected:
        size_t expectedSize() const override { return 1; }

        bool internalSerialise(WorldPacket& packet) override
        {
            if (m_protocol.isMop())
            {
                packet.writeBits(0, 20);
                packet.flushBits();

                return true;
            }

            return false;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
