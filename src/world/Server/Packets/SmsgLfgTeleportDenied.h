/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include <cstdint>

namespace AscEmu::Packets
{
    class SmsgLfgTeleportDenied : public ManagedPacket
    {
    public:
        uint32_t error;

        SmsgLfgTeleportDenied() : SmsgLfgTeleportDenied(0)
        {
        }

        SmsgLfgTeleportDenied(uint32_t error) :
            ManagedPacket(SMSG_LFG_TELEPORT_DENIED, 0),
            error(error)
        {
        }

    protected:

        size_t expectedSize() const override { return 4; }

        bool internalSerialise(WorldPacket& packet) override
        {
            if (m_protocol.isMop())
            {
                packet.writeBits(error, 4);
                packet.flushBits();

                return true;
            }
            else if (m_protocol.expansion > WoW::Expansion::_TBC)
            {
                packet << error;
                return true;
            }

            return false;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
