/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include <cstdint>

namespace AscEmu::Packets
{
    class SmsgPlayerVehicleData : public ManagedPacket
    {
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
            if (m_protocol.expansion > WoW::Expansion::_TBC)
            {
                packet << targetGuid << vehicleId;
                return true;
            }
            else
            {
                return false;
            }
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
