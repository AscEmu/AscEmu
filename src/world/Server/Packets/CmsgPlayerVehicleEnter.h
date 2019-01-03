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
    class CmsgPlayerVehicleEnter : public ManagedPacket
    {
#if VERSION_STRING > TBC
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
        bool internalSerialise(WorldPacket& /*packet*/) override
        {
            return false;
        }

        bool internalDeserialise(WorldPacket& packet) override
        {
            packet >> guid;
            return true;
        }
#endif
    };
}}
