/*
Copyright (c) 2014-2022 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/


#include "Storage/DBC/DBCStores.h"
#include "Objects/Item.hpp"
#include "Management/ItemInterface.h"
#include "Storage/MySQLDataStore.hpp"
#include "Server/MainServerDefines.h"
#include "Macros/ScriptMacros.hpp"
#include "Server/WorldConfig.h"
#include "Map/Management/MapMgr.hpp"
#include "Server/Packets/SmsgLootRemoved.h"
#include "Server/Packets/SmsgLootAllPassed.h"
#include "Server/Packets/SmsgLootRollWon.h"
#include "Server/Packets/SmsgLootRoll.h"
#include "LootMgr.h"
#include "Objects/GameObject.h"
#include "Objects/Units/Creatures/Creature.h"
#include "Server/Script/CreatureAIScript.h"
#include "Management/ObjectMgr.h"
#include <random>

using namespace AscEmu::Packets;

template <class T> // works for anything that has the field 'chance' and is stored in plain array
const T & RandomChoice(const T* variant, int count)
{
    float totalChance = 0;
    for (int i = 0; i < count; i++)
        totalChance += variant[i].chance;

    float val = Util::getRandomFloat(totalChance);

    for (int i = 0; i < count; i++)
    {
        val -= variant[i].chance;
        if (val <= 0) return variant[i];
    }

    // should not come here, buf if it does, we should return something reasonable
    return variant[count - 1];
}

template <class T> // works for anything that has the field 'chance' and is stored in plain array
T* RandomChoiceVector(std::vector<std::pair<T*, float> > & variant)
{
    float totalChance = 0;
    float val;
    typename std::vector<std::pair<T*, float> >::iterator itr;

    if (variant.empty())
        return nullptr;

    for (itr = variant.begin(); itr != variant.end(); ++itr)
        totalChance += itr->second;

    val = Util::getRandomFloat(totalChance);

    for (itr = variant.begin(); itr != variant.end(); ++itr)
    {
        val -= itr->second;
        if (val <= 0) return itr->first;
    }

    // should not come here, buf if it does, we should return something reasonable
    return variant.begin()->first;
}

//////////////////////////////////////////////////////////////////////////////////////////
// LootMgr
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
    for (auto& iter : CreatureLoot)
    {
        auto delIter = iter.second;
        delete delIter;
    }
    for (auto& iter : FishingLoot)
    {
        auto delIter = iter.second;
        delete delIter;
    }
    for (auto& iter : SkinningLoot)
    {
        auto delIter = iter.second;
        delete delIter;
    }
    for (auto& iter : GOLoot)
    {
        auto delIter = iter.second;
        delete delIter;
    }
    for (auto& iter : ItemLoot)
    {
        auto delIter = iter.second;
        delete delIter;
    }
    for (auto& iter : PickpocketingLoot)
    {
        auto delIter = iter.second;
        delete delIter;
    }
}

void LootMgr::fillCreatureLoot(Unit* lootOwner, Loot* loot, uint32_t loot_id, uint8_t type)
{
    loot->clear();
    loot->fillLoot(loot_id, CreatureLoot, lootOwner->ToPlayer(), false, type);
}

void LootMgr::fillGOLoot(Unit* lootOwner, Loot* loot, uint32_t loot_id, uint8_t type)
{
    loot->clear();
    loot->fillLoot(loot_id, GOLoot, lootOwner->ToPlayer(), false, type);
}

void LootMgr::fillFishingLoot(Unit* lootOwner, Loot* loot, uint32_t loot_id, uint8_t type)
{
    loot->clear();
    loot->fillLoot(loot_id, FishingLoot, lootOwner->ToPlayer(), false, type);
}

void LootMgr::fillSkinningLoot(Unit* lootOwner, Loot* loot, uint32_t loot_id, uint8_t type)
{
    loot->clear();
    loot->fillLoot(loot_id, SkinningLoot, lootOwner->ToPlayer(), false, type);
}

void LootMgr::fillPickpocketingLoot(Unit* lootOwner, Loot* loot, uint32_t loot_id, uint8_t type)
{
    loot->clear();
    loot->fillLoot(loot_id, PickpocketingLoot, lootOwner->ToPlayer(), false, type);
}

void LootMgr::fillItemLoot(Unit* lootOwner, Loot* loot, uint32_t loot_id, uint8_t type)
{
    loot->clear();
    loot->fillLoot(loot_id, ItemLoot, lootOwner->ToPlayer(), false, type);
}

bool LootMgr::isCreatureLootable(uint32_t loot_id)
{
    LootTemplateMap::iterator itr = CreatureLoot.find(loot_id);
    if (itr != CreatureLoot.end())
        return true;

    return false;
}

bool LootMgr::isPickpocketable(uint32_t creatureId)
{
    LootTemplateMap::iterator tab = PickpocketingLoot.find(creatureId);
    if (PickpocketingLoot.end() == tab)
        return false;

    return true;
}

bool LootMgr::isSkinnable(uint32_t creatureId)
{
    LootTemplateMap::iterator tab = SkinningLoot.find(creatureId);
    if (SkinningLoot.end() == tab)
        return false;

    return true;
}

bool LootMgr::isFishable(uint32_t zoneid)
{
    LootTemplateMap::iterator tab = FishingLoot.find(zoneid);
    return tab != FishingLoot.end();
}

void LootMgr::loadLoot()
{
    auto startTime = Util::TimeNow();

    //THIS MUST BE CALLED AFTER LOADING OF ITEMS
    is_loading = true;
    loadLootProp();
    is_loading = false;
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

void LootMgr::loadLootProp()
{
    QueryResult* result = WorldDatabase.Query("SELECT * FROM item_randomprop_groups");
    if (result)
    {
        do
        {
            uint32_t id = result->Fetch()[0].GetUInt32();
            uint32_t eid = result->Fetch()[1].GetUInt32();
            float ch = result->Fetch()[2].GetFloat();
            auto item_random_properties = sItemRandomPropertiesStore.LookupEntry(eid);
            if (item_random_properties == nullptr)
            {
                sLogger.failure("RandomProp group %u references non-existent randomprop %u.", id, eid);
                continue;
            }

            std::map<uint32_t, RandomPropertyVector>::iterator itr = _randomprops.find(id);

            if (itr == _randomprops.end())
            {
                RandomPropertyVector v;
                v.push_back(std::make_pair(item_random_properties, ch));
                _randomprops.insert(make_pair(id, v));
            }
            else
            {
                itr->second.push_back(std::make_pair(item_random_properties, ch));
            }
        } while (result->NextRow());
        delete result;
    }

    result = WorldDatabase.Query("SELECT * FROM item_randomsuffix_groups");
    if (result)
    {
        do
        {
            uint32_t id = result->Fetch()[0].GetUInt32();
            uint32_t eid = result->Fetch()[1].GetUInt32();
            float ch = result->Fetch()[2].GetFloat();
            auto item_random_suffix = sItemRandomSuffixStore.LookupEntry(eid);
            if (item_random_suffix == nullptr)
            {
                sLogger.debugFlag(AscEmu::Logging::LF_DB_TABLES, "RandomSuffix group %u references non-existent randomsuffix %u.", id, eid);
                continue;
            }

            std::map<uint32_t, RandomSuffixVector>::iterator itr = _randomsuffix.find(id);

            if (itr == _randomsuffix.end())
            {
                RandomSuffixVector v;
                v.push_back(std::make_pair(item_random_suffix, ch));
                _randomsuffix.insert(make_pair(id, v));
            }
            else
            {
                itr->second.push_back(std::make_pair(item_random_suffix, ch));
            }
        } while (result->NextRow());
        delete result;
    }
}

void LootMgr::loadLootTables(const char* szTableName, LootTemplateMap* LootTable)
{
    QueryResult* result = sMySQLStore.getWorldDBQuery("SELECT * FROM %s ORDER BY entryid ASC", szTableName);
    if (!result)
    {
        sLogger.failure("Loading loot from table %s failed.", szTableName);
        return;
    }

    LootTemplateMap::const_iterator tab;
    uint32_t count = 0;
    do
    {
        Field* fields = result->Fetch();

        std::vector<float> chance;
        chance.reserve(4);

        uint32_t entry = fields[0].GetUInt32();
        uint32_t itemId = fields[1].GetUInt32();
        chance.push_back(fields[2].GetFloat());
        chance.push_back(fields[3].GetFloat());
        chance.push_back(fields[4].GetFloat());
        chance.push_back(fields[5].GetFloat());
        uint32_t  mincount = fields[6].GetUInt32();
        uint32_t  maxcount = fields[7].GetUInt8();

        ItemProperties const* itemProto = sMySQLStore.getItemProperties(itemId);
        if (!itemProto)
        {
            sLogger.debugFlag(AscEmu::Logging::LF_DB_TABLES, "Invalid Item with entry %u set in %s", itemId, szTableName);
            continue;
        }

        LootStoreItem storeitem = LootStoreItem(itemProto, chance, mincount, maxcount);

        // Looking for the template of the entry
        if (LootTable->empty() || tab->first != entry)
        {
            // Searching the template (in case template Id changed)
            tab = LootTable->find(entry);
            if (tab == LootTable->end())
            {
                auto pr = LootTable->insert(LootTemplateMap::value_type(entry, new LootTemplate));
                tab = pr.first;
            }
        }

        // Add Item to our Tempelate
        tab->second->addEntry(storeitem);
        count++;
    } while (result->NextRow());

    sLogger.info("%u loot templates loaded from %s", count, szTableName);
    delete result;
}

DBC::Structures::ItemRandomPropertiesEntry const* LootMgr::GetRandomProperties(ItemProperties const* proto)
{
    if (proto->RandomPropId == 0)
        return nullptr;

    std::map<uint32_t, RandomPropertyVector>::iterator itr = _randomprops.find(proto->RandomPropId);
    if (itr == _randomprops.end())
        return nullptr;

    return RandomChoiceVector<DBC::Structures::ItemRandomPropertiesEntry const>(itr->second);
}

DBC::Structures::ItemRandomSuffixEntry const* LootMgr::GetRandomSuffix(ItemProperties const* proto)
{
    if (proto->RandomSuffixId == 0)
        return nullptr;

    std::map<uint32_t, RandomSuffixVector>::iterator itr = _randomsuffix.find(proto->RandomSuffixId);
    if (itr == _randomsuffix.end())
        return nullptr;

    return RandomChoiceVector<DBC::Structures::ItemRandomSuffixEntry const>(itr->second);
}


//////////////////////////////////////////////////////////////////////////////////////////
// Loot Roll
LootRoll::LootRoll(uint32_t /*timer*/, uint32_t groupcount, uint64_t guid, uint8_t slotid, uint32_t itemid, uint32_t randomsuffixid, uint32_t randompropertyid, WorldMap* mgr) : EventableObject()
{
    _mgr = mgr;
    sEventMgr.AddEvent(this, &LootRoll::finalize, EVENT_LOOT_ROLL_FINALIZE, 60000, 1, 0);
    _groupcount = groupcount;
    _guid = guid;
    _slotid = slotid;
    _itemid = itemid;
    _randomsuffixid = randomsuffixid;
    _randompropertyid = randompropertyid;
    _remaining = groupcount;
}

LootRoll::~LootRoll()
{
    sEventMgr.RemoveEvents(this);
}

void LootRoll::finalize()
{
    sEventMgr.RemoveEvents(this);
    // this we will have to finalize with groups types.. for now
    // we'll just assume need before greed. person with highest roll
    // in need gets the item.
    uint8_t highest = 0;
    int8_t hightype = -1;
    uint64_t player = 0;

    for (std::map<uint32_t, uint8_t>::iterator itr = m_NeedRolls.begin(); itr != m_NeedRolls.end(); ++itr)
    {
        if (itr->second > highest)
        {
            highest = itr->second;
            player = itr->first;
            hightype = ROLL_NEED;
        }
    }
    if (!highest)
    {
        for (std::map<uint32_t, uint8_t>::iterator itr = m_GreedRolls.begin(); itr != m_GreedRolls.end(); ++itr)
        {
            if (itr->second > highest)
            {
                highest = itr->second;
                player = itr->first;
                hightype = ROLL_GREED;
            }
        }
    }

    Creature* creature = nullptr;
    GameObject* gameObject = nullptr;

    Loot* pLoot = nullptr;
    WoWGuid wowGuid;
    wowGuid.Init(_guid);

    if (wowGuid.isUnit())
    {
        creature = _mgr->getCreature(wowGuid.getGuidLowPart());
        if (creature)
            pLoot = &creature->loot;
    }
    else if (wowGuid.isGameObject())
    {
        gameObject = _mgr->getGameObject(wowGuid.getGuidLowPart());
        if (gameObject)
        {
            if (gameObject->IsLootable())
            {
                GameObject_Lootable* pLGO = static_cast<GameObject_Lootable*>(gameObject);
                pLoot = &pLGO->loot;
            }
        }
    }

    if (!pLoot)
    {
        delete this;
        return;
    }

    if (_slotid >= pLoot->items.size())
    {
        delete this;
        return;
    }

    pLoot->items.at(_slotid).roll = nullptr;
    uint32_t amt = pLoot->items.at(_slotid).count;
    if (!amt)
    {
        delete this;
        return;
    }

    Player* _player = (player) ? _mgr->getPlayer((uint32_t)player) : nullptr;
    if (!player || !_player)
    {
        /* all passed */
        std::set<uint32_t>::iterator pitr = m_passRolls.begin();
        while (_player == nullptr && pitr != m_passRolls.end())
            _player = _mgr->getPlayer((*(pitr++)));
        if (_player != nullptr)
        {
            if (_player->isInGroup())
                _player->getGroup()->SendPacketToAll(SmsgLootAllPassed(_guid, _groupcount, _itemid, _randomsuffixid, _randompropertyid).serialise().get());
            else
                _player->getSession()->SendPacket(SmsgLootAllPassed(_guid, _groupcount, _itemid, _randomsuffixid, _randompropertyid).serialise().get());
        }

        /* item can now be looted by anyone :) */
        pLoot->items.at(_slotid).is_passed = true;
        pLoot->items.at(_slotid).is_blocked = false;
        delete this;
        return;
    }

    pLoot->items.at(_slotid).roll = nullptr;

    if (_player->isInGroup())
        _player->getGroup()->SendPacketToAll(SmsgLootRollWon(_guid, _slotid, _itemid, _randomsuffixid, _randompropertyid, _player->getGuid(), highest, hightype).serialise().get());
    else
        _player->getSession()->SendPacket(SmsgLootRollWon(_guid, _slotid, _itemid, _randomsuffixid, _randompropertyid, _player->getGuid(), highest, hightype).serialise().get());

    LootItem& item = _slotid >= pLoot->items.size() ? pLoot->quest_items[_slotid - pLoot->items.size()] : pLoot->items[_slotid];

    // Add Item to Player
    Item* newItem = _player->storeItem(&item);
    if (!newItem)
        return;

    if (creature)
    {
        if (creature->IsInWorld() && creature->isCreature() && creature->GetScript())
            creature->GetScript()->OnLootTaken(_player, item.itemproto);
    }
    else if (gameObject)
    {
        if (gameObject->GetScript())
            gameObject->GetScript()->OnLootTaken(_player, item.itemproto);
    }
    // mark as looted
    pLoot->items.at(_slotid).count = 0;
    pLoot->items.at(_slotid).is_looted = true;

    pLoot->itemRemoved(_slotid);
    --pLoot->unlootedCount;
    delete this;
}

void LootRoll::playerRolled(Player* player, uint8_t choice)
{
    if (m_NeedRolls.find(player->getGuidLow()) != m_NeedRolls.end() || m_GreedRolls.find(player->getGuidLow()) != m_GreedRolls.end())
        return; // don't allow cheaters

    uint8_t roll = static_cast<uint8_t>(Util::getRandomUInt(99) + 1);

    switch (choice)
    {
        case ROLL_PASS:
            m_passRolls.insert(player->getGuidLow());
            roll = 128;
            break;
        case ROLL_GREED:
            m_GreedRolls.insert(std::make_pair(player->getGuidLow(), roll));
            break;
        case ROLL_NEED:
            m_NeedRolls.insert(std::make_pair(player->getGuidLow(), roll));
            break;
        case ROLL_DISENCHANT:
            roll = 128;
            break;
    }

    if (player->isInGroup())
        player->getGroup()->SendPacketToAll(SmsgLootRoll(_guid, _slotid, player->getGuid(), _itemid, _randomsuffixid, _randompropertyid, roll, choice).serialise().get());
    else
        player->getSession()->SendPacket(SmsgLootRoll(_guid, _slotid, player->getGuid(), _itemid, _randomsuffixid, _randompropertyid, roll, choice).serialise().get());
    // check for early completion
    if (!--_remaining)
    {
        // kill event early
        finalize();
    }
}

//////////////////////////////////////////////////////////////////////////////////////////
// Loot Store Item
LootStoreItem::LootStoreItem(ItemProperties const* _itemproto, std::vector<float> _chance, uint32_t _mincount, uint32_t _maxcount)
{
    itemId = _itemproto->ItemId;
    itemproto = _itemproto;
    chance = _chance;
    mincount = _mincount;
    maxcount = _maxcount;
    needs_quest = (itemproto->QuestId > 0);
}

bool LootStoreItem::roll(uint8_t difficulty)
{
    if (chance[difficulty] >= 100.0f)
        return true;

    if (itemproto)
        return Util::checkChance(chance[difficulty]*worldConfig.getFloatRate(RATE_DROP0 + itemproto->Quality));

    return Util::checkChance(chance[difficulty]);
}

//////////////////////////////////////////////////////////////////////////////////////////
// Loot Item
LootItem::LootItem(LootStoreItem const& li)
{
    itemId = li.itemId;
    itemproto = li.itemproto;
    count = Util::getRandomUInt(li.mincount, li.maxcount);
    iRandomProperty = sLootMgr.GetRandomProperties(itemproto);
    iRandomSuffix = sLootMgr.GetRandomSuffix(itemproto);

    roll = nullptr;

    is_passed = false;
    is_looted = false;
    is_blocked = false;
    is_ffa = itemproto && itemproto->Flags & ITEM_FLAG_FREE_FOR_ALL;
    is_underthreshold = false;
    needs_quest = li.needs_quest;
}

bool LootItem::allowedForPlayer(Player* player) const
{
    if (!itemproto)
        return false;

    // not show loot for players without profession or those who already know the recipe
    if ((itemproto->Flags & ITEM_FLAG_SMART_LOOT) && (!player->hasSkillLine(itemproto->RequiredSkill) || player->hasSpell(itemproto->Spells[1].Id)))
        return false;

    // not show loot for not own team
    if ((itemproto->Flags2 & ITEM_FLAG2_HORDE_ONLY) && player->GetTeam() != TEAM_HORDE)
        return false;

    if ((itemproto->Flags2 & ITEM_FLAG2_ALLIANCE_ONLY) && player->GetTeam() != TEAM_ALLIANCE)
        return false;

    // check quest requirements
    if (needs_quest && itemproto->QuestId)
        if (!player->hasQuestForItem(itemId) || !player->hasQuestInQuestLog(itemproto->QuestId))
            return false;

    return true;
}

void LootItem::addAllowedLooter(Player* player)
{
    allowedLooters.insert(player->getGuidLow());
}

//////////////////////////////////////////////////////////////////////////////////////////
// Loot Template
void LootTemplate::addEntry(LootStoreItem& item)
{
    Entries.push_back(item);
}

void LootTemplate::generateLoot(Loot& loot, uint8_t lootDifficulty) const
{
    // Randomize our Loot
    std::random_device rd;
    std::mt19937 mt(rd());

    auto lootEntries = Entries;
    std::shuffle(lootEntries.begin(), lootEntries.end(), mt);

    // Rolling items
    for (auto lootStoreItem : lootEntries)
    {
        // check difficulty level
        if (lootStoreItem.chance[lootDifficulty] < 0.0f)
            continue;

        // Bad luck for the entry
        if (!lootStoreItem.roll(lootDifficulty))
            continue;   

        if (lootStoreItem.itemproto)
        {
            uint8_t item_counterEquipable = 0;
            uint8_t item_counternotEquipable = 0;
            LootItemList::const_iterator _item = loot.items.begin();
            for (; _item != loot.items.end(); ++_item)
            {
                // Non-equippable items are limited to 3 drops
                if (lootStoreItem.itemproto->InventoryType == INVTYPE_NON_EQUIP && item_counternotEquipable < MAX_NR_LOOT_NONEQUIPABLE)
                {
                    item_counternotEquipable++;
                }
                // Equippable item are limited to 2 drops
                else if (lootStoreItem.itemproto->InventoryType != INVTYPE_NON_EQUIP && item_counterEquipable < MAX_NR_LOOT_EQUIPABLE)
                {
                    item_counterEquipable++;
                }
            }

            // Stopp adding Items
            if ((item_counterEquipable == MAX_NR_LOOT_EQUIPABLE && lootStoreItem.itemproto->InventoryType != INVTYPE_NON_EQUIP) || (item_counternotEquipable == MAX_NR_LOOT_NONEQUIPABLE && lootStoreItem.itemproto->InventoryType == INVTYPE_NON_EQUIP))
                continue;
        }

        // add the item to our Loot list
        loot.addLootItem(lootStoreItem);
    }
}

void LootMgr::addLoot(Loot* loot, uint32_t itemid, std::vector<float> chance, uint32_t mincount, uint32_t maxcount, uint8_t lootDifficulty)
{
    if (loot->items.size() > 16)
    {
        sLogger.debug("LootMgr::addLoot cannot add item %u to Loot, Maximum drops reached", itemid);
        return;
    }

    ItemProperties const* itemprop = sMySQLStore.getItemProperties(itemid);

    if (!itemprop)
        return;

    LootStoreItem item = LootStoreItem(itemprop, chance, mincount, maxcount);

    // check difficulty level
    if (item.chance[lootDifficulty] < 0.0f)
    {
        delete &item;
        return;
    }

    // Bad luck for the entry
    if (!item.roll(0))
    {
        delete &item;
        return;
    }

    // add the Item to our Loot List
    loot->addLootItem(item);
}

bool LootTemplate::hasQuestDrop(LootTemplateMap const& /*store*/) const
{
    for (LootStoreItemList::const_iterator i = Entries.begin(); i != Entries.end(); ++i)
    {
        if (i->needs_quest)
            return true;    // quest drop found
    }

    return false;
}

bool LootTemplate::hasQuestDropForPlayer(LootTemplateMap const& /*store*/, Player* player) const
{
    for (LootStoreItemList::const_iterator i = Entries.begin(); i != Entries.end(); ++i)
    {
        if (player->hasQuestForItem(i->itemId))
            return true;    // active quest drop found
    }

    return false;
}

//////////////////////////////////////////////////////////////////////////////////////////
// Loot
uint32_t Loot::getMaxSlotInLootFor(Player* player) const
{
    PersonaltemMap::const_iterator itr = PlayerQuestItems.find(player->getGuidLow());
    return static_cast<uint32_t>(items.size() + (itr != PlayerQuestItems.end() ? itr->second->size() : 0));
}

bool Loot::fillLoot(uint32_t lootId, LootTemplateMap const& tempelateStore, Player* lootOwner, bool personal, uint8_t lootMode /*= InstanceDifficulty::DUNGEON_NORMAL*/)
{
    LootTemplate const* tempelate;

    // Must be provided
    if (!lootOwner)
        return false;

    auto temp = tempelateStore.find(lootId);
    if (temp == tempelateStore.end())
        tempelate = nullptr;
    else
        tempelate = temp->second;

    if (!tempelate)
        return false;

    items.reserve(MAX_NR_LOOT_ITEMS);
    quest_items.reserve(MAX_NR_LOOT_QUESTITEMS);

    tempelate->generateLoot(*this, lootMode);

    // If Player is in a Group add Personal loot to them
    Group* group = lootOwner->getGroup();
    if (!personal && group)
    {
        // Player allowed to Loot this Round
        roundRobinPlayer = lootOwner->getGuid();

        for (uint8_t i = 0; i < group->GetSubGroupCount(); i++)
        {
            if (group->GetSubGroup(i) != nullptr)
            {
                for (auto itr = group->GetSubGroup(i)->GetGroupMembersBegin(); itr != group->GetSubGroup(i)->GetGroupMembersEnd(); ++itr)
                {
                    if ((*itr) != nullptr)
                    {
                        if (Player* loggedInPlayer = sObjectMgr.GetPlayer((*itr)->guid))
                        {
                            fillNotNormalLootFor(loggedInPlayer, loggedInPlayer->isAtGroupRewardDistance(lootOwner));
                        }
                    }
                }
            }
        }

        for (uint8_t i = 0; i < items.size(); ++i)
        {
            if (items[i].itemproto)
                if (items[i].itemproto->Quality < uint32_t(group->GetThreshold()))
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

void Loot::fillNotNormalLootFor(Player* player, bool presentAtLooting)
{
    uint32_t plguid = player->getGuidLow();

    // Add Quest Items
    PersonaltemMap::const_iterator personaltem = PlayerQuestItems.find(plguid);
    if (personaltem == PlayerQuestItems.end())
    {
        if (items.size() == MAX_NR_LOOT_ITEMS)
            return;

        PersonaltemList* personalList = new PersonaltemList();

        for (uint8_t i = 0; i < quest_items.size(); ++i)
        {
            LootItem &item = quest_items[i];

            if (!item.is_looted && item.allowedForPlayer(player) && ((player->getGroup() && ((player->getGroup()->GetMethod() == PARTY_LOOT_MASTER_LOOTER && player->getGroup()->GetLooter() && player->getGroup()->GetLooter()->guid == player->getGuidLow()) || player->getGroup()->GetMethod() != PARTY_LOOT_MASTER_LOOTER))))
            {
                personalList->push_back(Personaltem(i));

                // increase once if one looter only, looter-times if free for all
                if (item.is_ffa || !item.is_blocked)
                    ++unlootedCount;
                if (!player->getGroup() || (player->getGroup()->GetMethod() != PARTY_LOOT_GROUP && player->getGroup()->GetMethod() != PARTY_LOOT_ROUND_ROBIN))
                    item.is_blocked = true;

                if (items.size() + personalList->size() == MAX_NR_LOOT_ITEMS)
                    break;
            }
        }
        if (personalList->empty())
            delete personalList;
        else
            PlayerQuestItems[plguid] = personalList;
    }

    // Add Free For All Items
    personaltem = PlayerFFAItems.find(plguid);
    if (personaltem == PlayerFFAItems.end())
    {
        PersonaltemList* personalList = new PersonaltemList();

        for (uint8_t i = 0; i < items.size(); ++i)
        {
            LootItem &item = items[i];
            if (!item.is_looted && item.is_ffa && item.allowedForPlayer(player))
            {
                personalList->push_back(Personaltem(i));
                ++unlootedCount;
            }
        }
        if (personalList->empty())
            delete personalList;
        else
            PlayerFFAItems[plguid] = personalList;
    }

    // Add NonQuest and Non FFA Items
    for (uint8_t i = 0; i < items.size(); ++i)
    {
        LootItem &item = items[i];
        if (!item.is_looted && !item.is_ffa && (item.allowedForPlayer(player) || 
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
    uint32_t max_slot = getMaxSlotInLootFor(player);
    LootItem* item = nullptr;
    uint32_t itemsSize = uint32_t(items.size());
    for (uint32_t i = 0; i < max_slot; ++i)
    {
        if (i < items.size())
            item = &items[i];
        else if ((i - itemsSize) < quest_items.size())
            item = &quest_items[i - itemsSize];

        if (!item->is_looted && item->is_ffa && item->allowedForPlayer(player))
            if (item->itemproto)
                if (item->itemproto->isCurrencyToken())
                    player->storeNewLootItem(i, this);
    }
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
        if (auto properties_difficulty = sMySQLStore.getCreatureProperties(creature_difficulty_entry))
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

void Loot::addLootItem(LootStoreItem const & item)
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
    for (PersonaltemMap::const_iterator itr = PlayerQuestItems.begin(); itr != PlayerQuestItems.end(); ++itr)
        delete itr->second;
    PlayerQuestItems.clear();

    for (PersonaltemMap::const_iterator itr = PlayerFFAItems.begin(); itr != PlayerFFAItems.end(); ++itr)
        delete itr->second;
    PlayerFFAItems.clear();

    PlayersLooting.clear();
    items.clear();
    quest_items.clear();
    gold = 0;
    unlootedCount = 0;
    roundRobinPlayer = 0;
}

LootItem* Loot::lootItemInSlot(uint32_t lootSlot, Player* player, Personaltem* *qitem, Personaltem* *ffaitem)
{
    LootItem* item = nullptr;
    bool is_looted = true;
    if (lootSlot >= items.size())
    {
        uint32_t questSlot = lootSlot - static_cast<uint32_t>(items.size());
        PersonaltemMap::const_iterator itr = PlayerQuestItems.find(player->getGuidLow());
        if (itr != PlayerQuestItems.end() && questSlot < itr->second->size())
        {
            Personaltem* qitem2 = &itr->second->at(questSlot);
            if (qitem)
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
            PersonaltemMap::const_iterator itr = PlayerFFAItems.find(player->getGuidLow());
            if (itr != PlayerFFAItems.end())
            {
                for (PersonaltemList::const_iterator iter = itr->second->begin(); iter != itr->second->end(); ++iter)
                    if (iter->index == lootSlot)
                    {
                        Personaltem* ffaitem2 = (Personaltem*)&(*iter);
                        if (ffaitem)
                            *ffaitem = ffaitem2;
                        is_looted = ffaitem2->is_looted;
                        break;
                    }
            }
        }
    }

    if (is_looted)
        return nullptr;

    return item;
}

LootItem* Loot::getlootItemInSlot(uint32_t lootSlot, Player* player)
{
    LootItem* item = nullptr;
    if (lootSlot >= items.size())
    {
        uint32_t questSlot = lootSlot - static_cast<uint32_t>(items.size());
        PersonaltemMap::const_iterator itr = PlayerQuestItems.find(player->getGuidLow());
        if (itr != PlayerQuestItems.end() && questSlot < itr->second->size())
        {
            Personaltem* qitem2 = &itr->second->at(questSlot);
            item = &quest_items[qitem2->index];
        }
    }
    else
    {
        item = &items[lootSlot];
    }

    return item;
}

bool Loot::hasItemFor(Player* player) const
{
    // Personal Quest Items
    if (getPlayerQuestItems().find(player->getGuidLow()) != getPlayerQuestItems().end())
    {
        PersonaltemList* quest_list = getPlayerQuestItems().find(player->getGuidLow())->second;
        for (PersonaltemList::const_iterator personalItem = quest_list->begin(); personalItem != quest_list->end(); ++personalItem)
        {
            const LootItem &item = quest_items[personalItem->index];
            if (!personalItem->is_looted && !item.is_looted && item.allowedForPlayer(player))
                return true;
        }
    }

    // Personal Free for all Items
    if (getPlayerFFAItems().find(player->getGuidLow()) != getPlayerFFAItems().end())
    {
        PersonaltemList* ffa_list = getPlayerFFAItems().find(player->getGuidLow())->second;
        for (PersonaltemList::const_iterator personalFfaItem = ffa_list->begin(); personalFfaItem != ffa_list->end(); ++personalFfaItem)
        {
            const LootItem &item = items[personalFfaItem->index];
            if (!personalFfaItem->is_looted && !item.is_looted && item.allowedForPlayer(player))
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
        if (Player* player = sObjectMgr.GetPlayer(playerGuid))
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
        if (Player* player = sObjectMgr.GetPlayer(playerGuid))
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
