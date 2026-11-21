/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include <cstdint>

namespace AscEmu::Packets
{
    class CmsgSetForeverExperiencePreset : public ManagedPacket
    {
    public:
        uint32_t value = 0;
        bool classic = false;

        CmsgSetForeverExperiencePreset() : ManagedPacket(CMSG_SET_FOREVER_EXPERIENCE_PRESET, 5)
        {
        }

    protected:
        bool internalSerialise(WorldPacket& /*packet*/) override { return false; }

        bool internalDeserialise(WorldPacket& packet) override
        {
            if (!m_protocol.isForever())
                return false;

            // a value of unknown meaning, the classic experience bit
            packet >> value;
            classic = packet.readBit();
            return !packet.hadReadFailure();
        }
    };
}
