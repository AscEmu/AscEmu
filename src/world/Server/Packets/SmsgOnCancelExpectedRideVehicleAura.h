/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"

namespace AscEmu::Packets
{
    class SmsgOnCancelExpectedRideVehicleAura : public ManagedPacket
    {
    public:
        SmsgOnCancelExpectedRideVehicleAura() : ManagedPacket(SMSG_ON_CANCEL_EXPECTED_RIDE_VEHICLE_AURA, 0)
        {
        }

    protected:
        size_t expectedSize() const override { return 0; }

        bool internalSerialise(WorldPacket& /*packet*/) override { return true; }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
