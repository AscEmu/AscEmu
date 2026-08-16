/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include <cstdint>

namespace AscEmu::Packets
{
    class SmsgSetProjectilePosition : public ManagedPacket
    {
    public:
        uint64_t casterGuid;
        uint8_t castCount;
        float x;
        float y;
        float z;

        SmsgSetProjectilePosition() : SmsgSetProjectilePosition(0, 0, 0.0f, 0.0f, 0.0f)
        {
        }

        SmsgSetProjectilePosition(uint64_t casterGuid, uint8_t castCount, float x, float y, float z) :
            ManagedPacket(SMSG_SET_PROJECTILE_POSITION, 21),
            casterGuid(casterGuid),
            castCount(castCount),
            x(x),
            y(y),
            z(z)
        {
        }

    protected:
        size_t expectedSize() const override { return 21; }

        bool internalSerialise(WorldPacket& packet) override
        {
            packet << uint64_t(casterGuid);
            packet << uint8_t(castCount);
            packet << float(x);
            packet << float(y);
            packet << float(z);
            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
