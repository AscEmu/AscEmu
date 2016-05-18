/*
Copyright (c) 2014-2016 AscEmu Team <http://www.ascemu.org/>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "StdAfx.h"

initialiseSingleton(MySQLDataStore);

MySQLDataStore::MySQLDataStore() {}
MySQLDataStore::~MySQLDataStore() {}

void MySQLDataStore::LoadItemsTable()
{
    uint32 start_time = getMSTime();

    QueryResult* item_result = WorldDatabase.Query("SELECT * FROM items");

    //                                                         0      1       2        3       4        5         6       7       8       9          10
    /*QueryResult* item_result = WorldDatabase.Query("SELECT entry, class, subclass, field4, name1, displayid, quality, flags, flags2, buyprice, sellprice, "
    //                                                   11             12              13           14            15            16               17
                                                   "inventorytype, allowableclass, allowablerace, itemlevel, requiredlevel, RequiredSkill, RequiredSkillRank, "
    //                                                   18                 19                    20                21                    22             23
                                                   "RequiredSpell, RequiredPlayerRank1, RequiredPlayerRank2, RequiredFaction, RequiredFactionStanding, Unique, "
    //                                                  24           25              26           27           28           29         30           31
                                                   "maxcount, ContainerSlots, itemstatscount, stat_type1, stat_value1, stat_type2, stat_value2, stat_type3, "
    //                                                  32           33          34           35          36           37           38          39
                                                   "stat_value3, stat_type4, stat_value4, stat_type5, stat_value5, stat_type6, stat_value6, stat_type7, "
    //                                                  40           41          42           43          44           45           46  
                                                   "stat_value7, stat_type8, stat_value8, stat_type9, stat_value9, stat_type10, stat_value10, "
    //                                                          47                           48                 49        50        51         52        53
                                                   "ScaledStatsDistributionId, ScaledStatsDistributionFlags, dmg_min1, dmg_max1, dmg_type1, dmg_min2, dmg_max2, "
    //                                                 54       55       56        57         58          59         60          61       62        63
                                                   "dmg_type2, armor, holy_res, fire_res, nature_res, frost_res, shadow_res, arcane_res, delay, ammo_type, "
    //                                                64      65             66             67              68               69                   70
                                                   "range, spellid_1, spelltrigger_1, spellcharges_1, spellcooldown_1, spellcategory_1, spellcategorycooldown_1, "
    //                                                  71           72              73              74               75                   76
                                                   "spellid_2, spelltrigger_2, spellcharges_2, spellcooldown_2, spellcategory_2, spellcategorycooldown_2, "
    //                                                  77           78              79              80               81                   82
                                                   "spellid_3, spelltrigger_3, spellcharges_3, spellcooldown_3, spellcategory_3, spellcategorycooldown_3, "
    //                                                  83           84              85              86               87                   88
                                                   "spellid_4, spelltrigger_4, spellcharges_4, spellcooldown_4, spellcategory_4, spellcategorycooldown_4, "
    //                                                  89           90              91              92               93                   94
                                                   "spellid_5, spelltrigger_5, spellcharges_5, spellcooldown_5, spellcategory_5, spellcategorycooldown_5, "
    //                                                 95         96          97         98             99          100      101         102          103
                                                   "bonding, description, page_id, page_language, page_material, quest_id, lock_id, lock_material, sheathID, "
    //                                                 104          105        106     107         108           109      110      111         112
                                                   "randomprop, randomsuffix, block, itemset, MaxDurability, ZoneNameID, mapid, bagfamily, TotemCategory, "
    //                                                   113           114         115          116          117           118         119          120
                                                   "socket_color_1, unk201_3, socket_color_2, unk201_5, socket_color_3, unk201_7, socket_bonus, GemProperties, "
    //                                                      121                122                 123                 124             125        126
                                                   "ReqDisenchantSkill, ArmorDamageModifier, existingduration, ItemLimitCategoryId, HolidayId, food_type FROM items");*/

    if (item_result == nullptr)
    {
        Log.Notice("MySQLDataLoads", "Table `items` is empty!");
        return;
    }

    Log.Notice("MySQLDataLoads", "Table `items` has %u columns", item_result->GetFieldCount());

    _itemPrototypeStore.rehash(item_result->GetRowCount());

    uint32 item_count = 0;
    do
    {
        Field* fields = item_result->Fetch();

        uint32 entry = fields[0].GetUInt32();

        ItemPrototype& itemProto = _itemPrototypeStore[entry];

        itemProto.ItemId = entry;
        itemProto.Class = fields[1].GetUInt32();
        itemProto.SubClass = fields[2].GetUInt32();
        itemProto.unknown_bc = fields[3].GetUInt32();
        itemProto.Name = fields[4].GetString();
        itemProto.DisplayInfoID = fields[5].GetUInt32();
        itemProto.Quality = fields[6].GetUInt32();
        itemProto.Flags = fields[7].GetUInt32();
        itemProto.Flags2 = fields[8].GetUInt32();
        itemProto.BuyPrice = fields[9].GetUInt32();
        itemProto.SellPrice = fields[10].GetUInt32();

        itemProto.InventoryType = fields[11].GetUInt32();
        itemProto.AllowableClass = fields[12].GetUInt32();
        itemProto.AllowableRace = fields[13].GetUInt32();
        itemProto.ItemLevel = fields[14].GetUInt32();
        itemProto.RequiredLevel = fields[15].GetUInt32();
        itemProto.RequiredSkill = fields[16].GetUInt32();
        itemProto.RequiredSkillRank = fields[17].GetUInt32();
        itemProto.RequiredSkillSubRank = fields[18].GetUInt32();
        itemProto.RequiredPlayerRank1 = fields[19].GetUInt32();
        itemProto.RequiredPlayerRank2 = fields[20].GetUInt32();
        itemProto.RequiredFaction = fields[21].GetUInt32();
        itemProto.RequiredFactionStanding = fields[22].GetUInt32();
        itemProto.Unique = fields[23].GetUInt32();
        itemProto.MaxCount = fields[24].GetUInt32();
        itemProto.ContainerSlots = fields[25].GetUInt32();
        itemProto.itemstatscount = fields[26].GetUInt32();

        for (uint8 i = 0; i < itemProto.itemstatscount; ++i)
        {
            itemProto.Stats[i].Type = fields[27 + i * 2].GetUInt32();
            itemProto.Stats[i].Value = fields[28 + i * 2].GetUInt32();
        }

        itemProto.ScalingStatsEntry = fields[47].GetUInt32();
        itemProto.ScalingStatsFlag = fields[48].GetUInt32();

        for (uint8 i = 0; i < MAX_ITEM_PROTO_DAMAGES; ++i)
        {
            itemProto.Damage[i].Min = fields[49 + i * 3].GetFloat();
            itemProto.Damage[i].Max = fields[50 + i * 3].GetFloat();
            itemProto.Damage[i].Type = fields[51 + i * 3].GetUInt32();
        }

        itemProto.Armor = fields[55].GetUInt32();
        itemProto.HolyRes = fields[56].GetUInt32();
        itemProto.FireRes = fields[57].GetUInt32();
        itemProto.NatureRes = fields[58].GetUInt32();
        itemProto.FrostRes = fields[59].GetUInt32();
        itemProto.ShadowRes = fields[60].GetUInt32();
        itemProto.ArcaneRes = fields[61].GetUInt32();
        itemProto.Delay = fields[62].GetUInt32();
        itemProto.AmmoType = fields[63].GetUInt32();
        itemProto.Range = fields[64].GetFloat();

        for (uint8 i = 0; i < MAX_ITEM_PROTO_SPELLS; ++i)
        {
            itemProto.Spells[i].Id = fields[65 + i * 6].GetUInt32();
            itemProto.Spells[i].Trigger = fields[66 + i * 6].GetUInt32();
            itemProto.Spells[i].Charges = fields[67 + i * 6].GetInt32();
            itemProto.Spells[i].Cooldown = fields[68 + i * 6].GetInt32();
            itemProto.Spells[i].Category = fields[69 + i * 6].GetUInt32();
            itemProto.Spells[i].CategoryCooldown = fields[70 + i * 6].GetInt32();
        }

        itemProto.Bonding = fields[95].GetUInt32();
        itemProto.Description = fields[96].GetString();
        itemProto.PageId = fields[97].GetUInt32();
        itemProto.PageLanguage = fields[98].GetUInt32();
        itemProto.PageMaterial = fields[99].GetUInt32();
        itemProto.QuestId = fields[100].GetUInt32();
        itemProto.LockId = fields[101].GetUInt32();
        itemProto.LockMaterial = fields[102].GetUInt32();
        itemProto.SheathID = fields[103].GetUInt32();
        itemProto.RandomPropId = fields[104].GetUInt32();
        itemProto.RandomSuffixId = fields[105].GetUInt32();
        itemProto.Block = fields[106].GetUInt32();
        itemProto.ItemSet = fields[107].GetInt32();
        itemProto.MaxDurability = fields[108].GetUInt32();
        itemProto.ZoneNameID = fields[109].GetUInt32();
        itemProto.MapID = fields[110].GetUInt32();
        itemProto.BagFamily = fields[111].GetUInt32();
        itemProto.TotemCategory = fields[112].GetUInt32();

        for (uint8 i = 0; i < MAX_ITEM_PROTO_SOCKETS; ++i)
        {
            itemProto.Sockets[i].SocketColor = uint32(fields[113 + i * 2].GetUInt8());
            itemProto.Sockets[i].Unk = fields[114 + i * 2].GetUInt32();
        }

        itemProto.SocketBonus = fields[119].GetUInt32();
        itemProto.GemProperties = fields[120].GetUInt32();
        itemProto.DisenchantReqSkill = fields[121].GetInt32();
        itemProto.ArmorDamageModifier = fields[122].GetUInt32();
        itemProto.ExistingDuration = fields[123].GetUInt32();
        itemProto.ItemLimitCategory = fields[124].GetUInt32();
        itemProto.HolidayId = fields[125].GetUInt32();
        itemProto.FoodType = fields[126].GetUInt32();

        //extra stuff
        itemProto.lowercase_name = itemProto.Name;
        for (uint32 j = 0; j < itemProto.lowercase_name.length(); ++j)
            itemProto.lowercase_name[j] = static_cast<char>(tolower(itemProto.lowercase_name[j]));

        //itemsets


        // Check the data with itemdbc, spelldbc, factiondbc....

        ++item_count;
    }
    while (item_result->NextRow());

    delete item_result;

    Log.Success("MySQLDataLoads", "Loaded %u items from `items` table in %u ms!", item_count, getMSTime() - start_time);
}

ItemPrototype const* MySQLDataStore::GetItemProto(uint32 entry)
{
    ItemPrototypeContainer::const_iterator itr = _itemPrototypeStore.find(entry);
    if (itr != _itemPrototypeStore.end())
        return &(itr->second);

    return nullptr;
}
