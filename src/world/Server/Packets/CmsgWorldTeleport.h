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
    class CmsgWorldTeleport : public ManagedPacket
    {
    public:
        uint32_t time;
        uint32_t mapId;
        LocationVector location;

        CmsgWorldTeleport() : CmsgWorldTeleport(0, 0, { 0, 0, 0, 0 })
        {
        }

        CmsgWorldTeleport(uint32_t time, uint32_t mapId, LocationVector location) :
            ManagedPacket(CMSG_WORLD_TELEPORT, 4 + 4 + 4 * 4),
            time(time),
            mapId(mapId),
            location(location)
        {
        }

    protected:
        bool internalSerialise(WorldPacket& /*packet*/) override
        {
            return false;
        }

        bool internalDeserialise(WorldPacket& packet) override
        {
            packet >> time >> mapId >> location.x >> location.y >> location.z >> location.o;
            return true;
        }
    };
}}
