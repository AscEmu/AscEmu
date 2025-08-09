/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Loot.hpp"
#include "LootItem.hpp"
#include "LootMgr.hpp"
#include "LootTemplate.hpp"
#include "Logging/Logger.hpp"
#include "Management/ItemProperties.hpp"
#include "Server/DatabaseDefinition.hpp"
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
    auto startTime = Util::TimeNow();

    //THIS MUST BE CALLED AFTER LOADING OF ITEMS
    is_loading = true;
    loadLootProp();
    is_loading = false;
}

void LootMgr::loadLootProp()
{
    auto result = WorldDatabase.Query("SELECT * FROM item_randomprop_groups");
    if (result != nullptr)
    {
        do
        {
            uint32_t id = result->Fetch()[0].asUint32();
            uint32_t eid = result->Fetch()[1].asUint32();
            float ch = result->Fetch()[2].asFloat();
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
        } while (result->NextRow());
    }

    result = WorldDatabase.Query("SELECT * FROM item_randomsuffix_groups");
    if (result != nullptr)
    {
        do
        {
            uint32_t id = result->Fetch()[0].asUint32();
            uint32_t eid = result->Fetch()[1].asUint32();
            float ch = result->Fetch()[2].asFloat();
            auto item_random_suffix = sItemRandomSuffixStore.lookupEntry(eid);
            if (item_random_suffix == nullptr)
            {
                sLogger.debugFlag(AscEmu::Logging::LF_DB_TABLES, "LootMgr::loadLootProp : RandomSuffix group {} references non-existent randomsuffix {}.", id, eid);
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
        } while (result->NextRow());
    }
}

void LootMgr::loadLootTables(std::string const& szTableName, LootTemplateMap* LootTable)
{
    auto result = sMySQLStore.getWorldDBQuery("SELECT * FROM %s ORDER BY entryid ASC", szTableName.c_str());
    if (result == nullptr)
    {
        sLogger.failure("LootMgr::loadLootTables : Loading loot from table {} failed.", szTableName);
        return;
    }

    LootTemplateMap::const_iterator tab;
    uint32_t count = 0;
    do
    {
        Field* fields = result->Fetch();

        std::vector<float> chance;
        chance.reserve(4);

        uint32_t entry = fields[0].asUint32();
        uint32_t itemId = fields[1].asUint32();
        chance.push_back(fields[2].asFloat());
        chance.push_back(fields[3].asFloat());
        chance.push_back(fields[4].asFloat());
        chance.push_back(fields[5].asFloat());
        uint32_t mincount = fields[6].asUint32();
        uint32_t maxcount = fields[7].asUint8();

        const auto itemProto = sMySQLStore.getItemProperties(itemId);
        if (itemProto == nullptr)
        {
            sLogger.debugFlag(AscEmu::Logging::LF_DB_TABLES, "LootMgr::loadLootTables : Invalid Item with entry {} set in {}", itemId, szTableName);
            continue;
        }

        LootStoreItem storeitem = LootStoreItem(itemProto, chance, mincount, maxcount);

        // Looking for the template of the entry
        if (LootTable->empty() || tab->first != entry)
        {
            // Searching the template (in case template Id changed)
            const auto [tabItr, _] = LootTable->try_emplace(entry, Util::LazyInstanceCreator([] {
                return std::make_unique<LootTemplate>();
            }));
            tab = tabItr;
        }

        // Add Item to our Tempelate
        tab->second->addEntry(storeitem);
        count++;
    } while (result->NextRow());

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

    LootStoreItem item = LootStoreItem(itemprop, chance, mincount, maxcount);

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
