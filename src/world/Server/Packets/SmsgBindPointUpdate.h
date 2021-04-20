/*
Copyright (c) 2014-2021 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once
#include <cstdint>

#include "ManagedPacket.h"

namespace AscEmu::Packets
{
    class SmsgBindPointUpdate : public ManagedPacket
    {
        static const size_t PACKET_SIZE = sizeof(float) * 3 + sizeof(uint32_t) * 2;
    public:
        LocationVector pos;
        uint32_t map_id;
        uint32_t zone_id;

        SmsgBindPointUpdate() : SmsgBindPointUpdate({0, 0, 0}, 0, 0)
        {
        }

        SmsgBindPointUpdate(LocationVector pos, uint32_t map_id, uint32_t zone_id) :
            ManagedPacket(SMSG_BINDPOINTUPDATE, PACKET_SIZE),
            pos(pos),
            map_id(map_id),
            zone_id(zone_id)
        {
        }

    protected:
        bool internalSerialise(WorldPacket& packet) override
        {
#if VERSION_STRING != Mop
            packet << pos.x << pos.y << pos.z << map_id << zone_id;
#else
            packet << pos.x << pos.z << pos.y << zone_id << map_id;
#endif
            return true;
        }

        bool internalDeserialise(WorldPacket& packet) override
        {
            packet >> pos.x >> pos.y >> pos.z >> map_id >> zone_id;
            return true;
        }
    };
}
