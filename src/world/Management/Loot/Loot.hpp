/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "LootItem.hpp"
#include "LootTemplate.hpp"
#include "Map/Maps/InstanceDefines.hpp"

#include <map>
#include <set>
#include <vector>

class Player;
struct CreatureProperties;

struct Personaltem
{
    uint8_t index;  // Inedx from Personal Items;
    bool is_looted; // Personal Item is looted

    Personaltem()
        : index(0), is_looted(false) {}

    Personaltem(uint8_t _index, bool _islooted = false)
        : index(_index), is_looted(_islooted) {}
};

typedef std::vector<Personaltem> PersonaltemList;
typedef std::map<uint32_t, std::unique_ptr<PersonaltemList>> PersonaltemMap;

struct Loot
{
    PersonaltemMap const& getPlayerQuestItems() const { return PlayerQuestItems; }
    PersonaltemMap const& getPlayerFFAItems() const { return PlayerFFAItems; }

    std::vector<LootItem> items;            // LootItems
    std::vector<LootItem> quest_items;      // PersonalItems

    uint32_t gold = 0;                      // Loot money
    uint8_t unlootedCount = 0;              // Unlooted Items count
    uint64_t roundRobinPlayer = 0;          // Current Round Robin Player from the Group

    Loot(uint32_t _gold = 0);
    ~Loot();

    // fill Current Loot Store for xx Items
    bool fillLoot(uint32_t lootId, LootTemplateMap const& tempelateStore, Player* lootOwner, bool personal, uint8_t lootMode = InstanceDifficulty::DUNGEON_NORMAL);

    // fill Current loot Store for xx money
    void generateGold(CreatureProperties const* property, uint8_t difficulty);

    // adds an Item to our current Loot Store
    void addLootItem(LootStoreItem const& item);

    // clear our Loot
    void clear();

    // return true when there is Loot left
    bool empty() const { return items.empty() && gold == 0; }

    // return true when all Items and Money are Looted
    bool isLooted() const { return gold == 0 && unlootedCount == 0; }

    // adds an Looter to the Current Loot (Players which are on the Loot Window)
    void addLooter(uint32_t GUID) { PlayersLooting.insert(GUID); }

    // removes an Looter from the Current Loot (Players which are on the Loot Window)
    void removeLooter(uint32_t GUID) { PlayersLooting.erase(GUID); }

    // returns an LootItem from given Slot and marks it as Looted
    LootItem* lootItemInSlot(uint32_t lootslot, Player const* player, Personaltem** qitem = nullptr, Personaltem** ffaitem = nullptr);

    // returns an LootItem from given Slot
    LootItem const* getlootItemInSlot(uint32_t lootslot, Player const* player) const;

    // returns the highestIndex of our Lootables for Player xx
    uint8_t getMaxSlotInLootFor(Player const* player) const;

    // return true when the Loot holds an Item for Player xx
    bool hasItemFor(Player const* player) const;

    // return true when the Loot contains Items over the Group threshold
    bool hasOverThresholdItem() const;

    // Notify all Looters an Item got Removed
    void itemRemoved(uint8_t slot);

    // Notify all Looters Money got Removed
    void moneyRemoved();

private:
    // Fills not normal Loot for Player xx
    void fillNotNormalLootFor(Player* player, bool presentAtLooting);

    // Adds Currency Items to the Player when Present
    void processCurrencyItems(Player* player);

    // All Players currently Looting
    std::set<uint32_t> PlayersLooting;

    // Personal Loot
    PersonaltemMap PlayerQuestItems;
    PersonaltemMap PlayerFFAItems;
};
