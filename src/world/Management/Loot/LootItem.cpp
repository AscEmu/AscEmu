/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "LootItem.hpp"
#include "LootMgr.hpp"
#include "LootRoll.hpp"
#include "Management/ItemProperties.hpp"
#include "Objects/Units/Players/Player.hpp"
#include "Server/World.h"
#include "Utilities/Random.hpp"
#include "Utilities/Util.hpp"

LootStoreItem::LootStoreItem(ItemProperties const* _itemproto, std::vector<float> _chance, uint32_t _mincount, uint32_t _maxcount) :
    itemId(_itemproto->ItemId), itemproto(_itemproto), chance(_chance), mincount(_mincount), maxcount(_maxcount)
{
    starts_quest = itemproto->QuestId != 0;
    // TODO: not all quest items have quest class
    // maybe we need some sort of condition column in loot tables or in item properties -Appled
    needs_quest = itemproto->Class == ITEM_CLASS_QUEST;
}

bool LootStoreItem::roll(uint8_t difficulty) const
{
    if (chance[difficulty] >= 100.0f)
        return true;

    if (itemproto != nullptr)
        return Util::checkChance(chance[difficulty] * worldConfig.getFloatRate(RATE_DROP0 + itemproto->Quality));

    return Util::checkChance(chance[difficulty]);
}

LootItem::LootItem(LootStoreItem const& li)
{
    itemId = li.itemId;
    itemproto = li.itemproto;
    count = Util::getRandomUInt(li.mincount, li.maxcount);
    is_ffa = itemproto != nullptr && itemproto->HasFlag(ITEM_FLAG_FREE_FOR_ALL);

    iRandomProperty = sLootMgr.getRandomProperties(itemproto);
    iRandomSuffix = sLootMgr.getRandomSuffix(itemproto);

    needs_quest = li.needs_quest;
    starts_quest = li.starts_quest;
}

void LootItem::playerRolled(Player* player, uint8_t choice)
{
    if (roll == nullptr)
        return;

    // Ensure the roll will be freed properly by handling rolling inside LootItem
    const auto rollFinished = roll->playerRolled(player, choice);
    if (rollFinished)
        roll = nullptr;
}

bool LootItem::isAllowedForPlayer(Player const* player) const
{
    if (itemproto == nullptr)
        return false;

    // not show loot for players without profession or those who already know the recipe
    if (itemproto->Flags & ITEM_FLAG_SMART_LOOT && (!player->hasSkillLine(itemproto->RequiredSkill) || player->hasSpell(itemproto->Spells[1].Id)))
        return false;

    // not show loot for not own team
    if (itemproto->Flags2 & ITEM_FLAG2_HORDE_ONLY && player->GetTeam() != TEAM_HORDE)
        return false;
    if (itemproto->Flags2 & ITEM_FLAG2_ALLIANCE_ONLY && player->GetTeam() != TEAM_ALLIANCE)
        return false;

    // If item starts quest do not show item if player is already on the quest
    // or if player has finished the quest
    if (starts_quest)
    {
        if (player->hasQuestInQuestLog(itemproto->QuestId) || player->hasQuestFinished(itemproto->QuestId))
            return false;
    }
    // or if item requires a quest player needs to have the quest active
    else if (needs_quest)
    {
        if (!player->hasQuestForItem(itemId))
            return false;
    }

    return true;
}

void LootItem::addAllowedLooter(Player const* player)
{
    allowedLooters.insert(player->getGuidLow());
}
