/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <cstdint>
#include <ctime>

namespace AscEmu::Packets
{
    // plain achievement values for the achievement data packets, filled by the achievement manager
    struct CompletedAchievementEntry
    {
        uint32_t achievementId = 0;
        time_t date = 0;
    };

    struct CriteriaProgressEntry
    {
        uint32_t criteriaId = 0;
        uint64_t counter = 0;
        time_t date = 0;
    };
}
