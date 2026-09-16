/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include "AchievementDataEntries.h"

#include <vector>

namespace AscEmu::Packets
{
    class SmsgAllAchievementData : public ManagedPacket
    {
    public:
        WoWGuid guid;
        std::vector<CriteriaProgressEntry> criteriaProgress;
        std::vector<CompletedAchievementEntry> completedAchievements;

        SmsgAllAchievementData(WoWGuid guid, std::vector<CriteriaProgressEntry> criteriaProgress, std::vector<CompletedAchievementEntry> completedAchievements) :
            ManagedPacket(SMSG_ALL_ACHIEVEMENT_DATA, 4),
            guid(guid), criteriaProgress(std::move(criteriaProgress)), completedAchievements(std::move(completedAchievements))
        {
        }

    protected:
        size_t expectedSize() const override
        {
            const size_t numCriteria = criteriaProgress.size();
            const size_t numAchievements = completedAchievements.size();

            return 4 + numAchievements * (4 + 4) + 4 + numCriteria * (4 + 4 + 4 + 4 + 8 + 8);
        }

        bool internalSerialise(WorldPacket& packet) override
        {
            if (m_protocol.isCata())
            {
                ByteBuffer criteriaData(criteriaProgress.size() * (4 + 4 + 4 + 4 + 8 + 8));
                WoWGuid counter;

                packet.writeBits(criteriaProgress.size(), 21);

                for (const auto& progress : criteriaProgress)
                {
                    counter = progress.counter;

                    packet.writeBit(guid[4]);
                    packet.writeBit(counter[3]);
                    packet.writeBit(guid[5]);
                    packet.writeBit(counter[0]);
                    packet.writeBit(counter[6]);
                    packet.writeBit(guid[3]);
                    packet.writeBit(guid[0]);
                    packet.writeBit(counter[4]);
                    packet.writeBit(guid[2]);
                    packet.writeBit(counter[7]);
                    packet.writeBit(guid[7]);
                    packet.writeBits(0u, 2);
                    packet.writeBit(guid[6]);
                    packet.writeBit(counter[2]);
                    packet.writeBit(counter[1]);
                    packet.writeBit(counter[5]);
                    packet.writeBit(guid[1]);

                    criteriaData.writeByteSeq(guid[3]);
                    criteriaData.writeByteSeq(counter[5]);
                    criteriaData.writeByteSeq(counter[6]);
                    criteriaData.writeByteSeq(guid[4]);
                    criteriaData.writeByteSeq(guid[6]);
                    criteriaData.writeByteSeq(counter[2]);
                    criteriaData << uint32_t(0);    // timer 2
                    criteriaData.writeByteSeq(guid[2]);

                    criteriaData << uint32_t(progress.criteriaId);   // criteria id
                    criteriaData.writeByteSeq(guid[5]);
                    criteriaData.writeByteSeq(counter[0]);
                    criteriaData.writeByteSeq(counter[3]);
                    criteriaData.writeByteSeq(counter[1]);
                    criteriaData.writeByteSeq(counter[4]);
                    criteriaData.writeByteSeq(guid[0]);
                    criteriaData.writeByteSeq(guid[7]);
                    criteriaData.writeByteSeq(counter[7]);
                    criteriaData << uint32_t(0); // timer 1
                    criteriaData.appendPackedTime(progress.date);   // criteria date
                    criteriaData.writeByteSeq(guid[1]);
                }

                packet.writeBits(completedAchievements.size(), 23);
                packet.flushBits();
                packet.append(criteriaData);

                for (const auto& completed : completedAchievements)
                {
                    packet << uint32_t(completed.achievementId);
                    packet.appendPackedTime(completed.date);
                }

                return true;
            }
            else if (m_protocol.isMop())
            {
                const size_t numAchievements = completedAchievements.size();

                ByteBuffer criteriaData(criteriaProgress.size() * (4 + 4 + 4 + 4 + 8 + 8));
                ByteBuffer completedData(numAchievements * (4 + 4 + 4 + 4 + 8));
                WoWGuid counter;

                packet.writeBits(criteriaProgress.size(), 19);

                for (const auto& progress : criteriaProgress)
                {
                    counter = progress.counter;

                    packet.writeBit(counter[3]);
                    packet.writeBit(guid[3]);
                    packet.writeBit(guid[6]);
                    packet.writeBit(counter[0]);
                    packet.writeBit(guid[7]);
                    packet.writeBit(counter[1]);
                    packet.writeBit(counter[5]);
                    packet.writeBit(guid[2]);
                    packet.writeBit(guid[1]);
                    packet.writeBit(counter[7]);
                    packet.writeBit(guid[4]);
                    packet.writeBit(guid[0]);
                    packet.writeBit(counter[2]);
                    packet.writeBit(guid[5]);
                    packet.writeBit(counter[4]);
                    packet.writeBits(0, 4);
                    packet.writeBit(counter[6]);

                    criteriaData.writeByteSeq(counter[7]);
                    criteriaData << uint32_t(0);                                // timer 1
                    criteriaData.writeByteSeq(counter[6]);
                    criteriaData.writeByteSeq(guid[1]);
                    criteriaData << uint32_t(progress.criteriaId);               // criteria id
                    criteriaData.writeByteSeq(counter[4]);
                    criteriaData.writeByteSeq(guid[0]);
                    criteriaData.writeByteSeq(guid[4]);
                    criteriaData.writeByteSeq(guid[6]);
                    criteriaData.writeByteSeq(counter[1]);
                    criteriaData.writeByteSeq(counter[5]);
                    criteriaData.writeByteSeq(guid[7]);
                    criteriaData.writeByteSeq(guid[2]);
                    criteriaData.writeByteSeq(counter[2]);
                    criteriaData.writeByteSeq(counter[0]);
                    criteriaData.writeByteSeq(guid[3]);
                    criteriaData.writeByteSeq(counter[3]);
                    criteriaData << uint32_t(0);                                // timer 2
                    criteriaData.writeByteSeq(guid[5]);
                    criteriaData.appendPackedTime(progress.date);   // criteria date
                }

                packet.writeBits(numAchievements, 20);
                for (const auto& completed : completedAchievements)
                {
                    packet.writeBit(guid[0]);
                    packet.writeBit(guid[7]);
                    packet.writeBit(guid[1]);
                    packet.writeBit(guid[5]);
                    packet.writeBit(guid[2]);
                    packet.writeBit(guid[4]);
                    packet.writeBit(guid[6]);
                    packet.writeBit(guid[3]);

                    completedData << uint32_t(completed.achievementId);              // achievement Id
                    completedData << uint32_t(1);
                    completedData.writeByteSeq(guid[5]);
                    completedData.writeByteSeq(guid[7]);
                    completedData << uint32_t(1);
                    completedData.appendPackedTime(completed.date);        // achievement date
                    completedData.writeByteSeq(guid[0]);
                    completedData.writeByteSeq(guid[4]);
                    completedData.writeByteSeq(guid[1]);
                    completedData.writeByteSeq(guid[6]);
                    completedData.writeByteSeq(guid[2]);
                    completedData.writeByteSeq(guid[3]);
                }

                packet.flushBits();
                packet.append(completedData);
                packet.append(criteriaData);

                return true;
            }

            return false;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
