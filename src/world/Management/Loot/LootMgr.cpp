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
        m_lootGroups[fields[0].asUint32()].push_back(row);
    } while (result->nextRow());

    sLogger.info("{} loot groups loaded from loot_groups", m_lootGroups.size());
}

void LootMgr::expandLootRows(std::vector<LootRow> const& rows, float multiplier, uint8_t depth, std::unordered_map<uint32_t, LootRow>& items) const
{
    // the rows of an exclusive group share one roll: explicit chances are their share, rows without a chance split the rest
    std::unordered_map<uint8_t, std::pair<float, uint32_t>> groups;    // index -> (sum of explicit chances, rows without a chance)
    for (const auto& row : rows)
    {
        if (row.groupIndex == 0)
            continue;

        auto& group = groups[row.groupIndex];
        if (row.chance > 0.0f)
            group.first += row.chance;
        else
            ++group.second;
    }

    for (const auto& row : rows)
    {
        float chance = row.chance;
        if (row.groupIndex != 0)
        {
            const auto& group = groups[row.groupIndex];
            if (row.chance <= 0.0f)
                chance = group.second != 0 ? std::max(0.0f, 100.0f - group.first) / static_cast<float>(group.second) : 0.0f;
            else if (group.first > 100.0f)
                chance = row.chance * 100.0f / group.first;
        }
        chance *= multiplier;

        if (row.groupId != 0)
        {
            // a reference: its group's items with the chance of the reference, rolled as often as the count says
            const auto group = m_lootGroups.find(row.groupId);
            if (group == m_lootGroups.end() || depth >= 5)
                continue;

            const float rolls = static_cast<float>(std::max<uint32_t>(1, row.maxCount));
            expandLootRows(group->second, std::min(100.0f, chance * rolls) / 100.0f, depth + 1, items);
            continue;
        }

        if (chance <= 0.0f)
            continue;

        auto& item = items[row.itemId];
        if (item.itemId == 0 || item.chance < chance)
        {
            item = row;
            item.chance = chance;
        }
    }
}

void LootMgr::buildLootTemplate(uint32_t entry, std::vector<LootRow> const& rows, LootTemplateMap* lootTable, std::string const& tableName, uint32_t& count) const
{
    // per chance index the rows of its own difficulty, the rows valid for every difficulty where there are none
    std::array<std::vector<LootRow>, 4> rowsOfIndex;
    for (const auto& row : rows)
        if (row.difficulty != 0)
            for (uint8_t index = 0; index < 4; ++index)
                if (chanceIndexMaskOfDifficulty(row.difficulty) & (1 << index))
                    rowsOfIndex[index].push_back(row);

    std::array<bool, 4> hasOwnRows{};
    for (uint8_t index = 0; index < 4; ++index)
        hasOwnRows[index] = !rowsOfIndex[index].empty();

    for (const auto& row : rows)
        if (row.difficulty == 0)
            for (uint8_t index = 0; index < 4; ++index)
                if (!hasOwnRows[index])
                    rowsOfIndex[index].push_back(row);

    // item -> chance per index, -1 where the item does not drop in that difficulty
    std::unordered_map<uint32_t, std::pair<LootRow, std::array<float, 4>>> chances;
    std::unordered_map<uint32_t, LootRow> sharedItems;      // the rows every difficulty shares expand once
    bool sharedExpanded = false;
    for (uint8_t index = 0; index < 4; ++index)
    {
        std::unordered_map<uint32_t, LootRow> ownItems;
        if (hasOwnRows[index])
        {
            expandLootRows(rowsOfIndex[index], 1.0f, 0, ownItems);
        }
        else if (!sharedExpanded)
        {
            expandLootRows(rowsOfIndex[index], 1.0f, 0, sharedItems);
            sharedExpanded = true;
        }

        for (const auto& [itemId, item] : hasOwnRows[index] ? ownItems : sharedItems)
        {
            auto itr = chances.find(itemId);
            if (itr == chances.end())
                itr = chances.emplace(itemId, std::make_pair(item, std::array<float, 4>{ -1.0f, -1.0f, -1.0f, -1.0f })).first;
            itr->second.second[index] = item.chance;
        }
    }

    if (chances.empty())
        return;

    auto tab = lootTable->try_emplace(entry, Util::LazyInstanceCreator([] {
        return std::make_unique<LootTemplate>();
    })).first;

    for (const auto& [itemId, item] : chances)
    {
#if VERSION_STRING >= Cata
        if (item.first.isCurrency)
        {
            LootStoreItem storeItem(itemId, item.second, item.first.minCount, item.first.maxCount);
            tab->second->addEntry(storeItem);
            ++count;
            continue;
        }
#endif
        const auto itemProto = sMySQLStore.getItemProperties(itemId);
        if (itemProto == nullptr)
        {
            sLogger.debugDbTables("LootMgr::buildLootTemplate : Invalid Item with entry {} set in {}.", itemId, tableName);
            continue;
        }

        LootStoreItem storeItem(itemProto, item.second, item.first.minCount, item.first.maxCount);
        tab->second->addEntry(storeItem);
        ++count;
    }
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

    // Convert incoming std::vector to fixed std::array<float, 4>
    std::array<float, 4> chanceArray{};
    for (size_t i = 0; i < std::min(chance.size(), chanceArray.size()); ++i)
    {
        chanceArray[i] = chance[i];
    }

    LootStoreItem item = LootStoreItem(itemprop, chanceArray, mincount, maxcount);

    // check difficulty level
    if (item.chance[lootDifficulty] < 0.0f)
    {
        return;
    }

    // Bad luck for the entry
    if (!item.roll(0))
    {
        return;
    }

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
