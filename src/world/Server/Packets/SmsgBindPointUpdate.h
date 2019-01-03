/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once
#include <cstdint>

#include "ManagedPacket.h"

namespace AscEmu { namespace Packets
{
    class SmsgBindPointUpdate : public ManagedPacket
    {
        static const size_t PACKET_SIZE = sizeof(float) * 3 + sizeof(uint32_t) * 2;
    public:
        float x;
        float y;
        float z;
        uint32_t map_id;
        uint32_t zone_id;

        SmsgBindPointUpdate() : SmsgBindPointUpdate(0, 0, 0, 0, 0)
        {
        }

        SmsgBindPointUpdate(float x, float y, float z, uint32_t map_id, uint32_t zone_id) :
            ManagedPacket(SMSG_BINDPOINTUPDATE, PACKET_SIZE),
            x(x),
            y(y),
            z(z),
            map_id(map_id),
            zone_id(zone_id)
        {
        }

    protected:
        bool internalSerialise(WorldPacket& packet) override
        {
            // All versions share same implementation
            packet << x << y << z << map_id << zone_id;
            return true;
        }

        bool internalDeserialise(WorldPacket& packet) override
        {
            packet >> x >> y >> z >> map_id >> zone_id;
            return true;
        }
    };
}}
