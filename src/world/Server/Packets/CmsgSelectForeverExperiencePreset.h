/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"

namespace AscEmu::Packets
{
    class CmsgSelectForeverExperiencePreset : public ManagedPacket
    {
    public:
        bool classic = false;

        CmsgSelectForeverExperiencePreset() : ManagedPacket(CMSG_SELECT_FOREVER_EXPERIENCE_PRESET, 1)
        {
        }

    protected:
        bool internalSerialise(WorldPacket& /*packet*/) override { return false; }

        bool internalDeserialise(WorldPacket& packet) override
        {
            if (!m_protocol.isForever())
                return false;

            // the classic experience bit
            classic = packet.readBit();
            return !packet.hadReadFailure();
        }
    };
}
