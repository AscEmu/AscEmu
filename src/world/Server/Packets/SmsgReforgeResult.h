/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include <cstdint>

namespace AscEmu::Packets
{
    class SmsgReforgeResult : public ManagedPacket
    {
    public:
        bool success;

        SmsgReforgeResult() : SmsgReforgeResult(false)
        {
        }

        SmsgReforgeResult(bool success) :
            ManagedPacket(SMSG_REFORGE_RESULT, 1),
            success(success)
        {
        }

    protected:
        size_t expectedSize() const override { return 1; }

        bool internalSerialise(WorldPacket& packet) override
        {
            if (m_protocol.isMop() || m_protocol.isCata())
            {
                packet.writeBit(success);
                packet.flushBits();
                return true;
            }

            return false;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
