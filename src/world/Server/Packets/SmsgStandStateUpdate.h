/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <cstdint>

#include "ManagedPacket.h"

namespace AscEmu::Packets
{
    class SmsgStandStateUpdate : public ManagedPacket
    {
    public:
        uint8_t state;

        SmsgStandStateUpdate() : SmsgStandStateUpdate(0)
        {
        }

        SmsgStandStateUpdate(uint8_t state) :
            ManagedPacket(SMSG_STAND_STATE_UPDATE, 1),
            state(state)
        {
        }

    protected:
        bool internalSerialise(WorldPacket& packet) override
        {
            packet << state;
            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override
        {
            return false;
        }
    };
}
