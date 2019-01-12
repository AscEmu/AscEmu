/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once
#include <cstdint>

#include "ManagedPacket.h"

namespace AscEmu { namespace Packets
{
    class SmsgNewWorld : public ManagedPacket
    {
    public:
        uint32_t mapId;
        LocationVector location;

        SmsgNewWorld() : SmsgNewWorld(0, {0, 0, 0, 0})
        {
        }

        SmsgNewWorld(uint32_t mapId, LocationVector location) :
            ManagedPacket(SMSG_NEW_WORLD, 20),
            mapId(mapId),
            location(location)
        {
        }

    protected:
        size_t expectedSize() const override { return m_minimum_size; }

        bool internalSerialise(WorldPacket& packet) override
        {
#if VERSION_STRING < Cata
            packet << mapId << location.x << location.y << location.z << location.o;
#else
            packet << location.x << location.o << location.z << mapId << location.y;
#endif
            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}}
