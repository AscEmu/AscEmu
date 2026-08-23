/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <cstdint>

namespace AscEmu::Packets
{
    // Shared by the SMSG_QUESTGIVER_* packet classes.
    struct QuestRewardItemEntry
    {
        uint32_t itemId = 0;
        uint32_t count = 0;
        uint32_t displayId = 0; // resolved via sMySQLStore.getItemProperties(itemId)->DisplayInfoID
    };

    struct QuestEmoteEntry
    {
        uint32_t emote = 0;
        uint32_t delay = 0;
    };
}
