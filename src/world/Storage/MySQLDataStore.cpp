/*
Copyright (c) 2014-2016 AscEmu Team <http://www.ascemu.org/>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "StdAfx.h"

initialiseSingleton(MySQLDataStore);

SERVER_DECL std::set<std::string> CreatureSpawnsTables;
SERVER_DECL std::set<std::string> GameObjectSpawnsTables;
SERVER_DECL std::set<std::string> GameObjectPropertiesTables;
SERVER_DECL std::set<std::string> CreaturePropertiesTables;
SERVER_DECL std::set<std::string> ItemPropertiesTables;
SERVER_DECL std::set<std::string> QuestPropertiesTables;

MySQLDataStore::MySQLDataStore() {}
MySQLDataStore::~MySQLDataStore() {}

void MySQLDataStore::LoadAdditionalTableConfig()
{
    // init basic tables
    CreatureSpawnsTables.insert(std::string("creature_spawns"));
    GameObjectSpawnsTables.insert(std::string("gameobject_spawns"));
    GameObjectPropertiesTables.insert(std::string("gameobject_properties"));
    CreaturePropertiesTables.insert(std::string("creature_properties"));
    ItemPropertiesTables.insert(std::string("item_properties"));
    QuestPropertiesTables.insert(std::string("quest_properties"));

    // get config
    std::string strData = Config.MainConfig.GetStringDefault("Startup", "LoadAdditionalTables", "");
    if (strData.empty())
        return;

    std::vector<std::string> strs = StrSplit(strData, ",");
    if (strs.empty())
        return;

    for (std::vector<std::string>::iterator itr = strs.begin(); itr != strs.end(); ++itr)
    {
        char additional_table[200];
        char target_table[200];

        if (sscanf((*itr).c_str(), "%s %s", additional_table, target_table) != 2)
            continue;

        if (!stricmp(target_table, "creature_spawns"))
            CreatureSpawnsTables.insert(std::string(additional_table));

        if (!stricmp(target_table, "gameobject_spawns"))
            GameObjectSpawnsTables.insert(std::string(additional_table));

        if (!stricmp(target_table, "gameobject_properties"))
            GameObjectPropertiesTables.insert(std::string(additional_table));

        if (!stricmp(target_table, "creature_properties"))
            CreaturePropertiesTables.insert(std::string(additional_table));

        if (!stricmp(target_table, "item_properties"))
            ItemPropertiesTables.insert(std::string(additional_table));

        if (!stricmp(target_table, "quest_properties"))
            QuestPropertiesTables.insert(std::string(additional_table));
    }
}

void MySQLDataStore::LoadItemPagesTable()
{
    uint32 start_time = getMSTime();

    QueryResult* itempages_result = WorldDatabase.Query("SELECT entry, text, next_page FROM item_pages");
    if (itempages_result == nullptr)
    {
        Log.Notice("MySQLDataLoads", "Table `item_pages` is empty!");
        return;
    }

    Log.Notice("MySQLDataLoads", "Table `item_pages` has %u columns", itempages_result->GetFieldCount());

    _itemPagesStore.rehash(itempages_result->GetRowCount());

    uint32 itempages_count = 0;
    do
    {
        Field* fields = itempages_result->Fetch();

        uint32 entry = fields[0].GetUInt32();

        ItemPage& itemPage = _itemPagesStore[entry];

        itemPage.id = entry;
        itemPage.text = fields[1].GetString();
        itemPage.next_page = fields[2].GetUInt32();


        ++itempages_count;
    } while (itempages_result->NextRow());

    delete itempages_result;

    Log.Success("MySQLDataLoads", "Loaded %u pages from `item_pages` table in %u ms!", itempages_count, getMSTime() - start_time);
}

ItemPage const* MySQLDataStore::GetItemPage(uint32 entry)
{
    ItemPageContainer::const_iterator itr = _itemPagesStore.find(entry);
    if (itr != _itemPagesStore.end())
        return &(itr->second);

    return nullptr;
}

void MySQLDataStore::LoadItemPropertiesTable()
{
    uint32 start_time = getMSTime();

    uint32 item_count = 0;
    uint32 basic_field_count = 0;

    std::set<std::string>::iterator tableiterator;
    for (tableiterator = ItemPropertiesTables.begin(); tableiterator != ItemPropertiesTables.end(); ++tableiterator)
    {
        std::string table_name = *tableiterator;
        QueryResult* item_result = WorldDatabase.Query("SELECT * FROM %s", table_name.c_str());

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
                                                       "ReqDisenchantSkill, ArmorDamageModifier, existingduration, ItemLimitCategoryId, HolidayId, food_type FROM item_properties");*/

        if (item_result == nullptr)
        {
            Log.Notice("MySQLDataLoads", "Table `%s` is empty!", table_name.c_str());
            return;
        }

        uint32 row_count = 0;
        if (table_name.compare("item_properties") == 0)
        {
            basic_field_count = item_result->GetFieldCount();
        }
        else
        {
            row_count = _itemPropertiesStore.size();
        }

        if (basic_field_count != item_result->GetFieldCount())
        {
            Log.Error("MySQLDataLoads", "Additional item_properties table `%s` has %u columns, but needs %u columns! Skipped!", table_name.c_str(), item_result->GetFieldCount(), basic_field_count);
            continue;
        }

        Log.Notice("MySQLDataLoads", "Table `%s` has %u columns", table_name.c_str(), item_result->GetFieldCount());

        _itemPropertiesStore.rehash(row_count + item_result->GetRowCount());

        do
        {
            Field* fields = item_result->Fetch();

            uint32 entry = fields[0].GetUInt32();

            ItemProperties& itemProperties = _itemPropertiesStore[entry];

            itemProperties.ItemId = entry;
            itemProperties.Class = fields[1].GetUInt32();
            itemProperties.SubClass = fields[2].GetUInt32();
            itemProperties.unknown_bc = fields[3].GetUInt32();
            itemProperties.Name = fields[4].GetString();
            itemProperties.DisplayInfoID = fields[5].GetUInt32();
            itemProperties.Quality = fields[6].GetUInt32();
            itemProperties.Flags = fields[7].GetUInt32();
            itemProperties.Flags2 = fields[8].GetUInt32();
            itemProperties.BuyPrice = fields[9].GetUInt32();
            itemProperties.SellPrice = fields[10].GetUInt32();

            itemProperties.InventoryType = fields[11].GetUInt32();
            itemProperties.AllowableClass = fields[12].GetUInt32();
            itemProperties.AllowableRace = fields[13].GetUInt32();
            itemProperties.ItemLevel = fields[14].GetUInt32();
            itemProperties.RequiredLevel = fields[15].GetUInt32();
            itemProperties.RequiredSkill = fields[16].GetUInt32();
            itemProperties.RequiredSkillRank = fields[17].GetUInt32();
            itemProperties.RequiredSkillSubRank = fields[18].GetUInt32();
            itemProperties.RequiredPlayerRank1 = fields[19].GetUInt32();
            itemProperties.RequiredPlayerRank2 = fields[20].GetUInt32();
            itemProperties.RequiredFaction = fields[21].GetUInt32();
            itemProperties.RequiredFactionStanding = fields[22].GetUInt32();
            itemProperties.Unique = fields[23].GetUInt32();
            itemProperties.MaxCount = fields[24].GetUInt32();
            itemProperties.ContainerSlots = fields[25].GetUInt32();
            itemProperties.itemstatscount = fields[26].GetUInt32();

            for (uint8 i = 0; i < itemProperties.itemstatscount; ++i)
            {
                itemProperties.Stats[i].Type = fields[27 + i * 2].GetUInt32();
                itemProperties.Stats[i].Value = fields[28 + i * 2].GetUInt32();
            }

            itemProperties.ScalingStatsEntry = fields[47].GetUInt32();
            itemProperties.ScalingStatsFlag = fields[48].GetUInt32();

            for (uint8 i = 0; i < MAX_ITEM_PROTO_DAMAGES; ++i)
            {
                itemProperties.Damage[i].Min = fields[49 + i * 3].GetFloat();
                itemProperties.Damage[i].Max = fields[50 + i * 3].GetFloat();
                itemProperties.Damage[i].Type = fields[51 + i * 3].GetUInt32();
            }

            itemProperties.Armor = fields[55].GetUInt32();
            itemProperties.HolyRes = fields[56].GetUInt32();
            itemProperties.FireRes = fields[57].GetUInt32();
            itemProperties.NatureRes = fields[58].GetUInt32();
            itemProperties.FrostRes = fields[59].GetUInt32();
            itemProperties.ShadowRes = fields[60].GetUInt32();
            itemProperties.ArcaneRes = fields[61].GetUInt32();
            itemProperties.Delay = fields[62].GetUInt32();
            itemProperties.AmmoType = fields[63].GetUInt32();
            itemProperties.Range = fields[64].GetFloat();

            for (uint8 i = 0; i < MAX_ITEM_PROTO_SPELLS; ++i)
            {
                itemProperties.Spells[i].Id = fields[65 + i * 6].GetUInt32();
                itemProperties.Spells[i].Trigger = fields[66 + i * 6].GetUInt32();
                itemProperties.Spells[i].Charges = fields[67 + i * 6].GetInt32();
                itemProperties.Spells[i].Cooldown = fields[68 + i * 6].GetInt32();
                itemProperties.Spells[i].Category = fields[69 + i * 6].GetUInt32();
                itemProperties.Spells[i].CategoryCooldown = fields[70 + i * 6].GetInt32();
            }

            itemProperties.Bonding = fields[95].GetUInt32();
            itemProperties.Description = fields[96].GetString();
            uint32 page_id = fields[97].GetUInt32();
            if (page_id != 0)
            {
                ItemPage const* item_page = GetItemPage(page_id);
                if (item_page == nullptr)
                {
                    Log.Error("MySQLDataLoads", "Table `%s` entry: %u includes invalid pageId %u! pageId is set to 0.", table_name.c_str(), entry, page_id);
                    itemProperties.PageId = 0;
                }
                else
                {
                    itemProperties.PageId = page_id;
                }
            }
            else
            {
                itemProperties.PageId = page_id;
            }

            itemProperties.PageLanguage = fields[98].GetUInt32();
            itemProperties.PageMaterial = fields[99].GetUInt32();
            itemProperties.QuestId = fields[100].GetUInt32();
            itemProperties.LockId = fields[101].GetUInt32();
            itemProperties.LockMaterial = fields[102].GetUInt32();
            itemProperties.SheathID = fields[103].GetUInt32();
            itemProperties.RandomPropId = fields[104].GetUInt32();
            itemProperties.RandomSuffixId = fields[105].GetUInt32();
            itemProperties.Block = fields[106].GetUInt32();
            itemProperties.ItemSet = fields[107].GetInt32();
            itemProperties.MaxDurability = fields[108].GetUInt32();
            itemProperties.ZoneNameID = fields[109].GetUInt32();
            itemProperties.MapID = fields[110].GetUInt32();
            itemProperties.BagFamily = fields[111].GetUInt32();
            itemProperties.TotemCategory = fields[112].GetUInt32();

            for (uint8 i = 0; i < MAX_ITEM_PROTO_SOCKETS; ++i)
            {
                itemProperties.Sockets[i].SocketColor = uint32(fields[113 + i * 2].GetUInt8());
                itemProperties.Sockets[i].Unk = fields[114 + i * 2].GetUInt32();
            }

            itemProperties.SocketBonus = fields[119].GetUInt32();
            itemProperties.GemProperties = fields[120].GetUInt32();
            itemProperties.DisenchantReqSkill = fields[121].GetInt32();
            itemProperties.ArmorDamageModifier = fields[122].GetUInt32();
            itemProperties.ExistingDuration = fields[123].GetUInt32();
            itemProperties.ItemLimitCategory = fields[124].GetUInt32();
            itemProperties.HolidayId = fields[125].GetUInt32();
            itemProperties.FoodType = fields[126].GetUInt32();

            //lowercase
            std::string lower_case_name = itemProperties.Name;
            std::transform(lower_case_name.begin(), lower_case_name.end(), lower_case_name.begin(), ::tolower);
            itemProperties.lowercase_name = lower_case_name;

            //forced pet entries (hacky stuff ->spells)
            switch (itemProperties.ItemId)
            {
                case 28071: //Grimoire of Anguish (Rank 1)
                case 28072: //Grimoire of Anguish (Rank 2)
                case 28073: //Grimoire of Anguish (Rank 3)
                case 25469: //Grimoire of Avoidance
                case 23734: //Grimoire of Cleave (Rank 1)
                case 23745: //Grimoire of Cleave (Rank 2)
                case 23755: //Grimoire of Cleave (Rank 3)
                case 25900: //Grimoire of Demonic Frenzy
                case 23711: //Grimoire of Intercept (Rank 1)
                case 23730: //Grimoire of Intercept (Rank 2)
                case 23731: //Grimoire of Intercept (Rank 3)
                            // Felguard
                    itemProperties.ForcedPetId = 17252;
                    break;

                case 16321: //Grimoire of Blood Pact (Rank 1)
                case 16322: //Grimoire of Blood Pact (Rank 2)
                case 16323: //Grimoire of Blood Pact (Rank 3)
                case 16324: //Grimoire of Blood Pact (Rank 4)
                case 16325: //Grimoire of Blood Pact (Rank 5)
                case 22180: //Grimoire of Blood Pact (Rank 6)
                case 16326: //Grimoire of Fire Shield (Rank 1)
                case 16327: //Grimoire of Fire Shield (Rank 2)
                case 16328: //Grimoire of Fire Shield (Rank 3)
                case 16329: //Grimoire of Fire Shield (Rank 4)
                case 16330: //Grimoire of Fire Shield (Rank 5)
                case 22181: //Grimoire of Fire Shield (Rank 6)
                case 16302: //Grimoire of Firebolt (Rank 2)
                case 16316: //Grimoire of Firebolt (Rank 3)
                case 16317: //Grimoire of Firebolt (Rank 4)
                case 16318: //Grimoire of Firebolt (Rank 5)
                case 16319: //Grimoire of Firebolt (Rank 6)
                case 16320: //Grimoire of Firebolt (Rank 7)
                case 22179: //Grimoire of Firebolt (Rank 8)
                case 16331: //Grimoire of Phase Shift
                            // Imp
                    itemProperties.ForcedPetId = 416;
                    break;

                case 16357: //Grimoire of Consume Shadows (Rank 1)
                case 16358: //Grimoire of Consume Shadows (Rank 2)
                case 16359: //Grimoire of Consume Shadows (Rank 3)
                case 16360: //Grimoire of Consume Shadows (Rank 4)
                case 16361: //Grimoire of Consume Shadows (Rank 5)
                case 16362: //Grimoire of Consume Shadows (Rank 6)
                case 22184: //Grimoire of Consume Shadows (Rank 7)
                case 16351: //Grimoire of Sacrifice (Rank 1)
                case 16352: //Grimoire of Sacrifice (Rank 2)
                case 16353: //Grimoire of Sacrifice (Rank 3)
                case 16354: //Grimoire of Sacrifice (Rank 4)
                case 16355: //Grimoire of Sacrifice (Rank 5)
                case 16356: //Grimoire of Sacrifice (Rank 6)
                case 22185: //Grimoire of Sacrifice (Rank 7)
                case 16363: //Grimoire of Suffering (Rank 1)
                case 16364: //Grimoire of Suffering (Rank 2)
                case 16365: //Grimoire of Suffering (Rank 3)
                case 16366: //Grimoire of Suffering (Rank 4)
                case 22183: //Grimoire of Suffering (Rank 5)
                case 28068: //Grimoire of Suffering (Rank 6)
                case 16346: //Grimoire of Torment (Rank 2)
                case 16347: //Grimoire of Torment (Rank 3)
                case 16348: //Grimoire of Torment (Rank 4)
                case 16349: //Grimoire of Torment (Rank 5)
                case 16350: //Grimoire of Torment (Rank 6)
                case 22182: //Grimoire of Torment (Rank 7)
                            // Voidwalker
                    itemProperties.ForcedPetId = 1860;
                    break;

                case 16368: //Grimoire of Lash of Pain (Rank 2)
                case 16371: //Grimoire of Lash of Pain (Rank 3)
                case 16372: //Grimoire of Lash of Pain (Rank 4)
                case 16373: //Grimoire of Lash of Pain (Rank 5)
                case 16374: //Grimoire of Lash of Pain (Rank 6)
                case 22186: //Grimoire of Lash of Pain (Rank 7)
                case 16380: //Grimoire of Lesser Invisibility
                case 16379: //Grimoire of Seduction
                case 16375: //Grimoire of Soothing Kiss (Rank 1)
                case 16376: //Grimoire of Soothing Kiss (Rank 2)
                case 16377: //Grimoire of Soothing Kiss (Rank 3)
                case 16378: //Grimoire of Soothing Kiss (Rank 4)
                case 22187: //Grimoire of Soothing Kiss (Rank 5)
                            // Succubus
                    itemProperties.ForcedPetId = 1863;
                    break;

                case 16381: //Grimoire of Devour Magic (Rank 2)
                case 16382: //Grimoire of Devour Magic (Rank 3)
                case 16383: //Grimoire of Devour Magic (Rank 4)
                case 22188: //Grimoire of Devour Magic (Rank 5)
                case 22189: //Grimoire of Devour Magic (Rank 6)
                case 16390: //Grimoire of Paranoia
                case 16388: //Grimoire of Spell Lock (Rank 1)
                case 16389: //Grimoire of Spell Lock (Rank 2)
                case 16384: //Grimoire of Tainted Blood (Rank 1)
                case 16385: //Grimoire of Tainted Blood (Rank 2)
                case 16386: //Grimoire of Tainted Blood (Rank 3)
                case 16387: //Grimoire of Tainted Blood (Rank 4)
                case 22190: //Grimoire of Tainted Blood (Rank 5)
                            //Felhunter
                    itemProperties.ForcedPetId = 417;
                    break;

                case 21283:
                case 3144:
                case 21282:
                case 9214:
                case 21281:
                case 22891:
                    // Player
                    itemProperties.ForcedPetId = 0;
                    break;

                default:
                    itemProperties.ForcedPetId = -1;
                    break;
            }


            // Check the data with itemdbc, spelldbc, factiondbc....

            ++item_count;
        } while (item_result->NextRow());

        delete item_result;
    }

    Log.Success("MySQLDataLoads", "Loaded %u item_properties in %u ms!", item_count, getMSTime() - start_time);
}

ItemProperties const* MySQLDataStore::GetItemProperties(uint32 entry)
{
    ItemPropertiesContainer::const_iterator itr = _itemPropertiesStore.find(entry);
    if (itr != _itemPropertiesStore.end())
        return &(itr->second);

    return nullptr;
}

void MySQLDataStore::LoadCreaturePropertiesTable()
{
    uint32 start_time = getMSTime();
    uint32 creature_properties_count = 0;
    uint32 basic_field_count = 0;

    std::set<std::string>::iterator tableiterator;
    for (tableiterator = CreaturePropertiesTables.begin(); tableiterator != CreaturePropertiesTables.end(); ++tableiterator)
    {
        std::string table_name = *tableiterator;
        //                                                                 0          1           2             3                 4               5                  6
        QueryResult* creature_properties_result = WorldDatabase.Query("SELECT entry, killcredit1, killcredit2, male_displayid, female_displayid, male_displayid2, female_displayid2, "
        //                                                         7      8         9       10     11     12     13       14           15             16           17
                                                                "name, subname, info_str, flags1, type, family, rank, encounter, unknown_float1, unknown_float2, leader, "
        //                                                          18        19        20        21         22      23     24      25          26         27
                                                                "minlevel, maxlevel, faction, minhealth, maxhealth, mana, scale, npcflags, attacktime, attacktype, "
        //                                                          28          29         30            31                 32                33            34        35
                                                                "mindamage, maxdamage, can_ranged, rangedattacktime, rangedmindamage, rangedmaxdamage, respawntime, armor, "
        //                                                            36           37           38            39          40           41            42             43
                                                                "resistance1, resistance2, resistance3, resistance4, resistance5, resistance6, combat_reach, bounding_radius, "
        //                                                         44    45     46         47                48         49        50          51            52     53      54 
                                                                "auras, boss, money, invisibility_type, walk_speed, run_speed, fly_speed, extra_a9_flags, spell1, spell2, spell3, "
        //                                                          55      56      57      58      59        60           61               62            63         64           65
                                                                "spell4, spell5, spell6, spell7, spell8, spell_flags, modImmunities, isTrainingDummy, guardtype, summonguard, spelldataid, "
        //                                                          66         67        68          69          70          71          72          73         74
                                                                "vehicleid, rooted, questitem1, questitem2, questitem3, questitem4, questitem5, questitem6, waypointid FROM %s", table_name.c_str());

        if (creature_properties_result == nullptr)
        {
            Log.Notice("MySQLDataLoads", "Table `%s` is empty!", table_name.c_str());
            return;
        }

        uint32 row_count = 0;
        if (table_name.compare("creature_properties") == 0)
        {
            basic_field_count = creature_properties_result->GetFieldCount();
        }
        else
        {
            row_count = _creaturePropertiesStore.size();
        }

        if (basic_field_count != creature_properties_result->GetFieldCount())
        {
            Log.Error("MySQLDataLoads", "Additional creature_properties table `%s` has %u columns, but needs %u columns! Skipped!", table_name.c_str(), creature_properties_result->GetFieldCount());
            delete creature_properties_result;
            continue;
        }

        Log.Notice("MySQLDataLoads", "Table `%s` has %u columns", table_name.c_str(), creature_properties_result->GetFieldCount());

        _creaturePropertiesStore.rehash(row_count + creature_properties_result->GetRowCount());

        do
        {
            Field* fields = creature_properties_result->Fetch();

            uint32 entry = fields[0].GetUInt32();

            CreatureProperties& creatureProperties = _creaturePropertiesStore[entry];

            creatureProperties.Id = entry;
            creatureProperties.killcredit[0] = fields[1].GetUInt32();
            creatureProperties.killcredit[1] = fields[2].GetUInt32();
            creatureProperties.Male_DisplayID = fields[3].GetUInt32();
            creatureProperties.Female_DisplayID = fields[4].GetUInt32();
            creatureProperties.Male_DisplayID2 = fields[5].GetUInt32();
            creatureProperties.Female_DisplayID2 = fields[6].GetUInt32();
            creatureProperties.Name = fields[7].GetString();
            creatureProperties.SubName = fields[8].GetString();
            creatureProperties.info_str = fields[9].GetString();
            creatureProperties.Flags1 = fields[10].GetUInt32();
            creatureProperties.Type = fields[11].GetUInt32();
            creatureProperties.Family = fields[12].GetUInt32();
            creatureProperties.Rank = fields[13].GetUInt32();
            creatureProperties.Encounter = fields[14].GetUInt32();
            creatureProperties.unkfloat1 = fields[15].GetFloat();
            creatureProperties.unkfloat2 = fields[16].GetFloat();
            creatureProperties.Leader = fields[17].GetUInt8();
            creatureProperties.MinLevel = fields[18].GetUInt32();
            creatureProperties.MaxLevel = fields[19].GetUInt32();
            creatureProperties.Faction = fields[20].GetUInt32();
            if (fields[21].GetUInt32() != 0)
            {
                creatureProperties.MinHealth = fields[21].GetUInt32();
            }
            else
            {
                Log.Error("MySQLDataLoads", "Table `%s` MinHealth = 0 is not a valid value! Default set to 1 for entry: %u.", table_name.c_str(), entry);
                creatureProperties.MinHealth = 1;
            }

            if (fields[22].GetUInt32() != 0)
            {
                creatureProperties.MaxHealth = fields[22].GetUInt32();
            }
            else
            {
                Log.Error("MySQLDataLoads", "Table `%s` MaxHealth = 0 is not a valid value! Default set to 1 for entry: %u.", table_name.c_str(), entry);
                creatureProperties.MaxHealth = 1;
            }

            creatureProperties.Mana = fields[23].GetUInt32();
            creatureProperties.Scale = fields[24].GetFloat();
            creatureProperties.NPCFLags = fields[25].GetUInt32();
            creatureProperties.AttackTime = fields[26].GetUInt32();
            creatureProperties.AttackType = fields[27].GetUInt32();
            if (fields[27].GetUInt32() <= SCHOOL_ARCANE)
            {
                creatureProperties.AttackType = fields[27].GetUInt32();
            }
            else
            {
                Log.Error("MySQLDataLoads", "Table `%s` AttackType: %u is not a valid value! Default set to 0 for entry: %u.", table_name.c_str(), fields[10].GetUInt32(), entry);
                creatureProperties.AttackType = SCHOOL_NORMAL;
            }

            creatureProperties.MinDamage = fields[28].GetFloat();
            creatureProperties.MaxDamage = fields[29].GetFloat();
            creatureProperties.CanRanged = fields[30].GetUInt32();
            creatureProperties.RangedAttackTime = fields[31].GetUInt32();
            creatureProperties.RangedMinDamage = fields[32].GetFloat();
            creatureProperties.RangedMaxDamage = fields[33].GetFloat();
            creatureProperties.RespawnTime = fields[34].GetUInt32();
            for (uint8 i = 0; i < SCHOOL_COUNT; ++i)
            {
                creatureProperties.Resistances[i] = fields[35 + i].GetUInt32();
            }

            creatureProperties.CombatReach = fields[42].GetFloat();
            creatureProperties.BoundingRadius = fields[43].GetFloat();
            creatureProperties.aura_string = fields[44].GetString();
            creatureProperties.isBoss = fields[45].GetBool();
            creatureProperties.money = fields[46].GetUInt32();
            creatureProperties.invisibility_type = fields[47].GetUInt32();
            creatureProperties.walk_speed = fields[48].GetFloat();
            creatureProperties.run_speed = fields[49].GetFloat();
            creatureProperties.fly_speed = fields[50].GetFloat();
            creatureProperties.extra_a9_flags = fields[51].GetUInt32();

            for (uint8 i = 0; i < creatureMaxProtoSpells; ++i)
            {
                // Process spell fields
                creatureProperties.AISpells[i] = fields[52 + i].GetUInt32();
                if (creatureProperties.AISpells[i] != 0)
                {
                    SpellEntry* sp = dbcSpell.LookupEntryForced(creatureProperties.AISpells[i]);
                    if (sp == nullptr)
                    {
                        uint8 spell_number = i;
                        Log.Error("MySQLDataStore", "spell %u in table %s column spell%u for creature entry: %u is not a valid spell!", creatureProperties.AISpells[i], table_name.c_str(), spell_number + 1, entry);
                        continue;
                    }
                    else
                    {
                        if ((sp->Attributes & ATTRIBUTES_PASSIVE) == 0)
                            creatureProperties.castable_spells.push_back(sp->Id);
                        else
                            creatureProperties.start_auras.insert(sp->Id);
                    }
                }
            }

            creatureProperties.AISpellsFlags = fields[60].GetUInt32();
            creatureProperties.modImmunities = fields[61].GetUInt32();
            creatureProperties.isTrainingDummy = fields[62].GetBool();
            creatureProperties.guardtype = fields[63].GetUInt32();
            creatureProperties.summonguard = fields[64].GetUInt32();
            creatureProperties.spelldataid = fields[65].GetUInt32();
            // process creature spells from creaturespelldata.dbc
            if (creatureProperties.spelldataid != 0)
            {
                auto creature_spell_data = sCreatureSpellDataStore.LookupEntry(creatureProperties.spelldataid);
                for (uint8 i = 0; i < 3; i++)
                {
                    if (creature_spell_data == nullptr)
                        continue;

                    if (creature_spell_data->Spells[i] == 0)
                        continue;

                    SpellEntry* sp = dbcSpell.LookupEntryForced(creature_spell_data->Spells[i]);
                    if (sp == nullptr)
                        continue;

                    if ((sp->Attributes & ATTRIBUTES_PASSIVE) == 0)
                        creatureProperties.castable_spells.push_back(sp->Id);
                    else
                        creatureProperties.start_auras.insert(sp->Id);
                }
            }

            creatureProperties.vehicleid = fields[66].GetUInt32();
            creatureProperties.rooted = fields[67].GetBool();

            for (uint8 i = 0; i < 6; ++i)
                creatureProperties.QuestItems[i] = fields[68 + i].GetUInt32();

            creatureProperties.waypointid = fields[74].GetUInt32();

            //process aura string
            if (creatureProperties.aura_string.size() != 0)
            {
                std::string auras = creatureProperties.aura_string;
                std::vector<std::string> split_auras = StrSplit(auras, " ");
                for (std::vector<std::string>::iterator it = split_auras.begin(); it != split_auras.end(); ++it)
                {
                    uint32 id = atol((*it).c_str());
                    if (id)
                        creatureProperties.start_auras.insert(id);
                }
            }

            //AI stuff
            creatureProperties.m_canFlee = false;
            creatureProperties.m_canRangedAttack = false;
            creatureProperties.m_canCallForHelp = false;
            creatureProperties.m_fleeHealth = 0.0f;
            creatureProperties.m_fleeDuration = 0;

            //Itemslot
            creatureProperties.itemslot_1 = 0;
            creatureProperties.itemslot_2 = 0;
            creatureProperties.itemslot_3 = 0;

            ++creature_properties_count;
        } while (creature_properties_result->NextRow());

        delete creature_properties_result;
    }

    Log.Success("MySQLDataLoads", "Loaded %u creature proto data in %u ms!", creature_properties_count, getMSTime() - start_time);
}

CreatureProperties const* MySQLDataStore::GetCreatureProperties(uint32 entry)
{
    CreaturePropertiesContainer::const_iterator itr = _creaturePropertiesStore.find(entry);
    if (itr != _creaturePropertiesStore.end())
        return &(itr->second);

    return nullptr;
}

void MySQLDataStore::LoadGameObjectPropertiesTable()
{
    uint32 start_time = getMSTime();
    uint32 gameobject_properties_count = 0;
    uint32 basic_field_count = 0;

    std::set<std::string>::iterator tableiterator;
    for (tableiterator = GameObjectPropertiesTables.begin(); tableiterator != GameObjectPropertiesTables.end(); ++tableiterator)
    {
        std::string table_name = *tableiterator;
        //                                                                  0       1        2        3         4              5          6          7            8             9
        QueryResult* gameobject_properties_result = WorldDatabase.Query("SELECT entry, type, display_id, name, category_name, cast_bar_text, UnkStr, parameter_0, parameter_1, parameter_2, "
        //                                                                10           11          12           13           14            15           16           17           18
                                                                    "parameter_3, parameter_4, parameter_5, parameter_6, parameter_7, parameter_8, parameter_9, parameter_10, parameter_11, "
        //                                                                19            20            21            22           23            24            25            26
                                                                    "parameter_12, parameter_13, parameter_14, parameter_15, parameter_16, parameter_17, parameter_18, parameter_19, "
        //                                                                27            28            29            30        31        32          33          34         35
                                                                    "parameter_20, parameter_21, parameter_22, parameter_23, size, QuestItem1, QuestItem2, QuestItem3, QuestItem4, "
        //                                                                36          37 
                                                                    "QuestItem5, QuestItem6 FROM %s", table_name.c_str());

        if (gameobject_properties_result == nullptr)
        {
            Log.Notice("MySQLDataLoads", "Table `%s` is empty!", table_name.c_str());
            return;
        }

        uint32 row_count = 0;
        if (table_name.compare("gameobject_properties") == 0)
        {
            basic_field_count = gameobject_properties_result->GetFieldCount();
        }
        else
        {
            row_count = _gameobjectPropertiesStore.size();
        }

        if (basic_field_count != gameobject_properties_result->GetFieldCount())
        {
            Log.Error("MySQLDataLoads", "Additional gameobject_properties table `%s` has %u columns, but needs %u columns! Skipped!", table_name.c_str(), gameobject_properties_result->GetFieldCount());
            delete gameobject_properties_result;
            continue;
        }

        Log.Notice("MySQLDataLoads", "Table `%s` has %u columns", table_name.c_str(), gameobject_properties_result->GetFieldCount());

        _gameobjectPropertiesStore.rehash(row_count + gameobject_properties_result->GetRowCount());

        do
        {
            Field* fields = gameobject_properties_result->Fetch();

            uint32 entry = fields[0].GetUInt32();

            GameObjectProperties& gameobjecProperties = _gameobjectPropertiesStore[entry];

            gameobjecProperties.entry = entry;
            gameobjecProperties.type = fields[1].GetUInt32();
            gameobjecProperties.display_id = fields[2].GetUInt32();
            gameobjecProperties.name = fields[3].GetString();
            gameobjecProperties.category_name = fields[4].GetString();
            gameobjecProperties.cast_bar_text = fields[5].GetString();
            gameobjecProperties.Unkstr = fields[6].GetString();

            gameobjecProperties.raw.parameter_0 = fields[7].GetUInt32();
            gameobjecProperties.raw.parameter_1 = fields[8].GetUInt32();
            gameobjecProperties.raw.parameter_2 = fields[9].GetUInt32();
            gameobjecProperties.raw.parameter_3 = fields[10].GetUInt32();
            gameobjecProperties.raw.parameter_4 = fields[11].GetUInt32();
            gameobjecProperties.raw.parameter_5 = fields[12].GetUInt32();
            gameobjecProperties.raw.parameter_6 = fields[13].GetUInt32();
            gameobjecProperties.raw.parameter_7 = fields[14].GetUInt32();
            gameobjecProperties.raw.parameter_8 = fields[15].GetUInt32();
            gameobjecProperties.raw.parameter_9 = fields[16].GetUInt32();
            gameobjecProperties.raw.parameter_10 = fields[17].GetUInt32();
            gameobjecProperties.raw.parameter_11 = fields[18].GetUInt32();
            gameobjecProperties.raw.parameter_12 = fields[19].GetUInt32();
            gameobjecProperties.raw.parameter_13 = fields[20].GetUInt32();
            gameobjecProperties.raw.parameter_14 = fields[21].GetUInt32();
            gameobjecProperties.raw.parameter_15 = fields[22].GetUInt32();
            gameobjecProperties.raw.parameter_16 = fields[23].GetUInt32();
            gameobjecProperties.raw.parameter_17 = fields[24].GetUInt32();
            gameobjecProperties.raw.parameter_18 = fields[25].GetUInt32();
            gameobjecProperties.raw.parameter_19 = fields[26].GetUInt32();
            gameobjecProperties.raw.parameter_20 = fields[27].GetUInt32();
            gameobjecProperties.raw.parameter_21 = fields[28].GetUInt32();
            gameobjecProperties.raw.parameter_22 = fields[29].GetUInt32();
            gameobjecProperties.raw.parameter_23 = fields[30].GetUInt32();

            gameobjecProperties.size = fields[31].GetFloat();

            for (uint8 i = 0; i < 6; ++i)
            {
                uint32 quest_item_entry = fields[32 + i].GetUInt32();
                if (quest_item_entry != 0)
                {
                    auto quest_item_proto = GetItemProperties(quest_item_entry);
                    if (quest_item_proto == nullptr)
                    {
                        Log.Error("MySQLDataLoads", "Table `%s` questitem%u : %u is not a valid item! Default set to 0 for entry: %u.", table_name.c_str(), i, quest_item_entry, entry);
                        gameobjecProperties.QuestItems[i] = 0;
                    }
                    else
                    {
                        gameobjecProperties.QuestItems[i] = quest_item_entry;
                    }
                }
            }


            ++gameobject_properties_count;
        } while (gameobject_properties_result->NextRow());

        delete gameobject_properties_result;
    }

    Log.Success("MySQLDataLoads", "Loaded %u gameobject data in %u ms!", gameobject_properties_count, getMSTime() - start_time);
}

GameObjectProperties const* MySQLDataStore::GetGameObjectProperties(uint32 entry)
{
    GameObjectPropertiesContainer::const_iterator itr = _gameobjectPropertiesStore.find(entry);
    if (itr != _gameobjectPropertiesStore.end())
        return &(itr->second);

    return nullptr;
}

//quests
void MySQLDataStore::LoadQuestPropertiesTable()
{
    uint32 start_time = getMSTime();
    uint32 quest_count = 0;
    uint32 basic_field_count = 0;

    std::set<std::string>::iterator tableiterator;
    for (tableiterator = QuestPropertiesTables.begin(); tableiterator != QuestPropertiesTables.end(); ++tableiterator)
    {
        std::string table_name = *tableiterator;
        //                                                        0       1     2      3       4          5        6          7              8                 9
        QueryResult* quest_result = WorldDatabase.Query("SELECT entry, ZoneId, sort, flags, MinLevel, questlevel, Type, RequiredRaces, RequiredClass, RequiredTradeskill, "
        //                                                          10                    11                 12             13          14            15           16         17
                                                        "RequiredTradeskillValue, RequiredRepFaction, RequiredRepValue, LimitTime, SpecialFlags, PrevQuestId, NextQuestId, srcItem, "
        //                                                     18        19     20         21            22              23          24          25               26
                                                        "SrcItemCount, Title, Details, Objectives, CompletionText, IncompleteText, EndText, ObjectiveText1, ObjectiveText2, "
        //                                                     26               27           28          29           30          31         32           33         34
                                                        "ObjectiveText3, ObjectiveText4, ReqItemId1, ReqItemId2, ReqItemId3, ReqItemId4, ReqItemId5, ReqItemId6, ReqItemCount1, "
        //                                                     35             36            37              38             39              40                 41
                                                        "ReqItemCount2, ReqItemCount3, ReqItemCount4, ReqItemCount5, ReqItemCount6, ReqKillMobOrGOId1, ReqKillMobOrGOId2, "
        //                                                     42                   43                    44                  45                      46                  47
                                                        "ReqKillMobOrGOId3, ReqKillMobOrGOId4, ReqKillMobOrGOCount1, ReqKillMobOrGOCount2, ReqKillMobOrGOCount3, ReqKillMobOrGOCount4, "
        //                                                     48                 49              50              51              52           53           54           55
                                                        "ReqCastSpellId1, ReqCastSpellId2, ReqCastSpellId3, ReqCastSpellId4, ReqEmoteId1, ReqEmoteId2, ReqEmoteId3, ReqEmoteId4, "
        //                                                     57                  58                59               60                61                 62                63
                                                        "RewChoiceItemId1, RewChoiceItemId2, RewChoiceItemId3, RewChoiceItemId4, RewChoiceItemId5, RewChoiceItemId6, RewChoiceItemCount1, "
        //                                                         64                   65                  66                   67                   68              69          70
                                                        "RewChoiceItemCount2, RewChoiceItemCount3, RewChoiceItemCount4, RewChoiceItemCount5, RewChoiceItemCount6, RewItemId1, RewItemId2, "
        //                                                    71          72           73              74            75             76              77             78             79
                                                        "RewItemId3, RewItemId4, RewItemCount1, RewItemCount2, RewItemCount3, RewItemCount4, RewRepFaction1, RewRepFaction2, RewRepFaction3, "
        //                                                    80               81               82            83           84             85          86              87            88
                                                        "RewRepFaction4, RewRepFaction5, RewRepFaction6, RewRepValue1, RewRepValue2, RewRepValue3, RewRepValue4, RewRepValue5, RewRepValue6, "
        //                                                    89         90       91       92       93            94             95             96           97        98      99      100
                                                        "RewRepLimit, RewMoney, RewXP, RewSpell, CastSpell, MailTemplateId, MailDelaySecs, MailSendItem, PointMapId, PointX, PointY, PointOpt, "
        //                                                         101                  102             103             104              105              106                107
                                                        "RewardMoneyAtMaxLevel, ExploreTrigger1, ExploreTrigger2, ExploreTrigger3, ExploreTrigger4, RequiredOneOfQuest, RequiredQuest1, "
        //                                                    108              109            110             111           112             113            114              115
                                                        "RequiredQuest2, RequiredQuest3, RequiredQuest4, RemoveQuests, ReceiveItemId1, ReceiveItemId2, ReceiveItemId3, ReceiveItemId4, "
        //                                                      116                117                  118               119             120          121            122             123
                                                        "ReceiveItemCount1, ReceiveItemCount2, ReceiveItemCount3, ReceiveItemCount4, IsRepeatable, bonushonor, bonusarenapoints, rewardtitleid, "
        //                                                    124              125               126             127           128           129           130            131
                                                        "rewardtalents, suggestedplayers, detailemotecount, detailemote1, detailemote2, detailemote3, detailemote4, detailemotedelay1, "
        //                                                       132                133                134                135                136               137               138
                                                        "detailemotedelay2, detailemotedelay3, detailemotedelay4, completionemotecnt, completionemote1, completionemote2, completionemote3, "
        //                                                      139                 140                     141                   142                    143                 144
                                                        "completionemote4, completionemotedelay1, completionemotedelay2, completionemotedelay3, completionemotedelay4, completeemote, "
        //                                                     145                   146              147
                                                        "incompleteemote, iscompletedbyspelleffect, RewXPId FROM %s", table_name.c_str());

        if (quest_result == nullptr)
        {
            Log.Notice("MySQLDataLoads", "Table `%s` is empty!", table_name.c_str());
            return;
        }

        uint32 row_count = 0;
        if (table_name.compare("quest_properties") == 0)
        {
            basic_field_count = quest_result->GetFieldCount();
        }
        else
        {
            row_count = _questPropertiesStore.size();
        }

        if (basic_field_count != quest_result->GetFieldCount())
        {
            Log.Error("MySQLDataLoads", "Additional quest_properties table `%s` has %u columns, but needs %u columns! Skipped!", table_name.c_str(), quest_result->GetFieldCount());
            delete quest_result;
            continue;
        }

        Log.Notice("MySQLDataLoads", "Table `%s` has %u columns", table_name.c_str(), quest_result->GetFieldCount());

        _questPropertiesStore.rehash(row_count + quest_result->GetRowCount());

        do
        {
            Field* fields = quest_result->Fetch();

            uint32 entry = fields[0].GetUInt32();

            QuestProperties& questInfo = _questPropertiesStore[entry];

            questInfo.id = entry;
            questInfo.zone_id = fields[1].GetUInt32();
            questInfo.quest_sort = fields[2].GetUInt32();
            questInfo.quest_flags = fields[3].GetUInt32();
            questInfo.min_level = fields[4].GetUInt32();
            questInfo.questlevel = fields[5].GetUInt32();
            questInfo.type = fields[6].GetUInt32();
            questInfo.required_races = fields[7].GetUInt32();
            questInfo.required_class = fields[8].GetUInt32();
            questInfo.required_tradeskill = fields[9].GetUInt32();
            questInfo.required_tradeskill_value = fields[10].GetUInt32();
            questInfo.required_rep_faction = fields[11].GetUInt32();
            questInfo.required_rep_value = fields[12].GetUInt32();

            questInfo.time = fields[13].GetUInt32();
            questInfo.special_flags = fields[14].GetUInt32();

            questInfo.previous_quest_id = fields[15].GetUInt32();
            questInfo.next_quest_id = fields[16].GetUInt32();

            questInfo.srcitem = fields[17].GetUInt32();
            questInfo.srcitemcount = fields[18].GetUInt32();

            questInfo.title = fields[19].GetString();
            questInfo.details = fields[20].GetString();
            questInfo.objectives = fields[21].GetString();
            questInfo.completiontext = fields[22].GetString();
            questInfo.incompletetext = fields[23].GetString();
            questInfo.endtext = fields[24].GetString();

            for (uint8 i = 0; i < 4; ++i)
            {
                questInfo.objectivetexts[i] = fields[25 + i].GetString();
            }

            for (uint8 i = 0; i < MAX_REQUIRED_QUEST_ITEM; ++i)
            {
                questInfo.required_item[i] = fields[28 + i].GetUInt32();
                questInfo.required_itemcount[i] = fields[34 + i].GetUInt32();
            }

            for (uint8 i = 0; i < 4; ++i)
            {
                questInfo.required_mob[i] = fields[40 + i].GetUInt32();
                questInfo.required_mobcount[i] = fields[44 + i].GetUInt32();
                questInfo.required_spell[i] = fields[48 + i].GetUInt32();
                questInfo.required_emote[i] = fields[52 + i].GetUInt32();
            }

            for (uint8 i = 0; i < 6; ++i)
            {
                questInfo.reward_choiceitem[i] = fields[57 + i].GetUInt32();
                questInfo.reward_choiceitemcount[i] = fields[63 + i].GetUInt32();
            }

            for (uint8 i = 0; i < 4; ++i)
            {
                questInfo.reward_item[i] = fields[69 + i].GetUInt32();
                questInfo.reward_itemcount[i] = fields[73 + i].GetUInt32();
            }

            for (uint8 i = 0; i < 6; ++i)
            {
                questInfo.reward_repfaction[i] = fields[77 + i].GetUInt32();
                questInfo.reward_repvalue[i] = fields[83 + i].GetUInt32();
            }

            questInfo.reward_replimit = fields[89].GetUInt32();

            questInfo.reward_money = fields[90].GetUInt32();
            questInfo.reward_xp = fields[91].GetUInt32();
            questInfo.reward_spell = fields[92].GetUInt32();
            questInfo.effect_on_player = fields[93].GetUInt32();

            questInfo.MailTemplateId = fields[94].GetUInt32();
            questInfo.MailDelaySecs = fields[95].GetUInt32();
            questInfo.MailSendItem = fields[96].GetUInt32();

            questInfo.point_mapid = fields[97].GetUInt32();
            questInfo.point_x = fields[98].GetUInt32();
            questInfo.point_y = fields[99].GetUInt32();
            questInfo.point_opt = fields[100].GetUInt32();

            questInfo.rew_money_at_max_level = fields[101].GetUInt32();

            for (uint8 i = 0; i < 4; ++i)
            {
                questInfo.required_triggers[i] = fields[102 + i].GetUInt32();
            }

            questInfo.x_or_y_quest_string = fields[106].GetString();

            for (uint8 i = 0; i < 4; ++i)
            {
                questInfo.required_quests[i] = fields[107 + i].GetUInt32();
            }

            questInfo.remove_quests = fields[111].GetUInt32();

            for (uint8 i = 0; i < 4; ++i)
            {
                questInfo.receive_items[i] = fields[112 + i].GetUInt32();
                questInfo.receive_itemcount[i] = fields[116 + i].GetUInt32();
            }

            questInfo.is_repeatable = fields[120].GetInt32();
            questInfo.bonushonor = fields[121].GetInt32();
            questInfo.bonusarenapoints = fields[122].GetInt32();
            questInfo.rewardtitleid = fields[123].GetInt32();
            questInfo.rewardtalents = fields[124].GetInt32();
            questInfo.suggestedplayers = fields[125].GetInt32();

            // emotes
            questInfo.detailemotecount = fields[126].GetInt32();

            for (uint8 i = 0; i < 4; ++i)
            {
                questInfo.detailemote[i] = fields[127 + i].GetUInt32();
                questInfo.detailemotedelay[i] = fields[131 + i].GetUInt32();
            }

            questInfo.completionemotecount = fields[135].GetInt32();

            for (uint8 i = 0; i < 4; ++i)
            {
                questInfo.completionemote[i] = fields[136 + i].GetUInt32();
                questInfo.completionemotedelay[i] = fields[140 + i].GetUInt32();
            }

            questInfo.completeemote = fields[144].GetInt32();
            questInfo.incompleteemote = fields[145].GetInt32();
            questInfo.iscompletedbyspelleffect = fields[146].GetInt32();
            questInfo.RewXPId = fields[147].GetInt32();


            ++quest_count;
        } while (quest_result->NextRow());

        delete quest_result;
    }

    Log.Success("MySQLDataLoads", "Loaded %u quest_properties data in %u ms!", quest_count, getMSTime() - start_time);
}

QuestProperties const* MySQLDataStore::GetQuestProperties(uint32 entry)
{
    QuestPropertiesContainer::const_iterator itr = _questPropertiesStore.find(entry);
    if (itr != _questPropertiesStore.end())
        return &(itr->second);

    return nullptr;
}

void MySQLDataStore::LoadGameObjectQuestItemBindingTable()
{
    uint32 start_time = getMSTime();

    //                                                                        0      1     2        3
    QueryResult* gameobject_quest_item_result = WorldDatabase.Query("SELECT entry, quest, item, item_count FROM gameobject_quest_item_binding");

    uint32 gameobject_quest_item_count = 0;

    if (gameobject_quest_item_result != nullptr)
    {
        do
        {
            Field* fields = gameobject_quest_item_result->Fetch();
            uint32 entry = fields[0].GetUInt32();

            GameObjectProperties const* gameobject_properties = sMySQLStore.GetGameObjectProperties(entry);
            if (gameobject_properties == nullptr)
            {
                Log.Error("MySQLDataLoads", "Table `gameobject_quest_item_binding` includes data for invalid gameobject_properties entry: %u. Skipped!", entry);
                continue;
            }

            uint32 quest_entry = fields[1].GetUInt32();
            QuestProperties const* quest = sMySQLStore.GetQuestProperties(quest_entry);
            if (quest == nullptr)
            {
                Log.Error("MySQLDataLoads", "Table `gameobject_quest_item_binding` includes data for invalid quest_properties : %u. Skipped!", quest_entry);
                continue;
            }
            else
            {
                const_cast<GameObjectProperties*>(gameobject_properties)->itemMap[quest].insert(std::make_pair(fields[2].GetUInt32(), fields[3].GetUInt32()));
            }

            ++gameobject_quest_item_count;
        } while (gameobject_quest_item_result->NextRow());

        delete gameobject_quest_item_result;
    }

    Log.Success("MySQLDataLoads", "Loaded %u data from `gameobject_quest_item_binding` table in %u ms!", gameobject_quest_item_count, getMSTime() - start_time);
}

void MySQLDataStore::LoadGameObjectQuestPickupBindingTable()
{
    uint32 start_time = getMSTime();

    //                                                                          0      1           2
    QueryResult* gameobject_quest_pickup_result = WorldDatabase.Query("SELECT entry, quest, required_count FROM gameobject_quest_pickup_binding");

    uint32 gameobject_quest_pickup_count = 0;

    if (gameobject_quest_pickup_result != nullptr)
    {
        do
        {
            Field* fields = gameobject_quest_pickup_result->Fetch();
            uint32 entry = fields[0].GetUInt32();

            GameObjectProperties const* gameobject_properties = sMySQLStore.GetGameObjectProperties(entry);
            if (gameobject_properties == nullptr)
            {
                Log.Error("MySQLDataLoads", "Table `gameobject_quest_pickup_binding` includes data for invalid gameobject_properties entry: %u. Skipped!", entry);
                continue;
            }

            uint32 quest_entry = fields[1].GetUInt32();
            QuestProperties const* quest = sMySQLStore.GetQuestProperties(quest_entry);
            if (quest == nullptr)
            {
                Log.Error("MySQLDataLoads", "Table `gameobject_quest_pickup_binding` includes data for invalid quest_properties : %u. Skipped!", quest_entry);
                continue;
            }
            else
            {
                uint32 required_count = fields[2].GetUInt32();
                const_cast<GameObjectProperties*>(gameobject_properties)->goMap.insert(std::make_pair(quest, required_count));
            }
                

            ++gameobject_quest_pickup_count;
        } while (gameobject_quest_pickup_result->NextRow());

        delete gameobject_quest_pickup_result;
    }

    Log.Success("MySQLDataLoads", "Loaded %u data from `gameobject_quest_pickup_binding` table in %u ms!", gameobject_quest_pickup_count, getMSTime() - start_time);
}

void MySQLDataStore::LoadDisplayBoundingBoxesTable()
{
    uint32 start_time = getMSTime();

    //                                                                            0       1    2     3      4      5      6         7
    QueryResult* display_bounding_boxes_result = WorldDatabase.Query("SELECT displayid, lowx, lowy, lowz, highx, highy, highz, boundradius FROM display_bounding_boxes");

    if (display_bounding_boxes_result == nullptr)
    {
        Log.Notice("MySQLDataLoads", "Table `display_bounding_boxes` is empty!");
        return;
    }

    Log.Notice("MySQLDataLoads", "Table `display_bounding_boxes` has %u columns", display_bounding_boxes_result->GetFieldCount());

    _displayBoundingBoxesStore.rehash(display_bounding_boxes_result->GetRowCount());

    uint32 display_bounding_boxes_count = 0;
    do
    {
        Field* fields = display_bounding_boxes_result->Fetch();

        uint32 entry = fields[0].GetUInt32();

        DisplayBounding& displayBounding = _displayBoundingBoxesStore[entry];

        displayBounding.displayid = entry;
        
        for (uint8 i = 0; i < 3; i++)
        {
            displayBounding.low[i] = fields[1 + i].GetFloat();
            displayBounding.high[i] = fields[4 + i].GetFloat();
        }
        
        displayBounding.boundradius = fields[7].GetFloat();


        ++display_bounding_boxes_count;
    } while (display_bounding_boxes_result->NextRow());

    delete display_bounding_boxes_result;

    Log.Success("MySQLDataLoads", "Loaded %u display bounding info from `display_bounding_boxes` table in %u ms!", display_bounding_boxes_count, getMSTime() - start_time);
}

DisplayBounding const* MySQLDataStore::GetDisplayBounding(uint32 entry)
{
    DisplayBoundingBoxesContainer::const_iterator itr = _displayBoundingBoxesStore.find(entry);
    if (itr != _displayBoundingBoxesStore.end())
        return &(itr->second);

    return nullptr;
}

void MySQLDataStore::LoadVendorRestrictionsTable()
{
    uint32 start_time = getMSTime();

    //                                                                      0       1          2            3              4
    QueryResult* vendor_restricitons_result = WorldDatabase.Query("SELECT entry, racemask, classmask, reqrepfaction, reqrepfactionvalue, "
    //                                                                    5                 6           7
                                                                  "canbuyattextid, cannotbuyattextid, flags FROM vendor_restrictions");

    if (vendor_restricitons_result == nullptr)
    {
        Log.Notice("MySQLDataLoads", "Table `vendor_restrictions` is empty!");
        return;
    }

    Log.Notice("MySQLDataLoads", "Table `vendor_restrictions` has %u columns", vendor_restricitons_result->GetFieldCount());

    _vendorRestrictionsStore.rehash(vendor_restricitons_result->GetRowCount());

    uint32 vendor_restricitons_count = 0;
    do
    {
        Field* fields = vendor_restricitons_result->Fetch();

        uint32 entry = fields[0].GetUInt32();

        VendorRestrictionEntry& vendorRestriction = _vendorRestrictionsStore[entry];

        vendorRestriction.entry = entry;
        vendorRestriction.racemask = fields[1].GetInt32();
        vendorRestriction.classmask = fields[2].GetInt32();
        vendorRestriction.reqrepfaction = fields[3].GetUInt32();
        vendorRestriction.reqrepvalue = fields[4].GetUInt32();
        vendorRestriction.canbuyattextid = fields[5].GetUInt32();
        vendorRestriction.cannotbuyattextid = fields[6].GetUInt32();
        vendorRestriction.flags = fields[7].GetUInt32();

        ++vendor_restricitons_count;
    } while (vendor_restricitons_result->NextRow());

    delete vendor_restricitons_result;

    Log.Success("MySQLDataLoads", "Loaded %u restrictions from `vendor_restrictions` table in %u ms!", vendor_restricitons_count, getMSTime() - start_time);
}

VendorRestrictionEntry const* MySQLDataStore::GetVendorRestriction(uint32 entry)
{
    VendorRestrictionContainer::const_iterator itr = _vendorRestrictionsStore.find(entry);
    if (itr != _vendorRestrictionsStore.end())
        return &(itr->second);

    return nullptr;
}

void MySQLDataStore::LoadAreaTriggersTable()
{
    uint32 start_time = getMSTime();

    //                                                                0      1    2     3      4        5           6           7
    QueryResult* area_triggers_result = WorldDatabase.Query("SELECT entry, type, map, screen, name, position_x, position_y, position_z, "
    //                                                            8                9                  10
                                                            "orientation, required_honor_rank, required_level FROM areatriggers");

    if (area_triggers_result == nullptr)
    {
        Log.Notice("MySQLDataLoads", "Table `areatriggers` is empty!");
        return;
    }

    Log.Notice("MySQLDataLoads", "Table `areatriggers` has %u columns", area_triggers_result->GetFieldCount());

    _areaTriggersStore.rehash(area_triggers_result->GetRowCount());

    uint32 area_triggers_count = 0;
    do
    {
        Field* fields = area_triggers_result->Fetch();

        uint32 entry = fields[0].GetUInt32();

        AreaTrigger& areaTrigger = _areaTriggersStore[entry];

        areaTrigger.AreaTriggerID = entry;
        areaTrigger.Type = fields[1].GetInt8();
        areaTrigger.Mapid = fields[2].GetInt32();
        areaTrigger.PendingScreen = fields[3].GetInt32();
        areaTrigger.Name = fields[4].GetString();
        areaTrigger.x = fields[5].GetFloat();
        areaTrigger.y = fields[6].GetFloat();
        areaTrigger.z = fields[7].GetFloat();
        areaTrigger.o = fields[8].GetFloat();
        areaTrigger.required_honor_rank = fields[9].GetInt32();
        areaTrigger.required_level = fields[10].GetInt32();

        ++area_triggers_count;
    } while (area_triggers_result->NextRow());

    delete area_triggers_result;

    Log.Success("MySQLDataLoads", "Loaded %u areatriggers from `areatriggers` table in %u ms!", area_triggers_count, getMSTime() - start_time);
}

AreaTrigger const* MySQLDataStore::GetAreaTrigger(uint32 entry)
{
    AreaTriggerContainer::const_iterator itr = _areaTriggersStore.find(entry);
    if (itr != _areaTriggersStore.end())
        return &(itr->second);

    return nullptr;
}

void MySQLDataStore::LoadNpcTextTable()
{
    uint32 start_time = getMSTime();

    //                                                           0
    QueryResult* npc_text_result = WorldDatabase.Query("SELECT entry, "
    //                                                     1       2        3       4          5           6            7           8            9           10
                                                        "prob0, text0_0, text0_1, lang0, EmoteDelay0_0, Emote0_0, EmoteDelay0_1, Emote0_1, EmoteDelay0_2, Emote0_2, "
    //                                                     11      12       13      14         15          16           17          18           19          20
                                                        "prob1, text1_0, text1_1, lang1, EmoteDelay1_0, Emote1_0, EmoteDelay1_1, Emote1_1, EmoteDelay1_2, Emote1_2, "
    //                                                     21      22       23      24         25          26           27          28           29          30
                                                        "prob2, text2_0, text2_1, lang2, EmoteDelay2_0, Emote2_0, EmoteDelay2_1, Emote2_1, EmoteDelay2_2, Emote2_2, "
    //                                                     31      32       33      34         35          36           37          38           39          40
                                                        "prob3, text3_0, text3_1, lang3, EmoteDelay3_0, Emote3_0, EmoteDelay3_1, Emote3_1, EmoteDelay3_2, Emote3_2, "
    //                                                     41      42       43      44         45          46           47          48           49          50
                                                        "prob4, text4_0, text4_1, lang4, EmoteDelay4_0, Emote4_0, EmoteDelay4_1, Emote4_1, EmoteDelay4_2, Emote4_2, "
    //                                                     51      52       53      54         55          56           57          58           59          60
                                                        "prob5, text5_0, text5_1, lang5, EmoteDelay5_0, Emote5_0, EmoteDelay5_1, Emote5_1, EmoteDelay5_2, Emote5_2, "
    //                                                     61      62       63      64         65          66           67          68           69          70
                                                        "prob6, text6_0, text6_1, lang6, EmoteDelay6_0, Emote6_0, EmoteDelay6_1, Emote6_1, EmoteDelay6_2, Emote6_2, "
    //                                                     71      72       73      74         75          76           77          78           79          80
                                                        "prob7, text7_0, text7_1, lang7, EmoteDelay7_0, Emote7_0, EmoteDelay7_1, Emote7_1, EmoteDelay7_2, Emote7_2 FROM npc_text");

    if (npc_text_result == nullptr)
    {
        Log.Notice("MySQLDataLoads", "Table `npc_text` is empty!");
        return;
    }

    Log.Notice("MySQLDataLoads", "Table `npc_text` has %u columns", npc_text_result->GetFieldCount());

    _npcTextStore.rehash(npc_text_result->GetRowCount());

    uint32 npc_text_count = 0;
    do
    {
        Field* fields = npc_text_result->Fetch();

        uint32 entry = fields[0].GetUInt32();

        NpcText& npcText = _npcTextStore[entry];

        npcText.ID = entry;
        for (uint8 i = 0; i < 8; ++i)
        {
            npcText.Texts[i].Prob = fields[1].GetFloat();

            for (uint8 j = 0; j < 2; ++j)
            {
                npcText.Texts[i].Text[j] = fields[2 + j].GetString();
            }

            npcText.Texts[i].Lang = fields[4].GetUInt32();

            for (uint8 k = 0; k < GOSSIP_EMOTE_COUNT; ++k)
            {
                npcText.Texts[i].Emotes[k].Delay = fields[5 + k * 2].GetUInt32();
                npcText.Texts[i].Emotes[k].Emote = fields[6 + k * 2].GetUInt32();
            }
        }


        ++npc_text_count;
    } while (npc_text_result->NextRow());

    delete npc_text_result;

    Log.Success("MySQLDataLoads", "Loaded %u rows from `npc_text` table in %u ms!", npc_text_count, getMSTime() - start_time);
}

NpcText const* MySQLDataStore::GetNpcText(uint32 entry)
{
    NpcTextContainer::const_iterator itr = _npcTextStore.find(entry);
    if (itr != _npcTextStore.end())
        return &(itr->second);

    return nullptr;
}

void MySQLDataStore::LoadNpcScriptTextTable()
{
    uint32 start_time = getMSTime();

    //                                                                  0      1           2       3     4       5          6         7       8        9         10
    QueryResult* npc_script_text_result = WorldDatabase.Query("SELECT entry, text, creature_entry, id, type, language, probability, emote, duration, sound, broadcast_id FROM npc_script_text");

    if (npc_script_text_result == nullptr)
    {
        Log.Notice("MySQLDataLoads", "Table `npc_script_text` is empty!");
        return;
    }

    Log.Notice("MySQLDataLoads", "Table `npc_script_text` has %u columns", npc_script_text_result->GetFieldCount());

    _npcScriptTextStore.rehash(npc_script_text_result->GetRowCount());

    uint32 npc_script_text_count = 0;
    do
    {
        Field* fields = npc_script_text_result->Fetch();

        uint32 entry = fields[0].GetUInt32();

        NpcScriptText& npcScriptText = _npcScriptTextStore[entry];

        npcScriptText.id = entry;
        npcScriptText.text = fields[1].GetString();
        npcScriptText.creature_entry = fields[2].GetUInt32();
        npcScriptText.text_id = fields[3].GetUInt32();
        npcScriptText.type = ChatMsg(fields[4].GetUInt32());
        npcScriptText.language = Languages(fields[5].GetUInt32());
        npcScriptText.probability = fields[6].GetFloat();
        npcScriptText.emote = EmoteType(fields[7].GetUInt32());
        npcScriptText.duration = fields[8].GetUInt32();
        npcScriptText.sound = fields[9].GetUInt32();
        npcScriptText.broadcast_id = fields[10].GetUInt32();

        ++npc_script_text_count;
    } while (npc_script_text_result->NextRow());

    delete npc_script_text_result;

    Log.Success("MySQLDataLoads", "Loaded %u rows from `npc_script_text` table in %u ms!", npc_script_text_count, getMSTime() - start_time);
}

NpcScriptText const* MySQLDataStore::GetNpcScriptText(uint32 entry)
{
    NpcScriptTextContainer::const_iterator itr = _npcScriptTextStore.find(entry);
    if (itr != _npcScriptTextStore.end())
        return &(itr->second);

    return nullptr;
}

void MySQLDataStore::LoadGossipMenuOptionTable()
{
    uint32 start_time = getMSTime();

    //                                                                      0         1
    QueryResult* gossip_menu_optiont_result = WorldDatabase.Query("SELECT entry, option_text FROM gossip_menu_option");

    if (gossip_menu_optiont_result == nullptr)
    {
        Log.Notice("MySQLDataLoads", "Table `gossip_menu_option` is empty!");
        return;
    }

    Log.Notice("MySQLDataLoads", "Table `gossip_menu_option` has %u columns", gossip_menu_optiont_result->GetFieldCount());

    _gossipMenuOptionStore.rehash(gossip_menu_optiont_result->GetRowCount());

    uint32 gossip_menu_optiont_count = 0;
    do
    {
        Field* fields = gossip_menu_optiont_result->Fetch();

        uint32 entry = fields[0].GetUInt32();

        GossipMenuOption& gossipMenuOptionText = _gossipMenuOptionStore[entry];

        gossipMenuOptionText.id = entry;
        gossipMenuOptionText.text = fields[1].GetString();

        ++gossip_menu_optiont_count;
    } while (gossip_menu_optiont_result->NextRow());

    delete gossip_menu_optiont_result;

    Log.Success("MySQLDataLoads", "Loaded %u rows from `gossip_menu_option` table in %u ms!", gossip_menu_optiont_count, getMSTime() - start_time);
}

GossipMenuOption const* MySQLDataStore::GetGossipMenuOption(uint32 entry)
{
    GossipMenuOptionContainer::const_iterator itr = _gossipMenuOptionStore.find(entry);
    if (itr != _gossipMenuOptionStore.end())
        return &(itr->second);

    return nullptr;
}

void MySQLDataStore::LoadGraveyardsTable()
{
    uint32 start_time = getMSTime();

    //                                                            0         1         2           3            4         5          6           7       8
    QueryResult* graveyards_result = WorldDatabase.Query("SELECT id, position_x, position_y, position_z, orientation, zoneid, adjacentzoneid, mapid, faction FROM graveyards");
    if (graveyards_result == nullptr)
    {
        Log.Notice("MySQLDataLoads", "Table `graveyards` is empty!");
        return;
    }

    Log.Notice("MySQLDataLoads", "Table `graveyards` has %u columns", graveyards_result->GetFieldCount());

    _graveyardsStore.rehash(graveyards_result->GetRowCount());

    uint32 graveyards_count = 0;
    do
    {
        Field* fields = graveyards_result->Fetch();

        uint32 entry = fields[0].GetUInt32();

        GraveyardTeleport& graveyardTeleport = _graveyardsStore[entry];

        graveyardTeleport.ID = entry;
        graveyardTeleport.X = fields[1].GetFloat();
        graveyardTeleport.Y = fields[2].GetFloat();
        graveyardTeleport.Z = fields[3].GetFloat();
        graveyardTeleport.O = fields[4].GetFloat();
        graveyardTeleport.ZoneId = fields[5].GetUInt32();
        graveyardTeleport.AdjacentZoneId = fields[6].GetUInt32();
        graveyardTeleport.MapId = fields[7].GetUInt32();
        graveyardTeleport.FactionID = fields[8].GetUInt32();

        ++graveyards_count;
    } while (graveyards_result->NextRow());

    delete graveyards_result;

    Log.Success("MySQLDataLoads", "Loaded %u rows from `graveyards` table in %u ms!", graveyards_count, getMSTime() - start_time);
}

GraveyardTeleport const* MySQLDataStore::GetGraveyard(uint32 entry)
{
    GraveyardsContainer::const_iterator itr = _graveyardsStore.find(entry);
    if (itr != _graveyardsStore.end())
        return &(itr->second);

    return nullptr;
}

void MySQLDataStore::LoadTeleportCoordsTable()
{
    uint32 start_time = getMSTime();

    //                                                                0     1         2           3           4
    QueryResult* teleport_coords_result = WorldDatabase.Query("SELECT id, mapId, position_x, position_y, position_z FROM spell_teleport_coords");
    if (teleport_coords_result == nullptr)
    {
        Log.Notice("MySQLDataLoads", "Table `spell_teleport_coords` is empty!");
        return;
    }

    Log.Notice("MySQLDataLoads", "Table `spell_teleport_coords` has %u columns", teleport_coords_result->GetFieldCount());

    _teleportCoordsStore.rehash(teleport_coords_result->GetRowCount());

    uint32 teleport_coords_count = 0;
    do
    {
        Field* fields = teleport_coords_result->Fetch();

        uint32 entry = fields[0].GetUInt32();

        TeleportCoords& teleportCoords = _teleportCoordsStore[entry];

        teleportCoords.id = entry;
        teleportCoords.mapId = fields[1].GetUInt32();
        teleportCoords.x = fields[2].GetFloat();
        teleportCoords.y = fields[3].GetFloat();
        teleportCoords.z = fields[4].GetFloat();

        ++teleport_coords_count;
    } while (teleport_coords_result->NextRow());

    delete teleport_coords_result;

    Log.Success("MySQLDataLoads", "Loaded %u rows from `spell_teleport_coords` table in %u ms!", teleport_coords_count, getMSTime() - start_time);
}

TeleportCoords const* MySQLDataStore::GetTeleportCoord(uint32 entry)
{
    TeleportCoordsContainer::const_iterator itr = _teleportCoordsStore.find(entry);
    if (itr != _teleportCoordsStore.end())
        return &(itr->second);

    return nullptr;
}

void MySQLDataStore::LoadFishingTable()
{
    uint32 start_time = getMSTime();

    //                                                          0      1         2
    QueryResult* fishing_result = WorldDatabase.Query("SELECT zone, MinSkill, MaxSkill FROM fishing");
    if (fishing_result == nullptr)
    {
        Log.Notice("MySQLDataLoads", "Table `fishing` is empty!");
        return;
    }

    Log.Notice("MySQLDataLoads", "Table `fishing` has %u columns", fishing_result->GetFieldCount());

    _fishingZonesStore.rehash(fishing_result->GetRowCount());

    uint32 fishing_count = 0;
    do
    {
        Field* fields = fishing_result->Fetch();

        uint32 entry = fields[0].GetUInt32();

        FishingZoneEntry& fishingZone = _fishingZonesStore[entry];

        fishingZone.ZoneID = entry;
        fishingZone.MinSkill = fields[1].GetUInt32();
        fishingZone.MaxSkill = fields[2].GetUInt32();

        ++fishing_count;
    } while (fishing_result->NextRow());

    delete fishing_result;

    Log.Success("MySQLDataLoads", "Loaded %u rows from `fishing` table in %u ms!", fishing_count, getMSTime() - start_time);
}

FishingZoneEntry const* MySQLDataStore::GetFishingZone(uint32 entry)
{
    FishingZonesContainer::const_iterator itr = _fishingZonesStore.find(entry);
    if (itr != _fishingZonesStore.end())
        return &(itr->second);

    return nullptr;
}

void MySQLDataStore::LoadWorldMapInfoTable()
{
    uint32 start_time = getMSTime();

    //                                                                0        1       2       3           4             5          6        7      8          9
    QueryResult* worldmap_info_result = WorldDatabase.Query("SELECT entry, screenid, type, maxplayers, minlevel, minlevel_heroic, repopx, repopy, repopz, repopentry, "
    //                                                           10       11      12         13           14                15              16
                                                            "area_name, flags, cooldown, lvl_mod_a, required_quest_A, required_quest_H, required_item, "
    //                                                              17              18              19                20
                                                            "heroic_keyid_1, heroic_keyid_2, viewingDistance, required_checkpoint FROM worldmap_info");
    if (worldmap_info_result == nullptr)
    {
        Log.Notice("MySQLDataLoads", "Table `worldmap_info` is empty!");
        return;
    }

    Log.Notice("MySQLDataLoads", "Table `worldmap_info` has %u columns", worldmap_info_result->GetFieldCount());

    _worldMapInfoStore.rehash(worldmap_info_result->GetRowCount());

    uint32 world_map_info_count = 0;
    do
    {
        Field* fields = worldmap_info_result->Fetch();

        uint32 entry = fields[0].GetUInt32();

        MapInfo& mapInfo = _worldMapInfoStore[entry];

        mapInfo.mapid = entry;
        mapInfo.screenid = fields[1].GetUInt32();
        mapInfo.type = fields[2].GetUInt32();
        mapInfo.playerlimit = fields[3].GetUInt32();
        mapInfo.minlevel = fields[4].GetUInt32();
        mapInfo.minlevel_heroic = fields[5].GetUInt32();
        mapInfo.repopx = fields[6].GetFloat();
        mapInfo.repopy = fields[7].GetFloat();
        mapInfo.repopz = fields[8].GetFloat();
        mapInfo.repopmapid = fields[9].GetUInt32();
        mapInfo.name = fields[10].GetString();
        mapInfo.flags = fields[11].GetUInt32();
        mapInfo.cooldown = fields[12].GetUInt32();
        mapInfo.lvl_mod_a = fields[13].GetUInt32();
        mapInfo.required_quest_A = fields[14].GetUInt32();
        mapInfo.required_quest_H = fields[15].GetUInt32();
        mapInfo.required_item = fields[16].GetUInt32();
        mapInfo.heroic_key_1 = fields[17].GetUInt32();
        mapInfo.heroic_key_2 = fields[18].GetUInt32();
        mapInfo.update_distance = fields[19].GetFloat();
        mapInfo.checkpoint_id = fields[20].GetUInt32();

        ++world_map_info_count;
    } while (worldmap_info_result->NextRow());

    delete worldmap_info_result;

    Log.Success("MySQLDataLoads", "Loaded %u rows from `worldmap_info` table in %u ms!", world_map_info_count, getMSTime() - start_time);
}

MapInfo const* MySQLDataStore::GetWorldMapInfo(uint32 entry)
{
    WorldMapInfoContainer::const_iterator itr = _worldMapInfoStore.find(entry);
    if (itr != _worldMapInfoStore.end())
        return &(itr->second);

    return nullptr;
}

void MySQLDataStore::LoadZoneGuardsTable()
{
    uint32 start_time = getMSTime();

    //                                                             0         1              2
    QueryResult* zone_guards_result = WorldDatabase.Query("SELECT zone, horde_entry, alliance_entry FROM zoneguards");
    if (zone_guards_result == nullptr)
    {
        Log.Notice("MySQLDataLoads", "Table `zoneguards` is empty!");
        return;
    }

    Log.Notice("MySQLDataLoads", "Table `zoneguards` has %u columns", zone_guards_result->GetFieldCount());

    _zoneGuardsStore.rehash(zone_guards_result->GetRowCount());

    uint32 zone_guards_count = 0;
    do
    {
        Field* fields = zone_guards_result->Fetch();

        uint32 entry = fields[0].GetUInt32();

        ZoneGuardEntry& zoneGuard = _zoneGuardsStore[entry];

        zoneGuard.ZoneID = entry;
        zoneGuard.HordeEntry = fields[1].GetUInt32();
        zoneGuard.AllianceEntry = fields[2].GetUInt32();

        ++zone_guards_count;
    } while (zone_guards_result->NextRow());

    delete zone_guards_result;

    Log.Success("MySQLDataLoads", "Loaded %u rows from `zoneguards` table in %u ms!", zone_guards_count, getMSTime() - start_time);
}

ZoneGuardEntry const* MySQLDataStore::GetZoneGuard(uint32 entry)
{
    ZoneGuardsContainer::const_iterator itr = _zoneGuardsStore.find(entry);
    if (itr != _zoneGuardsStore.end())
        return &(itr->second);

    return nullptr;
}

void MySQLDataStore::LoadBattleMastersTable()
{
    uint32 start_time = getMSTime();

    //                                                                      0                1
    QueryResult* battlemasters_result = WorldDatabase.Query("SELECT creature_entry, battleground_id FROM battlemasters");
    if (battlemasters_result == nullptr)
    {
        Log.Notice("MySQLDataLoads", "Table `battlemasters` is empty!");
        return;
    }

    Log.Notice("MySQLDataLoads", "Table `battlemasters` has %u columns", battlemasters_result->GetFieldCount());

    _battleMastersStore.rehash(battlemasters_result->GetRowCount());

    uint32 battlemasters_count = 0;
    do
    {
        Field* fields = battlemasters_result->Fetch();

        uint32 entry = fields[0].GetUInt32();

        BGMaster& bgMaster = _battleMastersStore[entry];

        bgMaster.entry = entry;
        bgMaster.bg = fields[1].GetUInt32();

        ++battlemasters_count;
    } while (battlemasters_result->NextRow());

    delete battlemasters_result;

    Log.Success("MySQLDataLoads", "Loaded %u rows from `battlemasters` table in %u ms!", battlemasters_count, getMSTime() - start_time);
}

BGMaster const* MySQLDataStore::GetBattleMaster(uint32 entry)
{
    BattleMastersContainer::const_iterator itr = _battleMastersStore.find(entry);
    if (itr != _battleMastersStore.end())
        return &(itr->second);

    return nullptr;
}

void MySQLDataStore::LoadTotemDisplayIdsTable()
{
    uint32 start_time = getMSTime();

    //                                                                      0         1        2       3
    QueryResult* totemdisplayids_result = WorldDatabase.Query("SELECT displayid, draeneiid, trollid, orcid FROM totemdisplayids");
    if (totemdisplayids_result == nullptr)
    {
        Log.Notice("MySQLDataLoads", "Table `totemdisplayids` is empty!");
        return;
    }

    Log.Notice("MySQLDataLoads", "Table `totemdisplayids` has %u columns", totemdisplayids_result->GetFieldCount());

    _totemDisplayIdsStore.rehash(totemdisplayids_result->GetRowCount());

    uint32 totemdisplayids_count = 0;
    do
    {
        Field* fields = totemdisplayids_result->Fetch();

        uint32 entry = fields[0].GetUInt32();

        TotemDisplayIdEntry& totemDisplayId = _totemDisplayIdsStore[entry];

        totemDisplayId.DisplayId = entry;
        totemDisplayId.DraeneiId = fields[1].GetUInt32();
        totemDisplayId.TrollId = fields[2].GetUInt32();
        totemDisplayId.OrcId = fields[3].GetUInt32();

        ++totemdisplayids_count;
    } while (totemdisplayids_result->NextRow());

    delete totemdisplayids_result;

    Log.Success("MySQLDataLoads", "Loaded %u rows from `totemdisplayids` table in %u ms!", totemdisplayids_count, getMSTime() - start_time);
}

TotemDisplayIdEntry const* MySQLDataStore::GetTotemDisplayId(uint32 entry)
{
    TotemDisplayIdContainer::const_iterator itr = _totemDisplayIdsStore.find(entry);
    if (itr != _totemDisplayIdsStore.end())
        return &(itr->second);

    return nullptr;
}

void MySQLDataStore::LoadSpellClickSpellsTable()
{
    uint32 start_time = getMSTime();

    //                                                                      0         1
    QueryResult* spellclickspells_result = WorldDatabase.Query("SELECT CreatureID, SpellID FROM spellclickspells");
    if (spellclickspells_result == nullptr)
    {
        Log.Notice("MySQLDataLoads", "Table `spellclickspells` is empty!");
        return;
    }

    Log.Notice("MySQLDataLoads", "Table `spellclickspells` has %u columns", spellclickspells_result->GetFieldCount());

    _spellClickSpellsStore.rehash(spellclickspells_result->GetRowCount());

    uint32 spellclickspells_count = 0;
    do
    {
        Field* fields = spellclickspells_result->Fetch();

        uint32 entry = fields[0].GetUInt32();

        SpellClickSpell& spellClickSpells = _spellClickSpellsStore[entry];

        spellClickSpells.CreatureID = entry;
        spellClickSpells.SpellID = fields[1].GetUInt32();

        ++spellclickspells_count;
    } while (spellclickspells_result->NextRow());

    delete spellclickspells_result;

    Log.Success("MySQLDataLoads", "Loaded %u rows from `spellclickspells` table in %u ms!", spellclickspells_count, getMSTime() - start_time);
}

SpellClickSpell const* MySQLDataStore::GetSpellClickSpell(uint32 entry)
{
    SpellClickSpellContainer::const_iterator itr = _spellClickSpellsStore.find(entry);
    if (itr != _spellClickSpellsStore.end())
        return &(itr->second);

    return nullptr;
}

void MySQLDataStore::LoadWorldStringsTable()
{
    uint32 start_time = getMSTime();

    //                                                                     0     1
    QueryResult* worldstring_tables_result = WorldDatabase.Query("SELECT entry, text FROM worldstring_tables");
    if (worldstring_tables_result == nullptr)
    {
        Log.Notice("MySQLDataLoads", "Table `worldstring_tables` is empty!");
        return;
    }

    Log.Notice("MySQLDataLoads", "Table `worldstring_tables` has %u columns", worldstring_tables_result->GetFieldCount());

    _worldStringsStore.rehash(worldstring_tables_result->GetRowCount());

    uint32 worldstring_tables_count = 0;
    do
    {
        Field* fields = worldstring_tables_result->Fetch();

        uint32 entry = fields[0].GetUInt32();

        WorldStringTable& worldString = _worldStringsStore[entry];

        worldString.id = entry;
        worldString.text = fields[1].GetString();

        ++worldstring_tables_count;
    } while (worldstring_tables_result->NextRow());

    delete worldstring_tables_result;

    Log.Success("MySQLDataLoads", "Loaded %u rows from `worldstring_tables` table in %u ms!", worldstring_tables_count, getMSTime() - start_time);
}

WorldStringTable const* MySQLDataStore::GetWorldString(uint32 entry)
{
    WorldStringContainer::const_iterator itr = _worldStringsStore.find(entry);
    if (itr != _worldStringsStore.end())
        return &(itr->second);

    return nullptr;
}

void MySQLDataStore::LoadWorldBroadcastTable()
{
    uint32 start_time = getMSTime();

    //                                                                 0      1       2
    QueryResult* worldbroadcast_result = WorldDatabase.Query("SELECT entry, text, percent FROM worldbroadcast");
    if (worldbroadcast_result == nullptr)
    {
        Log.Notice("MySQLDataLoads", "Table `worldbroadcast` is empty!");
        return;
    }

    Log.Notice("MySQLDataLoads", "Table `worldbroadcast` has %u columns", worldbroadcast_result->GetFieldCount());

    _worldBroadcastStore.rehash(worldbroadcast_result->GetRowCount());

    uint32 worldbroadcast_count = 0;
    do
    {
        Field* fields = worldbroadcast_result->Fetch();

        uint32 entry = fields[0].GetUInt32();

        WorldBroadCast& worldBroadCast = _worldBroadcastStore[entry];

        worldBroadCast.id = entry;
        worldBroadCast.text = fields[1].GetString();
        worldBroadCast.percent = fields[2].GetUInt32();

        ++worldbroadcast_count;
    } while (worldbroadcast_result->NextRow());

    delete worldbroadcast_result;

    Log.Success("MySQLDataLoads", "Loaded %u rows from `worldbroadcast` table in %u ms!", worldbroadcast_count, getMSTime() - start_time);
}

WorldBroadCast const* MySQLDataStore::GetWorldBroadcast(uint32 entry)
{
    WorldBroadCastContainer::const_iterator itr = _worldBroadcastStore.find(entry);
    if (itr != _worldBroadcastStore.end())
        return &(itr->second);

    return nullptr;
}

void MySQLDataStore::LoadPointOfInterestTable()
{
    uint32 start_time = getMSTime();

    //                                                                      0   1  2    3     4     5        6
    QueryResult* points_of_interest_result = WorldDatabase.Query("SELECT entry, x, y, icon, flags, data, icon_name FROM points_of_interest");
    if (points_of_interest_result == nullptr)
    {
        Log.Notice("MySQLDataLoads", "Table `points_of_interest` is empty!");
        return;
    }

    Log.Notice("MySQLDataLoads", "Table `points_of_interest` has %u columns", points_of_interest_result->GetFieldCount());

    _pointOfInterestStore.rehash(points_of_interest_result->GetRowCount());

    uint32 points_of_interest_count = 0;
    do
    {
        Field* fields = points_of_interest_result->Fetch();

        uint32 entry = fields[0].GetUInt32();

        PointOfInterest& pointOfInterest = _pointOfInterestStore[entry];

        pointOfInterest.entry = entry;
        pointOfInterest.x = fields[1].GetFloat();
        pointOfInterest.y = fields[2].GetFloat();
        pointOfInterest.icon = fields[3].GetUInt32();
        pointOfInterest.flags = fields[4].GetUInt32();
        pointOfInterest.data = fields[5].GetUInt32();
        pointOfInterest.icon_name = fields[6].GetString();

        ++points_of_interest_count;
    } while (points_of_interest_result->NextRow());

    delete points_of_interest_result;

    Log.Success("MySQLDataLoads", "Loaded %u rows from `points_of_interest` table in %u ms!", points_of_interest_count, getMSTime() - start_time);
}

PointOfInterest const* MySQLDataStore::GetPointOfInterest(uint32 entry)
{
    PointOfInterestContainer::const_iterator itr = _pointOfInterestStore.find(entry);
    if (itr != _pointOfInterestStore.end())
        return &(itr->second);

    return nullptr;
}
