/*
Copyright (c) 2014-2020 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <cstdint>

#include "ManagedPacket.h"

namespace AscEmu::Packets
{
    class SmsgItemTimeUpdate : public ManagedPacket
    {
    public:
        uint64_t itemGuid;
        uint32_t duration;

        SmsgItemTimeUpdate() : SmsgItemTimeUpdate(0, 0)
        {
        }

        SmsgItemTimeUpdate(uint64_t itemGuid, uint32_t duration) :
            ManagedPacket(SMSG_ITEM_TIME_UPDATE, 12),
            itemGuid(itemGuid),
            duration(duration)
        {
        }

    protected:
        bool internalSerialise(WorldPacket& packet) override
        {
            packet << itemGuid << duration;

            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override
        {
            return false;
        }
    };
}
