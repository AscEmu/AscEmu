/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include <cstdint>

namespace AscEmu::Packets
{
    class SmsgQuestupdateAddKill : public ManagedPacket
    {
    public:
        uint32_t questId;
        uint32_t mobEntry;
        uint32_t count;
        uint32_t tCount;
        WoWGuid guid;

        SmsgQuestupdateAddKill() : SmsgQuestupdateAddKill(0, 0, 0, 0, 0)
        {}

        SmsgQuestupdateAddKill(uint32_t questId, uint32_t mobEntry, uint32_t count, uint32_t tCount, WoWGuid guid) :
            ManagedPacket(SMSG_QUESTUPDATE_ADD_KILL, 4 + 4 + 4 + 4 + 8),
            questId(questId),
            mobEntry(mobEntry),
            count(count),
            tCount(tCount),
            guid(guid)
        {
        }

    protected:
        size_t expectedSize() const override { return m_minimum_size; }

        bool internalSerialise(WorldPacket& packet) override
        {
            packet << questId << mobEntry << count << tCount << guid;

            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
