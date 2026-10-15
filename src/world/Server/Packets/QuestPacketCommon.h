/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <cstdint>
#include <string>

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

    // 6.x and later: an objective of a quest, the storage index addresses its progress in the quest log
    struct QuestObjectiveEntry
    {
        uint32_t id = 0;
        int8_t storageIndex = 0;
        uint8_t type = 0;                   // 0 creature, 1 item, 2 game object, 5 spell
        int32_t objectId = 0;
        int32_t amount = 0;
        std::string description;
    };

    // 6.x and later: the rewards block of the quest details and of the reward offer
    struct QuestRewardsData
    {
        uint32_t choiceItemCount = 0;
        QuestRewardItemEntry choiceItems[6];
        uint32_t itemCount = 0;
        QuestRewardItemEntry items[4];
        uint32_t money = 0;
        uint32_t xp = 0;
        uint32_t honor = 0;
        uint32_t title = 0;
        uint32_t talents = 0;
        uint32_t factionId[5] = {};
        int32_t factionValue[5] = {};
        uint32_t displaySpell = 0;
        uint32_t spell = 0;
        uint32_t currencyId[4] = {};
        uint32_t currencyCount[4] = {};
    };
}
