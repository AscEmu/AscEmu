/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"

#include <cstdint>
#include <memory>
#include <vector>

namespace AscEmu::Packets
{
    // Pre-resolved, already-filtered achievement data. Populated by AchievementMgr, which owns
    // the DB lookups and visibility rules (showCompletedAchievement/canSendAchievementProgress) -
    // this struct only carries plain values so the packet class itself stays free of those
    // dependencies.
    struct AchievementDataChunkInput
    {
        struct CompletedEntry
        {
            uint32_t achievementId = 0;
            uint32_t dateBitfield = 0; // secsToTimeBitFields() already applied
        };

        struct ProgressEntry
        {
            uint32_t criteriaId = 0;
            uint64_t counter = 0;
            uint32_t dateBitfield = 0; // secsToTimeBitFields() already applied
        };

        bool isSelf = true;
        uint64_t achievingPlayerGuid = 0;
        std::vector<CompletedEntry> completed;
        std::vector<ProgressEntry> progress;
    };

    // Pre-Cata SMSG_ALL_ACHIEVEMENT_DATA / SMSG_RESPOND_INSPECT_ACHIEVEMENTS. The client can't
    // handle a packet bigger than ~0x7fff bytes, so unlike every other ManagedPacket this one can
    // produce several WorldPackets from a single instance - buildChunks() replaces serialise().
    class SmsgAllAchievementDataLegacy : public ManagedPacket
    {
    public:
        AchievementDataChunkInput input;

        SmsgAllAchievementDataLegacy() : SmsgAllAchievementDataLegacy(AchievementDataChunkInput{})
        {
        }

        explicit SmsgAllAchievementDataLegacy(AchievementDataChunkInput input) :
            ManagedPacket(input.isSelf ? SMSG_ALL_ACHIEVEMENT_DATA : SMSG_RESPOND_INSPECT_ACHIEVEMENTS, 0),
            input(std::move(input))
        {
        }

        std::vector<std::unique_ptr<WorldPacket>> buildChunks() const
        {
            std::vector<std::unique_ptr<WorldPacket>> chunks;

            const uint32_t estimatedSize = 18 + static_cast<uint32_t>(input.completed.size()) * 8 + static_cast<uint32_t>(input.progress.size()) * 36;

            size_t completeIndex = 0;
            size_t progressIndex = 0;
            bool doneCompleted = false;
            bool doneProgress = false;

            while (!doneCompleted || !doneProgress)
            {
                auto data = std::make_unique<WorldPacket>(estimatedSize < 0x8000 ? estimatedSize : 0x7fff);

                if (input.isSelf)
                {
                    data->setOpcode(SMSG_ALL_ACHIEVEMENT_DATA);
                }
                else
                {
                    data->setOpcode(SMSG_RESPOND_INSPECT_ACHIEVEMENTS);
                    FastGUIDPack(*data, input.achievingPlayerGuid);
                }

                bool packetFull = false;

                if (!doneCompleted)
                {
                    for (; completeIndex < input.completed.size() && !packetFull; ++completeIndex)
                    {
                        const auto& entry = input.completed[completeIndex];
                        *data << uint32_t(entry.achievementId);
                        *data << uint32_t(entry.dateBitfield);

                        packetFull = data->size() > 0x7f00;
                    }

                    if (completeIndex == input.completed.size())
                        doneCompleted = true;
                }

                *data << int32_t(-1);

                for (; progressIndex < input.progress.size() && !packetFull; ++progressIndex)
                {
                    const auto& entry = input.progress[progressIndex];
                    *data << uint32_t(entry.criteriaId);
                    data->appendPackGuid(entry.counter);
                    *data << WoWGuid(input.achievingPlayerGuid);
                    *data << uint32_t(0);
                    *data << uint32_t(entry.dateBitfield);
                    *data << uint32_t(0);
                    *data << uint32_t(0);

                    packetFull = data->size() > 0x7f00;
                }

                if (progressIndex == input.progress.size())
                    doneProgress = true;

                *data << int32_t(-1);

                chunks.push_back(std::move(data));
            }

            return chunks;
        }

    protected:
        bool internalSerialise(WorldPacket& /*packet*/) override { return false; }
        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
