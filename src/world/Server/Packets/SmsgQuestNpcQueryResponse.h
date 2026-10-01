/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"

#include <cstdint>
#include <utility>
#include <vector>

namespace AscEmu::Packets
{
    struct QuestNpcQueryEntry
    {
        uint32_t questId = 0;
        // creature entries, gameobject entries carry the 0x80000000 bit
        std::vector<uint32_t> finisherEntries;
    };

    class SmsgQuestNpcQueryResponse : public ManagedPacket
    {
    public:
        std::vector<QuestNpcQueryEntry> quests;

        SmsgQuestNpcQueryResponse() : SmsgQuestNpcQueryResponse(std::vector<QuestNpcQueryEntry>())
        {
        }

        SmsgQuestNpcQueryResponse(std::vector<QuestNpcQueryEntry> quests) :
            ManagedPacket(SMSG_QUEST_NPC_QUERY_RESPONSE, 0),
            quests(std::move(quests))
        {
        }

    protected:
        size_t expectedSize() const override
        {
            if (m_protocol.isForever())
            {
                size_t size = 4;
                for (const auto& quest : quests)
                    size += 8 + 4 * quest.finisherEntries.size();
                return size;
            }

            size_t size = 3;
            for (const auto& quest : quests)
                size += 4 + 4 * quest.finisherEntries.size();
            return size;
        }

        bool internalSerialise(WorldPacket& packet) override
        {
            if (m_protocol.isForever())
            {
                packet << static_cast<uint32_t>(quests.size());
                for (const auto& quest : quests)
                {
                    packet << static_cast<int32_t>(quest.questId);
                    packet << static_cast<uint32_t>(quest.finisherEntries.size());
                    for (const auto entry : quest.finisherEntries)
                        packet << entry;
                }
                return true;
            }

            if (m_protocol.isMop())
            {
                packet.writeBits(quests.size(), 21);
                for (const auto& quest : quests)
                    packet.writeBits(quest.finisherEntries.size(), 22);

                packet.flushBits();

                for (const auto& quest : quests)
                {
                    packet << quest.questId;
                    for (const auto entry : quest.finisherEntries)
                        packet << entry;
                }

                return true;
            }

            return false;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
