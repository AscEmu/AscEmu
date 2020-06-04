/*
Copyright (c) 2014-2020 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <cstdint>

#include "ManagedPacket.h"

namespace AscEmu::Packets
{
    class SmsgClearTarget : public ManagedPacket
    {
    public:
        uint64_t casterGuid;

        SmsgClearTarget() : SmsgClearTarget(0)
        {
        }

        SmsgClearTarget(uint64_t casterGuid) :
            ManagedPacket(SMSG_CLEAR_TARGET, 12),
            casterGuid(casterGuid)
        {
        }

    protected:
        bool internalSerialise(WorldPacket& packet) override
        {
            packet << casterGuid;

            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override
        {
            return false;
        }
    };
}
