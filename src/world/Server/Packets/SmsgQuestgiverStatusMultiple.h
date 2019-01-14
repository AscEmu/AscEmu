/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once
#include <cstdint>
#include <utility>

#include "ManagedPacket.h"
#include "Units/Creatures/Creature.h"

struct QuestgiverInrangeStatus
{
    uint64_t rawGuid;
    uint8_t status;
};

namespace AscEmu { namespace Packets
{
    class SmsgQuestgiverStatusMultiple : public ManagedPacket
    {
    public:
        uint32_t inrangeCount;
        std::vector<QuestgiverInrangeStatus> questgiverSet;

        SmsgQuestgiverStatusMultiple() : SmsgQuestgiverStatusMultiple(0, {})
        {
        }

        SmsgQuestgiverStatusMultiple(uint32_t inrangeCount, std::vector<QuestgiverInrangeStatus> questgiverSet) :
            ManagedPacket(SMSG_QUESTGIVER_STATUS_MULTIPLE, 1000),
            inrangeCount(inrangeCount),
            questgiverSet(questgiverSet)
        {
        }

    protected:
        size_t expectedSize() const override { return m_minimum_size; }

        bool internalSerialise(WorldPacket& packet) override
        {
            packet << inrangeCount;

            for (const auto& questGiver : questgiverSet)
                packet << questGiver.rawGuid << questGiver.status;

            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}}
