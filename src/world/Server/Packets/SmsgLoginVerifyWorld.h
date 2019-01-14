/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once
#include <cstdint>

#include "ManagedPacket.h"

namespace AscEmu { namespace Packets
{
    class SmsgLoginVerifyWorld : public ManagedPacket
    {
    public:
        uint32_t mapId;
        LocationVector location;

        SmsgLoginVerifyWorld() : SmsgLoginVerifyWorld(0, {0, 0, 0, 0})
        {
        }

        SmsgLoginVerifyWorld(uint32_t mapId, LocationVector location) :
            ManagedPacket(SMSG_LOGIN_VERIFY_WORLD, 0),
            mapId(mapId),
            location(location)
        {
        }

    protected:
        size_t expectedSize() const override
        {
            return 4 + 4 + 4 + 4 + 4;
        }

        bool internalSerialise(WorldPacket& packet) override
        {
#if VERSION_STRING != Mop
            packet << mapId << location.x << location.y << location.z << location.o;
#elif VERSION_STRING == Mop
            packet << location.x << location.o << location.y << mapId << location.z;
#endif
            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}}
