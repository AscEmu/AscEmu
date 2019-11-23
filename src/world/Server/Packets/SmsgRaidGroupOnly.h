/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <cstdint>

#include "ManagedPacket.h"

namespace AscEmu::Packets
{
    class SmsgRaidGroupOnly : public ManagedPacket
    {
    public:
        uint32_t timeInMs;
        uint32_t type;

        SmsgRaidGroupOnly() : SmsgRaidGroupOnly(0, 0)
        {
        }

        SmsgRaidGroupOnly(uint32_t timeInMs, uint32_t type) :
            ManagedPacket(SMSG_RAID_GROUP_ONLY, 0),
            timeInMs(timeInMs),
            type(type)
        {
        }

    protected:
        size_t expectedSize() const override { return 4 + 4; }

        bool internalSerialise(WorldPacket& packet) override
        {
            packet << timeInMs << type;

            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override
        {
            return false;
        }
    };
}
