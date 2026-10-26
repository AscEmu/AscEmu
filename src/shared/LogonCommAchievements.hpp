/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "Network/ByteBuffer.hpp"

#include <cstdint>
#include <vector>

// account wide achievements in the logonserver: REQUEST loads, SAVE adds completions and raises criteria progress
namespace LogonComm
{
    struct AccountAchievementRecord
    {
        uint32_t achievementId = 0;
        uint64_t date = 0;
    };

    struct AccountCriteriaRecord
    {
        uint32_t criteriaId = 0;
        uint32_t counter = 0;
        uint64_t date = 0;
    };

    inline void writeAccountAchievements(ByteBuffer& buffer, std::vector<AccountAchievementRecord> const& achievements, std::vector<AccountCriteriaRecord> const& criteria)
    {
        buffer << uint32_t(achievements.size());
        for (auto const& achievement : achievements)
            buffer << achievement.achievementId << achievement.date;

        buffer << uint32_t(criteria.size());
        for (auto const& progress : criteria)
            buffer << progress.criteriaId << progress.counter << progress.date;
    }

    inline void readAccountAchievements(ByteBuffer& buffer, std::vector<AccountAchievementRecord>& achievements, std::vector<AccountCriteriaRecord>& criteria)
    {
        uint32_t count = 0;
        buffer >> count;
        achievements.resize(count);
        for (auto& achievement : achievements)
            buffer >> achievement.achievementId >> achievement.date;

        buffer >> count;
        criteria.resize(count);
        for (auto& progress : criteria)
            buffer >> progress.criteriaId >> progress.counter >> progress.date;
    }
}
