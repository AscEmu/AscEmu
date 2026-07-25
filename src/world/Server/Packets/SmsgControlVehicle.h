/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"

namespace AscEmu::Packets
{
    class SmsgControlVehicle : public ManagedPacket
    {
    public:

        SmsgControlVehicle() :
            ManagedPacket(SMSG_CONTROL_VEHICLE, 0)  // SMSG_ON_CANCEL_EXPECTED_RIDE_VEHICLE_AURA
        {
        }

    protected:
        size_t expectedSize() const override { return m_minimum_size; }

        bool internalSerialise(WorldPacket& /*packet*/) override
        {
            if (m_protocol.expansion <= WoW::Expansion::_TBC)
                return false;

            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
