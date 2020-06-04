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
    class SmsgSpellDispellLog : public ManagedPacket
    {
    public:
        uint64_t casterGuid;
        uint64_t targetGuid;
        uint32_t spellId;
        uint8_t unk1;
        uint32_t displellCount;

        std::list<uint32_t> dispellSpells;

        SmsgSpellDispellLog() : SmsgSpellDispellLog(0, 0, 0, {})
        {
        }

        SmsgSpellDispellLog(uint64_t casterGuid, uint64_t targetGuid, uint32_t spellId, std::list<uint32_t> dispellSpells) :
            ManagedPacket(SMSG_SPELLDISPELLOG, 25 + dispellSpells.size() * 5),
            casterGuid(casterGuid),
            targetGuid(targetGuid),
            spellId(spellId),
            unk1(0),
            displellCount(dispellSpells.size()),
            dispellSpells(std::move(dispellSpells))
        {
        }

    protected:
        bool internalSerialise(WorldPacket& packet) override
        {
            packet << casterGuid << targetGuid << spellId << unk1 << displellCount;
            for (const auto stealedSpellId: dispellSpells)
                packet << stealedSpellId << uint8_t(0); // 0 = dispelled, else cleansed

            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override
        {
            return false;
        }
    };
}
