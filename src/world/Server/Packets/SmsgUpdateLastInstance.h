/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <cstdint>

#include "ManagedPacket.h"

namespace AscEmu::Packets
{
    class SmsgUpdateLastInstance : public ManagedPacket
    {
    public:
        uint32_t mapId;

        SmsgUpdateLastInstance() : SmsgUpdateLastInstance(0)
        {
        }

        SmsgUpdateLastInstance(uint32_t mapId) :
            ManagedPacket(SMSG_UPDATE_LAST_INSTANCE, 0),
            mapId(mapId)
        {
        }

    protected:
        size_t expectedSize() const override
        {
            return 4;
        }

        bool internalSerialise(WorldPacket& packet) override
        {
            packet << mapId;
            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
