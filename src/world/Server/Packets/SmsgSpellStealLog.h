/*
Copyright (c) 2014-2020 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <cstdint>
#include <utility>

#include "ManagedPacket.h"

namespace AscEmu::Packets
{
    class SmsgSpellStealLog : public ManagedPacket
    {
    public:
        uint64_t casterGuid;
        uint64_t targetGuid;
        uint32_t spellId;
        uint8_t unk1;
        uint32_t stealedCount;

        std::list<uint32_t> stealedSpells;

        SmsgSpellStealLog() : SmsgSpellStealLog(0, 0, 0, {})
        {
        }

        SmsgSpellStealLog(uint64_t casterGuid, uint64_t targetGuid, uint32_t spellId, std::list<uint32_t> stealedSpells) :
            ManagedPacket(SMSG_SPELLSTEALLOG, 25 + stealedSpells.size() * 5),
            casterGuid(casterGuid),
            targetGuid(targetGuid),
            spellId(spellId),
            unk1(0),
            stealedCount(stealedSpells.size()),
            stealedSpells(std::move(stealedSpells))
        {
        }

    protected:
        bool internalSerialise(WorldPacket& packet) override
        {
            packet << casterGuid << targetGuid << spellId << unk1 << stealedCount;
            for (const auto stealedSpellId: stealedSpells)
                packet << stealedSpellId << uint8_t(0); // 0 = stealed, else transferred

            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override
        {
            return false;
        }
    };
}
