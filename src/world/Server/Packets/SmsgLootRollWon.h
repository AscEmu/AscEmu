/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include <cstdint>

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
        uint32_t highestRoll;
        uint8_t highestType;

        SmsgLootRollWon() : SmsgLootRollWon(0, 0, 0, 0, 0, 0, 0, 0)
        {
        }

        SmsgLootRollWon(uint64_t itemGuid, uint32_t slotId, uint32_t itemId, uint32_t randomSuffix,
            uint32_t randomPropertyId, uint64_t winnerGuid, uint32_t highestRoll, uint8_t highestType) :
            ManagedPacket(SMSG_LOOT_ROLL_WON, 0),
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
        size_t expectedSize() const override
        {
            return m_protocol.expansion < WoW::Expansion::_Cata ? 33 : 36;
        }

        bool internalSerialise(WorldPacket& packet) override
        {
            packet << itemGuid << slotId << itemId << randomSuffix << randomPropertyId << winnerGuid;

            if (m_protocol.expansion < WoW::Expansion::_Cata)
                packet << static_cast<uint8_t>(highestRoll);
            else
                packet << highestRoll;

            packet << highestType;
            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
