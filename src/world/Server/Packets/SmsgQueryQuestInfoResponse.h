/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include "QuestPacketCommon.h"

#include <cstdint>
#include <string>
#include <vector>

namespace AscEmu::Packets
{
    // 6.x and later: the quest as the client caches it
    struct QueryQuestInfoInput
    {
        uint32_t questId = 0;
        int32_t questLevel = 0;
        int32_t questMinLevel = 0;
        int32_t questSortId = 0;
        int32_t questInfoId = 0;
        int32_t suggestedGroupNum = 0;
        int32_t rewardNextQuest = 0;
        int32_t rewardXpDifficulty = 0;
        int32_t rewardMoney = 0;
        int32_t rewardBonusMoney = 0;
        int32_t rewardDisplaySpell = 0;
        int32_t rewardSpell = 0;
        int32_t rewardHonor = 0;
        int32_t startItem = 0;
        uint32_t flags = 0;
        QuestRewardItemEntry rewardItems[4];
        QuestRewardItemEntry choiceItems[6];
        int32_t poiContinent = 0;
        float poiX = 0.0f;
        float poiY = 0.0f;
        int32_t poiPriority = 0;
        int32_t rewardTitle = 0;
        int32_t rewardTalents = 0;
        int32_t rewardArenaPoints = 0;
        int32_t factionId[5] = {};
        int32_t factionValue[5] = {};
        int32_t currencyId[4] = {};
        int32_t currencyCount[4] = {};
        int32_t timeAllowed = 0;
        std::vector<QuestObjectiveEntry> objectives;
        std::string logTitle;
        std::string logDescription;
        std::string questDescription;
        std::string areaDescription;
    };

    class SmsgQueryQuestInfoResponse : public ManagedPacket
    {
    public:
        QueryQuestInfoInput input;

        SmsgQueryQuestInfoResponse() : SmsgQueryQuestInfoResponse(QueryQuestInfoInput{})
        {
        }

        explicit SmsgQueryQuestInfoResponse(QueryQuestInfoInput input) :
            ManagedPacket(SMSG_QUEST_QUERY_RESPONSE, 0),
            input(std::move(input))
        {
        }

    protected:
        size_t expectedSize() const override { return 1000; }

        void writeObjective(WorldPacket& packet, QuestObjectiveEntry const& objective) const
        {
            packet << uint32_t(objective.id);
            if (m_protocol.isTheWarWithinOrLater())
                packet << int32_t(objective.type);
            else
                packet << uint8_t(objective.type);
            packet << int8_t(objective.storageIndex);
            packet << int32_t(objective.objectId);
            packet << int32_t(objective.amount);
            if (m_protocol.isTheWarWithinOrLater())
                packet << int32_t(0);               // secondary amount
            packet << uint32_t(0);                  // flags
            if (!m_protocol.isWoD())
                packet << uint32_t(0);              // flags 2
            packet << float(0.0f);                  // progress bar weight
            packet << int32_t(0);                   // visual effects
            if (m_protocol.isTheWarWithinOrLater())
                packet << int32_t(0);               // parent objective
            packet.writeBits(static_cast<uint32_t>(objective.description.length()), 8);
            if (m_protocol.isTheWarWithinOrLater())
                packet.writeBit(true);              // visible
            packet.flushBits();
            packet.writeString(objective.description);
        }

        void writeTextLengths(WorldPacket& packet) const
        {
            packet.writeBits(static_cast<uint32_t>(input.logTitle.length()), 9);
            packet.writeBits(static_cast<uint32_t>(input.logDescription.length()), 12);
            packet.writeBits(static_cast<uint32_t>(input.questDescription.length()), 12);
            packet.writeBits(static_cast<uint32_t>(input.areaDescription.length()), 9);
            packet.writeBits(0, 10);                // portrait giver text
            packet.writeBits(0, 8);                 // portrait giver name
            packet.writeBits(0, 10);                // portrait turn in text
            packet.writeBits(0, 8);                 // portrait turn in name
            packet.writeBits(0, 11);                // completion log
            if (m_protocol.isTheWarWithinOrLater())
                packet.writeBit(false);             // reset by scheduler
            if (m_protocol.isShadowlands() || m_protocol.isDragonflight() || m_protocol.isTheWarWithinOrLater())
                packet.writeBit(false);             // ready for translation
            packet.flushBits();
        }

        void writeTexts(WorldPacket& packet) const
        {
            packet.writeString(input.logTitle);
            packet.writeString(input.logDescription);
            packet.writeString(input.questDescription);
            packet.writeString(input.areaDescription);
        }

        bool internalSerialise(WorldPacket& packet) override
        {
            if (!(m_protocol.isWoD() || m_protocol.isLegion() || m_protocol.isBfA() || m_protocol.isShadowlands() || m_protocol.isDragonflight() || m_protocol.isTheWarWithinOrLater()))
                return false;

            const bool isWoD = m_protocol.isWoD();
            const bool isLegion = m_protocol.isLegion();
            const bool isBfA = m_protocol.isBfA();
            const bool contentTuning = !isWoD && !isLegion && !isBfA;        // 9.x and later
            const bool isTheWarWithinOrLater = m_protocol.isTheWarWithinOrLater();
            const bool isMidnight = m_protocol.isMidnight();

            packet << uint32_t(input.questId);
            packet.writeBit(true);                  // allow
            packet.flushBits();

            packet << int32_t(input.questId);
            packet << int32_t(2);                   // quest type: normal
            if (contentTuning)
            {
                packet << int32_t(0);               // quest package
                packet << int32_t(0);               // content tuning
            }
            else
            {
                packet << int32_t(input.questLevel);
                if (isBfA)
                    packet << int32_t(0);           // scaling faction group
                if (!isWoD)
                    packet << int32_t(255);         // max scaling level
                packet << int32_t(0);               // quest package
                packet << int32_t(input.questMinLevel);
            }
            packet << int32_t(input.questSortId);
            packet << int32_t(input.questInfoId);
            packet << int32_t(input.suggestedGroupNum);
            packet << int32_t(input.rewardNextQuest);
            packet << int32_t(input.rewardXpDifficulty);
            packet << float(1.0f);                  // xp multiplier
            packet << int32_t(input.rewardMoney);
            packet << int32_t(0);                   // money difficulty
            packet << float(1.0f);                  // money multiplier
            packet << int32_t(input.rewardBonusMoney);

            if (isWoD)
            {
                packet << int32_t(input.rewardDisplaySpell);
            }
            else if (isLegion || isBfA)
            {
                packet << int32_t(input.rewardDisplaySpell);
                packet << int32_t(0);
                packet << int32_t(0);
            }
            else
            {
                packet << uint32_t(0);              // display spells, listed further down
            }

            packet << int32_t(input.rewardSpell);
            packet << int32_t(input.rewardHonor);
            packet << float(0.0f);                  // kill honor
            if (isMidnight)
                packet << int32_t(0);               // favor
            if (!isWoD)
            {
                packet << int32_t(0);               // artifact xp
                packet << float(1.0f);              // artifact xp multiplier
                packet << int32_t(0);               // artifact category
            }
            packet << int32_t(input.startItem);
            packet << uint32_t(input.flags);
            packet << uint32_t(0);                  // flags 2
            if (!isWoD && !isLegion)
                packet << uint32_t(0);              // flags 3
            if (isTheWarWithinOrLater)
                packet << uint32_t(0);              // flags 4

            for (const auto& item : input.rewardItems)
            {
                packet << int32_t(item.itemId);
                packet << int32_t(item.count);
                packet << int32_t(0);               // item drop
                packet << int32_t(0);               // item drop quantity
            }

            for (const auto& item : input.choiceItems)
            {
                packet << int32_t(item.itemId);
                packet << int32_t(item.count);
                packet << int32_t(item.displayId);
            }

            packet << int32_t(input.poiContinent);
            packet << float(input.poiX);
            packet << float(input.poiY);
            packet << int32_t(input.poiPriority);
            packet << int32_t(input.rewardTitle);
            if (isWoD)
                packet << int32_t(input.rewardTalents);
            packet << int32_t(input.rewardArenaPoints);
            packet << int32_t(0);                   // skill line
            packet << int32_t(0);                   // skill ups
            packet << int32_t(0);                   // portrait giver
            if (!isWoD && !isLegion)
                packet << int32_t(0);               // portrait giver mount
            if (contentTuning)
                packet << int32_t(0);               // portrait giver model scene
            packet << int32_t(0);                   // portrait turn in

            for (uint8_t i = 0; i < 5; ++i)
            {
                packet << int32_t(input.factionId[i]);
                packet << int32_t(0);               // value from the reward table
                packet << int32_t(input.factionValue[i] * 100);
                if (!isWoD)
                    packet << int32_t(0);           // cap
            }
            packet << int32_t(0);                   // faction flags

            for (uint8_t i = 0; i < 4; ++i)
            {
                packet << int32_t(input.currencyId[i]);
                packet << int32_t(input.currencyCount[i]);
            }

            packet << int32_t(0);                   // accept sound
            packet << int32_t(0);                   // complete sound
            packet << int32_t(0);                   // area group
            if (m_protocol.isDragonflight() || isTheWarWithinOrLater)
                packet << int64_t(input.timeAllowed);
            else
                packet << int32_t(input.timeAllowed);

            if (isWoD)
            {
                packet << int32_t(input.objectives.size());
                packet << int32_t(-1);              // allowable races
                for (const auto& objective : input.objectives)
                    writeObjective(packet, objective);
                writeTextLengths(packet);
                writeTexts(packet);
                return true;
            }

            packet << uint32_t(input.objectives.size());
            if (isMidnight)
            {
                packet << int32_t(-1);              // allowable races
                packet << int32_t(-1);
            }
            else
            {
                packet << uint64_t(UINT64_C(0xFFFFFFFFFFFFFFFF));  // allowable races
            }

            if (isLegion)
            {
                packet << int32_t(0);               // reward id
                packet << int32_t(0);               // expansion
                writeTextLengths(packet);
                for (const auto& objective : input.objectives)
                    writeObjective(packet, objective);
                writeTexts(packet);
                return true;
            }

            if (isTheWarWithinOrLater)
            {
                packet << uint32_t(0);              // treasure pickers
                packet << uint32_t(0);              // second treasure pickers
            }
            else
            {
                packet << int32_t(0);               // treasure picker
            }
            packet << int32_t(0);                   // expansion
            packet << int32_t(0);                   // managed world state
            packet << int32_t(0);                   // quest session bonus
            if (m_protocol.isDragonflight() || isTheWarWithinOrLater)
            {
                packet << int32_t(0);               // quest giver creature
                packet << uint32_t(0);              // conditional descriptions
                packet << uint32_t(0);              // conditional completion logs
            }
            if (isTheWarWithinOrLater)
            {
                packet << uint32_t(0);              // house rooms
                packet << uint32_t(0);              // house decors
            }

            if (isMidnight)
            {
                // 12.x: objectives before the text lengths
                for (const auto& objective : input.objectives)
                    writeObjective(packet, objective);
                writeTextLengths(packet);
                writeTexts(packet);
                return true;
            }

            writeTextLengths(packet);
            for (const auto& objective : input.objectives)
                writeObjective(packet, objective);
            writeTexts(packet);
            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
