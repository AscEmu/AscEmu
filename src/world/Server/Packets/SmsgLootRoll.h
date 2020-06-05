/*
Copyright (c) 2014-2020 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <cstdint>

#include "ManagedPacket.h"

namespace AscEmu::Packets
{
    class SmsgLootRoll : public ManagedPacket
    {
    public:
        uint64_t itemGuid;
        uint32_t slotId;
        uint64_t playerGuid;
        uint32_t itemId;
        uint32_t randomSuffix;
        uint32_t randomPropertyId;
        
#if VERSION_STRING >=Cata
        uint32_t rollNumber;
#else
        uint8_t rollNumber;
#endif
        uint8_t rollType;
        uint8_t autoPass;

        SmsgLootRoll() : SmsgLootRoll(0, 0, 0, 0, 0, 0, 0, 0, 0)
        {
        }

        SmsgLootRoll(uint64_t itemGuid, uint32_t slotId, uint64_t playerGuid, uint32_t itemId, uint32_t randomSuffix, uint32_t randomPropertyId, uint8_t rollNumber, uint8_t rollType, uint8_t autoPass = 0) :
            ManagedPacket(SMSG_LOOT_ROLL, 35),
            itemGuid(itemGuid),
            slotId(slotId),
            playerGuid(playerGuid),
            itemId(itemId),
            randomSuffix(randomSuffix),
            randomPropertyId(randomPropertyId),
            rollNumber(rollNumber),
            rollType(rollType),
            autoPass(autoPass)
        {
        }

    protected:
        bool internalSerialise(WorldPacket& packet) override
        {
            packet << itemGuid << slotId << playerGuid << itemId << randomSuffix << randomPropertyId
                << rollNumber << rollType << autoPass;

            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override
        {
            return false;
        }
    };
}
