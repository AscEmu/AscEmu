/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once
#include <cstdint>

#include "ManagedPacket.h"

namespace AscEmu { namespace Packets
{
    class SmsgDeathReleaseLoc : public ManagedPacket
    {
    public:
        uint32_t mapId;
        LocationVector location;

        SmsgDeathReleaseLoc() : SmsgDeathReleaseLoc(0, { 0, 0, 0 })
        {
        }

        SmsgDeathReleaseLoc(uint32_t mapId, LocationVector location) :
            ManagedPacket(SMSG_DEATH_RELEASE_LOC, 0),
            mapId(mapId),
            location(location)
        {
        }

    protected:
        size_t expectedSize() const override
        {
            return 4 + 4 + 4 + 4;
        }

        bool internalSerialise(WorldPacket& packet) override
        {
            packet << mapId << location;
            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}}
