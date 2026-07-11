/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include <cstdint>

namespace AscEmu::Packets
{
    class CmsgPlayerVehicleEnter : public ManagedPacket
    {
    public:
        uint64_t guid;

        CmsgPlayerVehicleEnter() : CmsgPlayerVehicleEnter(0)
        {
        }

        CmsgPlayerVehicleEnter(uint64_t guid) :
            ManagedPacket(CMSG_PLAYER_VEHICLE_ENTER, 0),
            guid(guid)
        {
        }

    protected:
        bool internalDeserialise(WorldPacket& packet) override
        {
            packet >> guid;
            return true;
        }
    };
}
