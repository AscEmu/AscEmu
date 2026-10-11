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
        uint32_t reason;
        int32_t counter;
        uint64_t instanceId;

        SmsgNewWorld() : SmsgNewWorld(0, {0, 0, 0, 0}, 16, 0, 0)
        {
        }

        SmsgNewWorld(uint32_t mapId, LocationVector location, uint32_t reason = 16, int32_t counter = 0, uint64_t instanceId = 0) :
            ManagedPacket(SMSG_NEW_WORLD, 52),
            mapId(mapId),
            location(location),
            reason(reason),
            counter(counter),
            instanceId(instanceId)
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
            else if (m_protocol.isForever())
            {
                packet << int32_t(mapId);
                packet << location.x << location.y << location.z << location.o;
                packet << int32_t(-1); // FloorDifficulty
                packet << int32_t(-1); // FloorIndex
                packet << reason;
                packet << float(0.0f) << float(0.0f) << float(0.0f); // MovementOffset
                packet << counter;
                packet << instanceId;
                return true;
            }

            return false;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
