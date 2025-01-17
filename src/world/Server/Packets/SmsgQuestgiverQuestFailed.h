/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <cstdint>

#include "ManagedPacket.h"

namespace AscEmu::Packets
{
    class SmsgQuestgiverQuestFailed : public ManagedPacket
    {
    public:
        uint32_t questId;
        uint32_t reason;

        SmsgQuestgiverQuestFailed() : SmsgQuestgiverQuestFailed(0, 0)
        {
        }

        SmsgQuestgiverQuestFailed(uint32_t questId, uint32_t reason) :
            ManagedPacket(SMSG_QUESTGIVER_QUEST_FAILED, 4 + 4),
            questId(questId),
            reason(reason)
        {
        }

    protected:
        size_t expectedSize() const override { return m_minimum_size; }

        bool internalSerialise(WorldPacket& packet) override
        {
            packet << questId << reason;

            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
