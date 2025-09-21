/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Loot.hpp"
#include "LootItem.hpp"
#include "LootTemplate.hpp"
#include "Management/ItemProperties.hpp"
#include "Objects/Units/Players/Player.hpp"
#include "Utilities/Random.hpp"
#include "Utilities/Util.hpp"

LootTemplate::LootTemplate() = default;
LootTemplate::~LootTemplate() = default;

void LootTemplate::addEntry(LootStoreItem& item)
{
    Entries.push_back(item);
}

void LootTemplate::generateLoot(Loot& loot, uint8_t lootDifficulty) const
{
    // Randomize our Loot
    LootStoreItemList lootEntries = Entries;
    Util::randomShuffleVector(lootEntries);

    // Rolling items
    for (const auto& lootStoreItem : lootEntries)
    {
        // check difficulty level
        if (lootStoreItem.chance[lootDifficulty] < 0.0f)
            continue;

        // Bad luck for the entry
        if (!lootStoreItem.roll(lootDifficulty))
            continue;

        if (lootStoreItem.itemproto != nullptr)
        {
            uint8_t item_counterEquipable = 0;
            uint8_t item_counternotEquipable = 0;
            for (auto _item = loot.items.cbegin(); _item != loot.items.cend(); ++_item)
            {
                // Non-equippable items are limited to 3 drops
                if (lootStoreItem.itemproto->InventoryType == INVTYPE_NON_EQUIP && item_counternotEquipable < MAX_NR_LOOT_NONEQUIPABLE)
                    item_counternotEquipable++;
                // Equippable item are limited to 2 drops
                else if (lootStoreItem.itemproto->InventoryType != INVTYPE_NON_EQUIP && item_counterEquipable < MAX_NR_LOOT_EQUIPABLE)
                    item_counterEquipable++;
            }

            // Stopp adding Items
            if ((item_counterEquipable == MAX_NR_LOOT_EQUIPABLE && lootStoreItem.itemproto->InventoryType != INVTYPE_NON_EQUIP) ||
                (item_counternotEquipable == MAX_NR_LOOT_NONEQUIPABLE && lootStoreItem.itemproto->InventoryType == INVTYPE_NON_EQUIP))
                continue;
        }

        // add the item to our Loot list
        loot.addLootItem(lootStoreItem);
    }
}

bool LootTemplate::hasQuestDrop(LootTemplateMap const& /*store*/, Player const* forPlayer/* = nullptr*/) const
{
    for (const auto& lootEntry : Entries)
    {
        if (forPlayer != nullptr)
        {
            if (forPlayer->hasQuestForItem(lootEntry.itemId))
                return true; // active quest drop found
        }
        else
        {
            if (lootEntry.needs_quest)
                return true; // quest drop found
        }
    }

    return false;
}
