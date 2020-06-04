/*
Copyright (c) 2014-2020 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <cstdint>
#include <utility>

#include "ManagedPacket.h"

namespace AscEmu::Packets
{
    class SmsgPlayerVehicleData : public ManagedPacket
    {
#if VERSION_STRING > TBC
    public:
        WoWGuid targetGuid;
        uint32_t vehicleId;

        SmsgPlayerVehicleData() : SmsgPlayerVehicleData(WoWGuid(), 0)
        {
        }

        SmsgPlayerVehicleData(WoWGuid targetGuid, uint32_t vehicleId) :
            ManagedPacket(SMSG_PLAYER_VEHICLE_DATA, 8 + 4),
            targetGuid(targetGuid),
            vehicleId(vehicleId)
        {
        }

    protected:
        bool internalSerialise(WorldPacket& packet) override
        {
            packet << targetGuid << vehicleId;

            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override
        {
            return false;
        }
#endif
    };
}
