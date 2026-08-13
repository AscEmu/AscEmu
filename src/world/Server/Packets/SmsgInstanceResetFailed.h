/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include <cstdint>

namespace AscEmu::Packets
{
    class SmsgInstanceResetFailed : public ManagedPacket
    {
    public:
        uint32_t reason;
        uint32_t mapId;

        SmsgInstanceResetFailed() : SmsgInstanceResetFailed(0, 0)
        {
        }

        SmsgInstanceResetFailed(uint32_t reason, uint32_t mapId) :
            ManagedPacket(SMSG_INSTANCE_RESET_FAILED, 4 + 4),
            reason(reason),
            mapId(mapId)
        {
        }

    protected:
        size_t expectedSize() const override{ return 4 + 4; }

        bool internalSerialise(WorldPacket& packet) override
        {
            if (m_protocol.expansion <= WoW::Expansion::_Cata)
            {
                packet << reason << mapId;
                return true;
            }
            else if (m_protocol.isMop())
            {
                packet.writeBits(reason, 2);

                packet.flushBits();

                packet << mapId;

                return true;
            }

            return false;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
