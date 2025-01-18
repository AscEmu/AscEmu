/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Storage/MySQLDataStore.hpp"
#include "QuestProperties.hpp"

uint32_t QuestProperties::GetRewardItemCount() const
{
    uint32_t count = 0;
    for (uint8_t i = 0; i < 4; ++i)
        if (reward_item[i] != 0)
            count++;

    return count;
}

