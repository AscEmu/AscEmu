/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"

#include <cstdint>

namespace AscEmu::Packets
{
    class SmsgRecentAllyDataResponse : public ManagedPacket
    {
    public:
        SmsgRecentAllyDataResponse() :
            ManagedPacket(SMSG_RECENT_ALLY_DATA_RESPONSE, 9)
        {
        }

    protected:
        size_t expectedSize() const override
        {
            return 9;
        }

        bool internalSerialise(WorldPacket& packet) override
        {
            if (!m_protocol.isForever())
                return false;

            // Current reference implementation Forever flow (official 70235+):
            // uint32 state, uint8 list type, uint32 entry count.
            packet << uint32_t(0x08ECBD10U);
            packet << uint8_t(7);
            packet << uint32_t(0);
            return true;
        }

        bool internalDeserialise(WorldPacket&) override
        {
            return false;
        }
    };
}
