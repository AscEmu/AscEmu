/*
Copyright (c) 2014-2020 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <cstdint>

#include "ManagedPacket.h"

namespace AscEmu::Packets
{
    class SmsgLootRollWon : public ManagedPacket
    {
    public:
        uint64_t itemGuid;
        uint32_t slotId;
        uint32_t itemId;
        uint32_t randomSuffix;
        uint32_t randomPropertyId;
        uint64_t winnerGuid;
#if VERSION_STRING >=Cata
        uint32_t highestRoll;
#else
        uint8_t highestRoll;
#endif
        uint8_t highestType;

        SmsgLootRollWon() : SmsgLootRollWon(0, 0, 0, 0, 0, 0, 0, 0)
        {
        }

        SmsgLootRollWon(uint64_t itemGuid, uint32_t slotId, uint32_t itemId, uint32_t randomSuffix, uint32_t randomPropertyId, uint64_t winnerGuid, uint8_t highestRoll, uint8_t highestType) :
            ManagedPacket(SMSG_LOOT_ROLL_WON, 34),
            itemGuid(itemGuid),
            slotId(slotId),
            itemId(itemId),
            randomSuffix(randomSuffix),
            randomPropertyId(randomPropertyId),
            winnerGuid(winnerGuid),
            highestRoll(highestRoll),
            highestType(highestType)
        {
        }

    protected:
        bool internalSerialise(WorldPacket& packet) override
        {
            packet << itemGuid << slotId << itemId << randomSuffix << randomPropertyId
                << winnerGuid << highestRoll << highestType;

            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override
        {
            return false;
        }
    };
}
