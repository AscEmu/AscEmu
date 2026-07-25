/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include <cstdint>

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
        uint32_t rollNumber;
        uint8_t rollType;
        uint8_t autoPass;

        SmsgLootRoll() : SmsgLootRoll(0, 0, 0, 0, 0, 0, 0, 0, 0)
        {
        }

        SmsgLootRoll(uint64_t itemGuid, uint32_t slotId, uint64_t playerGuid, uint32_t itemId,
            uint32_t randomSuffix, uint32_t randomPropertyId, uint32_t rollNumber,
            uint8_t rollType, uint8_t autoPass = 0) :
            ManagedPacket(SMSG_LOOT_ROLL, 0),
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
        size_t expectedSize() const override
        {
            return m_protocol.expansion < WoW::Expansion::_Cata ? 35 : 38;
        }

        bool internalSerialise(WorldPacket& packet) override
        {
            packet << itemGuid << slotId << playerGuid << itemId << randomSuffix << randomPropertyId;

            if (m_protocol.expansion < WoW::Expansion::_Cata)
                packet << static_cast<uint8_t>(rollNumber);
            else
                packet << rollNumber;

            packet << rollType << autoPass;
            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
