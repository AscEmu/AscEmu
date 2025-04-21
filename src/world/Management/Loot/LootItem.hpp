/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "LootDefines.hpp"

#include <cstdint>
#include <memory>
#include <vector>

namespace WDB::Structures
{
    struct ItemRandomSuffixEntry;
    struct ItemRandomPropertiesEntry;
}

class LootRoll;
struct ItemProperties;
class Player;

struct LootStoreItem
{
    explicit LootStoreItem(ItemProperties const* _itemproto, std::vector<float> _chance, uint32_t _mincount, uint32_t _maxcount);

    // the item that drops
    uint32_t itemId = 0;
    // Item properties
    ItemProperties const* itemproto = nullptr;
    // chance to drop the Item
    std::vector<float> chance;
    // minimum quantity to drop
    uint32_t mincount = 0;
    // maximum quantity to drop
    uint32_t maxcount = 0;

    // rolls a chance and determines if the item can be added to the loottable
    bool roll(uint8_t difficulty) const;

    // determines if a item is a quest drop
    bool needs_quest = false;
    // determines if an item starts a quest
    bool starts_quest = false;
};

struct LootItem
{
    LootItem() = default;
    explicit LootItem(LootStoreItem const& lootItem);

    // LootItem entry
    uint32_t itemId = 0;
    // LootItem itemproperties
    ItemProperties const* itemproto = nullptr;
    // LootItem count
    uint8_t count = 0;
    // LootItem randomProperty
    WDB::Structures::ItemRandomPropertiesEntry const* iRandomProperty = nullptr;
    // LootItem randomSuffix
    WDB::Structures::ItemRandomSuffixEntry const* iRandomSuffix = nullptr;

    // Lootitem roll ongoing
    std::unique_ptr<LootRoll> roll;
    void playerRolled(Player* player, uint8_t choice);

    // LootItem allowed Players to Loot
    LooterSet allowedLooters;

    // LootItem passes on rolling
    bool is_passed = false;
    // LootItem is looted
    bool is_looted = false;
    // LootItem is blocked from looting
    bool is_blocked = false;
    // LootItem is free for all
    bool is_ffa = false;
    // LootItem is under group threshold
    bool is_underthreshold = false;
    // LootItem needs a quest to be shown
    bool needs_quest = false;
    bool starts_quest = false;

    // return true when the Player is allowed to loot the item
    bool isAllowedForPlayer(Player const* player) const;

    // adds an allowed Looter for the current Item
    void addAllowedLooter(Player const* player);

    // gets all allowed Looters for our current Item
    LooterSet const& getAllowedLooters() const { return allowedLooters; }
};
