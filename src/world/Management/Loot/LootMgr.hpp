/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "CommonTypes.hpp"
#include "Management/Loot/LootTemplate.hpp"

#include <map>
#include <string>

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

    // Load Loot Tables
    void loadLoot();
    void loadLootProp();
    void loadLootTables(std::string const& szTableName, LootTemplateMap* LootTable);
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

    std::map<uint32_t, RandomPropertyVector> _randomprops;
    std::map<uint32_t, RandomSuffixVector> _randomsuffix;
};

#define sLootMgr LootMgr::getInstance()
