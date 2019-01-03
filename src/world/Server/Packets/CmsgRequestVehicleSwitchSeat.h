/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once
#include <cstdint>

#include "ManagedPacket.h"
#include "WorldPacket.h"

namespace AscEmu { namespace Packets
{
    class CmsgRequestVehicleSwitchSeat : public ManagedPacket
    {
#if VERSION_STRING > TBC
    public:
        WoWGuid guid;
        uint8_t seat;

        CmsgRequestVehicleSwitchSeat() : CmsgRequestVehicleSwitchSeat(0, 0)
        {
        }

        CmsgRequestVehicleSwitchSeat(uint64_t guid, uint8_t seat) :
            ManagedPacket(CMSG_REQUEST_VEHICLE_SWITCH_SEAT, 0),
            guid(guid),
            seat(seat)
        {
        }

    protected:
        bool internalSerialise(WorldPacket& /*packet*/) override
        {
            return false;
        }

        bool internalDeserialise(WorldPacket& packet) override
        {
            uint64_t unpacked_guid;
            packet >> unpacked_guid >> seat;
            guid.Init(unpacked_guid);
            return true;
        }
#endif
    };
}}
