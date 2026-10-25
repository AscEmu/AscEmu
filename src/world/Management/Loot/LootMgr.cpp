/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Loot.hpp"
#include "LootItem.hpp"
#include "LootMgr.hpp"
#include "LootTemplate.hpp"
#include "Logging/Logger.hpp"
#include "Management/ItemProperties.hpp"
#include "Server/DatabaseDefinition.hpp"
#include "Server/World.h"
#include "Storage/MySQLDataStore.hpp"
#include "Storage/WDB/WDBStores.hpp"
#include "Utilities/Random.hpp"
#include "Utilities/Util.hpp"

#include <array>
#include <algorithm>

template <class T> // works for anything that has the field 'chance' and is stored in plain array
T* randomChoiceVector(std::vector<std::pair<T*, float>> const& variant)
{
    typename std::vector<std::pair<T*, float>>::const_iterator itr;

    if (variant.empty())
        return nullptr;

    float totalChance = 0.0f;
    for (itr = variant.cbegin(); itr != variant.cend(); ++itr)
        totalChance += itr->second;

    auto val = Util::getRandomFloat(totalChance);
    for (itr = variant.cbegin(); itr != variant.cend(); ++itr)
    {
        val -= itr->second;
        if (val <= 0)
            return itr->first;
    }

    // should not come here, buf if it does, we should return something reasonable
    return variant.cbegin()->first;
}

LootMgr& LootMgr::getInstance()
{
    static LootMgr mInstance;
    return mInstance;
}

void LootMgr::initialize()
{
    is_loading = false;
}

void LootMgr::finalize()
{
    sLogger.info(" Deleting Loot Tables...");
    CreatureLoot.clear();
    FishingLoot.clear();
    SkinningLoot.clear();
    GOLoot.clear();
    ItemLoot.clear();
    PickpocketingLoot.clear();
}

void LootMgr::fillCreatureLoot(Player* lootOwner, Loot* loot, uint32_t loot_id, uint8_t type) const
{
    loot->clear();
    loot->fillLoot(loot_id, CreatureLoot, lootOwner, false, type);
}

void LootMgr::fillGOLoot(Player* lootOwner, Loot* loot, uint32_t loot_id, uint8_t type) const
{
    loot->clear();
    loot->fillLoot(loot_id, GOLoot, lootOwner, false, type);
}

void LootMgr::fillItemLoot(Player* lootOwner, Loot* loot, uint32_t loot_id, uint8_t type) const
{
    loot->clear();
    loot->fillLoot(loot_id, ItemLoot, lootOwner, false, type);
}

void LootMgr::fillFishingLoot(Player* lootOwner, Loot* loot, uint32_t loot_id, uint8_t type) const
{
    loot->clear();
    loot->fillLoot(loot_id, FishingLoot, lootOwner, false, type);
}

void LootMgr::fillSkinningLoot(Player* lootOwner, Loot* loot, uint32_t loot_id, uint8_t type) const
{
    loot->clear();
    loot->fillLoot(loot_id, SkinningLoot, lootOwner, false, type);
}

void LootMgr::fillPickpocketingLoot(Player* lootOwner, Loot* loot, uint32_t loot_id, uint8_t type) const
{
    loot->clear();
    loot->fillLoot(loot_id, PickpocketingLoot, lootOwner, false, type);
}

bool LootMgr::isCreatureLootable(uint32_t loot_id) const
{
    const auto itr = CreatureLoot.find(loot_id);
    return itr != CreatureLoot.cend();
}

bool LootMgr::isPickpocketable(uint32_t creatureId) const
{
    const auto itr = PickpocketingLoot.find(creatureId);
    return itr != PickpocketingLoot.cend();
}

bool LootMgr::isSkinnable(uint32_t creatureId) const
{
    const auto itr = SkinningLoot.find(creatureId);
    return itr != SkinningLoot.cend();
}

bool LootMgr::isFishable(uint32_t zoneid) const
{
    const auto itr = FishingLoot.find(zoneid);
    return itr != FishingLoot.cend();
}

void LootMgr::loadLoot()
{
    //THIS MUST BE CALLED AFTER LOADING OF ITEMS
    is_loading = true;
    loadLootProp();
    is_loading = false;
}

void LootMgr::loadLootProp()
{
    auto result = WorldDatabase.query("SELECT * FROM item_randomprop_groups");
    if (result != nullptr)
    {
        do
        {
            uint32_t id = result->fetch()[0].asUint32();
            uint32_t eid = result->fetch()[1].asUint32();
            float ch = result->fetch()[2].asFloat();
            auto item_random_properties = sItemRandomPropertiesStore.lookupEntry(eid);
            if (item_random_properties == nullptr)
            {
                sLogger.failure("LootMgr::loadLootProp : RandomProperty group {} references non-existent randomproperty {}.", id, eid);
                continue;
            }

            const auto itr = _randomprops.find(id);
            if (itr == _randomprops.cend())
            {
                RandomPropertyVector v;
                v.push_back(std::make_pair(item_random_properties, ch));
                _randomprops.insert(std::make_pair(id, v));
            }
            else
            {
                itr->second.push_back(std::make_pair(item_random_properties, ch));
            }
        } while (result->nextRow());
    }

    result = WorldDatabase.query("SELECT * FROM item_randomsuffix_groups");
    if (result != nullptr)
    {
        do
        {
            uint32_t id = result->fetch()[0].asUint32();
            uint32_t eid = result->fetch()[1].asUint32();
            float ch = result->fetch()[2].asFloat();
            auto item_random_suffix = sItemRandomSuffixStore.lookupEntry(eid);
            if (item_random_suffix == nullptr)
            {
                sLogger.debugDbTables("LootMgr::loadLootProp : RandomSuffix group {} references non-existent randomsuffix {}.", id, eid);
                continue;
            }

            auto itr = _randomsuffix.find(id);
            if (itr == _randomsuffix.cend())
            {
                RandomSuffixVector v;
                v.push_back(std::make_pair(item_random_suffix, ch));
                _randomsuffix.insert(std::make_pair(id, v));
            }
            else
            {
                itr->second.push_back(std::make_pair(item_random_suffix, ch));
            }
        } while (result->nextRow());
    }
}

namespace
{
    // the chance indexes of the loot store items (normal 10, normal 25, heroic 10, heroic 25) a difficulty id of the
    // client data stands for; 0 stands for every difficulty. The chance indexes a difficulty feeds, one bit per index
    uint8_t chanceIndexMaskOfDifficulty(uint8_t difficulty)
    {
        switch (difficulty)
        {
            case 0:
                return 0xF;
            case 4:     // 25 player normal
                return 1 << 1;
            case 2:     // heroic
            case 5:     // 10 player heroic
            case 8:     // challenge
            case 15:    // heroic raid
            case 23:    // mythic
            case 24:    // timewalking
                return 1 << 2;
            case 6:     // 25 player heroic
            case 16:    // mythic raid
                return 1 << 3;
            default:    // normal, 10 player normal, normal raid, raid finder and the rest
                return 1 << 0;
        }
    }
}

void LootMgr::loadLootGroups()
{
    m_lootGroups.clear();

    auto result = sMySQLStore.getWorldDBQuery(
        "SELECT base.groupid, itemid, group_link, chance, groupindex, mincount, maxcount FROM loot_groups base "
        "JOIN (SELECT groupid, MAX(build) build FROM loot_groups WHERE build <= %u GROUP BY groupid) latest "
        "ON latest.groupid = base.groupid AND latest.build = base.build", WoW::getConfigBuild());
    if (result == nullptr)
    {
        sLogger.info("LootMgr::loadLootGroups : Table `loot_groups` is empty!");
        return;
    }

    do
    {
        Field* fields = result->fetch();
        LootRow row;
        row.itemId = fields[1].asUint32();
        row.groupId = fields[2].asUint32();
        row.chance = fields[3].asFloat();
        row.groupIndex = fields[4].asUint8();
        row.minCount = fields[5].asUint32();
        row.maxCount = fields[6].asUint32();

        auto& group = m_lootGroups[fields[0].asUint32()];
        if (group == nullptr)
            group = std::make_shared<LootRowSet>();
        addLootRow(*group, row, "loot_groups");
    } while (result->nextRow());

    sLogger.info("{} loot groups loaded from loot_groups", m_lootGroups.size());
}

LootRowSet const* LootMgr::getLootGroup(uint32_t groupId) const
{
    const auto itr = m_lootGroups.find(groupId);
    return itr != m_lootGroups.cend() ? itr->second.get() : nullptr;
}

bool LootMgr::addLootRow(LootRowSet& set, LootRow const& row, std::string const& tableName) const
{
    LootEntry entry;
    if (row.groupId != 0)
    {
        // a reference rolls on its own, its group index picks the exclusive group of the referenced set
        entry.groupId = row.groupId;
        entry.rolls = std::max<uint32_t>(1, row.maxCount);
        entry.referencedGroup = row.groupIndex;
        entry.item.chance = row.chance;
        set.addIndependent(entry);
        return true;
    }
#if VERSION_STRING >= Cata
    else if (row.isCurrency)
    {
        entry.item = LootStoreItem(row.itemId, row.chance, row.minCount, row.maxCount);
    }
#endif
    else
    {
        const auto itemProto = sMySQLStore.getItemProperties(row.itemId);
        if (itemProto == nullptr)
        {
            sLogger.debugDbTables("LootMgr::addLootRow : Invalid Item with entry {} set in {}.", row.itemId, tableName);
            return false;
        }

        entry.item = LootStoreItem(itemProto, row.chance, row.minCount, row.maxCount);
    }

    if (row.groupIndex == 0)
        set.addIndependent(entry);
    else
        set.addExclusive(row.groupIndex, entry);

    return true;
}

void LootMgr::buildLootTemplate(uint32_t entry, std::vector<LootRow> const& rows, LootTemplateMap* lootTable, std::string const& tableName, uint32_t& count) const
{
    // per chance index the rows of its own difficulty, the rows valid for every difficulty where there are none
    std::array<std::vector<LootRow const*>, 4> rowsOfIndex;
    for (const auto& row : rows)
        if (row.difficulty != 0)
            for (uint8_t index = 0; index < 4; ++index)
                if (chanceIndexMaskOfDifficulty(row.difficulty) & (1 << index))
                    rowsOfIndex[index].push_back(&row);

    auto lootTemplate = std::make_unique<LootTemplate>();
    std::shared_ptr<LootRowSet> shared;
    for (uint8_t index = 0; index < 4; ++index)
    {
        if (!rowsOfIndex[index].empty())
        {
            auto set = std::make_shared<LootRowSet>();
            for (const auto row : rowsOfIndex[index])
                addLootRow(*set, *row, tableName);
            lootTemplate->setRowSet(index, set);
            continue;
        }

        if (shared == nullptr)
        {
            shared = std::make_shared<LootRowSet>();
            for (const auto& row : rows)
                if (row.difficulty == 0)
                    addLootRow(*shared, row, tableName);
        }
        lootTemplate->setRowSet(index, shared);
    }

    if (lootTemplate->size() == 0)
        return;

    count += static_cast<uint32_t>(lootTemplate->size());
    lootTable->insert_or_assign(entry, std::move(lootTemplate));
}

void LootMgr::loadLootTables(std::string const& szTableName, LootTemplateMap* lootTable)
{
    if (m_lootGroups.empty())
        loadLootGroups();

    // the set of the highest build per entry: items and references with the chance of the roll
    auto result = sMySQLStore.getWorldDBQuery(
        "SELECT base.entryid, difficulty, itemid, groupid, chance, groupindex, mincount, maxcount, is_currency FROM %s base "
        "JOIN (SELECT entryid, MAX(build) build FROM %s WHERE build <= %u GROUP BY entryid) latest "
        "ON latest.entryid = base.entryid AND latest.build = base.build ORDER BY base.entryid ASC", szTableName.c_str(), szTableName.c_str(), WoW::getConfigBuild());
    if (result == nullptr)
    {
        sLogger.failure("LootMgr::loadLootTables : Loading loot from table {} failed.", szTableName);
        return;
    }

    const bool isCreatureTable = (szTableName == "loot_creatures");

    uint32_t currentEntry = std::numeric_limits<uint32_t>::max();
    bool isCurrentEntryValid = true;
    std::vector<LootRow> rows;

    uint32_t count = 0;
    do
    {
        Field* fields = result->fetch();

        const uint32_t entry = fields[0].asUint32();
        if (entry != currentEntry)
        {
            if (!rows.empty())
                buildLootTemplate(currentEntry, rows, lootTable, szTableName, count);

            rows.clear();
            currentEntry = entry;
            isCurrentEntryValid = !isCreatureTable || sMySQLStore.getCreatureProperties(entry) != nullptr;
        }

        if (!isCurrentEntryValid)
            continue;

        LootRow row;
        row.difficulty = fields[1].asUint8();
        row.itemId = fields[2].asUint32();
        row.groupId = fields[3].asUint32();
        row.chance = fields[4].asFloat();
        row.groupIndex = fields[5].asUint8();
        row.minCount = fields[6].asUint32();
        row.maxCount = fields[7].asUint32();
        row.isCurrency = fields[8].asUint8() != 0;
        rows.push_back(row);
    } while (result->nextRow());

    if (!rows.empty())
        buildLootTemplate(currentEntry, rows, lootTable, szTableName, count);

    sLogger.info("{} loot templates loaded from {}", count, szTableName);
}

void LootMgr::loadAndGenerateLoot(uint8_t type)
{
    switch (type)
    {
        case 0:
            loadLootTables("loot_creatures", &CreatureLoot);
            break;
        case 1:
            loadLootTables("loot_gameobjects", &GOLoot);
            break;
        case 2:
            loadLootTables("loot_skinning", &SkinningLoot);
            break;
        case 3:
            loadLootTables("loot_fishing", &FishingLoot);
            break;
        case 4:
            loadLootTables("loot_items", &ItemLoot);
            break;
        case 5:
            loadLootTables("loot_pickpocketing", &PickpocketingLoot);
            break;
    }
}

void LootMgr::addLoot(Loot* loot, uint32_t itemid, std::vector<float> chance, uint32_t mincount, uint32_t maxcount, uint8_t lootDifficulty)
{
    if (loot->items.size() > 16)
    {
        sLogger.debug("LootMgr::addLoot cannot add item {} to Loot, Maximum drops reached", itemid);
        return;
    }

    const auto itemprop = sMySQLStore.getItemProperties(itemid);
    if (itemprop == nullptr)
        return;

    // the chance of the difficulty, the first one where the script gives fewer
    if (chance.empty())
        return;

    const float itemChance = lootDifficulty < chance.size() ? chance[lootDifficulty] : chance[0];
    if (itemChance < 0.0f)
        return;

    LootStoreItem item(itemprop, itemChance, mincount, maxcount);
    if (!item.roll())
        return;

    // add the Item to our Loot List
    loot->addLootItem(item);
}

WDB::Structures::ItemRandomPropertiesEntry const* LootMgr::getRandomProperties(ItemProperties const* proto) const
{
    if (proto->RandomPropId == 0)
        return nullptr;

    const auto itr = _randomprops.find(proto->RandomPropId);
    if (itr == _randomprops.cend())
        return nullptr;

    return randomChoiceVector<WDB::Structures::ItemRandomPropertiesEntry const>(itr->second);
}

WDB::Structures::ItemRandomSuffixEntry const* LootMgr::getRandomSuffix(ItemProperties const* proto) const
{
    if (proto->RandomSuffixId == 0)
        return nullptr;

    const auto itr = _randomsuffix.find(proto->RandomSuffixId);
    if (itr == _randomsuffix.cend())
        return nullptr;

    return randomChoiceVector<WDB::Structures::ItemRandomSuffixEntry const>(itr->second);
}

bool LootMgr::isLoading() const
{
    return is_loading;
}

LootTemplateMap const& LootMgr::getCreatureLoot() const
{
    return CreatureLoot;
}

LootTemplateMap const& LootMgr::getFishingLoot() const
{
    return FishingLoot;
}

LootTemplateMap const& LootMgr::getSkinningLoot() const
{
    return SkinningLoot;
}

LootTemplateMap const& LootMgr::getGameobjectLoot() const
{
    return GOLoot;
}

LootTemplateMap const& LootMgr::getItemLoot() const
{
    return ItemLoot;
}

LootTemplateMap const& LootMgr::getPickpocketingLoot() const
{
    return PickpocketingLoot;
}
