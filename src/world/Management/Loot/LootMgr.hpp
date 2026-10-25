/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "Platform/SymbolVisibility.hpp"
#include "Management/Loot/LootTemplate.hpp"

#include <map>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

struct Loot;
struct ItemProperties;
class Player;

namespace WDB::Structures
{
    struct ItemRandomSuffixEntry;
    struct ItemRandomPropertiesEntry;
}

typedef std::vector<std::pair<WDB::Structures::ItemRandomPropertiesEntry const*, float>> RandomPropertyVector;
typedef std::vector<std::pair<WDB::Structures::ItemRandomSuffixEntry const*, float>> RandomSuffixVector;

class SERVER_DECL LootMgr
{
private:
    LootMgr() = default;
    ~LootMgr() = default;

public:
    static LootMgr& getInstance();
    void initialize();
    void finalize();

    LootMgr(LootMgr&&) = delete;
    LootMgr(LootMgr const&) = delete;
    LootMgr& operator=(LootMgr&&) = delete;
    LootMgr& operator=(LootMgr const&) = delete;

    // fills Creature loot for the given loot owner
    void fillCreatureLoot(Player* lootOwner, Loot* loot, uint32_t loot_id, uint8_t type) const;

    // fills GameObject loot for the given loot owner
    void fillGOLoot(Player* lootOwner, Loot* loot, uint32_t loot_id, uint8_t type) const;

    // fills Item loot for the given loot owner
    void fillItemLoot(Player* lootOwner, Loot* loot, uint32_t loot_id, uint8_t type) const;

    // fills Fishing loot for the given loot owner
    void fillFishingLoot(Player* lootOwner, Loot* loot, uint32_t loot_id, uint8_t type) const;

    // fills Skinning loot for the given loot owner
    void fillSkinningLoot(Player* lootOwner, Loot* loot, uint32_t loot_id, uint8_t type) const;

    // fills Pickpocketing loot for the given loot owner
    void fillPickpocketingLoot(Player* lootOwner, Loot* loot, uint32_t loot_id, uint8_t type) const;

    // return true when the Creature is Lootable (has Loot in the Template)
    bool isCreatureLootable(uint32_t loot_id) const;

    // return true when the Creature is Pickpocketable (has Loot in the Tempelate)
    bool isPickpocketable(uint32_t creatureId) const;

    // return true when the Creature is Skinnable (has Loot in the Tempelate)
    bool isSkinnable(uint32_t creatureId) const;

    // return true when Gameobject is Fishable (has Loot in the Tempelate)
    bool isFishable(uint32_t zoneid) const;

    // the rows of a loot group (loot_groups), referenced by the loot tables
    LootRowSet const* getLootGroup(uint32_t groupId) const;

    // a row of loot_groups or of a loot table: an item (or currency) or a reference into loot_groups, the chance of the
    // roll, the exclusive group index of the set (0 = independent roll) and the counts
    struct LootRow
    {
        uint8_t difficulty = 0;
        uint32_t itemId = 0;
        uint32_t groupId = 0;
        float chance = 0.0f;
        uint8_t groupIndex = 0;
        uint32_t minCount = 1;
        uint32_t maxCount = 1;
        bool isCurrency = false;
    };

    // Load Loot Tables
    void loadLoot();
    void loadLootProp();
    void loadLootGroups();
    void loadLootTables(std::string const& szTableName, LootTemplateMap* lootTable);
    void loadAndGenerateLoot(uint8_t type);
    void addLoot(Loot* loot, uint32_t itemid, std::vector<float> chance, uint32_t mincount, uint32_t maxcount, uint8_t lootDifficulty);

    WDB::Structures::ItemRandomPropertiesEntry const* getRandomProperties(ItemProperties const* proto) const;
    WDB::Structures::ItemRandomSuffixEntry const* getRandomSuffix(ItemProperties const* proto) const;

    bool isLoading() const;

    LootTemplateMap const& getCreatureLoot() const;
    LootTemplateMap const& getFishingLoot() const;
    LootTemplateMap const& getSkinningLoot() const;
    LootTemplateMap const& getGameobjectLoot() const;
    LootTemplateMap const& getItemLoot() const;
    LootTemplateMap const& getPickpocketingLoot() const;

private:
    LootTemplateMap CreatureLoot;
    LootTemplateMap FishingLoot;
    LootTemplateMap SkinningLoot;
    LootTemplateMap GOLoot;
    LootTemplateMap ItemLoot;
    LootTemplateMap PickpocketingLoot;

    bool is_loading = false;

    // the loot groups of the build (loot_groups), referenced by the loot tables
    std::unordered_map<uint32_t, std::shared_ptr<LootRowSet>> m_lootGroups;

    // a row joins its set as an item, currency or group reference; false for an unknown item
    bool addLootRow(LootRowSet& set, LootRow const& row, std::string const& tableName) const;
    // the template of one entry: own rows per difficulty index, the shared rows where there are none
    void buildLootTemplate(uint32_t entry, std::vector<LootRow> const& rows, LootTemplateMap* lootTable, std::string const& tableName, uint32_t& count) const;

    std::map<uint32_t, RandomPropertyVector> _randomprops;
    std::map<uint32_t, RandomSuffixVector> _randomsuffix;
};

#define sLootMgr LootMgr::getInstance()
