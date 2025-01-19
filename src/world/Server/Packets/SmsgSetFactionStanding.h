/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <cstdint>

#include "ManagedPacket.h"

namespace AscEmu::Packets
{
    class SmsgSetFactionStanding : public ManagedPacket
    {
    public:
        uint32_t unknown;
        uint8_t isIncreased;
        uint32_t count;
        uint32_t listId;
        uint32_t standing;

        SmsgSetFactionStanding() : SmsgSetFactionStanding(0, 0)
        {
        }

        SmsgSetFactionStanding(uint32_t listId, uint32_t standing) :
            ManagedPacket(SMSG_SET_FACTION_STANDING, 4),
            unknown(0),
            isIncreased(1),
            count(1),
            listId(listId),
            standing(standing)
        {
        }

    protected:
        size_t expectedSize() const override { return m_minimum_size; }

        bool internalSerialise(WorldPacket& packet) override
        {
            packet << unknown << isIncreased << count << listId << standing;

            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
