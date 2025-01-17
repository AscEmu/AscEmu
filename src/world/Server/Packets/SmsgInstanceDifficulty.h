/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <cstdint>

#include "ManagedPacket.h"
#include "Map/Maps/InstanceDefines.hpp"

namespace AscEmu::Packets
{
    class SmsgInstanceDifficulty : public ManagedPacket
    {
    public:
        uint32_t difficulty;
        uint32_t isHeroic;

        SmsgInstanceDifficulty() : SmsgInstanceDifficulty(0)
        {
        }

        SmsgInstanceDifficulty(uint32_t difficulty) :
            ManagedPacket(SMSG_INSTANCE_DIFFICULTY, 0),
            difficulty(difficulty),
            isHeroic(difficulty == InstanceDifficulty::RAID_10MAN_HEROIC || difficulty == InstanceDifficulty::RAID_25MAN_HEROIC)
        {
        }

    protected:
        size_t expectedSize() const override
        {
            return 4 + 4;
        }

        bool internalSerialise(WorldPacket& packet) override
        {
            packet << difficulty << isHeroic;
            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
