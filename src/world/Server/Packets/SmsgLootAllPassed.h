/*
Copyright (c) 2014-2020 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <cstdint>

#include "ManagedPacket.h"

namespace AscEmu::Packets
{
    class SmsgLootAllPassed : public ManagedPacket
    {
    public:
        uint64_t guid;
        uint32_t groupCount;
        uint32_t itemId;
        uint32_t randomSuffix;
        uint32_t randomPropertyId;

        SmsgLootAllPassed() : SmsgLootAllPassed(0, 0, 0, 0, 0)
        {
        }

        SmsgLootAllPassed(uint64_t guid, uint32_t groupCount, uint32_t itemId, uint32_t randomSuffix, uint32_t randomPropertyId) :
            ManagedPacket(SMSG_LOOT_ALL_PASSED, 24),
            guid(guid),
            groupCount(groupCount),
            itemId(itemId),
            randomSuffix(randomSuffix),
            randomPropertyId(randomPropertyId)
        {
        }

    protected:
        bool internalSerialise(WorldPacket& packet) override
        {
            packet << guid << groupCount << itemId << randomSuffix << randomPropertyId;

            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override
        {
            return false;
        }
    };
}
