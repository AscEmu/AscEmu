/*
Copyright (c) 2014-2020 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once
#include <cstdint>

#include "ManagedPacket.h"

namespace AscEmu::Packets
{
    class SmsgControlVehicle : public ManagedPacket
    {
#if VERSION_STRING > TBC
    public:

        SmsgControlVehicle() :
            ManagedPacket(SMSG_CONTROL_VEHICLE, 0)  // SMSG_ON_CANCEL_EXPECTED_RIDE_VEHICLE_AURA
        {
        }

    protected:
        size_t expectedSize() const override { return m_minimum_size; }

        bool internalSerialise(WorldPacket& packet) override
        {
            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
#endif
    };
}
