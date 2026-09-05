/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include <cstdint>

namespace AscEmu::Packets
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
            if (m_protocol.expansion < WoW::Expansion::_Cata)
            {
                packet << mapId << location.x << location.y << location.z << location.o;
                return true;
            }
            else if (m_protocol.isMop())
            {
                packet << location.x << mapId << location.y << location.z << location.o;
                return true;
            }
            else if (m_protocol.isCata())
            {
                packet << location.x << location.o << location.z << mapId << location.y;
                return true;
            }

            return false;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
