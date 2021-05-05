/*
* AscEmu Framework based on ArcEmu MMORPG Server
* Copyright (c) 2014-2021 AscEmu Team <http://www.ascemu.org>
* Copyright (C) 2008-2012 ArcEmu Team <http://www.ArcEmu.org/>
* Copyright (C) 2005-2007 Ascent Team
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU Affero General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU Affero General Public License for more details.
*
* You should have received a copy of the GNU Affero General Public License
* along with this program. If not, see <http://www.gnu.org/licenses/>.
*
*/

#include "StdAfx.h"
#include "Storage/DBC/DBCStores.h"
#include "Management/Item.h"
#include "Management/ItemInterface.h"
#include "Storage/MySQLDataStore.hpp"
#include "Server/MainServerDefines.h"
#include "Server/WorldConfig.h"
#include "Map/MapMgr.h"
#include "Server/Packets/SmsgLootRemoved.h"
#include "Server/Packets/SmsgLootAllPassed.h"
#include "Server/Packets/SmsgLootRollWon.h"
#include "Server/Packets/SmsgLootRoll.h"

using namespace AscEmu::Packets;

struct loot_tb
{
    uint32 itemid;
    float chance;
};

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

bool Loot::any() const
{
    return gold > 0 || !items.empty();
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

void LootMgr::LoadLoot()
{
    auto startTime = Util::TimeNow();

    //THIS MUST BE CALLED AFTER LOADING OF ITEMS
    is_loading = true;
    LoadLootProp();
    is_loading = false;
}

void LootMgr::loadAndGenerateLoot(uint8_t type)
{
    switch (type)
    {
        case 0:
            LoadLootTables("loot_creatures", &CreatureLoot);
            break;
        case 1:
            LoadLootTables("loot_gameobjects", &GOLoot);
            break;
        case 2:
            LoadLootTables("loot_skinning", &SkinningLoot);
            break;
        case 3:
            LoadLootTables("loot_fishing", &FishingLoot);
            break;
        case 4:
            LoadLootTables("loot_items", &ItemLoot);
            break;
        case 5:
            LoadLootTables("loot_pickpocketing", &PickpocketingLoot);
            break;
    }
}

DBC::Structures::ItemRandomPropertiesEntry const* LootMgr::GetRandomProperties(ItemProperties const* proto)
{
    if (proto->RandomPropId == 0)
        return nullptr;

    std::map<uint32, RandomPropertyVector>::iterator itr = _randomprops.find(proto->RandomPropId);
    if (itr == _randomprops.end())
        return nullptr;

    return RandomChoiceVector<DBC::Structures::ItemRandomPropertiesEntry const>(itr->second);
}

DBC::Structures::ItemRandomSuffixEntry const* LootMgr::GetRandomSuffix(ItemProperties const* proto)
{
    if (proto->RandomSuffixId == 0)
        return nullptr;

    std::map<uint32, RandomSuffixVector>::iterator itr = _randomsuffix.find(proto->RandomSuffixId);
    if (itr == _randomsuffix.end())
        return nullptr;

    return RandomChoiceVector<DBC::Structures::ItemRandomSuffixEntry const>(itr->second);
}

void LootMgr::LoadLootProp()
{
    QueryResult* result = WorldDatabase.Query("SELECT * FROM item_randomprop_groups");
    if (result)
    {
        do
        {
            uint32 id = result->Fetch()[0].GetUInt32();
            uint32 eid = result->Fetch()[1].GetUInt32();
            float ch = result->Fetch()[2].GetFloat();
            auto item_random_properties = sItemRandomPropertiesStore.LookupEntry(eid);
            if (item_random_properties == nullptr)
            {
                LOG_ERROR("RandomProp group %u references non-existent randomprop %u.", id, eid);
                continue;
            }

            std::map<uint32, RandomPropertyVector>::iterator itr = _randomprops.find(id);

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
        }
        while (result->NextRow());
        delete result;
    }

    result = WorldDatabase.Query("SELECT * FROM item_randomsuffix_groups");
    if (result)
    {
        do
        {
            uint32 id = result->Fetch()[0].GetUInt32();
            uint32 eid = result->Fetch()[1].GetUInt32();
            float ch = result->Fetch()[2].GetFloat();
            auto item_random_suffix = sItemRandomSuffixStore.LookupEntry(eid);
            if (item_random_suffix == nullptr)
            {
                LOG_ERROR("RandomSuffix group %u references non-existent randomsuffix %u.", id, eid);
                continue;
            }

            std::map<uint32, RandomSuffixVector>::iterator itr = _randomsuffix.find(id);

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
        }
        while (result->NextRow());
        delete result;
    }
}

void LootMgr::finalize()
{
    LOG_DETAIL(" Deleting Loot Tables...");
    for (LootStore::iterator iter = CreatureLoot.begin(); iter != CreatureLoot.end(); ++iter)
        delete[] iter->second.items;
    for (LootStore::iterator iter = FishingLoot.begin(); iter != FishingLoot.end(); ++iter)
        delete[] iter->second.items;
    for (LootStore::iterator iter = SkinningLoot.begin(); iter != SkinningLoot.end(); ++iter)
        delete[] iter->second.items;
    for (LootStore::iterator iter = GOLoot.begin(); iter != GOLoot.end(); ++iter)
        delete[] iter->second.items;
    for (LootStore::iterator iter = ItemLoot.begin(); iter != ItemLoot.end(); ++iter)
        delete[] iter->second.items;
    for (LootStore::iterator iter = PickpocketingLoot.begin(); iter != PickpocketingLoot.end(); ++iter)
        delete[] iter->second.items;
}

void LootMgr::LoadLootTables(const char* szTableName, LootStore* LootTable)
{
    std::vector< std::pair< uint32, std::vector< tempy > > > db_cache;
    db_cache.reserve(10000);
    LootStore::iterator tab;
    QueryResult* result = WorldDatabase.Query("SELECT * FROM %s ORDER BY entryid ASC", szTableName);
    if (!result)
    {
        LOG_ERROR("Loading loot from table %s failed.", szTableName);
        return;
    }

    uint32 last_entry = 0;
    std::vector< tempy > ttab;
    
    do
    {
        Field* fields = result->Fetch();
        uint32 entry_id = fields[0].GetUInt32();
        if (entry_id < last_entry)
        {
            LOG_ERROR("WARNING: Out of order loot table being loaded.");
            return;
        }

        if (entry_id != last_entry)
        {
            if (last_entry != 0)
                db_cache.push_back(make_pair(last_entry, ttab));
            ttab.clear();
        }

        tempy t;
        t.itemid = fields[1].GetUInt32();
        t.chance = fields[2].GetFloat();
        t.chance_2 = fields[3].GetFloat();
        t.chance3 = fields[4].GetFloat();
        t.chance4 = fields[5].GetFloat();
        t.mincount = fields[6].GetUInt32();
        t.maxcount = fields[7].GetUInt32();
        ttab.push_back(t);
        last_entry = entry_id;
    }
    while (result->NextRow());
    //last list was not pushed in
    if (last_entry != 0 && ttab.size())
        db_cache.push_back(make_pair(last_entry, ttab));

    for (std::vector<std::pair<uint32, std::vector<tempy>>>::iterator itr = db_cache.begin(); itr != db_cache.end(); ++itr)
    {
        uint32 entry_id = (*itr).first;
        if (LootTable->end() == LootTable->find(entry_id))
        {
            StoreLootList list;
            list.count = static_cast<uint32>(itr->second.size());
            list.items = new StoreLootItem[list.count];
            uint32 ind = 0;
            for (std::vector< tempy >::iterator itr2 = itr->second.begin(); itr2 != itr->second.end(); ++itr2)
            {
                //Omit items that are not in db to prevent future bugs
                uint32 itemid = itr2->itemid;
                ItemProperties const* proto = sMySQLStore.getItemProperties(itemid);
                if (!proto)
                {
                    list.items[ind].item.itemproto = nullptr;
                    LogDebugFlag(LF_DB_TABLES, "Loot for %u contains non-existant item %u . (%s)", entry_id, itemid, szTableName);
                }
                else
                {
                    list.items[ind].item.itemproto = proto;
                    list.items[ind].item.displayid = proto->DisplayInfoID;
                    list.items[ind].chance = itr2->chance;
                    list.items[ind].chance2 = itr2->chance_2;
                    list.items[ind].chance3 = itr2->chance3;
                    list.items[ind].chance4 = itr2->chance4;
                    list.items[ind].mincount = itr2->mincount;
                    list.items[ind].maxcount = itr2->maxcount;

                    if (proto->HasFlag(ITEM_FLAG_FREE_FOR_ALL))
                        list.items[ind].ffa_loot = 1;
                    else
                        list.items[ind].ffa_loot = 0;

                    if (LootTable == &GOLoot)
                    {
                        if (proto->Class == ITEM_CLASS_QUEST)
                        {
                            sQuestMgr.SetGameObjectLootQuest(itr->first, itemid);
                            quest_loot_go[entry_id].insert(proto->ItemId);
                        }
                    }
                }
                ind++;
            }
            (*LootTable)[entry_id] = list;
        }
    }
    LogDetail("%u loot templates loaded from %s", static_cast<uint32_t>(db_cache.size()), szTableName);
    delete result;
}

void LootMgr::PushLoot(StoreLootList* list, Loot* loot, uint8 type)
{
    uint32 i;
    uint32 count;
    if (type >= NUM_LOOT_TYPES)
        return;

    for (uint32 x = 0; x < list->count; x++)
    {
        if (list->items[x].item.itemproto) // this check is needed until loot DB is fixed
        {
            float chance = 0.0f;
            switch (type)
            {
                case LOOT_NORMAL10:
                    chance = list->items[x].chance;
                    break;
                case LOOT_NORMAL25:
                    chance = list->items[x].chance2;
                    break;
                case LOOT_HEROIC10:
                    chance = list->items[x].chance3;
                    break;
                case LOOT_HEROIC25:
                    chance = list->items[x].chance4;
                    break;
            }
            // drop chance cannot be larger than 100% or smaller than 0%
            if (chance <= 0.0f || chance > 100.0f)
                continue;

            ItemProperties const* itemproto = list->items[x].item.itemproto;
            if (Util::checkChance(chance * worldConfig.getFloatRate((WorldConfigRates)(RATE_DROP0 + itemproto->Quality)))) //|| itemproto->Class == ITEM_CLASS_QUEST)
            {
                if (list->items[x].mincount == list->items[x].maxcount)
                    count = list->items[x].maxcount;
                else
                    count = Util::getRandomUInt(list->items[x].maxcount - list->items[x].mincount) + list->items[x].mincount;

                for (i = 0; i < loot->items.size(); ++i)
                {
                    //itemid rand match a already placed item, if item is stackable and unique(stack), increment it, otherwise skips
                    if ((loot->items[i].item.itemproto == list->items[x].item.itemproto) && itemproto->MaxCount && ((loot->items[i].iItemsCount + count) < itemproto->MaxCount))
                    {
                        if (itemproto->Unique && ((loot->items[i].iItemsCount + count) < itemproto->Unique))
                        {
                            loot->items[i].iItemsCount += count;
                            break;
                        }
                        if (!itemproto->Unique)
                        {
                            loot->items[i].iItemsCount += count;
                            break;
                        }
                    }
                }

                if (i != loot->items.size())
                    continue;

                __LootItem itm;
                itm.item = list->items[x].item;
                itm.iItemsCount = count;
                itm.roll = nullptr;
                itm.passed = false;
                itm.ffa_loot = list->items[x].ffa_loot;
                itm.has_looted.clear();

                if (itemproto->Quality > 1 && itemproto->ContainerSlots == 0)
                {
                    itm.iRandomProperty = GetRandomProperties(itemproto);
                    itm.iRandomSuffix = GetRandomSuffix(itemproto);
                }
                else
                {
                    // save some calls :P
                    itm.iRandomProperty = nullptr;
                    itm.iRandomSuffix = nullptr;
                }
                loot->items.push_back(itm);
            }
        }
    }
    if (loot->items.size() > 16)
    {
        while (loot->items.size() > 16)
        {
            std::vector<__LootItem>::iterator item_to_remove = loot->items.begin();
            for (std::vector<__LootItem>::iterator itr = loot->items.begin(); itr != loot->items.end(); ++itr)
            {
                uint32 item_quality = (*itr).item.itemproto->Quality;
                bool quest_item = (*itr).item.itemproto->Class == ITEM_CLASS_QUEST;
                if ((*item_to_remove).item.itemproto->Quality > item_quality && !quest_item)
                {
                    item_to_remove = itr;
                }
            }
            loot->items.erase(item_to_remove);
        }
    }
}

void LootMgr::AddLoot(Loot* loot, uint32 itemid, uint32 mincount, uint32 maxcount)
{
    uint32 i;
    uint32 count;

    ItemProperties const* itemproto = sMySQLStore.getItemProperties(itemid);
    if (itemproto) // this check is needed until loot DB is fixed
    {
        if (mincount == maxcount)
            count = maxcount;
        else
            count = Util::getRandomUInt(maxcount - mincount) + mincount;

        for (i = 0; i < loot->items.size(); ++i)
        {
            //itemid rand match a already placed item, if item is stackable and unique(stack), increment it, otherwise skips
            if ((loot->items[i].item.itemproto == itemproto) && itemproto->MaxCount && ((loot->items[i].iItemsCount + count) < itemproto->MaxCount))
            {
                if (itemproto->Unique && ((loot->items[i].iItemsCount + count) < itemproto->Unique))
                {
                    loot->items[i].iItemsCount += count;
                    break;
                }

                if (!itemproto->Unique)
                {
                    loot->items[i].iItemsCount += count;
                    break;
                }
            }
        }

        if (i != loot->items.size())
            return;

        _LootItem item;
        item.itemproto = itemproto;
        item.displayid = itemproto->DisplayInfoID;
        __LootItem itm;
        itm.item = item;
        itm.iItemsCount = count;
        itm.roll = nullptr;
        itm.passed = false;

        if (itemproto->HasFlag(ITEM_FLAG_FREE_FOR_ALL))
            itm.ffa_loot = 1;
        else
            itm.ffa_loot = 0;

        itm.has_looted.clear();
        if (itemproto->Quality > 1 && itemproto->ContainerSlots == 0)
        {
            itm.iRandomProperty = GetRandomProperties(itemproto);
            itm.iRandomSuffix = GetRandomSuffix(itemproto);
        }
        else
        {
            // save some calls :P
            itm.iRandomProperty = nullptr;
            itm.iRandomSuffix = nullptr;
        }
        loot->items.push_back(itm);
    }
    if (loot->items.size() > 16)
    {
        while (loot->items.size() > 16)
        {
            std::vector<__LootItem>::iterator item_to_remove = loot->items.begin();
            uint32 item_quality = 0;
            bool quest_item = false;
            for (std::vector<__LootItem>::iterator itr = loot->items.begin(); itr != loot->items.end(); ++itr)
            {
                item_quality = (*itr).item.itemproto->Quality;
                quest_item = (*itr).item.itemproto->Class == ITEM_CLASS_QUEST;
                if ((*item_to_remove).item.itemproto->Quality > item_quality && !quest_item)
                {
                    item_to_remove = itr;
                }
            }
            loot->items.erase(item_to_remove);
        }
    }
}

bool LootMgr::HasLootForCreature(uint32 loot_id)
{
    LootStore::iterator itr = CreatureLoot.find(loot_id);
    if (itr != CreatureLoot.end())
        return true;

    return false;
}

void LootMgr::FillCreatureLoot(Loot* loot, uint32 loot_id, uint8 type)
{
    loot->items.clear();
    loot->gold = 0;
    LootStore::iterator tab = CreatureLoot.find(loot_id);
    if (CreatureLoot.end() != tab)
        PushLoot(&tab->second, loot, type);
}

void LootMgr::FillGOLoot(Loot* loot, uint32 loot_id, uint8 type)
{
    loot->items.clear();
    loot->gold = 0;
    LootStore::iterator tab = GOLoot.find(loot_id);
    if (GOLoot.end() != tab)
        PushLoot(&tab->second, loot, type);
}

void LootMgr::FillFishingLoot(Loot* loot, uint32 loot_id)
{
    loot->items.clear();
    loot->gold = 0;
    LootStore::iterator tab = FishingLoot.find(loot_id);
    if (FishingLoot.end() != tab)
        PushLoot(&tab->second, loot, 0);
}

void LootMgr::FillSkinningLoot(Loot* loot, uint32 loot_id)
{
    loot->items.clear();
    loot->gold = 0;
    LootStore::iterator tab = SkinningLoot.find(loot_id);
    if (SkinningLoot.end() != tab)
        PushLoot(&tab->second, loot, 0);
}

void LootMgr::FillPickpocketingLoot(Loot* loot, uint32 loot_id)
{
    loot->items.clear();
    loot->gold = 0;
    LootStore::iterator tab = PickpocketingLoot.find(loot_id);
    if (PickpocketingLoot.end() != tab)
        PushLoot(&tab->second, loot, 0);
}

bool LootMgr::CanGODrop(uint32 LootId, uint32 itemid)
{
    LootStore::iterator tab = GOLoot.find(LootId);
    if (GOLoot.end() == tab)
        return false;
    StoreLootList* list = &(tab->second);
    for (uint32 x = 0; x < list->count; x++)
        if (list->items[x].item.itemproto->ItemId == itemid)
            return true;
    return false;
}

//THIS should be cached
bool LootMgr::IsPickpocketable(uint32 creatureId)
{
    LootStore::iterator tab = PickpocketingLoot.find(creatureId);
    if (PickpocketingLoot.end() == tab)
        return false;

    return true;
}

//THIS should be cached
bool LootMgr::IsSkinnable(uint32 creatureId)
{
    LootStore::iterator tab = SkinningLoot.find(creatureId);
    if (SkinningLoot.end() == tab)
        return false;

    return true;
}

//THIS should be cached
bool LootMgr::IsFishable(uint32 zoneid)
{
    LootStore::iterator tab = FishingLoot.find(zoneid);
    return tab != FishingLoot.end();
}

LootRoll::LootRoll(uint32 /*timer*/, uint32 groupcount, uint64 guid, uint32 slotid, uint32 itemid, uint32 randomsuffixid, uint32 randompropertyid, MapMgr* mgr) : EventableObject()
{
    _mgr = mgr;
    sEventMgr.AddEvent(this, &LootRoll::Finalize, EVENT_LOOT_ROLL_FINALIZE, 60000, 1, 0);
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

void LootRoll::Finalize()
{
    sEventMgr.RemoveEvents(this);
    // this we will have to finalize with groups types.. for now
    // we'll just assume need before greed. person with highest roll
    // in need gets the item.
    uint8_t highest = 0;
    int8 hightype = -1;
    uint64 player = 0;

    for (std::map<uint32, uint8_t>::iterator itr = m_NeedRolls.begin(); itr != m_NeedRolls.end(); ++itr)
    {
        if (itr->second > highest)
        {
            highest = itr->second;
            player = itr->first;
            hightype = NEED;
        }
    }
    if (!highest)
    {
        for (std::map<uint32, uint8_t>::iterator itr = m_GreedRolls.begin(); itr != m_GreedRolls.end(); ++itr)
        {
            if (itr->second > highest)
            {
                highest = itr->second;
                player = itr->first;
                hightype = GREED;
            }
        }
    }

    Loot* pLoot = nullptr;
    WoWGuid wowGuid;
    wowGuid.Init(_guid);

    if (wowGuid.isUnit())
    {
        Creature* pc = _mgr->GetCreature(wowGuid.getGuidLowPart());
        if (pc)
            pLoot = &pc->loot;
    }
    else if (wowGuid.isGameObject())
    {
        GameObject* go = _mgr->GetGameObject(wowGuid.getGuidLowPart());
        if (go != nullptr)
        {
            if (go->IsLootable())
            {
                GameObject_Lootable* pLGO = static_cast<GameObject_Lootable*>(go);
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
    uint32 itemid = pLoot->items.at(_slotid).item.itemproto->ItemId;
    uint32 amt = pLoot->items.at(_slotid).iItemsCount;
    if (!amt)
    {
        delete this;
        return;
    }

    Player* _player = (player) ? _mgr->GetPlayer((uint32)player) : nullptr;
    if (!player || !_player)
    {
        /* all passed */
        std::set<uint32>::iterator pitr = m_passRolls.begin();
        while (_player == nullptr && pitr != m_passRolls.end())
            _player = _mgr->GetPlayer((*(pitr++)));
        if (_player != nullptr)
        {
            if (_player->isInGroup())
                _player->getGroup()->SendPacketToAll(SmsgLootAllPassed(_guid, _groupcount, _itemid, _randomsuffixid, _randompropertyid).serialise().get());
            else
                _player->GetSession()->SendPacket(SmsgLootAllPassed(_guid, _groupcount, _itemid, _randomsuffixid, _randompropertyid).serialise().get());
        }

        /* item can now be looted by anyone :) */
        pLoot->items.at(_slotid).passed = true;
        delete this;
        return;
    }

    pLoot->items.at(_slotid).roll = nullptr;

    if (_player->isInGroup())
        _player->getGroup()->SendPacketToAll(SmsgLootRollWon(_guid, _slotid, _itemid, _randomsuffixid, _randompropertyid, _player->getGuid(), highest, hightype).serialise().get());
    else
        _player->GetSession()->SendPacket(SmsgLootRollWon(_guid, _slotid, _itemid, _randomsuffixid, _randompropertyid, _player->getGuid(), highest, hightype).serialise().get());

    ItemProperties const* it = sMySQLStore.getItemProperties(itemid);
    int8 error;
    if ((error = _player->getItemInterface()->CanReceiveItem(it, 1)) != 0)
    {
        _player->getItemInterface()->buildInventoryChangeError(nullptr, nullptr, error, itemid);
        return;
    }

    Item* add = _player->getItemInterface()->FindItemLessMax(itemid, amt, false);
    if (!add)
    {
        SlotResult slotresult = _player->getItemInterface()->FindFreeInventorySlot(it);
        if (!slotresult.Result)
        {
            _player->getItemInterface()->buildInventoryChangeError(nullptr, nullptr, INV_ERR_INVENTORY_FULL);
            return;
        }
        LOG_DEBUG("AutoLootItem MISC");

        Item* item = sObjectMgr.CreateItem(itemid, _player);
        if (item == nullptr)
            return;

        item->setStackCount(amt);

        if (pLoot->items.at(_slotid).iRandomProperty != nullptr)
        {
            item->setRandomPropertiesId(pLoot->items.at(_slotid).iRandomProperty->ID);
            item->ApplyRandomProperties(false);
        }
        else if (pLoot->items.at(_slotid).iRandomSuffix != nullptr)
        {
            item->SetRandomSuffix(pLoot->items.at(_slotid).iRandomSuffix->id);
            item->ApplyRandomProperties(false);
        }

        if (_player->getItemInterface()->SafeAddItem(item, slotresult.ContainerSlot, slotresult.Slot))
        {
            _player->sendItemPushResultPacket(false, true, true, slotresult.ContainerSlot, slotresult.Slot, 1, item->getEntry(), item->getPropertySeed(), item->getRandomPropertiesId(), item->getStackCount());
            sQuestMgr.OnPlayerItemPickup(_player, item);
#if VERSION_STRING > TBC
            _player->GetAchievementMgr().UpdateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_LOOT_ITEM, item->getEntry(), 1, 0);
#endif
        }
        else
            item->DeleteMe();
    }
    else
    {
        add->setStackCount(add->getStackCount() + amt);
        add->m_isDirty = true;
        sQuestMgr.OnPlayerItemPickup(_player, add);
        _player->sendItemPushResultPacket(false, true, true, (uint8)_player->getItemInterface()->GetBagSlotByGuid(add->getGuid()), 0, 1, add->getEntry(), add->getPropertySeed(), add->getRandomPropertiesId(), add->getStackCount());
#if VERSION_STRING > TBC
        _player->GetAchievementMgr().UpdateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_LOOT_ITEM, add->getEntry(), 1, 0);
#endif
    }
    pLoot->items.at(_slotid).iItemsCount = 0;

    for (LooterSet::iterator itr = pLoot->looters.begin(); itr != pLoot->looters.end(); ++itr)
    {
        if (const auto plr = _player->GetMapMgr()->GetPlayer(*itr))
            plr->GetSession()->SendPacket(AscEmu::Packets::SmsgLootRemoved(_slotid).serialise().get());
    }
    delete this;
}

void LootRoll::PlayerRolled(Player* player, uint8 choice)
{
    if (m_NeedRolls.find(player->getGuidLow()) != m_NeedRolls.end() || m_GreedRolls.find(player->getGuidLow()) != m_GreedRolls.end())
        return; // don't allow cheaters

    uint8_t roll = static_cast<uint8_t>(Util::getRandomUInt(99) + 1);
    uint8_t rollType = choice;

    if (choice == NEED)
    {
        m_NeedRolls.insert(std::make_pair(player->getGuidLow(), roll));
    }
    else if (choice == GREED)
    {
        m_GreedRolls.insert(std::make_pair(player->getGuidLow(), roll));
    }
    else
    {
        m_passRolls.insert(player->getGuidLow());
        roll = 128;
        rollType = 128;
    }

    if (player->isInGroup())
        player->getGroup()->SendPacketToAll(SmsgLootRoll(_guid, _slotid, player->getGuid(), _itemid, _randomsuffixid, _randompropertyid, roll, rollType).serialise().get());
    else
        player->GetSession()->SendPacket(SmsgLootRoll(_guid, _slotid, player->getGuid(), _itemid, _randomsuffixid, _randompropertyid, roll, rollType).serialise().get());
    // check for early completion
    if (!--_remaining)
    {
        // kill event early
        //sEventMgr.RemoveEvents(this);
        Finalize();
    }
}

void LootMgr::FillItemLoot(Loot* loot, uint32 loot_id)
{
    loot->items.clear();
    loot->gold = 0;
    LootStore::iterator tab = ItemLoot.find(loot_id);
    if (ItemLoot.end() != tab)
        PushLoot(&tab->second, loot, false);
}

int32 LootRoll::event_GetInstanceID()
{
    return _mgr->GetInstanceID();
}
