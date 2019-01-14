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
    class MsgCorspeQuery : public ManagedPacket
    {
    public:
        uint8_t show;
        uint32_t mapId;
        LocationVector position;
        uint32_t corpseMapId;
        uint32_t unknown;

        MsgCorspeQuery() : MsgCorspeQuery(0, 0, {}, 0, 0)
        {
        }

        MsgCorspeQuery(uint8_t show, uint32_t mapId, LocationVector position, uint32_t corpseMapId, uint32_t unknown) :
            ManagedPacket(MSG_CORPSE_QUERY, 25),
            show(show),
            mapId(mapId),
            position(position),
            corpseMapId(corpseMapId),
            unknown(unknown)
        {
        }

        bool internalSerialise(WorldPacket& packet) override
        {
            packet << show << mapId << position << corpseMapId << unknown;
            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override
        {
            return false;
        }
    };
}}
