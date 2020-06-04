/*
Copyright (c) 2014-2020 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <cstdint>

#include "ManagedPacket.h"

namespace AscEmu::Packets
{
    class SmsgSpellLogMiss : public ManagedPacket
    {
    public:
        uint32_t spellId;
        uint64_t casterGuid;
        uint8_t unknown1;
        uint32_t targetCount;
        uint64_t targetGuid;
        uint8_t logType;

        SmsgSpellLogMiss() : SmsgSpellLogMiss(0, 0, 0, 0)
        {
        }

        SmsgSpellLogMiss(uint32_t spellId, uint64_t casterGuid, uint64_t targetGuid, uint8_t logType) :
            ManagedPacket(SMSG_SPELLLOGMISS, 4 + 8 + 1 + 4 + 8 + 1),
            spellId(spellId),
            casterGuid(casterGuid),
            unknown1(0),
            targetCount(1),
            targetGuid(targetGuid),
            logType(logType)
        {
        }

    protected:
        bool internalSerialise(WorldPacket& packet) override
        {
            packet << spellId << casterGuid << unknown1 << targetCount << targetGuid << logType;

            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override
        {
            return false;
        }
    };
}
