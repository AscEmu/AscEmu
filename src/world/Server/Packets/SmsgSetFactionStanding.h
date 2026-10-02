/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"

#include <cstdint>

namespace AscEmu::Packets
{
    class SmsgSetFactionStanding : public ManagedPacket
    {
    public:
        uint32_t unknown;
        uint8_t isIncreased;
        uint32_t count;
        uint32_t listId;
        int32_t standingChange;
        int32_t standing;

        SmsgSetFactionStanding() : SmsgSetFactionStanding(0, 0, 0)
        {
        }

        SmsgSetFactionStanding(uint32_t listId, int32_t standing) : SmsgSetFactionStanding(listId, 0, standing)
        {
        }

        SmsgSetFactionStanding(uint32_t listId, int32_t standingChange, int32_t standing) :
            ManagedPacket(SMSG_SET_FACTION_STANDING, 4),
            unknown(0),
            isIncreased(1),
            count(1),
            listId(listId),
            standingChange(standingChange),
            standing(standing)
        {
        }

    protected:
        size_t expectedSize() const override { return m_protocol.isForever() ? size_t(25) : m_minimum_size; }

        bool internalSerialise(WorldPacket& packet) override
        {
            if (m_protocol.isForever())
            {
                // Forever 1.60.1.70124, observed after quest reputation gains:
                // float 0.0, uint32 count, then per entry int32 reputationIndex,
                // int32 standingChange, int32 standing, int32 unknown(0), followed by one flushed visual bit.
                packet << float(0.0f);
                packet << count;
                packet << static_cast<int32_t>(listId);
                packet << standingChange;
                packet << standing;
                packet << int32_t(0);
                packet.writeBit(true);
                packet.flushBits();

                return true;
            }

            packet << unknown << isIncreased << count << listId << static_cast<uint32_t>(standing);

            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
