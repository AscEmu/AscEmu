/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Loot.hpp"
#include "LootItem.hpp"
#include "LootRoll.hpp"
#include "LootTemplate.hpp"
#include "Management/Group.h"
#include "Management/ItemProperties.hpp"
#include "Management/ObjectMgr.hpp"
#include "Objects/Units/Players/Player.hpp"
#include "Server/Packets/SmsgLootRemoved.h"
#include "Server/World.h"
#include "Server/WorldSession.h"
#include "Storage/MySQLDataStore.hpp"

using namespace AscEmu::Packets;

Loot::Loot(uint32_t _gold/* = 0*/) : gold(_gold)
{ }

Loot::~Loot()
{
    clear();
}

bool Loot::fillLoot(uint32_t lootId, LootTemplateMap const& tempelateStore, Player* lootOwner, bool personal, uint8_t lootMode /*= InstanceDifficulty::DUNGEON_NORMAL*/)
{
    // Must be provided
    if (lootOwner == nullptr)
        return false;

    auto temp = tempelateStore.find(lootId);
    if (temp == tempelateStore.cend())
        return false;

    const auto& tempelate = temp->second;

    items.reserve(MAX_NR_LOOT_ITEMS);
    quest_items.reserve(MAX_NR_LOOT_QUESTITEMS);

    tempelate->generateLoot(*this, lootMode);

    // If Player is in a Group add Personal loot to them
    const auto group = lootOwner->getGroup();
    if (!personal && group != nullptr)
    {
        // Player allowed to Loot this Round
        roundRobinPlayer = lootOwner->getGuid();

        for (uint8_t i = 0; i < group->GetSubGroupCount(); ++i)
        {
            if (group->GetSubGroup(i) != nullptr)
            {
                for (const auto& itr : group->GetSubGroup(i)->getGroupMembers())
                {
                    if (auto* const loggedInPlayer = sObjectMgr.getPlayer(itr->guid))
                        fillNotNormalLootFor(loggedInPlayer, loggedInPlayer->isAtGroupRewardDistance(lootOwner));
                }
            }
        }

        for (uint8_t i = 0; i < items.size(); ++i)
        {
            if (items[i].itemproto)
                if (items[i].itemproto->Quality < group->GetThreshold())
                    items[i].is_underthreshold = true;
        }
    }
    // ... for personal loot
    else
    {
        fillNotNormalLootFor(lootOwner, true);
    }

    return true;
}

void Loot::generateGold(CreatureProperties const* property, uint8_t difficulty)
{
    uint32_t amount = 0;

    // Base Gold
    amount = property->money;

    // Difficulty Gold
    if (difficulty != 0)
    {
        uint32_t creature_difficulty_entry = sMySQLStore.getCreatureDifficulty(property->Id, difficulty);
        if (const auto properties_difficulty = sMySQLStore.getCreatureProperties(creature_difficulty_entry))
        {
            if (properties_difficulty->money != property->money)
                amount = properties_difficulty->money;
        }
    }

    // Gold rates
    amount = static_cast<uint32_t>(amount * worldConfig.getFloatRate(RATE_MONEY));

    if (amount)
        gold = amount;
}

void Loot::addLootItem(LootStoreItem const& item)
{
    if (item.needs_quest)                                   // Quest drop
    {
        if (quest_items.size() < MAX_NR_LOOT_QUESTITEMS)
            quest_items.push_back(LootItem(item));
    }
    else if (items.size() < MAX_NR_LOOT_ITEMS)              // Non-quest drop
    {
        items.push_back(LootItem(item));

        // one-player only items are counted here,
        // ffa/non-ffa items are added at fillNotNormalLootFor
        if (!item.itemproto || (item.itemproto->Flags & ITEM_FLAG_FREE_FOR_ALL) == 0)
            ++unlootedCount;
    }
}

void Loot::clear()
{
    PlayerQuestItems.clear();
    PlayerFFAItems.clear();
    PlayersLooting.clear();
    items.clear();
    quest_items.clear();
    gold = 0;
    unlootedCount = 0;
    roundRobinPlayer = 0;
}

LootItem* Loot::lootItemInSlot(uint32_t lootSlot, Player const* player, Personaltem* *qitem, Personaltem* *ffaitem)
{
    LootItem* item = nullptr;
    bool is_looted = true;
    if (lootSlot >= items.size())
    {
        const uint32_t questSlot = lootSlot - static_cast<uint32_t>(items.size());
        const auto itr = PlayerQuestItems.find(player->getGuidLow());
        if (itr != PlayerQuestItems.cend() && questSlot < itr->second->size())
        {
            Personaltem* qitem2 = &itr->second->at(questSlot);
            if (qitem != nullptr)
                *qitem = qitem2;
            item = &quest_items[qitem2->index];
            is_looted = qitem2->is_looted;
        }
    }
    else
    {
        item = &items[lootSlot];
        is_looted = item->is_looted;
        if (item->is_ffa)
        {
            const auto itr = PlayerFFAItems.find(player->getGuidLow());
            if (itr != PlayerFFAItems.cend())
            {
                for (auto iter = itr->second->begin(); iter != itr->second->cend(); ++iter)
                {
                    if (iter->index == lootSlot)
                    {
                        Personaltem* ffaitem2 = &(*iter);
                        if (ffaitem != nullptr)
                            *ffaitem = ffaitem2;
                        is_looted = ffaitem2->is_looted;
                        break;
                    }
                }
            }
        }
    }

    if (is_looted)
        return nullptr;

    return item;
}

LootItem const* Loot::getlootItemInSlot(uint32_t lootSlot, Player const* player) const
{
    LootItem const* item = nullptr;
    if (lootSlot >= items.size())
    {
        const uint32_t questSlot = lootSlot - static_cast<uint32_t>(items.size());
        const auto itr = PlayerQuestItems.find(player->getGuidLow());
        if (itr != PlayerQuestItems.cend() && questSlot < itr->second->size())
        {
            Personaltem const* qitem2 = &itr->second->at(questSlot);
            item = &quest_items[qitem2->index];
        }
    }
    else
    {
        item = &items[lootSlot];
    }

    return item;
}

uint8_t Loot::getMaxSlotInLootFor(Player const* player) const
{
    const auto itr = PlayerQuestItems.find(player->getGuidLow());
    return static_cast<uint8_t>(items.size() + (itr != PlayerQuestItems.cend() ? itr->second->size() : 0));
}

bool Loot::hasItemFor(Player const* player) const
{
    // Personal Quest Items
    auto itr = PlayerQuestItems.find(player->getGuidLow());
    if (itr != PlayerQuestItems.cend())
    {
        const auto& quest_list = itr->second;
        for (auto personalItem = quest_list->cbegin(); personalItem != quest_list->cend(); ++personalItem)
        {
            LootItem const& item = quest_items[personalItem->index];
            if (!personalItem->is_looted && !item.is_looted && item.isAllowedForPlayer(player))
                return true;
        }
    }

    // Personal Free for all Items
    itr = PlayerFFAItems.find(player->getGuidLow());
    if (itr != PlayerFFAItems.cend())
    {
        const auto& ffa_list = itr->second;
        for (auto personalFfaItem = ffa_list->cbegin(); personalFfaItem != ffa_list->cend(); ++personalFfaItem)
        {
            LootItem const& item = items[personalFfaItem->index];
            if (!personalFfaItem->is_looted && !item.is_looted && item.isAllowedForPlayer(player))
                return true;
        }
    }

    return false;
}

bool Loot::hasOverThresholdItem() const
{
    for (uint8_t i = 0; i < items.size(); ++i)
    {
        if (!items[i].is_looted && !items[i].is_underthreshold && !items[i].is_ffa)
            return true;
    }

    return false;
}

void Loot::itemRemoved(uint8_t lootIndex)
{
    // notify all players that are looting this that the item was removed
    for (auto playerGuid : PlayersLooting)
    {
        if (const auto* player = sObjectMgr.getPlayer(playerGuid))
            player->getSession()->SendPacket(SmsgLootRemoved(lootIndex).serialise().get());
        else
            removeLooter(playerGuid);
    }
}

void Loot::moneyRemoved()
{
    // notify all players that are looting this that the money was removed
    for (auto playerGuid : PlayersLooting)
    {
        if (const auto* player = sObjectMgr.getPlayer(playerGuid))
        {
            WorldPacket data(SMSG_LOOT_CLEAR_MONEY, 0);
            player->getSession()->SendPacket(&data);
        }
        else
        {
            removeLooter(playerGuid);
        }
    }
}

void Loot::fillNotNormalLootFor(Player* player, bool presentAtLooting)
{
    uint32_t plguid = player->getGuidLow();

    // Add Quest Items
    auto personaltem = std::as_const(PlayerQuestItems).find(plguid);
    if (personaltem == PlayerQuestItems.cend())
    {
        if (items.size() == MAX_NR_LOOT_ITEMS)
            return;

        auto personalList = std::make_unique<PersonaltemList>();

        for (uint8_t i = 0; i < quest_items.size(); ++i)
        {
            LootItem& item = quest_items[i];

            if (item.is_looted)
                continue;

            if (item.isAllowedForPlayer(player) || (player->getGroup() && ((player->getGroup()->GetMethod() == PARTY_LOOT_MASTER_LOOTER && player->getGroup()->GetLooter() && player->getGroup()->GetLooter()->guid == player->getGuidLow()) || player->getGroup()->GetMethod() != PARTY_LOOT_MASTER_LOOTER)))
            {
                personalList->push_back(Personaltem(i));

                // increase once if one looter only, looter-times if free for all
                if (item.is_ffa || !item.is_blocked)
                    ++unlootedCount;
                if (!player->getGroup() || (player->getGroup()->GetMethod() != PARTY_LOOT_GROUP && player->getGroup()->GetMethod() != PARTY_LOOT_ROUND_ROBIN))
                    item.is_blocked = true;

                if ((items.size() + personalList->size()) >= MAX_NR_LOOT_ITEMS)
                    break;
            }
        }

        if (!personalList->empty())
            PlayerQuestItems[plguid] = std::move(personalList);
    }

    // Add Free For All Items
    personaltem = std::as_const(PlayerFFAItems).find(plguid);
    if (personaltem == PlayerFFAItems.cend())
    {
        auto personalList = std::make_unique<PersonaltemList>();

        for (uint8_t i = 0; i < items.size(); ++i)
        {
            LootItem const& item = items[i];
            if (!item.is_looted && item.is_ffa && item.isAllowedForPlayer(player))
            {
                personalList->push_back(Personaltem(i));
                ++unlootedCount;
            }
        }

        if (!personalList->empty())
            PlayerFFAItems[plguid] = std::move(personalList);
    }

    // Add NonQuest and Non FFA Items
    for (uint8_t i = 0; i < items.size(); ++i)
    {
        LootItem& item = items[i];
        if (!item.is_looted && !item.is_ffa && (item.isAllowedForPlayer(player) ||
           (player->getGroup() && ((player->getGroup()->GetMethod() == PARTY_LOOT_MASTER_LOOTER &&
            player->getGroup()->GetLooter()->guid == player->getGuidLow()) ||
            player->getGroup()->GetMethod() != PARTY_LOOT_MASTER_LOOTER))))
        {
            if (presentAtLooting)
                item.addAllowedLooter(player);
        }
    }

    // if not present at looting player has to pick it up manually
    if (!presentAtLooting)
        return;

    // Process currency items
    processCurrencyItems(player);
}

void Loot::processCurrencyItems(Player* player)
{
    const auto max_slot = getMaxSlotInLootFor(player);
    const auto itemsCount = static_cast<uint8_t>(items.size());

    for (uint8_t i = 0; i < max_slot; ++i)
    {
        LootItem const* item = nullptr;
        if (i < itemsCount)
        {
            item = &items[i];
        }
        else
        {
            const uint8_t questIndex = i - itemsCount;
            if (questIndex < quest_items.size())
                item = &quest_items[questIndex];
        }

        if (item != nullptr && !item->is_looted && item->is_ffa && item->isAllowedForPlayer(player))
            if (item->itemproto != nullptr && item->itemproto->isCurrencyToken())
                player->storeNewLootItem(i, this);
    }
}
