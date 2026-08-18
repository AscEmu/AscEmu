/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include "Management/AchievementMgr.h"
#include "Storage/WDB/WDBStores.hpp"
#include "Storage/WDB/WDBStructures.hpp"
#include <algorithm>

#if VERSION_STRING >= WotLK
namespace AscEmu::Packets
{
    class SmsgRespondInspectAchievements : public ManagedPacket
    {
    public:
        WoWGuid guid;
        CriteriaProgressMap const* criteriaProgress = nullptr;
        CompletedAchievementMap const* completedAchievements = nullptr;

        SmsgRespondInspectAchievements(WoWGuid guid, CriteriaProgressMap const& criteriaProgress,
            CompletedAchievementMap const& completedAchievements) :
            ManagedPacket(SMSG_RESPOND_INSPECT_ACHIEVEMENTS, 1),
            guid(guid), criteriaProgress(&criteriaProgress), completedAchievements(&completedAchievements)
        {
        }

    protected:
        static bool isVisibleAchievement(CompletedAchievementMap::value_type const& completedAchievementPair)
        {
            auto achievement = sAchievementStore.lookupEntry(completedAchievementPair.first);
            return achievement && !(achievement->flags & ACHIEVEMENT_FLAG_HIDDEN);
        }

        size_t expectedSize() const override
        {
            const size_t numCriteria = criteriaProgress->size();
            const size_t numAchievements = std::count_if(completedAchievements->begin(), completedAchievements->end(), isVisibleAchievement);

            return 1 + 8 + 3 + 3 + numAchievements * (4 + 4) + numCriteria * (0);
        }

        bool internalSerialise(WorldPacket& packet) override
        {
            if (m_protocol.isCata())
            {
                const size_t numCriteria = criteriaProgress->size();
                const size_t numAchievements = std::count_if(completedAchievements->begin(), completedAchievements->end(), isVisibleAchievement);
                ByteBuffer criteriaData(numCriteria * (0));
                WoWGuid counter;

                packet.writeBit(guid[7]);
                packet.writeBit(guid[4]);
                packet.writeBit(guid[1]);
                packet.writeBits(numAchievements, 23);
                packet.writeBit(guid[0]);
                packet.writeBit(guid[3]);
                packet.writeBits(numCriteria, 21);
                packet.writeBit(guid[2]);

                for (const auto& progressIter : *criteriaProgress)
                {
                    WDB::Structures::AchievementCriteriaEntry const* acEntry = sAchievementCriteriaStore.lookupEntry(progressIter.first);
                    if (!acEntry)
                        continue;

                    if (!sAchievementStore.lookupEntry(acEntry->referredAchievement))
                        continue;

                    counter = uint64_t(progressIter.second->counter);

                    packet.writeBit(counter[5]);
                    packet.writeBit(counter[3]);
                    packet.writeBit(guid[1]);
                    packet.writeBit(guid[4]);
                    packet.writeBit(guid[2]);
                    packet.writeBit(counter[6]);
                    packet.writeBit(guid[0]);
                    packet.writeBit(counter[4]);
                    packet.writeBit(counter[1]);
                    packet.writeBit(counter[2]);
                    packet.writeBit(guid[3]);
                    packet.writeBit(guid[7]);
                    packet.writeBits(0, 2);   // criteria progress flags
                    packet.writeBit(counter[0]);
                    packet.writeBit(guid[5]);
                    packet.writeBit(guid[6]);
                    packet.writeBit(counter[7]);

                    criteriaData.writeByteSeq(guid[3]);
                    criteriaData.writeByteSeq(counter[4]);
                    criteriaData << uint32_t(0);    // timer 1
                    criteriaData.writeByteSeq(guid[1]);
                    criteriaData.appendPackedTime(progressIter.second->date);
                    criteriaData.writeByteSeq(counter[3]);
                    criteriaData.writeByteSeq(counter[7]);
                    criteriaData.writeByteSeq(guid[5]);
                    criteriaData.writeByteSeq(counter[0]);
                    criteriaData.writeByteSeq(guid[4]);
                    criteriaData.writeByteSeq(guid[2]);
                    criteriaData.writeByteSeq(guid[6]);
                    criteriaData.writeByteSeq(guid[7]);
                    criteriaData.writeByteSeq(counter[6]);
                    criteriaData << uint32_t(progressIter.first);
                    criteriaData << uint32_t(0);    // timer 2
                    criteriaData.writeByteSeq(counter[1]);
                    criteriaData.writeByteSeq(counter[5]);
                    criteriaData.writeByteSeq(guid[0]);
                    criteriaData.writeByteSeq(counter[2]);
                }

                packet.writeBit(guid[6]);
                packet.writeBit(guid[5]);
                packet.flushBits();
                packet.append(criteriaData);
                packet.writeByteSeq(guid[1]);
                packet.writeByteSeq(guid[6]);
                packet.writeByteSeq(guid[3]);
                packet.writeByteSeq(guid[0]);
                packet.writeByteSeq(guid[2]);

                for (auto completeIter : *completedAchievements)
                {
                    if (!isVisibleAchievement(completeIter))
                        continue;

                    packet << uint32_t(completeIter.first);
                    packet.appendPackedTime(completeIter.second);
                }

                packet.writeByteSeq(guid[7]);
                packet.writeByteSeq(guid[4]);
                packet.writeByteSeq(guid[5]);

                return true;
            }
            else if (m_protocol.isMop())
            {
                const size_t numCriteria = criteriaProgress->size();
                const size_t numAchievements = std::count_if(completedAchievements->begin(), completedAchievements->end(), isVisibleAchievement);
                ByteBuffer criteriaData(numCriteria * 32);
                ByteBuffer achievementsData(numAchievements * 24);
                WoWGuid counter;

                packet.writeBit(guid[3]);
                packet.writeBit(guid[6]);
                packet.writeBit(guid[0]);
                packet.writeBit(guid[2]);
                packet.writeBits(numAchievements, 20);
                packet.writeBits(numCriteria, 19);

                for (const auto& progressIter : *criteriaProgress)
                {
                    counter = uint64_t(progressIter.second->counter);

                    packet.writeBit(guid[1]);
                    packet.writeBit(guid[4]);
                    packet.writeBit(guid[5]);
                    packet.writeBit(counter[7]);
                    packet.writeBit(counter[4]);
                    packet.writeBit(counter[3]);
                    packet.writeBit(guid[7]);
                    packet.writeBit(guid[0]);
                    packet.writeBit(guid[6]);
                    packet.writeBits(0, 4);   // criteria progress flags
                    packet.writeBit(guid[2]);
                    packet.writeBit(counter[5]);
                    packet.writeBit(counter[6]);
                    packet.writeBit(counter[0]);
                    packet.writeBit(counter[2]);
                    packet.writeBit(counter[1]);
                    packet.writeBit(guid[3]);

                    criteriaData.writeByteSeq(counter[4]);
                    criteriaData << uint32_t(0);    // timer 1
                    criteriaData.writeByteSeq(counter[1]);
                    criteriaData.writeByteSeq(guid[1]);
                    criteriaData.writeByteSeq(counter[7]);
                    criteriaData << uint32_t(progressIter.first);
                    criteriaData.writeByteSeq(guid[3]);
                    criteriaData.writeByteSeq(counter[3]);
                    criteriaData.writeByteSeq(counter[5]);
                    criteriaData.writeByteSeq(counter[2]);
                    criteriaData.writeByteSeq(guid[4]);
                    criteriaData.writeByteSeq(counter[0]);
                    criteriaData.writeByteSeq(guid[0]);
                    criteriaData << uint32_t(0);    // timer 2
                    criteriaData.writeByteSeq(guid[7]);
                    criteriaData.appendPackedTime(progressIter.second->date);
                    criteriaData.writeByteSeq(counter[6]);
                    criteriaData.writeByteSeq(guid[2]);
                    criteriaData.writeByteSeq(guid[6]);
                    criteriaData.writeByteSeq(guid[5]);
                }

                packet.writeBit(guid[5]);

                for (auto completeIter : *completedAchievements)
                {
                    packet.writeBit(guid[0]);
                    packet.writeBit(guid[2]);
                    packet.writeBit(guid[5]);
                    packet.writeBit(guid[4]);
                    packet.writeBit(guid[3]);
                    packet.writeBit(guid[6]);
                    packet.writeBit(guid[1]);
                    packet.writeBit(guid[7]);

                    achievementsData.writeByteSeq(guid[1]);
                    achievementsData.writeByteSeq(guid[0]);
                    achievementsData.appendPackedTime(completeIter.second);        // achievement date
                    achievementsData << uint32_t(0);                               // realmId
                    achievementsData << uint32_t(completeIter.first);              // achievement Id
                    achievementsData.writeByteSeq(guid[7]);
                    achievementsData.writeByteSeq(guid[4]);
                    achievementsData.writeByteSeq(guid[6]);
                    achievementsData.writeByteSeq(guid[2]);
                    achievementsData.writeByteSeq(guid[3]);
                    achievementsData.writeByteSeq(guid[5]);
                    achievementsData << uint32_t(0);                              // realmId
                }

                packet.writeBit(guid[4]);
                packet.writeBit(guid[7]);
                packet.writeBit(guid[1]);

                packet.flushBits();
                packet.writeByteSeq(guid[5]);

                packet.append(achievementsData);
                packet.append(criteriaData);

                packet.writeByteSeq(guid[0]);
                packet.writeByteSeq(guid[3]);
                packet.writeByteSeq(guid[6]);
                packet.writeByteSeq(guid[2]);
                packet.writeByteSeq(guid[7]);
                packet.writeByteSeq(guid[4]);
                packet.writeByteSeq(guid[1]);

                return true;
            }

            return false;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
#endif
