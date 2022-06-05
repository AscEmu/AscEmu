/*
Copyright (c) 2014-2022 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/


#include "Storage/MySQLDataStore.hpp"
#include "Spell/SpellClickInfo.hpp"
#include "Server/MainServerDefines.h"
#include "Spell/SpellMgr.hpp"
#include "Util/Strings.hpp"
#include <regex>

SERVER_DECL std::vector<MySQLAdditionalTable> MySQLAdditionalTables;

MySQLDataStore& MySQLDataStore::getInstance()
{
    static MySQLDataStore mInstance;
    return mInstance;
}

void MySQLDataStore::finalize()
{
    for (auto&& professionDiscovery : _professionDiscoveryStore)
    {
        delete professionDiscovery;
    }
}

void MySQLDataStore::loadAdditionalTableConfig()
{
    // get config
    const std::string strData = worldConfig.startup.additionalTableLoads;
    if (strData.empty())
        return;

    const std::vector<std::string> strs = AscEmu::Util::Strings::split(strData, ",");
    if (strs.empty())
        return;

    for (auto& str : strs)
    {
        std::stringstream additionTableStream(str);
        std::string additional_table;
        std::string target_table;

        additionTableStream >> additional_table;
        additionTableStream >> target_table;

        if (additional_table.empty() || target_table.empty())
            continue;

        // Zyres: new way for general additional tables
        MySQLAdditionalTable myTable;
        myTable.mainTable = target_table;
        myTable.tableVector.push_back(target_table);
        myTable.tableVector.push_back(additional_table);

        MySQLAdditionalTables.push_back(myTable);
        sLogger.info("MySQLDataLoads : Table %s added as additional table for %s", additional_table.c_str(), target_table.c_str());
    }
}

QueryResult* MySQLDataStore::getWorldDBQuery(std::string query, ...)
{
    // fill in values
    const char* rawQuery = query.c_str();
    char finalizedQuery[16384];

    va_list vlist;
    va_start(vlist, rawQuery);
    vsnprintf(finalizedQuery, 16384, rawQuery, vlist);
    va_end(vlist);

    // save query as prepared
    std::string preparedQuery = finalizedQuery;

    // checkout additional tables
    for (auto additionalTable : MySQLAdditionalTables)
    {
        // query includes table which has additional tables
        if (AscEmu::Util::Strings::contains(additionalTable.mainTable, preparedQuery))
        {
            // set up new query including the original one
            std::string completeQuery = preparedQuery;

            if (additionalTable.tableVector.size() > 1)
            {
                // go through tables, note: main table is always part of it
                for (const auto& table : additionalTable.tableVector)
                {
                    // we already have the query for the main table, if it is an additional table add UNION query
                    if (table != additionalTable.mainTable)
                    {
                        std::string changeQuery = preparedQuery;

                        changeQuery = std::regex_replace(changeQuery, std::regex(additionalTable.mainTable), table);

                        completeQuery += " UNION ";
                        completeQuery += changeQuery;

                        sLogger.debugFlag(AscEmu::Logging::DebugFlags::LF_DB_TABLES, "MySQLDataLoads : Added additional query '%s'", changeQuery.c_str());
                    }
                }
            }

            sLogger.debugFlag(AscEmu::Logging::DebugFlags::LF_DB_TABLES, "MySQLDataLoads : AdditionalTableLoading - Query: '%s'", completeQuery.c_str());
            return WorldDatabase.Query(completeQuery.c_str());
        }
    }

    // no additional tables defined, just send our query
    return WorldDatabase.Query(preparedQuery.c_str());
}

void MySQLDataStore::loadItemPagesTable()
{
    auto startTime = Util::TimeNow();

    QueryResult* itempages_result = WorldDatabase.Query("SELECT entry, text, next_page FROM item_pages");
    if (itempages_result == nullptr)
    {
        sLogger.info("MySQLDataLoads : Table `item_pages` is empty!");
        return;
    }

    sLogger.info("MySQLDataLoads : Table `item_pages` has %u columns", itempages_result->GetFieldCount());

    _itemPagesStore.rehash(itempages_result->GetRowCount());

    uint32_t itempages_count = 0;
    do
    {
        Field* fields = itempages_result->Fetch();

        uint32_t entry = fields[0].GetUInt32();

        MySQLStructure::ItemPage& itemPage = _itemPagesStore[entry];

        itemPage.id = entry;
        itemPage.text = fields[1].GetString();
        itemPage.nextPage = fields[2].GetUInt32();


        ++itempages_count;
    } while (itempages_result->NextRow());

    delete itempages_result;

    sLogger.info("MySQLDataLoads : Loaded %u pages from `item_pages` table in %u ms!", itempages_count, static_cast<uint32_t>(Util::GetTimeDifferenceToNow(startTime)));
}

MySQLStructure::ItemPage const* MySQLDataStore::getItemPage(uint32_t entry)
{
    ItemPageContainer::const_iterator itr = _itemPagesStore.find(entry);
    if (itr != _itemPagesStore.end())
        return &(itr->second);

    return nullptr;
}

void MySQLDataStore::loadItemPropertiesTable()
{
    auto startTime = Util::TimeNow();

    uint32_t item_count = 0;

    QueryResult* item_result = sMySQLStore.getWorldDBQuery("SELECT * FROM item_properties base "
        "WHERE build=(SELECT MAX(build) FROM item_properties spec WHERE base.entry = spec.entry AND build <= %u)", VERSION_STRING);

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
        sLogger.info("MySQLDataLoads : Table `item_properties` is empty!");
        return;
    }

    sLogger.info("MySQLDataLoads : Table `item_properties` has %u columns", item_result->GetFieldCount());

    _itemPropertiesStore.rehash(item_result->GetRowCount());

    do
    {
        Field* fields = item_result->Fetch();

        uint32_t entry = fields[0].GetUInt32();

        ItemProperties& itemProperties = _itemPropertiesStore[entry];

        itemProperties.ItemId = entry;
        itemProperties.Class = fields[2].GetUInt32();
        itemProperties.SubClass = fields[3].GetUInt16();
        itemProperties.unknown_bc = fields[4].GetUInt32();
        itemProperties.Name = fields[5].GetString();
        itemProperties.DisplayInfoID = fields[6].GetUInt32();
        itemProperties.Quality = fields[7].GetUInt32();
        itemProperties.Flags = fields[8].GetUInt32();
        itemProperties.Flags2 = fields[9].GetUInt32();
        itemProperties.BuyPrice = fields[10].GetUInt32();
        itemProperties.SellPrice = fields[11].GetUInt32();

        itemProperties.InventoryType = fields[12].GetUInt32();
        itemProperties.AllowableClass = fields[13].GetUInt32();
        itemProperties.AllowableRace = fields[14].GetUInt32();
        itemProperties.ItemLevel = fields[15].GetUInt32();
        itemProperties.RequiredLevel = fields[16].GetUInt32();
        itemProperties.RequiredSkill = fields[17].GetUInt16();
        itemProperties.RequiredSkillRank = fields[18].GetUInt32();
        itemProperties.RequiredSkillSubRank = fields[19].GetUInt32();
        itemProperties.RequiredPlayerRank1 = fields[20].GetUInt32();
        itemProperties.RequiredPlayerRank2 = fields[21].GetUInt32();
        itemProperties.RequiredFaction = fields[22].GetUInt32();
        itemProperties.RequiredFactionStanding = fields[23].GetUInt32();
        itemProperties.Unique = fields[24].GetUInt32();
        itemProperties.MaxCount = fields[25].GetUInt32();
        itemProperties.ContainerSlots = fields[26].GetUInt32();
        itemProperties.itemstatscount = fields[27].GetUInt32();

        for (uint8_t i = 0; i < itemProperties.itemstatscount; ++i)
        {
            itemProperties.Stats[i].Type = fields[28 + i * 2].GetUInt32();
            itemProperties.Stats[i].Value = fields[29 + i * 2].GetInt32();
        }

        itemProperties.ScalingStatsEntry = fields[48].GetUInt32();
        itemProperties.ScalingStatsFlag = fields[49].GetUInt32();

        for (uint8_t i = 0; i < MAX_ITEM_PROTO_DAMAGES; ++i)
        {
            itemProperties.Damage[i].Min = fields[50 + i * 3].GetFloat();
            itemProperties.Damage[i].Max = fields[51 + i * 3].GetFloat();
            itemProperties.Damage[i].Type = fields[52 + i * 3].GetUInt32();
        }

        itemProperties.Armor = fields[56].GetUInt32();
        itemProperties.HolyRes = fields[57].GetUInt32();
        itemProperties.FireRes = fields[58].GetUInt32();
        itemProperties.NatureRes = fields[59].GetUInt32();
        itemProperties.FrostRes = fields[60].GetUInt32();
        itemProperties.ShadowRes = fields[61].GetUInt32();
        itemProperties.ArcaneRes = fields[62].GetUInt32();
        itemProperties.Delay = fields[63].GetUInt32();
        itemProperties.AmmoType = fields[64].GetUInt32();
        itemProperties.Range = fields[65].GetFloat();

        for (uint8_t i = 0; i < MAX_ITEM_PROTO_SPELLS; ++i)
        {
            itemProperties.Spells[i].Id = fields[66 + i * 6].GetUInt32();
            itemProperties.Spells[i].Trigger = fields[67 + i * 6].GetUInt32();
            itemProperties.Spells[i].Charges = fields[68 + i * 6].GetInt32();
            itemProperties.Spells[i].Cooldown = fields[69 + i * 6].GetInt32();
            itemProperties.Spells[i].Category = fields[70 + i * 6].GetUInt32();
            itemProperties.Spells[i].CategoryCooldown = fields[71 + i * 6].GetInt32();
        }

        itemProperties.Bonding = fields[96].GetUInt32();
        itemProperties.Description = fields[97].GetString();
        uint32_t page_id = fields[98].GetUInt32();
        if (page_id != 0)
        {
            MySQLStructure::ItemPage const* item_page = getItemPage(page_id);
            if (item_page == nullptr)
            {
                sLogger.failure("Table `item_properties` entry: %u includes invalid pageId %u! pageId is set to 0.", entry, page_id);
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

        itemProperties.PageLanguage = fields[99].GetUInt32();
        itemProperties.PageMaterial = fields[100].GetUInt32();
        itemProperties.QuestId = fields[101].GetUInt32();
        itemProperties.LockId = fields[102].GetUInt32();
        itemProperties.LockMaterial = fields[103].GetUInt32();
        itemProperties.SheathID = fields[104].GetUInt32();
        itemProperties.RandomPropId = fields[105].GetUInt32();
        itemProperties.RandomSuffixId = fields[106].GetUInt32();
        itemProperties.Block = fields[107].GetUInt32();
        itemProperties.ItemSet = fields[108].GetInt32();
        itemProperties.MaxDurability = fields[109].GetUInt32();
        itemProperties.ZoneNameID = fields[110].GetUInt32();
        itemProperties.MapID = fields[111].GetUInt32();
        itemProperties.BagFamily = fields[112].GetUInt32();
        itemProperties.TotemCategory = fields[113].GetUInt32();

        for (uint8_t i = 0; i < MAX_ITEM_PROTO_SOCKETS; ++i)
        {
            itemProperties.Sockets[i].SocketColor = uint32_t(fields[114 + i * 2].GetUInt8());
            itemProperties.Sockets[i].Unk = fields[115 + i * 2].GetUInt32();
        }

        itemProperties.SocketBonus = fields[120].GetUInt32();
        itemProperties.GemProperties = fields[121].GetUInt32();
        itemProperties.DisenchantReqSkill = fields[122].GetInt32();
        itemProperties.ArmorDamageModifier = fields[123].GetUInt32();
        itemProperties.ExistingDuration = fields[124].GetUInt32();
        itemProperties.ItemLimitCategory = fields[125].GetUInt32();
        itemProperties.HolidayId = fields[126].GetUInt32();
        itemProperties.FoodType = fields[127].GetUInt32();

        //lowercase
        std::string lower_case_name = itemProperties.Name;
        AscEmu::Util::Strings::toLowerCase(lower_case_name);
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


    sLogger.info("MySQLDataLoads : Loaded %u item_properties in %u ms!", item_count, static_cast<uint32_t>(Util::GetTimeDifferenceToNow(startTime)));
}

ItemProperties const* MySQLDataStore::getItemProperties(uint32_t entry)
{
    ItemPropertiesContainer::const_iterator itr = _itemPropertiesStore.find(entry);
    if (itr != _itemPropertiesStore.end())
        return &(itr->second);

    return nullptr;
}

//\ brief: On versions lower than wotlk our db includes the item entry instead of the displayid.
//         In wotlk and newer the database includes the displayid since no more additional data is required for creature equipment.
// Actually this is item entry id on all versions but wotlk and newer clients can get the display id by themselves from entry id - Appled
uint32_t const MySQLDataStore::getItemDisplayIdForEntry(uint32_t entry)
{
    if (entry != 0)
    {
#if VERSION_STRING == TBC
        // get display id for equipped item entry
        uint32_t dbcDisplay = 0;
        uint32_t mysqlDisplay = 0;

        if (const auto ItemDBC = sItemStore.LookupEntry(entry))
            dbcDisplay = ItemDBC->DisplayId;

        if (const auto itemProperties = getItemProperties(entry))
            mysqlDisplay = itemProperties->DisplayInfoID;

        if (mysqlDisplay != 0 && mysqlDisplay != dbcDisplay)
        {
            if (const auto itemDisplayInfo = sItemDisplayInfoStore.LookupEntry(mysqlDisplay))
                return mysqlDisplay;
        }

        if (dbcDisplay != 0)
        {
            sLogger.debug("Item entry %u not available in item_properties or has an invalid displayId! Using dbcDisplayId %u!", entry, dbcDisplay);
            return dbcDisplay;
        }

        sLogger.debug("Invalid item entry %u is not in item_properties table or in Item.dbc! Please create a item_properties entry to return a valid displayId", entry);
#else
        return entry;
#endif
    }

    return 0;
}

void MySQLDataStore::loadCreaturePropertiesTable()
{
    auto startTime = Util::TimeNow();
    uint32_t creature_properties_count = 0;

    //                                                                    0          1           2             3                 4               5                  6
    QueryResult* creature_properties_result = getWorldDBQuery("SELECT entry, killcredit1, killcredit2, male_displayid, female_displayid, male_displayid2, female_displayid2, "
        //7      8         9         10       11     12     13       14            15              16           17
        "name, subname, info_str, type_flags, type, family, `rank`, encounter, base_attack_mod, range_attack_mod, leader, "
        //  18        19        20        21         22      23     24      25          26           27
        "minlevel, maxlevel, faction, minhealth, maxhealth, mana, scale, npcflags, attacktime, attack_school, "
        //   28          29         30            31                 32                33            34        35
        "mindamage, maxdamage, can_ranged, rangedattacktime, rangedmindamage, rangedmaxdamage, respawntime, armor, "
        //   36           37           38            39          40           41            42             43
        "resistance1, resistance2, resistance3, resistance4, resistance5, resistance6, combat_reach, bounding_radius, "
        // 44    45     46         47          48         49        50          51            52     53      54
        "auras, boss, money, isTriggerNpc, walk_speed, run_speed, fly_speed, extra_a9_flags, spell1, spell2, spell3, "
        // 55      56      57      58      59        60           61               62            63         64           65
        "spell4, spell5, spell6, spell7, spell8, spell_flags, modImmunities, isTrainingDummy, guardtype, summonguard, spelldataid, "
        //  66         67        68          69          70          71          72          73         74         75
        "vehicleid, rooted, questitem1, questitem2, questitem3, questitem4, questitem5, questitem6, waypointid, gossipId FROM creature_properties base "
        //
        "WHERE build=(SELECT MAX(build) FROM creature_properties buildspecific WHERE base.entry = buildspecific.entry AND build <= %u)", VERSION_STRING);

    if (creature_properties_result == nullptr)
    {
        sLogger.info("MySQLDataLoads : Table `creature_properties` is empty!");
        return;
    }

    sLogger.info("MySQLDataLoads : Table creature_properties has %u columns", creature_properties_result->GetFieldCount());

    _creaturePropertiesStore.rehash(creature_properties_result->GetRowCount());

    do
    {
        Field* fields = creature_properties_result->Fetch();

        uint32_t entry = fields[0].GetUInt32();

        CreatureProperties& creatureProperties = _creaturePropertiesStore[entry];

        creatureProperties.Id = entry;
        creatureProperties.killcredit[0] = fields[1].GetUInt32();
        creatureProperties.killcredit[1] = fields[2].GetUInt32();
        creatureProperties.Male_DisplayID = fields[3].GetUInt32();
        if (creatureProperties.Male_DisplayID != 0)
        {
            const auto* creature_display = sObjectMgr.getCreatureDisplayInfoData(creatureProperties.Male_DisplayID);
            if (creature_display == nullptr)
            {
                sLogger.debugFlag(AscEmu::Logging::LF_DB_TABLES, "Table creature_properties includes invalid Male_DisplayID %u for npc entry: %u. Set to 0!", creatureProperties.Male_DisplayID, entry);
                creatureProperties.Male_DisplayID = 0;
            }
        }
        creatureProperties.Female_DisplayID = fields[4].GetUInt32();
        if (creatureProperties.Female_DisplayID != 0)
        {
            const auto* creature_display = sObjectMgr.getCreatureDisplayInfoData(creatureProperties.Female_DisplayID);
            if (creature_display == nullptr)
            {
                sLogger.debugFlag(AscEmu::Logging::LF_DB_TABLES, "Table creature_properties includes invalid Female_DisplayID %u for npc entry: %u. Set to 0!", creatureProperties.Female_DisplayID, entry);
                creatureProperties.Female_DisplayID = 0;
            }
        }
        creatureProperties.Male_DisplayID2 = fields[5].GetUInt32();
        if (creatureProperties.Male_DisplayID2 != 0)
        {
            const auto* creature_display = sObjectMgr.getCreatureDisplayInfoData(creatureProperties.Male_DisplayID2);
            if (creature_display == nullptr)
            {
                sLogger.debugFlag(AscEmu::Logging::LF_DB_TABLES, "Table creature_properties includes invalid Male_DisplayID2 %u for npc entry: %u. Set to 0!", creatureProperties.Male_DisplayID2, entry);
                creatureProperties.Male_DisplayID2 = 0;
            }
        }
        creatureProperties.Female_DisplayID2 = fields[6].GetUInt32();
        if (creatureProperties.Female_DisplayID2 != 0)
        {
            const auto* creature_display = sObjectMgr.getCreatureDisplayInfoData(creatureProperties.Female_DisplayID2);
            if (creature_display == nullptr)
            {
                sLogger.debugFlag(AscEmu::Logging::LF_DB_TABLES, "Table creature_properties includes invalid Female_DisplayID2 %u for npc entry: %u. Set to 0!", creatureProperties.Female_DisplayID2, entry);
                creatureProperties.Female_DisplayID2 = 0;
            }
        }

        creatureProperties.Name = fields[7].GetString();

        //lowercase
        std::string lower_case_name = creatureProperties.Name;
        AscEmu::Util::Strings::toLowerCase(lower_case_name);
        creatureProperties.lowercase_name = lower_case_name;

        creatureProperties.SubName = fields[8].GetString();
        creatureProperties.info_str = fields[9].GetString();
        creatureProperties.typeFlags = fields[10].GetUInt32();
        creatureProperties.Type = fields[11].GetUInt32();
        creatureProperties.Family = fields[12].GetUInt32();
        creatureProperties.Rank = fields[13].GetUInt32();
        creatureProperties.Encounter = fields[14].GetUInt32();
        creatureProperties.baseAttackMod = fields[15].GetFloat();
        creatureProperties.rangeAttackMod = fields[16].GetFloat();
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
            sLogger.failure("Table `creature_properties` MinHealth = 0 is not a valid value! Default set to 1 for entry: %u.", entry);
            creatureProperties.MinHealth = 1;
        }

        if (fields[22].GetUInt32() != 0)
        {
            creatureProperties.MaxHealth = fields[22].GetUInt32();
        }
        else
        {
            sLogger.failure("Table `creature_properties` MaxHealth = 0 is not a valid value! Default set to 1 for entry: %u.", entry);
            creatureProperties.MaxHealth = 1;
        }

        creatureProperties.Mana = fields[23].GetUInt32();
        creatureProperties.Scale = fields[24].GetFloat();
        creatureProperties.NPCFLags = fields[25].GetUInt32();
        creatureProperties.AttackTime = fields[26].GetUInt32();
        if (fields[27].GetUInt8() <= SCHOOL_ARCANE)
        {
            creatureProperties.attackSchool = fields[27].GetUInt8();
        }
        else
        {
            sLogger.failure("Table `creature_properties` AttackType: %u is not a valid value! Default set to 0 for entry: %u.", fields[10].GetUInt32(), entry);
            creatureProperties.attackSchool = SCHOOL_NORMAL;
        }

        creatureProperties.MinDamage = fields[28].GetFloat();
        creatureProperties.MaxDamage = fields[29].GetFloat();
        creatureProperties.CanRanged = fields[30].GetUInt32();
        creatureProperties.RangedAttackTime = fields[31].GetUInt32();
        creatureProperties.RangedMinDamage = fields[32].GetFloat();
        creatureProperties.RangedMaxDamage = fields[33].GetFloat();
        creatureProperties.RespawnTime = fields[34].GetUInt32();
        for (uint8_t i = 0; i < TOTAL_SPELL_SCHOOLS; ++i)
        {
            creatureProperties.Resistances[i] = fields[35 + i].GetUInt32();
        }

        creatureProperties.CombatReach = fields[42].GetFloat();
        creatureProperties.BoundingRadius = fields[43].GetFloat();
        creatureProperties.aura_string = fields[44].GetString();
        creatureProperties.isBoss = fields[45].GetBool();
        creatureProperties.money = fields[46].GetUInt32();
        creatureProperties.isTriggerNpc = fields[47].GetBool();
        creatureProperties.walk_speed = fields[48].GetFloat();
        creatureProperties.run_speed = fields[49].GetFloat();
        creatureProperties.fly_speed = fields[50].GetFloat();
        creatureProperties.extra_a9_flags = fields[51].GetUInt32();

        for (uint8_t i = 0; i < creatureMaxProtoSpells; ++i)
        {
            // Process spell fields
            creatureProperties.AISpells[i] = fields[52 + i].GetUInt32();
            if (creatureProperties.AISpells[i] != 0)
            {
                SpellInfo const* sp = sSpellMgr.getSpellInfo(creatureProperties.AISpells[i]);
                if (sp == nullptr)
                {
                    uint8_t spell_number = i;
                    sLogger.debugFlag(AscEmu::Logging::LF_DB_TABLES, "spell %u in table creature_properties column spell%u for creature entry: %u is not a valid spell!", creatureProperties.AISpells[i], spell_number + 1, entry);
                    continue;
                }
                else
                {
                    if ((sp->getAttributes() & ATTRIBUTES_PASSIVE) == 0)
                        creatureProperties.castable_spells.push_back(sp->getId());
                    else
                        creatureProperties.start_auras.insert(sp->getId());
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
            for (uint8_t i = 0; i < 3; i++)
            {
                if (creature_spell_data == nullptr)
                    continue;

                if (creature_spell_data->Spells[i] == 0)
                    continue;

                SpellInfo const* sp = sSpellMgr.getSpellInfo(creature_spell_data->Spells[i]);
                if (sp == nullptr)
                    continue;

                if ((sp->getAttributes() & ATTRIBUTES_PASSIVE) == 0)
                    creatureProperties.castable_spells.push_back(sp->getId());
                else
                    creatureProperties.start_auras.insert(sp->getId());
            }
        }

        creatureProperties.vehicleid = fields[66].GetUInt32();
        creatureProperties.rooted = fields[67].GetBool();

        for (uint8_t i = 0; i < 6; ++i)
            creatureProperties.QuestItems[i] = fields[68 + i].GetUInt32();

        creatureProperties.waypointid = fields[74].GetUInt32();

        creatureProperties.gossipId = fields[75].GetUInt32();

        auto movement = getCreaturePropertiesMovement(entry);
        if (movement)
        {
            creatureProperties.MovementType = movement->MovementType;
            creatureProperties.Movement.Ground = movement->Movement.Ground;
            creatureProperties.Movement.Swim = movement->Movement.Swim;
            creatureProperties.Movement.Flight = movement->Movement.Flight;
            creatureProperties.Movement.Rooted = movement->Movement.Rooted;
            creatureProperties.Movement.Chase = movement->Movement.Chase;
            creatureProperties.Movement.Random = movement->Movement.Random;
        }
        else
        {
            creatureProperties.MovementType = IDLE_MOTION_TYPE;
            creatureProperties.Movement.Ground = static_cast<CreatureGroundMovementType>(1);
            creatureProperties.Movement.Swim = false;
            creatureProperties.Movement.Flight = static_cast<CreatureFlightMovementType>(0);
            creatureProperties.Movement.Rooted = false;
            creatureProperties.Movement.Chase = static_cast<CreatureChaseMovementType>(0);
            creatureProperties.Movement.Random = static_cast<CreatureRandomMovementType>(0);
        }

        //process aura string
        if (creatureProperties.aura_string.size() != 0)
        {
            std::string auras = creatureProperties.aura_string;
            std::vector<std::string> split_auras = AscEmu::Util::Strings::split(auras, " ");
            for (std::vector<std::string>::iterator it = split_auras.begin(); it != split_auras.end(); ++it)
            {
                uint32_t id = atol((*it).c_str());
                if (id)
                    creatureProperties.start_auras.insert(id);
            }
        }

        //Itemslot
        creatureProperties.itemslot_1 = 0;
        creatureProperties.itemslot_2 = 0;
        creatureProperties.itemslot_3 = 0;

        /*for (uint8_t i = 0; i < NUM_MONSTER_SAY_EVENTS; ++i)
        {
            creatureProperties.MonsterSay[i] = nullptr;
        }*/

        ++creature_properties_count;
    } while (creature_properties_result->NextRow());

    delete creature_properties_result;

    sLogger.info("MySQLDataLoads : Loaded %u creature proto data in %u ms!", creature_properties_count, static_cast<uint32_t>(Util::GetTimeDifferenceToNow(startTime)));
}

void MySQLDataStore::loadCreaturePropertiesMovementTable()
{
    auto startTime = Util::TimeNow();
    uint32_t creature_properties_movement_count = 0;

    //                                                                      0          1           2             3                 4               5                  6
    QueryResult* creature_properties_movement_result = WorldDatabase.Query("SELECT CreatureId, Ground, Swim, Flight, Rooted, Chase, Random, InteractionPauseTimer FROM creature_properties_movement");

    if (creature_properties_movement_result == nullptr)
    {
        sLogger.info("MySQLDataLoads : Table creature_properties_movement is empty!");
        return;
    }

    uint32_t row_count = 0;   
    row_count = static_cast<uint32_t>(_creaturePropertiesMovementStore.size());

    sLogger.info("MySQLDataLoads : Table creature_properties_movement has %u columns", creature_properties_movement_result->GetFieldCount());

    _creaturePropertiesMovementStore.rehash(row_count + creature_properties_movement_result->GetRowCount());
    do
    {
        Field* fields = creature_properties_movement_result->Fetch();

        uint32_t entry = fields[0].GetUInt32();

        CreaturePropertiesMovement& creaturePropertiesMovement = _creaturePropertiesMovementStore[entry];

        creaturePropertiesMovement.Id = entry;
        creaturePropertiesMovement.MovementType = IDLE_MOTION_TYPE;
        creaturePropertiesMovement.Movement.Ground = static_cast<CreatureGroundMovementType>(fields[1].GetUInt8());
        creaturePropertiesMovement.Movement.Swim = fields[2].GetBool();
        creaturePropertiesMovement.Movement.Flight = static_cast<CreatureFlightMovementType>(fields[3].GetUInt8());
        creaturePropertiesMovement.Movement.Rooted = fields[4].GetBool();
        creaturePropertiesMovement.Movement.Chase = static_cast<CreatureChaseMovementType>(fields[5].GetUInt8());
        creaturePropertiesMovement.Movement.Random = static_cast<CreatureRandomMovementType>(fields[6].GetUInt8());

        ++creature_properties_movement_count;
        } while (creature_properties_movement_result->NextRow());

    sLogger.info("MySQLDataLoads : Loaded %u creature movement data in %u ms!", creature_properties_movement_count, static_cast<uint32_t>(Util::GetTimeDifferenceToNow(startTime)));
}

CreatureProperties const* MySQLDataStore::getCreatureProperties(uint32_t entry)
{
    CreaturePropertiesContainer::const_iterator itr = _creaturePropertiesStore.find(entry);
    if (itr != _creaturePropertiesStore.end())
        return &(itr->second);

    return nullptr;
}

CreaturePropertiesMovement const* MySQLDataStore::getCreaturePropertiesMovement(uint32_t entry)
{
    CreaturePropertiesMovementContainer::const_iterator itr = _creaturePropertiesMovementStore.find(entry);
    if (itr != _creaturePropertiesMovementStore.end())
        return &(itr->second);

    return nullptr;
}

void MySQLDataStore::loadGameObjectPropertiesTable()
{
    auto startTime = Util::TimeNow();
    uint32_t gameobject_properties_count = 0;

    //                                                                                  0      1        2        3         4              5          6          7            8             9
    QueryResult* gameobject_properties_result = sMySQLStore.getWorldDBQuery("SELECT entry, type, display_id, name, category_name, cast_bar_text, UnkStr, parameter_0, parameter_1, parameter_2, "
        //     10           11          12           13           14            15           16           17           18
        "parameter_3, parameter_4, parameter_5, parameter_6, parameter_7, parameter_8, parameter_9, parameter_10, parameter_11, "
        //     19            20            21            22           23            24            25            26
        "parameter_12, parameter_13, parameter_14, parameter_15, parameter_16, parameter_17, parameter_18, parameter_19, "
        //     27            28            29            30        31        32          33          34         35
        "parameter_20, parameter_21, parameter_22, parameter_23, size, QuestItem1, QuestItem2, QuestItem3, QuestItem4, "
        //     36          37
        "QuestItem5, QuestItem6 FROM gameobject_properties base "
        "WHERE build=(SELECT MAX(build) FROM gameobject_properties buildspecific WHERE base.entry = buildspecific.entry AND build <= %u)", VERSION_STRING);

    if (gameobject_properties_result == nullptr)
    {
        sLogger.info("MySQLDataLoads : Table `gameobject_properties` is empty!");
        return;
    }

    sLogger.info("MySQLDataLoads : Table `gameobject_properties` has %u columns", gameobject_properties_result->GetFieldCount());

    _gameobjectPropertiesStore.rehash(gameobject_properties_result->GetRowCount());

    do
    {
        Field* fields = gameobject_properties_result->Fetch();

        uint32_t entry = fields[0].GetUInt32();

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

        for (uint8_t i = 0; i < 6; ++i)
        {
            uint32_t quest_item_entry = fields[32 + i].GetUInt32();
            if (quest_item_entry != 0)
            {
                auto quest_item_proto = getItemProperties(quest_item_entry);
                if (quest_item_proto == nullptr)
                {
                    sLogger.failure("Table `gameobject_properties` questitem%u : %u is not a valid item! Default set to 0 for entry: %u.", i, quest_item_entry, entry);
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

    sLogger.info("MySQLDataLoads : Loaded %u gameobject data in %u ms!", gameobject_properties_count, static_cast<uint32_t>(Util::GetTimeDifferenceToNow(startTime)));
}

GameObjectProperties const* MySQLDataStore::getGameObjectProperties(uint32_t entry)
{
    GameObjectPropertiesContainer::const_iterator itr = _gameobjectPropertiesStore.find(entry);
    if (itr != _gameobjectPropertiesStore.end())
        return &(itr->second);

    return nullptr;
}

//quests
void MySQLDataStore::loadQuestPropertiesTable()
{
    auto startTime = Util::TimeNow();
    uint32_t quest_count = 0;


              //                                                        0       1     2      3       4          5        6          7              8                 9
    QueryResult* quest_result = sMySQLStore.getWorldDBQuery("SELECT entry, ZoneId, sort, flags, MinLevel, questlevel, Type, RequiredRaces, RequiredClass, RequiredTradeskill, "
        //           10                    11                 12             13          14            15           16         17
        "RequiredTradeskillValue, RequiredRepFaction, RequiredRepValue, LimitTime, SpecialFlags, PrevQuestId, NextQuestId, srcItem, "
        //     18        19     20         21            22              23          24          25               26
        "SrcItemCount, Title, Details, Objectives, CompletionText, IncompleteText, EndText, ObjectiveText1, ObjectiveText2, "
        //     27               28           29          30           31          32         33           34         35
        "ObjectiveText3, ObjectiveText4, ReqItemId1, ReqItemId2, ReqItemId3, ReqItemId4, ReqItemId5, ReqItemId6, ReqItemCount1, "
        //     36             37            38              39             40              41                 42
        "ReqItemCount2, ReqItemCount3, ReqItemCount4, ReqItemCount5, ReqItemCount6, ReqKillMobOrGOId1, ReqKillMobOrGOId2, "
        //     43                   44                    45                  46                      47                  48
        "ReqKillMobOrGOId3, ReqKillMobOrGOId4, ReqKillMobOrGOCount1, ReqKillMobOrGOCount2, ReqKillMobOrGOCount3, ReqKillMobOrGOCount4, "
        //     49                 50              51              52              53           54           55           56
        "ReqCastSpellId1, ReqCastSpellId2, ReqCastSpellId3, ReqCastSpellId4, ReqEmoteId1, ReqEmoteId2, ReqEmoteId3, ReqEmoteId4, "
        //     57                  58                59               60                61                 62                63
        "RewChoiceItemId1, RewChoiceItemId2, RewChoiceItemId3, RewChoiceItemId4, RewChoiceItemId5, RewChoiceItemId6, RewChoiceItemCount1, "
        //     64                   65                  66                   67                   68              69          70
        "RewChoiceItemCount2, RewChoiceItemCount3, RewChoiceItemCount4, RewChoiceItemCount5, RewChoiceItemCount6, RewItemId1, RewItemId2, "
        //     71          72           73              74            75             76              77             78             79
        "RewItemId3, RewItemId4, RewItemCount1, RewItemCount2, RewItemCount3, RewItemCount4, RewRepFaction1, RewRepFaction2, RewRepFaction3, "
        //     80               81               82            83           84             85          86              87            88
        "RewRepFaction4, RewRepFaction5, RewRepFaction6, RewRepValue1, RewRepValue2, RewRepValue3, RewRepValue4, RewRepValue5, RewRepValue6, "
        //     89         90       91       92       93            94             95             96           97        98      99      100
        "RewRepLimit, RewMoney, RewXP, RewSpell, CastSpell, MailTemplateId, MailDelaySecs, MailSendItem, PointMapId, PointX, PointY, PointOpt, "
        //      101                  102             103             104              105              106                107
        "RewardMoneyAtMaxLevel, ExploreTrigger1, ExploreTrigger2, ExploreTrigger3, ExploreTrigger4, RequiredOneOfQuest, RequiredQuest1, "
        //     108              109            110             111           112             113            114              115
        "RequiredQuest2, RequiredQuest3, RequiredQuest4, RemoveQuests, ReceiveItemId1, ReceiveItemId2, ReceiveItemId3, ReceiveItemId4, "
        //     116                117                  118               119             120          121            122             123
        "ReceiveItemCount1, ReceiveItemCount2, ReceiveItemCount3, ReceiveItemCount4, IsRepeatable, bonushonor, bonusarenapoints, rewardtitleid, "
        //     124              125               126             127           128           129           130            131
        "rewardtalents, suggestedplayers, detailemotecount, detailemote1, detailemote2, detailemote3, detailemote4, detailemotedelay1, "
        //     132                133                134                135                136               137               138
        "detailemotedelay2, detailemotedelay3, detailemotedelay4, completionemotecnt, completionemote1, completionemote2, completionemote3, "
        //     139                 140                     141                   142                    143                 144
        "completionemote4, completionemotedelay1, completionemotedelay2, completionemotedelay3, completionemotedelay4, completeemote, "
        //      145                   146              147
        "incompleteemote, iscompletedbyspelleffect, RewXPId FROM quest_properties base "
        "WHERE build=(SELECT MAX(build) FROM quest_properties buildspecific WHERE base.entry = buildspecific.entry AND build <= %u)", VERSION_STRING);

    if (quest_result == nullptr)
    {
        sLogger.info("MySQLDataLoads : Table `quest_properties` is empty!");
        return;
    }

    uint32_t row_count = 0;

    sLogger.info("MySQLDataLoads : Table `quest_properties` has %u columns", quest_result->GetFieldCount());

    _questPropertiesStore.rehash(row_count + quest_result->GetRowCount());

    do
    {
        Field* fields = quest_result->Fetch();

        uint32_t entry = fields[0].GetUInt32();

        QuestProperties& questInfo = _questPropertiesStore[entry];

        questInfo.id = entry;
        questInfo.zone_id = fields[1].GetUInt32();
        questInfo.quest_sort = fields[2].GetUInt32();
        questInfo.quest_flags = fields[3].GetUInt32();
        questInfo.min_level = fields[4].GetUInt32();
        questInfo.questlevel = fields[5].GetInt32();
        questInfo.type = fields[6].GetUInt32();
        questInfo.required_races = fields[7].GetUInt32();
        questInfo.required_class = fields[8].GetUInt32();
        questInfo.required_tradeskill = fields[9].GetUInt16();
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

        for (uint8_t i = 0; i < 4; ++i)
        {
            questInfo.objectivetexts[i] = fields[25 + i].GetString();
        }

        for (uint8_t i = 0; i < MAX_REQUIRED_QUEST_ITEM; ++i)
        {
            questInfo.required_item[i] = fields[29 + i].GetUInt32();
            questInfo.required_itemcount[i] = fields[35 + i].GetUInt32();
        }

        for (uint8_t i = 0; i < 4; ++i)
        {
            questInfo.required_mob_or_go[i] = fields[41 + i].GetInt32();
            if (questInfo.required_mob_or_go[i] != 0)
            {
                if (questInfo.required_mob_or_go[i] > 0)
                {
                    if (!getCreatureProperties(questInfo.required_mob_or_go[i]))
                    {
                        sLogger.debugFlag(AscEmu::Logging::LF_DB_TABLES, "Quest %u has `ReqCreatureOrGOId%d` = %i but creature with entry %u does not exist in creature_properties table!",
                            entry, i, questInfo.required_mob_or_go[i], questInfo.required_mob_or_go[i]);
                    }
                }
                else
                {
                    if (!getGameObjectProperties(-questInfo.required_mob_or_go[i]))
                    {
                        sLogger.debugFlag(AscEmu::Logging::LF_DB_TABLES, "Quest %u has `ReqCreatureOrGOId%d` = %i but gameobject %u does not exist in gameobject_properties table!",
                            entry, i, questInfo.required_mob_or_go[i], -questInfo.required_mob_or_go[i]);
                    }
                }
            }

            questInfo.required_mob_or_go_count[i] = fields[45 + i].GetUInt32();
        }

        for (uint8_t i = 0; i < 4; ++i)
        {
            questInfo.required_spell[i] = fields[49 + i].GetUInt32();
            questInfo.required_emote[i] = fields[53 + i].GetUInt32();
        }

        for (uint8_t i = 0; i < 6; ++i)
        {
            questInfo.reward_choiceitem[i] = fields[57 + i].GetUInt32();
            questInfo.reward_choiceitemcount[i] = fields[63 + i].GetUInt32();
        }

        for (uint8_t i = 0; i < 4; ++i)
        {
            questInfo.reward_item[i] = fields[69 + i].GetUInt32();
            questInfo.reward_itemcount[i] = fields[73 + i].GetUInt32();
        }

        for (uint8_t i = 0; i < 6; ++i)
        {
            questInfo.reward_repfaction[i] = fields[77 + i].GetUInt32();
            questInfo.reward_repvalue[i] = fields[83 + i].GetInt32();
        }

        questInfo.reward_replimit = fields[89].GetUInt32();

        questInfo.reward_money = fields[90].GetInt32();
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

        for (uint8_t i = 0; i < 4; ++i)
        {
            questInfo.required_triggers[i] = fields[102 + i].GetUInt32();
        }

        questInfo.x_or_y_quest_string = fields[106].GetString();

        for (uint8_t i = 0; i < 4; ++i)
        {
            questInfo.required_quests[i] = fields[107 + i].GetUInt32();
        }

        questInfo.remove_quests = fields[111].GetString();

        for (uint8_t i = 0; i < 4; ++i)
        {
            questInfo.receive_items[i] = fields[112 + i].GetUInt32();
            questInfo.receive_itemcount[i] = fields[116 + i].GetUInt32();
        }

        questInfo.is_repeatable = fields[120].GetInt32();
        questInfo.bonushonor = fields[121].GetUInt32();
        questInfo.bonusarenapoints = fields[122].GetUInt32();
        questInfo.rewardtitleid = fields[123].GetUInt32();
        questInfo.rewardtalents = fields[124].GetUInt32();
        questInfo.suggestedplayers = fields[125].GetUInt32();

        // emotes
        questInfo.detailemotecount = fields[126].GetUInt32();

        for (uint8_t i = 0; i < 4; ++i)
        {
            questInfo.detailemote[i] = fields[127 + i].GetUInt32();
            questInfo.detailemotedelay[i] = fields[131 + i].GetUInt32();
        }

        questInfo.completionemotecount = fields[135].GetUInt32();

        for (uint8_t i = 0; i < 4; ++i)
        {
            questInfo.completionemote[i] = fields[136 + i].GetUInt32();
            questInfo.completionemotedelay[i] = fields[140 + i].GetUInt32();
        }

        questInfo.completeemote = fields[144].GetUInt32();
        questInfo.incompleteemote = fields[145].GetUInt32();
        questInfo.iscompletedbyspelleffect = fields[146].GetUInt32();
        questInfo.RewXPId = fields[147].GetUInt32();

        ++quest_count;
    } while (quest_result->NextRow());

    delete quest_result;

    sLogger.info("MySQLDataLoads : Loaded %u quest_properties data in %u ms!", quest_count, static_cast<uint32_t>(Util::GetTimeDifferenceToNow(startTime)));
}

QuestProperties const* MySQLDataStore::getQuestProperties(uint32_t entry)
{
    QuestPropertiesContainer::const_iterator itr = _questPropertiesStore.find(entry);
    if (itr != _questPropertiesStore.end())
        return &(itr->second);

    return nullptr;
}

void MySQLDataStore::loadGameObjectQuestItemBindingTable()
{
    auto startTime = Util::TimeNow();

    //                                                                        0      1     2        3
    QueryResult* gameobject_quest_item_result = WorldDatabase.Query("SELECT entry, quest, item, item_count FROM gameobject_quest_item_binding");

    uint32_t gameobject_quest_item_count = 0;

    if (gameobject_quest_item_result != nullptr)
    {
        do
        {
            Field* fields = gameobject_quest_item_result->Fetch();
            uint32_t entry = fields[0].GetUInt32();

            GameObjectProperties const* gameobject_properties = sMySQLStore.getGameObjectProperties(entry);
            if (gameobject_properties == nullptr)
            {
                sLogger.debug("Table `gameobject_quest_item_binding` includes data for invalid gameobject_properties entry: %u. Skipped!", entry);
                continue;
            }

            uint32_t quest_entry = fields[1].GetUInt32();
            QuestProperties const* quest = sMySQLStore.getQuestProperties(quest_entry);
            if (quest == nullptr)
            {
                sLogger.debug("Table `gameobject_quest_item_binding` includes data for invalid quest_properties : %u. Skipped!", quest_entry);
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

    sLogger.info("MySQLDataLoads : Loaded %u data from `gameobject_quest_item_binding` table in %u ms!", gameobject_quest_item_count, static_cast<uint32_t>(Util::GetTimeDifferenceToNow(startTime)));
}

void MySQLDataStore::loadGameObjectQuestPickupBindingTable()
{
    auto startTime = Util::TimeNow();

    //                                                                          0      1           2
    QueryResult* gameobject_quest_pickup_result = WorldDatabase.Query("SELECT entry, quest, required_count FROM gameobject_quest_pickup_binding");

    uint32_t gameobject_quest_pickup_count = 0;

    if (gameobject_quest_pickup_result != nullptr)
    {
        do
        {
            Field* fields = gameobject_quest_pickup_result->Fetch();
            uint32_t entry = fields[0].GetUInt32();

            GameObjectProperties const* gameobject_properties = sMySQLStore.getGameObjectProperties(entry);
            if (gameobject_properties == nullptr)
            {
                sLogger.debug("Table `gameobject_quest_pickup_binding` includes data for invalid gameobject_properties entry: %u. Skipped!", entry);
                continue;
            }

            uint32_t quest_entry = fields[1].GetUInt32();
            QuestProperties const* quest = sMySQLStore.getQuestProperties(quest_entry);
            if (quest == nullptr)
            {
                sLogger.debug("Table `gameobject_quest_pickup_binding` includes data for invalid quest_properties : %u. Skipped!", quest_entry);
                continue;
            }
            else
            {
                uint32_t required_count = fields[2].GetUInt32();
                const_cast<GameObjectProperties*>(gameobject_properties)->goMap.insert(std::make_pair(quest, required_count));
            }


            ++gameobject_quest_pickup_count;
        } while (gameobject_quest_pickup_result->NextRow());

        delete gameobject_quest_pickup_result;
    }

    sLogger.info("MySQLDataLoads : Loaded %u data from `gameobject_quest_pickup_binding` table in %u ms!", gameobject_quest_pickup_count, static_cast<uint32_t>(Util::GetTimeDifferenceToNow(startTime)));
}

void MySQLDataStore::loadCreatureDifficultyTable()
{
    auto startTime = Util::TimeNow();

    //                                                                         0          1            2             3
    QueryResult* creature_difficulty_result = WorldDatabase.Query("SELECT entry, difficulty_1, difficulty_2, difficulty_3 FROM creature_difficulty");

    if (creature_difficulty_result == nullptr)
    {
        sLogger.info("MySQLDataLoads : Table `creature_difficulty` is empty!");
        return;
    }

    sLogger.info("MySQLDataLoads : Table `creature_difficulty` has %u columns", creature_difficulty_result->GetFieldCount());

    _creatureDifficultyStore.rehash(creature_difficulty_result->GetRowCount());

    uint32_t creature_difficulty_count = 0;
    do
    {
        Field* fields = creature_difficulty_result->Fetch();

        uint32_t entry = fields[0].GetUInt32();

        MySQLStructure::CreatureDifficulty& creatureDifficulty = _creatureDifficultyStore[entry];

        creatureDifficulty.id = entry;

        creatureDifficulty.difficultyEntry1 = fields[1].GetUInt32();
        creatureDifficulty.difficultyEntry2 = fields[2].GetUInt32();
        creatureDifficulty.difficultyEntry3 = fields[3].GetUInt32();


        ++creature_difficulty_count;
    } while (creature_difficulty_result->NextRow());

    delete creature_difficulty_result;

    sLogger.info("MySQLDataLoads : Loaded %u creature difficulties info from `creature_difficulty` table in %u ms!", creature_difficulty_count, static_cast<uint32_t>(Util::GetTimeDifferenceToNow(startTime)));
}

uint32_t MySQLDataStore::getCreatureDifficulty(uint32_t entry, uint8_t difficulty_type)
{
    for (auto itr = _creatureDifficultyStore.begin(); itr != _creatureDifficultyStore.end(); ++itr)
    {
        switch (difficulty_type)
        {
            case 1:
            {
                if (itr->first == entry && itr->second.difficultyEntry1 != 0)
                    return itr->second.difficultyEntry1;
            }
            break;
            case 2:
            {
                if (itr->first == entry && itr->second.difficultyEntry2 != 0)
                    return itr->second.difficultyEntry2;
            }
            break;
            case 3:
            {
                if (itr->first == entry && itr->second.difficultyEntry3 != 0)
                    return itr->second.difficultyEntry3;
            }
            break;
            default:
                return 0;
        }
    }
    return 0;
}

void MySQLDataStore::loadDisplayBoundingBoxesTable()
{
    auto startTime = Util::TimeNow();

    //                                                                            0       1    2     3      4      5      6         7
    //QueryResult* display_bounding_boxes_result = WorldDatabase.Query("SELECT displayid, lowx, lowy, lowz, highx, highy, highz, boundradius FROM display_bounding_boxes");
    QueryResult* display_bounding_boxes_result = WorldDatabase.Query("SELECT displayid, highz FROM display_bounding_boxes");

    if (display_bounding_boxes_result == nullptr)
    {
        sLogger.info("MySQLDataLoads : Table `display_bounding_boxes` is empty!");
        return;
    }

    sLogger.info("MySQLDataLoads : Table `display_bounding_boxes` has %u columns", display_bounding_boxes_result->GetFieldCount());

    _displayBoundingBoxesStore.rehash(display_bounding_boxes_result->GetRowCount());

    uint32_t display_bounding_boxes_count = 0;
    do
    {
        Field* fields = display_bounding_boxes_result->Fetch();

        uint32_t entry = fields[0].GetUInt32();

        MySQLStructure::DisplayBoundingBoxes& displayBounding = _displayBoundingBoxesStore[entry];

        displayBounding.displayid = entry;

        //for (uint8_t i = 0; i < 3; i++)
        //{
        //    displayBounding.low[i] = fields[1 + i].GetFloat();
        //    displayBounding.high[i] = fields[4 + i].GetFloat();
        //}

        //displayBounding.boundradius = fields[7].GetFloat();

        // highz is the only value used in Unit::EventModelChange()
        displayBounding.high[2] = fields[1].GetFloat();


        ++display_bounding_boxes_count;
    } while (display_bounding_boxes_result->NextRow());

    delete display_bounding_boxes_result;

    sLogger.info("MySQLDataLoads : Loaded %u display bounding info from `display_bounding_boxes` table in %u ms!", display_bounding_boxes_count, static_cast<uint32_t>(Util::GetTimeDifferenceToNow(startTime)));
}

MySQLStructure::DisplayBoundingBoxes const* MySQLDataStore::getDisplayBounding(uint32_t entry)
{
    DisplayBoundingBoxesContainer::const_iterator itr = _displayBoundingBoxesStore.find(entry);
    if (itr != _displayBoundingBoxesStore.end())
        return &(itr->second);

    return nullptr;
}

void MySQLDataStore::loadVendorRestrictionsTable()
{
    auto startTime = Util::TimeNow();

    //                                                                      0       1          2            3              4
    QueryResult* vendor_restricitons_result = WorldDatabase.Query("SELECT entry, racemask, classmask, reqrepfaction, reqrepfactionvalue, "
    //                                                                    5                 6           7
                                                                  "canbuyattextid, cannotbuyattextid, flags FROM vendor_restrictions");

    if (vendor_restricitons_result == nullptr)
    {
        sLogger.info("MySQLDataLoads : Table `vendor_restrictions` is empty!");
        return;
    }

    sLogger.info("MySQLDataLoads : Table `vendor_restrictions` has %u columns", vendor_restricitons_result->GetFieldCount());

    _vendorRestrictionsStore.rehash(vendor_restricitons_result->GetRowCount());

    uint32_t vendor_restricitons_count = 0;
    do
    {
        Field* fields = vendor_restricitons_result->Fetch();

        uint32_t entry = fields[0].GetUInt32();

        MySQLStructure::VendorRestrictions& vendorRestriction = _vendorRestrictionsStore[entry];

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

    sLogger.info("MySQLDataLoads : Loaded %u restrictions from `vendor_restrictions` table in %u ms!", vendor_restricitons_count, static_cast<uint32_t>(Util::GetTimeDifferenceToNow(startTime)));
}

MySQLStructure::VendorRestrictions const* MySQLDataStore::getVendorRestriction(uint32_t entry)
{
    VendorRestrictionContainer::const_iterator itr = _vendorRestrictionsStore.find(entry);
    if (itr != _vendorRestrictionsStore.end())
        return &(itr->second);

    return nullptr;
}

void MySQLDataStore::loadNpcTextTable()
{
    auto startTime = Util::TimeNow();

    //                                                                  0
    QueryResult* npc_gossip_text_result = WorldDatabase.Query("SELECT entry, "
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
                                                        "prob7, text7_0, text7_1, lang7, EmoteDelay7_0, Emote7_0, EmoteDelay7_1, Emote7_1, EmoteDelay7_2, Emote7_2 FROM npc_gossip_texts");

    if (npc_gossip_text_result == nullptr)
    {
        sLogger.info("MySQLDataLoads : Table `npc_gossip_texts` is empty!");
        return;
    }

    sLogger.info("MySQLDataLoads : Table `npc_gossip_texts` has %u columns", npc_gossip_text_result->GetFieldCount());

    _npcGossipTextStore.rehash(npc_gossip_text_result->GetRowCount());

    uint32_t npc_text_count = 0;
    do
    {
        Field* fields = npc_gossip_text_result->Fetch();

        uint32_t entry = fields[0].GetUInt32();

        MySQLStructure::NpcGossipText& npcText = _npcGossipTextStore[entry];

        npcText.entry = entry;
        for (uint8_t i = 0; i < 8; ++i)
        {
            npcText.textHolder[i].probability = fields[1].GetFloat();

            for (uint8_t j = 0; j < 2; ++j)
            {
                npcText.textHolder[i].texts[j] = fields[2 + j].GetString();
            }

            npcText.textHolder[i].language = fields[4].GetUInt32();

            for (uint8_t k = 0; k < GOSSIP_EMOTE_COUNT; ++k)
            {
                npcText.textHolder[i].gossipEmotes[k].delay = fields[5 + k * 2].GetUInt32();
                npcText.textHolder[i].gossipEmotes[k].emote = fields[6 + k * 2].GetUInt32();
            }
        }


        ++npc_text_count;
    } while (npc_gossip_text_result->NextRow());

    delete npc_gossip_text_result;

    sLogger.info("MySQLDataLoads : Loaded %u rows from `npc_gossip_texts` table in %u ms!", npc_text_count, static_cast<uint32_t>(Util::GetTimeDifferenceToNow(startTime)));
}

MySQLStructure::NpcGossipText const* MySQLDataStore::getNpcGossipText(uint32_t entry) const
{
    NpcGossipTextContainer::const_iterator itr = _npcGossipTextStore.find(entry);
    if (itr != _npcGossipTextStore.end())
    {
        return &(itr->second);
    }

    return nullptr;
}

void MySQLDataStore::loadNpcScriptTextTable()
{
    auto startTime = Util::TimeNow();

    //                                                                  0      1           2       3     4       5          6         7       8        9         10
    QueryResult* npc_script_text_result = WorldDatabase.Query("SELECT entry, text, creature_entry, id, type, language, probability, emote, duration, sound, broadcast_id FROM npc_script_text");

    if (npc_script_text_result == nullptr)
    {
        sLogger.info("MySQLDataLoads : Table `npc_script_text` is empty!");
        return;
    }

    sLogger.info("MySQLDataLoads : Table `npc_script_text` has %u columns", npc_script_text_result->GetFieldCount());

    _npcScriptTextStore.rehash(npc_script_text_result->GetRowCount());

    uint32_t npc_script_text_count = 0;
    do
    {
        Field* fields = npc_script_text_result->Fetch();

        uint32_t entry = fields[0].GetUInt32();

        MySQLStructure::NpcScriptText& npcScriptText = _npcScriptTextStore[entry];

        npcScriptText.id = entry;
        npcScriptText.text = fields[1].GetString();
        npcScriptText.creature_entry = fields[2].GetUInt32();
        npcScriptText.text_id = fields[3].GetUInt32();
        npcScriptText.type = fields[4].GetUInt8();
        npcScriptText.language = Languages(fields[5].GetUInt32());
        npcScriptText.probability = fields[6].GetFloat();
        npcScriptText.emote = EmoteType(fields[7].GetUInt32());
        npcScriptText.duration = fields[8].GetUInt32();
        npcScriptText.sound = fields[9].GetUInt32();
        npcScriptText.broadcast_id = fields[10].GetUInt32();

        ++npc_script_text_count;
    } while (npc_script_text_result->NextRow());

    delete npc_script_text_result;

    sLogger.info("MySQLDataLoads : Loaded %u rows from `npc_script_text` table in %u ms!", npc_script_text_count, static_cast<uint32_t>(Util::GetTimeDifferenceToNow(startTime)));
}

MySQLStructure::NpcScriptText const* MySQLDataStore::getNpcScriptText(uint32_t entry)
{
    NpcScriptTextContainer::const_iterator itr = _npcScriptTextStore.find(entry);
    if (itr != _npcScriptTextStore.end())
        return &(itr->second);

    return nullptr;
}

void MySQLDataStore::loadGossipMenuOptionTable()
{
    auto startTime = Util::TimeNow();

    //                                                                      0         1
    QueryResult* gossip_menu_optiont_result = WorldDatabase.Query("SELECT entry, option_text FROM gossip_menu_option");

    if (gossip_menu_optiont_result == nullptr)
    {
        sLogger.info("MySQLDataLoads : Table `gossip_menu_option` is empty!");
        return;
    }

    sLogger.info("MySQLDataLoads : Table `gossip_menu_option` has %u columns", gossip_menu_optiont_result->GetFieldCount());

    _gossipMenuOptionStore.rehash(gossip_menu_optiont_result->GetRowCount());

    uint32_t gossip_menu_optiont_count = 0;
    do
    {
        Field* fields = gossip_menu_optiont_result->Fetch();

        uint32_t entry = fields[0].GetUInt32();

        MySQLStructure::GossipMenuOption& gossipMenuOptionText = _gossipMenuOptionStore[entry];

        gossipMenuOptionText.id = entry;
        gossipMenuOptionText.text = fields[1].GetString();

        ++gossip_menu_optiont_count;
    } while (gossip_menu_optiont_result->NextRow());

    delete gossip_menu_optiont_result;

    sLogger.info("MySQLDataLoads : Loaded %u rows from `gossip_menu_option` table in %u ms!", gossip_menu_optiont_count, static_cast<uint32_t>(Util::GetTimeDifferenceToNow(startTime)));
}

MySQLStructure::GossipMenuOption const* MySQLDataStore::getGossipMenuOption(uint32_t entry)
{
    GossipMenuOptionContainer::const_iterator itr = _gossipMenuOptionStore.find(entry);
    if (itr != _gossipMenuOptionStore.end())
        return &(itr->second);

    return nullptr;
}

void MySQLDataStore::loadGraveyardsTable()
{
    auto startTime = Util::TimeNow();

    //                                                            0         1         2           3            4         5          6           7       8
    QueryResult* graveyards_result = WorldDatabase.Query("SELECT id, position_x, position_y, position_z, orientation, zoneid, adjacentzoneid, mapid, faction FROM graveyards");
    if (graveyards_result == nullptr)
    {
        sLogger.info("MySQLDataLoads : Table `graveyards` is empty!");
        return;
    }

    sLogger.info("MySQLDataLoads : Table `graveyards` has %u columns", graveyards_result->GetFieldCount());

    _graveyardsStore.rehash(graveyards_result->GetRowCount());

    uint32_t graveyards_count = 0;
    do
    {
        Field* fields = graveyards_result->Fetch();

        uint32_t entry = fields[0].GetUInt32();

        MySQLStructure::Graveyards& graveyardTeleport = _graveyardsStore[entry];

        graveyardTeleport.id = entry;
        graveyardTeleport.position_x = fields[1].GetFloat();
        graveyardTeleport.position_y = fields[2].GetFloat();
        graveyardTeleport.position_z = fields[3].GetFloat();
        graveyardTeleport.orientation = fields[4].GetFloat();
        graveyardTeleport.zoneId = fields[5].GetUInt32();
        graveyardTeleport.adjacentZoneId = fields[6].GetUInt32();
        graveyardTeleport.mapId = fields[7].GetUInt32();
        graveyardTeleport.factionId = fields[8].GetUInt32();

        ++graveyards_count;
    } while (graveyards_result->NextRow());

    delete graveyards_result;

    sLogger.info("MySQLDataLoads : Loaded %u rows from `graveyards` table in %u ms!", graveyards_count, static_cast<uint32_t>(Util::GetTimeDifferenceToNow(startTime)));
}

MySQLStructure::Graveyards const* MySQLDataStore::getGraveyard(uint32_t entry)
{
    GraveyardsContainer::const_iterator itr = _graveyardsStore.find(entry);
    if (itr != _graveyardsStore.end())
        return &(itr->second);

    return nullptr;
}

void MySQLDataStore::loadTeleportCoordsTable()
{
    auto startTime = Util::TimeNow();

    //                                                                0     1         2           3           4
    QueryResult* teleport_coords_result = WorldDatabase.Query("SELECT id, mapId, position_x, position_y, position_z FROM spell_teleport_coords");
    if (teleport_coords_result == nullptr)
    {
        sLogger.info("MySQLDataLoads : Table `spell_teleport_coords` is empty!");
        return;
    }

    sLogger.info("MySQLDataLoads : Table `spell_teleport_coords` has %u columns", teleport_coords_result->GetFieldCount());

    _teleportCoordsStore.rehash(teleport_coords_result->GetRowCount());

    uint32_t teleport_coords_count = 0;
    do
    {
        Field* fields = teleport_coords_result->Fetch();

        uint32_t entry = fields[0].GetUInt32();

        TeleportCoords& teleportCoords = _teleportCoordsStore[entry];

        teleportCoords.id = entry;
        teleportCoords.mapId = fields[1].GetUInt32();
        teleportCoords.x = fields[2].GetFloat();
        teleportCoords.y = fields[3].GetFloat();
        teleportCoords.z = fields[4].GetFloat();

        ++teleport_coords_count;
    } while (teleport_coords_result->NextRow());

    delete teleport_coords_result;

    sLogger.info("MySQLDataLoads : Loaded %u rows from `spell_teleport_coords` table in %u ms!", teleport_coords_count, static_cast<uint32_t>(Util::GetTimeDifferenceToNow(startTime)));
}

TeleportCoords const* MySQLDataStore::getTeleportCoord(uint32_t entry)
{
    TeleportCoordsContainer::const_iterator itr = _teleportCoordsStore.find(entry);
    if (itr != _teleportCoordsStore.end())
        return &(itr->second);

    return nullptr;
}

void MySQLDataStore::loadFishingTable()
{
    auto startTime = Util::TimeNow();

    //                                                          0      1         2
    QueryResult* fishing_result = WorldDatabase.Query("SELECT zone, MinSkill, MaxSkill FROM fishing");
    if (fishing_result == nullptr)
    {
        sLogger.info("MySQLDataLoads : Table `fishing` is empty!");
        return;
    }

    sLogger.info("MySQLDataLoads : Table `fishing` has %u columns", fishing_result->GetFieldCount());

    _fishingZonesStore.rehash(fishing_result->GetRowCount());

    uint32_t fishing_count = 0;
    do
    {
        Field* fields = fishing_result->Fetch();

        uint32_t entry = fields[0].GetUInt32();

        MySQLStructure::FishingZones& fishingZone = _fishingZonesStore[entry];

        fishingZone.zoneId = entry;
        fishingZone.minSkill = fields[1].GetUInt32();
        fishingZone.maxSkill = fields[2].GetUInt32();

        ++fishing_count;
    } while (fishing_result->NextRow());

    delete fishing_result;

    sLogger.info("MySQLDataLoads : Loaded %u rows from `fishing` table in %u ms!", fishing_count, static_cast<uint32_t>(Util::GetTimeDifferenceToNow(startTime)));
}

MySQLStructure::FishingZones const* MySQLDataStore::getFishingZone(uint32_t entry)
{
    FishingZonesContainer::const_iterator itr = _fishingZonesStore.find(entry);
    if (itr != _fishingZonesStore.end())
        return &(itr->second);

    return nullptr;
}

void MySQLDataStore::loadWorldMapInfoTable()
{
    auto startTime = Util::TimeNow();

    //                                                                0        1       2       3           4             5          6        7      8          9
    QueryResult* worldmap_info_result = WorldDatabase.Query("SELECT entry, screenid, type, maxplayers, minlevel, minlevel_heroic, repopx, repopy, repopz, repopentry, "
    //                                                           10       11      12         13           14                15              16
                                                            "area_name, flags, cooldown, lvl_mod_a, required_quest_A, required_quest_H, required_item, "
    //                                                              17              18              19                20
                                                            "heroic_keyid_1, heroic_keyid_2, viewingDistance, required_checkpoint FROM worldmap_info base "
                                                            "WHERE build=(SELECT MAX(build) FROM worldmap_info buildspecific WHERE base.entry = buildspecific.entry AND build <= %u)", VERSION_STRING);
    if (worldmap_info_result == nullptr)
    {
        sLogger.info("MySQLDataLoads : Table `worldmap_info` is empty!");
        return;
    }

    sLogger.info("MySQLDataLoads : Table `worldmap_info` has %u columns", worldmap_info_result->GetFieldCount());

    _worldMapInfoStore.rehash(worldmap_info_result->GetRowCount());

    uint32_t world_map_info_count = 0;
    do
    {
        Field* fields = worldmap_info_result->Fetch();

        uint32_t entry = fields[0].GetUInt32();

        MySQLStructure::MapInfo& mapInfo = _worldMapInfoStore[entry];

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

    sLogger.info("MySQLDataLoads : Loaded %u rows from `worldmap_info` table in %u ms!", world_map_info_count, static_cast<uint32_t>(Util::GetTimeDifferenceToNow(startTime)));
}

MySQLStructure::MapInfo const* MySQLDataStore::getWorldMapInfo(uint32_t entry)
{
    WorldMapInfoContainer::const_iterator itr = _worldMapInfoStore.find(entry);
    if (itr != _worldMapInfoStore.end())
        return &(itr->second);

    return nullptr;
}

void MySQLDataStore::loadZoneGuardsTable()
{
    auto startTime = Util::TimeNow();

    //                                                             0         1              2
    QueryResult* zone_guards_result = WorldDatabase.Query("SELECT zone, horde_entry, alliance_entry FROM zoneguards");
    if (zone_guards_result == nullptr)
    {
        sLogger.info("MySQLDataLoads : Table `zoneguards` is empty!");
        return;
    }

    sLogger.info("MySQLDataLoads : Table `zoneguards` has %u columns", zone_guards_result->GetFieldCount());

    _zoneGuardsStore.rehash(zone_guards_result->GetRowCount());

    uint32_t zone_guards_count = 0;
    do
    {
        Field* fields = zone_guards_result->Fetch();

        uint32_t entry = fields[0].GetUInt32();

        MySQLStructure::ZoneGuards& zoneGuard = _zoneGuardsStore[entry];

        zoneGuard.zoneId = entry;
        zoneGuard.hordeEntry = fields[1].GetUInt32();
        zoneGuard.allianceEntry = fields[2].GetUInt32();

        ++zone_guards_count;
    } while (zone_guards_result->NextRow());

    delete zone_guards_result;

    sLogger.info("MySQLDataLoads : Loaded %u rows from `zoneguards` table in %u ms!", zone_guards_count, static_cast<uint32_t>(Util::GetTimeDifferenceToNow(startTime)));
}

MySQLStructure::ZoneGuards const* MySQLDataStore::getZoneGuard(uint32_t entry)
{
    ZoneGuardsContainer::const_iterator itr = _zoneGuardsStore.find(entry);
    if (itr != _zoneGuardsStore.end())
        return &(itr->second);

    return nullptr;
}

void MySQLDataStore::loadBattleMastersTable()
{
    auto startTime = Util::TimeNow();

    //                                                                      0                1
    QueryResult* battlemasters_result = WorldDatabase.Query("SELECT creature_entry, battleground_id FROM battlemasters");
    if (battlemasters_result == nullptr)
    {
        sLogger.info("MySQLDataLoads : Table `battlemasters` is empty!");
        return;
    }

    sLogger.info("MySQLDataLoads : Table `battlemasters` has %u columns", battlemasters_result->GetFieldCount());

    _battleMastersStore.rehash(battlemasters_result->GetRowCount());

    uint32_t battlemasters_count = 0;
    do
    {
        Field* fields = battlemasters_result->Fetch();

        uint32_t entry = fields[0].GetUInt32();

        MySQLStructure::Battlemasters& bgMaster = _battleMastersStore[entry];

        bgMaster.creatureEntry = entry;
        bgMaster.battlegroundId = fields[1].GetUInt32();

        ++battlemasters_count;
    } while (battlemasters_result->NextRow());

    delete battlemasters_result;

    sLogger.info("MySQLDataLoads : Loaded %u rows from `battlemasters` table in %u ms!", battlemasters_count, static_cast<uint32_t>(Util::GetTimeDifferenceToNow(startTime)));
}

MySQLStructure::Battlemasters const* MySQLDataStore::getBattleMaster(uint32_t entry)
{
    BattleMastersContainer::const_iterator itr = _battleMastersStore.find(entry);
    if (itr != _battleMastersStore.end())
    {
        return &(itr->second);
    }

    return nullptr;
}

void MySQLDataStore::loadTotemDisplayIdsTable()
{
    auto startTime = Util::TimeNow();

    //                                                                  0     1        2
    QueryResult* totemdisplayids_result = WorldDatabase.Query("SELECT race, totem, displayid FROM totemdisplayids base "
        "WHERE build=(SELECT MAX(build) FROM totemdisplayids spec WHERE base.race = spec.race AND base.totem = spec.totem AND build <= %u)", VERSION_STRING);

    if (totemdisplayids_result == nullptr)
    {
        sLogger.info("MySQLDataLoads : Table `totemdisplayids` is empty!");
        return;
    }

    sLogger.info("MySQLDataLoads : Table `totemdisplayids` has %u columns", totemdisplayids_result->GetFieldCount());

    uint32_t totemdisplayids_count = 0;
    do
    {
        Field* fields = totemdisplayids_result->Fetch();

        MySQLStructure::TotemDisplayIds totemDisplayId;

        totemDisplayId._race = static_cast<uint8_t>(fields[0].GetUInt32());
        totemDisplayId.display_id = fields[1].GetUInt32();
        totemDisplayId.race_specific_id = fields[2].GetUInt32();

        _totemDisplayIdsStore.push_back(totemDisplayId);

        ++totemdisplayids_count;
    } while (totemdisplayids_result->NextRow());

    delete totemdisplayids_result;

    sLogger.info("MySQLDataLoads : Loaded %u rows from `totemdisplayids` table in %u ms!", static_cast<uint32_t>(_totemDisplayIdsStore.size()), static_cast<uint32_t>(Util::GetTimeDifferenceToNow(startTime)));
}

MySQLStructure::TotemDisplayIds const* MySQLDataStore::getTotemDisplayId(uint8_t race, uint32_t entry)
{
    for (auto & i : _totemDisplayIdsStore)
    {
        if (i._race == race && i.display_id == entry)
            return &i;
    }
    return nullptr;
}

void MySQLDataStore::loadSpellClickSpellsTable()
{
    auto startTime = Util::TimeNow();
    _spellClickInfoStore.clear();

    //                                                0          1         2            3
    QueryResult* spellclickspells_result = WorldDatabase.Query("SELECT npc_entry, spell_id, cast_flags, user_type FROM npc_spellclick_spells");
    if (spellclickspells_result == nullptr)
    {
        sLogger.info("MySQLDataLoads : Table `spellclickspells` is empty!");
        return;
    }

    sLogger.info("MySQLDataLoads : Table `spellclickspells` has %u columns", spellclickspells_result->GetFieldCount());

    uint32_t spellclickspells_count = 0;
    do
    {
        Field* fields = spellclickspells_result->Fetch();

        uint32_t npc_entry = fields[0].GetUInt32();
        CreatureProperties const* cInfo = sMySQLStore.getCreatureProperties(npc_entry);
        if (!cInfo)
        {
            sLogger.failure("Table npc_spellclick_spells references unknown creature_properties %u. Skipping entry.", npc_entry);
            continue;
        }

        uint32_t spellid = fields[1].GetUInt32();
        SpellInfo const* spellinfo = sSpellMgr.getSpellInfo(spellid);
        if (!spellinfo)
        {
            sLogger.failure("Table npc_spellclick_spells creature: %u references unknown spellid %u. Skipping entry.", npc_entry, spellid);
            continue;
        }

        uint8_t userType = fields[3].GetUInt8();
        if (userType >= SPELL_CLICK_USER_MAX)
            sLogger.failure("Table npc_spellclick_spells creature: %u references unknown user type %u. Skipping entry.", npc_entry, uint32(userType));

        uint8_t castFlags = fields[2].GetUInt8();

        SpellClickInfo info;
        info.spellId = spellid;
        info.castFlags = castFlags;
        info.userType = SpellClickUserTypes(userType);
        _spellClickInfoStore.insert(SpellClickInfoContainer::value_type(npc_entry, info));
    } while (spellclickspells_result->NextRow());

    delete spellclickspells_result;

    sLogger.info("MySQLDataLoads : Loaded %u rows from `spellclickspells` table in %u ms!", spellclickspells_count, static_cast<uint32_t>(Util::GetTimeDifferenceToNow(startTime)));
}

std::vector<SpellClickInfo> const MySQLDataStore::getSpellClickInfo(uint32_t creature_id)
{
    std::vector<SpellClickInfo> list;
    for (auto const& itr : _spellClickInfoStore)
    {
        if (itr.first == creature_id)
            list.emplace_back(itr.second);
    }

    return list;
}

void MySQLDataStore::loadWorldStringsTable()
{
    auto startTime = Util::TimeNow();

    //                                                                     0     1
    QueryResult* worldstring_tables_result = WorldDatabase.Query("SELECT entry, text FROM worldstring_tables");
    if (worldstring_tables_result == nullptr)
    {
        sLogger.info("MySQLDataLoads : Table `worldstring_tables` is empty!");
        return;
    }

    sLogger.info("MySQLDataLoads : Table `worldstring_tables` has %u columns", worldstring_tables_result->GetFieldCount());

    _worldStringsStore.rehash(worldstring_tables_result->GetRowCount());

    uint32_t worldstring_tables_count = 0;
    do
    {
        Field* fields = worldstring_tables_result->Fetch();

        uint32_t entry = fields[0].GetUInt32();

        MySQLStructure::WorldStringTable& worldString = _worldStringsStore[entry];

        worldString.id = entry;
        worldString.text = fields[1].GetString();

        ++worldstring_tables_count;
    } while (worldstring_tables_result->NextRow());

    delete worldstring_tables_result;

    sLogger.info("MySQLDataLoads : Loaded %u rows from `worldstring_tables` table in %u ms!", worldstring_tables_count, static_cast<uint32_t>(Util::GetTimeDifferenceToNow(startTime)));
}

MySQLStructure::WorldStringTable const* MySQLDataStore::getWorldString(uint32_t entry)
{
    WorldStringContainer::const_iterator itr = _worldStringsStore.find(entry);
    if (itr != _worldStringsStore.end())
        return &(itr->second);

    return nullptr;
}

void MySQLDataStore::loadPointsOfInterestTable()
{
    auto startTime = Util::TimeNow();

    //                                                                      0   1  2    3     4     5        6
    QueryResult* points_of_interest_result = WorldDatabase.Query("SELECT entry, x, y, icon, flags, data, icon_name FROM points_of_interest");
    if (points_of_interest_result == nullptr)
    {
        sLogger.info("MySQLDataLoads : Table `points_of_interest` is empty!");
        return;
    }

    sLogger.info("MySQLDataLoads : Table `points_of_interest` has %u columns", points_of_interest_result->GetFieldCount());

    _pointsOfInterestStore.rehash(points_of_interest_result->GetRowCount());

    uint32_t points_of_interest_count = 0;
    do
    {
        Field* fields = points_of_interest_result->Fetch();

        uint32_t entry = fields[0].GetUInt32();

        MySQLStructure::PointsOfInterest& pointOfInterest = _pointsOfInterestStore[entry];

        pointOfInterest.id = entry;
        pointOfInterest.x = fields[1].GetFloat();
        pointOfInterest.y = fields[2].GetFloat();
        pointOfInterest.icon = fields[3].GetUInt32();
        pointOfInterest.flags = fields[4].GetUInt32();
        pointOfInterest.data = fields[5].GetUInt32();
        pointOfInterest.iconName = fields[6].GetString();

        ++points_of_interest_count;
    } while (points_of_interest_result->NextRow());

    delete points_of_interest_result;

    sLogger.info("MySQLDataLoads : Loaded %u rows from `points_of_interest` table in %u ms!", points_of_interest_count, static_cast<uint32_t>(Util::GetTimeDifferenceToNow(startTime)));
}

MySQLStructure::PointsOfInterest const* MySQLDataStore::getPointOfInterest(uint32_t entry)
{
    PointsOfInterestContainer::const_iterator itr = _pointsOfInterestStore.find(entry);
    if (itr != _pointsOfInterestStore.end())
        return &(itr->second);

    return nullptr;
}

void MySQLDataStore::loadItemSetLinkedSetBonusTable()
{
    auto startTime = Util::TimeNow();

    //                                                                    0            1
    QueryResult* linked_set_bonus_result = WorldDatabase.Query("SELECT itemset, itemset_bonus FROM itemset_linked_itemsetbonus");
    if (linked_set_bonus_result == nullptr)
    {
        sLogger.info("MySQLDataLoads : Table `itemset_linked_itemsetbonus` is empty!");
        return;
    }

    sLogger.info("MySQLDataLoads : Table `itemset_linked_itemsetbonus` has %u columns", linked_set_bonus_result->GetFieldCount());

    _definedItemSetBonusStore.rehash(linked_set_bonus_result->GetRowCount());

    uint32_t linked_set_bonus_count = 0;
    do
    {
        Field* fields = linked_set_bonus_result->Fetch();

        int32_t entry = fields[0].GetInt32();

        MySQLStructure::ItemSetLinkedItemSetBonus& itemSetLinkedItemSetBonus = _definedItemSetBonusStore[entry];

        itemSetLinkedItemSetBonus.itemset = entry;
        itemSetLinkedItemSetBonus.itemset_bonus  = fields[1].GetUInt32();

        ++linked_set_bonus_count;

    } while (linked_set_bonus_result->NextRow());

    delete linked_set_bonus_result;

    sLogger.info("MySQLDataLoads : Loaded %u rows from `itemset_linked_itemsetbonus` table in %u ms!", linked_set_bonus_count, static_cast<uint32_t>(Util::GetTimeDifferenceToNow(startTime)));
}

uint32_t MySQLDataStore::getItemSetLinkedBonus(int32_t itemset)
{
    auto itr = _definedItemSetBonusStore.find(itemset);
    if (itr == _definedItemSetBonusStore.end())
    {
        return 0;
    }
    else
    {
        return itr->second.itemset_bonus;
    }
}

void MySQLDataStore::loadCreatureInitialEquipmentTable()
{
    auto startTime = Util::TimeNow();

    //                                                                        0              1           2          3
    QueryResult* initial_equipment_result = WorldDatabase.Query("SELECT creature_entry, itemslot_1, itemslot_2, itemslot_3 FROM creature_initial_equip;");
    if (initial_equipment_result == nullptr)
    {
        sLogger.info("MySQLDataLoads : Table `creature_initial_equip` is empty!");
        return;
    }

    sLogger.info("MySQLDataLoads : Table `creature_initial_equip` has %u columns", initial_equipment_result->GetFieldCount());

    uint32_t initial_equipment_count = 0;
    do
    {
        Field* fields = initial_equipment_result->Fetch();
        uint32_t entry = fields[0].GetUInt32();
        CreatureProperties const* creature_properties = sMySQLStore.getCreatureProperties(entry);
        if (creature_properties == nullptr)
        {
            //sLogger.debug("Invalid creature_entry %u in table creature_initial_equip!", entry);
            continue;
        }

        const_cast<CreatureProperties*>(creature_properties)->itemslot_1 = sMySQLStore.getItemDisplayIdForEntry(fields[1].GetUInt32());
        const_cast<CreatureProperties*>(creature_properties)->itemslot_2 = sMySQLStore.getItemDisplayIdForEntry(fields[2].GetUInt32());
        const_cast<CreatureProperties*>(creature_properties)->itemslot_3 = sMySQLStore.getItemDisplayIdForEntry(fields[3].GetUInt32());

        ++initial_equipment_count;

    } while (initial_equipment_result->NextRow());

    delete initial_equipment_result;

    sLogger.info("MySQLDataLoads : Loaded %u rows from `creature_initial_equip` table in %u ms!", initial_equipment_count, static_cast<uint32_t>(Util::GetTimeDifferenceToNow(startTime)));
}

void MySQLDataStore::loadPlayerCreateInfoTable()
{
    auto startTime = Util::TimeNow();

    //                                                                     1     2      3      4          5          6         7           8
    QueryResult* player_create_info_result = WorldDatabase.Query("SELECT race, class, mapID, zoneID, positionX, positionY, positionZ, orientation FROM playercreateinfo pi "

        "WHERE build=(SELECT MAX(build) FROM playercreateinfo buildspecific WHERE pi.race = buildspecific.race AND pi.class = buildspecific.class AND build <= %u)", VERSION_STRING);
    if (player_create_info_result == nullptr)
    {
        sLogger.info("MySQLDataLoads : Table `playercreateinfo` is empty!");
        return;
    }

    sLogger.info("MySQLDataLoads : Table `playercreateinfo` has %u columns", player_create_info_result->GetFieldCount());

    uint32_t player_create_info_count = 0;
    do
    {
        Field* fields = player_create_info_result->Fetch();
        PlayerCreateInfo* playerCreateInfo = new PlayerCreateInfo;

        uint8_t _race = fields[0].GetUInt8();
        uint8_t _class = fields[1].GetUInt8();
        playerCreateInfo->mapId = fields[2].GetUInt32();
        playerCreateInfo->zoneId = fields[3].GetUInt32();
        playerCreateInfo->positionX = fields[4].GetFloat();
        playerCreateInfo->positionY = fields[5].GetFloat();
        playerCreateInfo->positionZ = fields[6].GetFloat();
        playerCreateInfo->orientation = fields[7].GetFloat();
        _playerCreateInfoStoreNew[_race][_class] = playerCreateInfo;

        player_create_info_count++;

    } while (player_create_info_result->NextRow());

    delete player_create_info_result;

    sLogger.info("MySQLDataLoads : Loaded %u rows from `playercreateinfo` table in %u ms!", player_create_info_count, static_cast<uint32_t>(Util::GetTimeDifferenceToNow(startTime)));
}


void MySQLDataStore::loadPlayerCreateInfoBars()
{

    //                                                                          0     1      2        3      4     5
    QueryResult* player_create_info_bars_result = WorldDatabase.Query("SELECT race, class, button, action, type, misc FROM playercreateinfo_bars WHERE build = %u", VERSION_STRING);

    if (player_create_info_bars_result == nullptr)
    {
        sLogger.info("MySQLDataLoads : Table `playercreateinfo_bars` has no data");
        return;
    }

    uint32_t player_create_info_bars_count = 0;
    do
    {
        Field* fields = player_create_info_bars_result->Fetch();

        uint8_t _race = fields[0].GetUInt8();
        uint8_t _class = fields[1].GetUInt8();

        if (auto& playerCreateInfo = _playerCreateInfoStoreNew[_race][_class])
        {
            CreateInfo_ActionBarStruct bar;
            bar.button = fields[2].GetUInt8();
            bar.action = fields[3].GetUInt32();
            bar.type = fields[4].GetUInt8();
            bar.misc = fields[5].GetUInt8();

            playerCreateInfo->actionbars.push_back(bar);

            ++player_create_info_bars_count;
        }

    } while (player_create_info_bars_result->NextRow());

    delete player_create_info_bars_result;
}

void MySQLDataStore::loadPlayerCreateInfoItems()
{
    auto startTime = Util::TimeNow();

    //                                                                           0     1       2       3       4
    QueryResult* player_create_info_items_result = WorldDatabase.Query("SELECT race, class, protoid, slotid, amount FROM playercreateinfo_items WHERE build = %u", VERSION_STRING);

    if (player_create_info_items_result == nullptr)
    {
        sLogger.info("MySQLDataLoads : Table `playercreateinfo_items` is empty!");
        return;
    }

    sLogger.info("MySQLDataLoads : Table `playercreateinfo_items` has %u columns", player_create_info_items_result->GetFieldCount());

    uint32_t player_create_info_items_count = 0;
    do
    {
        Field* fields = player_create_info_items_result->Fetch();

        uint8_t _race = fields[0].GetUInt8();
        uint8_t _class = fields[1].GetUInt8();
        uint32_t item_id = fields[2].GetUInt32();

#if VERSION_STRING < Cata
        auto player_item = sMySQLStore.getItemProperties(item_id);
#else
        DB2::Structures::ItemEntry const* player_item = sItemStore.LookupEntry(item_id);
#endif
        if (player_item == nullptr)
        {
            sLogger.failure("Table `old_playercreateinfo_items` includes invalid item %u", item_id);
            continue;
        }

        if (auto& playerCreateInfo = _playerCreateInfoStoreNew[_race][_class])
        {
            CreateInfo_ItemStruct itm;
            itm.id = item_id;
            itm.slot = fields[3].GetUInt8();
            itm.amount = fields[4].GetUInt32();

            playerCreateInfo->items.push_back(itm);

            ++player_create_info_items_count;
        }

    } while (player_create_info_items_result->NextRow());

    delete player_create_info_items_result;

    sLogger.info("MySQLDataLoads : Loaded %u rows from `playercreateinfo_items` table in %u ms!", player_create_info_items_count, static_cast<uint32_t>(Util::GetTimeDifferenceToNow(startTime)));
}

void MySQLDataStore::loadPlayerCreateInfoSkills()
{
    auto startTime = Util::TimeNow();

    //                                                                              0         1         2       3
    QueryResult* player_create_info_skills_result = WorldDatabase.Query("SELECT raceMask, classMask, skillid, level FROM playercreateinfo_skills WHERE min_build <= %u AND max_build >= %u", getAEVersion(), getAEVersion());

    if (player_create_info_skills_result == nullptr)
    {
        sLogger.info("MySQLDataLoads : Table `playercreateinfo_skills` is empty!");
        return;
    }

    sLogger.info("MySQLDataLoads : Table `playercreateinfo_skills` has %u columns", player_create_info_skills_result->GetFieldCount());

    uint32_t player_create_info_skills_count = 0;
    do
    {
        Field* fields = player_create_info_skills_result->Fetch();

        uint32_t raceMask = fields[0].GetUInt32();
        uint32_t classMask = fields[1].GetUInt32();
        auto skill_id = fields[2].GetUInt16();

        auto player_skill = sSkillLineStore.LookupEntry(skill_id);
        if (player_skill == nullptr)
        {
            sLogger.failure("Table `playercreateinfo_skills` includes invalid skill id %u", skill_id);
            continue;
        }

        CreateInfo_SkillStruct tsk;
        tsk.skillid = skill_id;
        tsk.currentval = fields[3].GetUInt16();

        for (uint32_t raceIndex = RACE_HUMAN; raceIndex < DBC_NUM_RACES; ++raceIndex)
        {
            if (raceMask == 0 || ((1 << (raceIndex - 1)) & raceMask))
            {
                for (uint32_t classIndex = WARRIOR; classIndex < MAX_PLAYER_CLASSES; ++classIndex)
                {
                    if (classMask == 0 || ((1 << (classIndex - 1)) & classMask))
                    {
                        if (auto& playerCreateInfo = _playerCreateInfoStoreNew[raceIndex][classIndex])
                        {
                            playerCreateInfo->skills.push_back(tsk);
                            ++player_create_info_skills_count;
                        }
                    }
                }
            }
        }

    } while (player_create_info_skills_result->NextRow());

    delete player_create_info_skills_result;

    sLogger.info("MySQLDataLoads : Loaded %u rows from `playercreateinfo_skills` table in %u ms!", player_create_info_skills_count, static_cast<uint32_t>(Util::GetTimeDifferenceToNow(startTime)));
}

void MySQLDataStore::loadPlayerCreateInfoSpellLearn()
{
    auto startTime = Util::TimeNow();

    //                                                                              0         1         2
    QueryResult* player_create_info_spells_result = WorldDatabase.Query("SELECT raceMask, classMask, spellid FROM playercreateinfo_spell_learn WHERE min_build <= %u AND max_build >= %u", getAEVersion(), getAEVersion());

    if (player_create_info_spells_result == nullptr)
    {
        sLogger.info("MySQLDataLoads : Table `playercreateinfo_spell_learn` is empty!");
        return;
    }

    sLogger.info("MySQLDataLoads : Table `playercreateinfo_spell_learn` has %u columns", player_create_info_spells_result->GetFieldCount());

    uint32_t player_create_info_spells_count = 0;
    do
    {
        Field* fields = player_create_info_spells_result->Fetch();

        uint32_t raceMask = fields[0].GetUInt32();
        uint32_t classMask = fields[1].GetUInt32();
        uint32_t spell_id = fields[2].GetUInt32();

        auto player_spell = sSpellStore.LookupEntry(spell_id);
        if (player_spell == nullptr)
        {
            sLogger.failure("Table `playercreateinfo_spell_learn` includes invalid spell %u", spell_id);
            continue;
        }

        for (uint32_t raceIndex = RACE_HUMAN; raceIndex < DBC_NUM_RACES; ++raceIndex)
        {
            if (raceMask == 0 || ((1 << (raceIndex - 1)) & raceMask))
            {
                for (uint32_t classIndex = WARRIOR; classIndex < MAX_PLAYER_CLASSES; ++classIndex)
                {
                    if (classMask == 0 || ((1 << (classIndex - 1)) & classMask))
                    {
                        if (auto& playerCreateInfo = _playerCreateInfoStoreNew[raceIndex][classIndex])
                        {
                            playerCreateInfo->spell_list.insert(spell_id);
                            ++player_create_info_spells_count;
                        }
                    }
                }
            }
        }

    } while (player_create_info_spells_result->NextRow());

    delete player_create_info_spells_result;

    sLogger.info("MySQLDataLoads : Loaded %u rows from `playercreateinfo_spell_learn` table in %u ms!", player_create_info_spells_count, static_cast<uint32_t>(Util::GetTimeDifferenceToNow(startTime)));
}

void MySQLDataStore::loadPlayerCreateInfoSpellCast()
{
    auto startTime = Util::TimeNow();

    //                                                                              0         1         2
    QueryResult* player_create_info_spells_result = WorldDatabase.Query("SELECT raceMask, classMask, spellid FROM playercreateinfo_spell_cast WHERE build = %u", VERSION_STRING);

    if (player_create_info_spells_result == nullptr)
    {
        sLogger.info("MySQLDataLoads : Table `playercreateinfo_spell_cast` is empty!");
        return;
    }

    sLogger.info("MySQLDataLoads : Table `playercreateinfo_spell_cast` has %u columns", player_create_info_spells_result->GetFieldCount());

    uint32_t player_create_info_spells_count = 0;
    do
    {
        Field* fields = player_create_info_spells_result->Fetch();

        uint32_t raceMask = fields[0].GetUInt32();
        uint32_t classMask = fields[1].GetUInt32();
        uint32_t spell_id = fields[2].GetUInt32();

        auto player_spell = sSpellStore.LookupEntry(spell_id);
        if (player_spell == nullptr)
        {
            sLogger.failure("Table `playercreateinfo_spell_cast` includes invalid spell %u", spell_id);
            continue;
        }

        for (uint32_t raceIndex = RACE_HUMAN; raceIndex < DBC_NUM_RACES; ++raceIndex)
        {
            if (raceMask == 0 || ((1 << (raceIndex - 1)) & raceMask))
            {
                for (uint32_t classIndex = WARRIOR; classIndex < MAX_PLAYER_CLASSES; ++classIndex)
                {
                    if (classMask == 0 || ((1 << (classIndex - 1)) & classMask))
                    {
                        if (auto& playerCreateInfo = _playerCreateInfoStoreNew[raceIndex][classIndex])
                        {
                            playerCreateInfo->spell_cast_list.insert(spell_id);
                            ++player_create_info_spells_count;
                        }
                    }
                }
            }
        }

    } while (player_create_info_spells_result->NextRow());

    delete player_create_info_spells_result;

    sLogger.info("MySQLDataLoads : Loaded %u rows from `playercreateinfo_spell_cast` table in %u ms!", player_create_info_spells_count, static_cast<uint32_t>(Util::GetTimeDifferenceToNow(startTime)));
}

void MySQLDataStore::loadPlayerCreateInfoLevelstats()
{
    auto startTime = Util::TimeNow();

    //                                                                    0     1      2          3           4            5             6             7
    QueryResult* player_levelstats_result = WorldDatabase.Query("SELECT race, class, level, BaseStrength, BaseAgility, BaseStamina, BaseIntellect, BaseSpirit FROM player_levelstats WHERE build = %u", VERSION_STRING);

    if (player_levelstats_result == nullptr)
    {
        sLogger.info("MySQLDataLoads : Table `player_levelstats` is empty!");
        return;
    }

    sLogger.info("MySQLDataLoads : Table `player_levelstats` has %u columns", player_levelstats_result->GetFieldCount());

    uint32_t player_levelstats_count = 0;
    do
    {
        Field* fields = player_levelstats_result->Fetch();

        uint32_t _race = fields[0].GetUInt32();
        uint32_t _class = fields[1].GetUInt32();
        uint32_t level = fields[2].GetUInt32();


        if (auto& playerCreateInfo = _playerCreateInfoStoreNew[_race][_class])
        {
            CreateInfo_Levelstats lvl;
            lvl.strength = fields[3].GetUInt32();
            lvl.agility = fields[4].GetUInt32();
            lvl.stamina = fields[5].GetUInt32();
            lvl.intellect = fields[6].GetUInt32();
            lvl.spirit = fields[7].GetUInt32();

            playerCreateInfo->level_stats.insert(std::make_pair(level, lvl));

            ++player_levelstats_count;
        }

    } while (player_levelstats_result->NextRow());

    delete player_levelstats_result;

    sLogger.info("MySQLDataLoads : Loaded %u rows from `player_levelstats` table in %u ms!", player_levelstats_count, static_cast<uint32_t>(Util::GetTimeDifferenceToNow(startTime)));

    //Zyres: load required and missing levelstats
    for (uint8_t _race = 0; _race < DBC_NUM_RACES; ++_race)
    {
        if (!sChrRacesStore.LookupEntry(_race))
            continue;

        for (uint8_t _class = 0; _class < MAX_PLAYER_CLASSES; ++_class)
        {
            if (!sChrClassesStore.LookupEntry(_class))
                continue;

            auto info = _playerCreateInfoStoreNew[_race][_class];
            if (!info)
                continue;

            for (uint8_t level = 1; level < DBC_STAT_LEVEL_CAP; ++level)
            {
                if (info->level_stats[level].strength == 0)
                {
                    sLogger.info("Race %i Class %i Level %i does not have stats data. Using stats data of level % i.", _race, _class, level + 1, level);
                    info->level_stats[level] = info->level_stats[level - 1U];
                }
            }
        }
    }
}

void MySQLDataStore::loadPlayerCreateInfoClassLevelstats()
{
    auto startTime = Util::TimeNow();

    //                                                                         0      1        2          3
    QueryResult* player_classlevelstats_result = WorldDatabase.Query("SELECT class, level, BaseHealth, BaseMana FROM player_classlevelstats WHERE build = %u", VERSION_STRING);

    if (player_classlevelstats_result)
    {
        sLogger.info("MySQLDataLoads : Table `player_classlevelstats` has %u columns", player_classlevelstats_result->GetFieldCount());

        uint32_t player_classlevelstats_count = 0;
        do
        {
            Field* fields = player_classlevelstats_result->Fetch();

            uint32_t _class = fields[0].GetUInt32();
            uint32_t level = fields[1].GetUInt32();

            CreateInfo_ClassLevelStats lvl;
            lvl.health = fields[2].GetUInt32();
            lvl.mana = fields[3].GetUInt32();

            _playerClassLevelStatsStore[_class].insert(std::make_pair(level, lvl));

            ++player_classlevelstats_count;

        } while (player_classlevelstats_result->NextRow());

        delete player_classlevelstats_result;

        sLogger.info("MySQLDataLoads : Loaded %u rows from `player_classlevelstats` table in %u ms!", player_classlevelstats_count, static_cast<uint32_t>(Util::GetTimeDifferenceToNow(startTime)));
    }
    else
    {
#if VERSION_STRING < Cata
        sLogger.info("MySQLDataLoads : Table `player_classlevelstats` is empty!");
#endif
    }

#if VERSION_STRING > WotLK
    //Zyres: load missing and required data from dbc!
    int32_t player_classlevelstats_count = 0;

    for (uint8_t player_class = 1; player_class < MAX_PLAYER_CLASSES - 1; ++player_class)
    {
        for (uint8_t level = 1; level < DBC_STAT_LEVEL_CAP; ++level)
        {
            // check if we already loaded data for level/class from db
            if (getPlayerClassLevelStats(level, player_class))
                continue;

            DBC::Structures::GtOCTBaseHPByClassEntry const* hp = sGtOCTBaseHPByClassStore.LookupEntry((player_class - 1) * DBC_STAT_LEVEL_CAP + level - 1);
            DBC::Structures::GtOCTBaseMPByClassEntry const* mp = sGtOCTBaseMPByClassStore.LookupEntry((player_class - 1) * DBC_STAT_LEVEL_CAP + level - 1);

            if (hp && mp)
            {
                CreateInfo_ClassLevelStats lvl;
                lvl.health = static_cast<uint32_t>(hp->ratio);
                lvl.mana = static_cast<uint32_t>(mp->ratio);

                _playerClassLevelStatsStore[player_class].insert(std::make_pair(level, lvl));
                ++player_classlevelstats_count;
            }
        }
    }

    sLogger.info("MySQLDataLoads : Loaded %u missing classlevelstats from dbc!", player_classlevelstats_count);

#endif
}


PlayerCreateInfo const* MySQLDataStore::getPlayerCreateInfo(uint8_t player_race, uint8_t player_class)
{
    return _playerCreateInfoStoreNew[player_race][player_class];
}

CreateInfo_Levelstats const* MySQLDataStore::getPlayerLevelstats(uint32_t level, uint8_t player_race, uint8_t player_class)
{
    if (auto playerCreateInfo = getPlayerCreateInfo(player_race, player_class))
    {
        CreateInfo_LevelstatsVector::const_iterator itr = playerCreateInfo->level_stats.find(level);
        if (itr != playerCreateInfo->level_stats.end())
            return &(itr->second);

        return nullptr;
    }

    return nullptr;
}

CreateInfo_ClassLevelStats const* MySQLDataStore::getPlayerClassLevelStats(uint32_t level, uint8_t player_class)
{
    CreateInfo_ClassLevelStatsVector::const_iterator itr = _playerClassLevelStatsStore[player_class].find(level);
    if (itr != _playerClassLevelStatsStore[player_class].end())
        return &(itr->second);

    return nullptr;
}


void MySQLDataStore::loadPlayerXpToLevelTable()
{
    auto startTime = Util::TimeNow();

    _playerXPperLevelStore.clear();
    _playerXPperLevelStore.resize(worldConfig.player.playerLevelCap);

    for (uint32_t level = 0; level < worldConfig.player.playerLevelCap; ++level)
        _playerXPperLevelStore[level] = 0;

    QueryResult* player_xp_to_level_result = WorldDatabase.Query("SELECT player_lvl, next_lvl_req_xp FROM player_xp_for_level base "
        "WHERE build=(SELECT MAX(build) FROM player_xp_for_level spec WHERE base.player_lvl = spec.player_lvl AND build <= %u)", VERSION_STRING);

    if (player_xp_to_level_result == nullptr)
    {
        sLogger.info("MySQLDataLoads : Table `player_xp_for_level` is empty!");
        return;
    }

    sLogger.info("MySQLDataLoads : Table `player_xp_for_level` has %u columns", player_xp_to_level_result->GetFieldCount());

    uint32_t player_xp_to_level_count = 0;
    do
    {
        Field* fields = player_xp_to_level_result->Fetch();
        uint32_t current_level = fields[0].GetUInt8();
        uint32_t current_xp = fields[1].GetUInt32();

        if (current_level >= worldConfig.player.playerLevelCap)
        {
            sLogger.debugFlag(AscEmu::Logging::LF_DB_TABLES, "Table `player_xp_for_level` includes invalid xp definitions for level %u which is higher than the defined levelcap in your config file! <skipped>", current_level);
            continue;
        }

        _playerXPperLevelStore[current_level] = current_xp;

        ++player_xp_to_level_count;

    } while (player_xp_to_level_result->NextRow());

    delete player_xp_to_level_result;

    sLogger.info("MySQLDataLoads : Loaded %u rows from `player_xp_for_level` table in %u ms!", player_xp_to_level_count, static_cast<uint32_t>(Util::GetTimeDifferenceToNow(startTime)));

    if (player_xp_to_level_count < (worldConfig.player.playerLevelCap - 1))
        sLogger.failure("Table `player_xp_for_level` includes definitions for %u level, but your defined level cap is %u!", player_xp_to_level_count, worldConfig.player.playerLevelCap);
}

uint32_t MySQLDataStore::getPlayerXPForLevel(uint32_t level)
{
    if (level < _playerXPperLevelStore.size())
    {
        return _playerXPperLevelStore[level];
    }

    return 0;
}

void MySQLDataStore::loadSpellOverrideTable()
{
    QueryResult* spelloverride_result = WorldDatabase.Query("SELECT DISTINCT overrideId FROM spelloverride");
    if (spelloverride_result == nullptr)
    {
        sLogger.info("MySQLDataLoads : Table `spelloverride` is empty!");
        return;
    }

    do
    {
        Field* fields = spelloverride_result->Fetch();
        uint32_t distinct_override_id = fields[0].GetUInt32();

        QueryResult* spellid_for_overrideid_result = WorldDatabase.Query("SELECT spellId FROM spelloverride WHERE overrideId = %u", distinct_override_id);
        std::list<SpellInfo const*>* list = new std::list <SpellInfo const*>;
        if (spellid_for_overrideid_result != nullptr)
        {
            do
            {
                Field* fieldsIn = spellid_for_overrideid_result->Fetch();
                uint32_t spellid = fieldsIn[0].GetUInt32();
                SpellInfo const* spell = sSpellMgr.getSpellInfo(spellid);
                if (spell == nullptr)
                {
                    sLogger.debugFlag(AscEmu::Logging::LF_DB_TABLES, "Table `spelloverride` includes invalid spellId %u for overrideId %u! <skipped>", spellid, distinct_override_id);
                    continue;
                }

                list->push_back(spell);

            } while (spellid_for_overrideid_result->NextRow());

            delete spellid_for_overrideid_result;
        }

        if (list->size() == 0)
        {
            delete list;
        }
        else
        {
            _spellOverrideIdStore.emplace(SpellOverrideIdMap::value_type(distinct_override_id, list));
        }

    } while (spelloverride_result->NextRow());

    delete spelloverride_result;

    sLogger.info("MySQLDataLoads : %u spell overrides loaded.", static_cast<uint32_t>(_spellOverrideIdStore.size()));
}

void MySQLDataStore::loadNpcGossipTextIdTable()
{
    auto startTime = Util::TimeNow();
    //                                                    0         1
    QueryResult* npc_gossip_properties_result = WorldDatabase.Query("SELECT creatureid, textid FROM npc_gossip_properties");
    if (npc_gossip_properties_result == nullptr)
    {
        sLogger.info("MySQLDataLoads : Table `npc_gossip_properties` is empty!");
        return;
    }

    sLogger.info("MySQLDataLoads : Table `npc_gossip_properties` has %u columns", npc_gossip_properties_result->GetFieldCount());

    uint32_t npc_gossip_properties_count = 0;
    do
    {
        Field* fields = npc_gossip_properties_result->Fetch();
        uint32_t entry = fields[0].GetUInt32();
        auto creature_properties = sMySQLStore.getCreatureProperties(entry);
        if (creature_properties == nullptr)
        {
            sLogger.debugFlag(AscEmu::Logging::LF_DB_TABLES, "Table `npc_gossip_properties` includes invalid creatureid %u! <skipped>", entry);
            continue;
        }

        uint32_t text = fields[1].GetUInt32();

        _npcGossipTextIdStore[entry] = text;

        ++npc_gossip_properties_count;

    } while (npc_gossip_properties_result->NextRow());

    delete npc_gossip_properties_result;

    sLogger.info("MySQLDataLoads : Loaded %u rows from `npc_gossip_properties` table in %u ms!", npc_gossip_properties_count, static_cast<uint32_t>(Util::GetTimeDifferenceToNow(startTime)));
}

uint32_t MySQLDataStore::getGossipTextIdForNpc(uint32_t entry)
{
    return _npcGossipTextIdStore[entry];
}

void MySQLDataStore::loadPetLevelAbilitiesTable()
{
    auto startTime = Util::TimeNow();
    //                                                                      0       1      2        3        4        5         6         7
    QueryResult* pet_level_abilities_result = WorldDatabase.Query("SELECT level, health, armor, strength, agility, stamina, intellect, spirit FROM pet_level_abilities");
    if (pet_level_abilities_result == nullptr)
    {
        sLogger.info("MySQLDataLoads : Table `pet_level_abilities` is empty!");
        return;
    }

    sLogger.info("MySQLDataLoads : Table `pet_level_abilities` has %u columns", pet_level_abilities_result->GetFieldCount());

    _petLevelAbilitiesStore.rehash(pet_level_abilities_result->GetRowCount());

    uint32_t pet_level_abilities_count = 0;
    do
    {
        Field* fields = pet_level_abilities_result->Fetch();

        uint32_t entry = fields[0].GetUInt32();

        MySQLStructure::PetLevelAbilities& petAbilities = _petLevelAbilitiesStore[entry];

        petAbilities.level = entry;
        petAbilities.health = fields[1].GetUInt32();
        petAbilities.armor = fields[2].GetUInt32();
        petAbilities.strength = fields[3].GetUInt32();
        petAbilities.agility = fields[4].GetUInt32();
        petAbilities.stamina = fields[5].GetUInt32();
        petAbilities.intellect = fields[6].GetUInt32();
        petAbilities.spirit = fields[7].GetUInt32();

        ++pet_level_abilities_count;

    } while (pet_level_abilities_result->NextRow());

    delete pet_level_abilities_result;

    sLogger.info("MySQLDataLoads : Loaded %u rows from `pet_level_abilities` table in %u ms!", pet_level_abilities_count, static_cast<uint32_t>(Util::GetTimeDifferenceToNow(startTime)));

    if (pet_level_abilities_count < worldConfig.player.playerLevelCap)
        sLogger.failure("Table `pet_level_abilities` includes definitions for %u level, but your defined level cap is %u!", pet_level_abilities_count, worldConfig.player.playerLevelCap);
}

MySQLStructure::PetLevelAbilities const* MySQLDataStore::getPetLevelAbilities(uint32_t level)
{
    PetLevelAbilitiesContainer::const_iterator itr = _petLevelAbilitiesStore.find(level);
    if (itr != _petLevelAbilitiesStore.end())
        return &(itr->second);

    return nullptr;
}

void MySQLDataStore::loadBroadcastTable()
{
    auto startTime = Util::TimeNow();

    QueryResult* broadcast_result = WorldDatabase.Query("SELECT * FROM worldbroadcast");
    if (broadcast_result == nullptr)
    {
        sLogger.info("MySQLDataLoads : Table `worldbroadcast` is empty!");
        return;
    }

    sLogger.info("MySQLDataLoads : Table `worldbroadcast` has %u columns", broadcast_result->GetFieldCount());

    _worldBroadcastStore.rehash(broadcast_result->GetRowCount());

    uint32_t broadcast_count = 0;
    do
    {
        Field* fields = broadcast_result->Fetch();

        uint32_t entry = fields[0].GetUInt32();

        MySQLStructure::WorldBroadCast& broadcast = _worldBroadcastStore[entry];

        broadcast.id = entry;

        uint32_t interval = fields[1].GetUInt32();
        broadcast.interval = interval * 60;
        uint32_t random_interval = fields[2].GetUInt32();
        broadcast.randomInterval = random_interval * 60;
        broadcast.nextUpdate = broadcast.interval + (uint32_t)UNIXTIME;
        broadcast.text = fields[3].GetString();

        ++broadcast_count;

    } while (broadcast_result->NextRow());

    delete broadcast_result;

    sLogger.info("MySQLDataLoads : Loaded %u rows from `worldbroadcast` table in %u ms!", broadcast_count, static_cast<uint32_t>(Util::GetTimeDifferenceToNow(startTime)));
}

MySQLStructure::WorldBroadCast const* MySQLDataStore::getWorldBroadcastById(uint32_t id)
{
    WorldBroadcastContainer::const_iterator itr = _worldBroadcastStore.find(id);
    if (itr != _worldBroadcastStore.end())
        return &(itr->second);

    return nullptr;
}

void MySQLDataStore::loadAreaTriggerTable()
{
    auto startTime = Util::TimeNow();
    //                                                               0      1    2     3       4       5           6          7             8               9                  10
    QueryResult* area_trigger_result = WorldDatabase.Query("SELECT entry, type, map, screen, name, position_x, position_y, position_z, orientation, required_honor_rank, required_level FROM areatriggers");
    if (area_trigger_result == nullptr)
    {
        sLogger.info("MySQLDataLoads : Table `areatriggers` is empty!");
        return;
    }

    sLogger.info("MySQLDataLoads : Table `areatriggers` has %u columns", area_trigger_result->GetFieldCount());

    _areaTriggerStore.rehash(area_trigger_result->GetRowCount());

    uint32_t areaTrigger_count = 0;
    do
    {
        Field* fields = area_trigger_result->Fetch();

        MySQLStructure::AreaTrigger areaTrigger;
        areaTrigger.id = fields[0].GetUInt32();
        areaTrigger.type = fields[1].GetUInt8();
        areaTrigger.mapId = fields[2].GetUInt16();
        areaTrigger.pendingScreen = fields[3].GetUInt32();
        areaTrigger.name = fields[4].GetString();
        areaTrigger.x = fields[5].GetFloat();
        areaTrigger.y = fields[6].GetFloat();
        areaTrigger.z = fields[7].GetFloat();
        areaTrigger.o = fields[8].GetFloat();
        areaTrigger.requiredHonorRank = fields[9].GetUInt32();
        areaTrigger.requiredLevel = fields[10].GetUInt32();

        DBC::Structures::AreaTriggerEntry const* area_trigger_entry = sAreaTriggerStore.LookupEntry(areaTrigger.id);
        if (!area_trigger_entry)
        {
            sLogger.debugFlag(AscEmu::Logging::LF_DB_TABLES, "AreaTrigger : Area trigger (ID:%u) does not exist in `AreaTrigger.dbc`.", areaTrigger.id);
            continue;
        }

        DBC::Structures::MapEntry const* map_entry = sMapStore.LookupEntry(areaTrigger.mapId);
        if (!map_entry)
        {
            sLogger.debugFlag(AscEmu::Logging::LF_DB_TABLES, "AreaTrigger : Area trigger (ID:%u) target map (ID: %u) does not exist in `Map.dbc`.", areaTrigger.id, areaTrigger.mapId);
            continue;
        }

        if (areaTrigger.x == 0 && areaTrigger.y == 0 && areaTrigger.z == 0 && (areaTrigger.type == ATTYPE_INSTANCE || areaTrigger.type == ATTYPE_TELEPORT))    // check target coordinates only for teleport triggers
        {
            sLogger.debugFlag(AscEmu::Logging::LF_DB_TABLES, "AreaTrigger : Area trigger (ID:%u) target coordinates not provided.", areaTrigger.id);
            continue;
        }

        _areaTriggerStore[areaTrigger.id] = areaTrigger;
        ++areaTrigger_count;

    } while (area_trigger_result->NextRow());

    delete area_trigger_result;

    sLogger.info("MySQLDataLoads : Loaded %u rows from `areatriggers` table in %u ms!", areaTrigger_count, static_cast<uint32_t>(Util::GetTimeDifferenceToNow(startTime)));
}

MySQLStructure::AreaTrigger const* MySQLDataStore::getAreaTrigger(uint32_t entry)
{
    AreaTriggerContainer::const_iterator itr = _areaTriggerStore.find(entry);
    if (itr != _areaTriggerStore.end())
        return &(itr->second);

    return nullptr;
}

MySQLStructure::AreaTrigger const* MySQLDataStore::getMapEntranceTrigger(uint32_t mapId)
{
    for (AreaTriggerContainer::const_iterator itr = _areaTriggerStore.begin(); itr != _areaTriggerStore.end(); ++itr)
    {
        if (itr->second.mapId == mapId)
        {
            DBC::Structures::AreaTriggerEntry const* area_trigger_entry = sAreaTriggerStore.LookupEntry(itr->first);
            if (area_trigger_entry)
                return &itr->second;
        }
    }
    return nullptr;
}

#if VERSION_STRING > Classic
MySQLStructure::AreaTrigger const* MySQLDataStore::getMapGoBackTrigger(uint32_t mapId)
{
    bool useParentDbValue = false;
    uint32_t parentId = 0;
    DBC::Structures::MapEntry const* mapEntry = sMapStore.LookupEntry(mapId);
    if (!mapEntry || mapEntry->parent_map < 0)
        return nullptr;

    if (mapEntry->isDungeon())
    {
        auto const* iTemplate = sMySQLStore.getWorldMapInfo(mapId);

        if (!iTemplate)
            return nullptr;

        parentId = iTemplate->repopmapid;
        useParentDbValue = true;
    }

    uint32_t entrance_map = static_cast<uint32_t>(mapEntry->parent_map);
    for (AreaTriggerContainer::const_iterator itr = _areaTriggerStore.begin(); itr != _areaTriggerStore.end(); ++itr)
    {
        if ((!useParentDbValue && itr->second.mapId == entrance_map) || (useParentDbValue && itr->second.mapId == parentId))
        {
            DBC::Structures::AreaTriggerEntry const* atEntry = sAreaTriggerStore.LookupEntry(itr->first);
            if (atEntry && atEntry->mapid == mapId)
                return &itr->second;
        }
    }
    return nullptr;
}
#else
MySQLStructure::AreaTrigger const* MySQLDataStore::getMapGoBackTrigger(uint32_t mapId)
{
    bool useParentDbValue = false;
    uint32_t parentId = 0;
    auto const* mapEntry = sMySQLStore.getWorldMapInfo(mapId);
    if (!mapEntry || mapEntry->repopmapid < 0)
        return nullptr;

    if (mapEntry->isDungeon())
    {
        auto const* iTemplate = sMySQLStore.getWorldMapInfo(mapId);

        if (!iTemplate)
            return nullptr;

        parentId = iTemplate->repopmapid;
        useParentDbValue = true;
    }

    uint32_t entrance_map = static_cast<uint32_t>(mapEntry->repopmapid);
    for (AreaTriggerContainer::const_iterator itr = _areaTriggerStore.begin(); itr != _areaTriggerStore.end(); ++itr)
    {
        if ((!useParentDbValue && itr->second.mapId == entrance_map) || (useParentDbValue && itr->second.mapId == parentId))
        {
            DBC::Structures::AreaTriggerEntry const* atEntry = sAreaTriggerStore.LookupEntry(itr->first);
            if (atEntry && atEntry->mapid == mapId)
                return &itr->second;
        }
    }
    return nullptr;
}
#endif

void MySQLDataStore::loadWordFilterCharacterNames()
{
    auto startTime = Util::TimeNow();

    QueryResult* filter_character_names_result = WorldDatabase.Query("SELECT * FROM wordfilter_character_names");
    if (filter_character_names_result == nullptr)
    {
        sLogger.info("MySQLDataLoads : Table `wordfilter_character_names` is empty!");
        return;
    }

    sLogger.info("MySQLDataLoads : Table `wordfilter_character_names` has %u columns", filter_character_names_result->GetFieldCount());

    _wordFilterCharacterNamesStore.clear();

    uint32_t filter_character_names_count = 0;
    do
    {
        Field* fields = filter_character_names_result->Fetch();

        MySQLStructure::WordFilterCharacterNames wfCharacterNames;
        wfCharacterNames.name = fields[0].GetString();
        wfCharacterNames.nameReplace = fields[1].GetString();
        if (wfCharacterNames.nameReplace.empty())
        {
            wfCharacterNames.nameReplace = "?%$?%$";
        }

        _wordFilterCharacterNamesStore.push_back(wfCharacterNames);

        ++filter_character_names_count;

    } while (filter_character_names_result->NextRow());

    delete filter_character_names_result;

    sLogger.info("MySQLDataLoads : Loaded %u rows from `wordfilter_character_names` table in %u ms!", filter_character_names_count, static_cast<uint32_t>(Util::GetTimeDifferenceToNow(startTime)));
}

bool MySQLDataStore::isCharacterNameAllowed(std::string charName)
{
    std::list<MySQLStructure::WordFilterCharacterNames>::const_iterator iterator;
    for (iterator = _wordFilterCharacterNamesStore.begin(); iterator != _wordFilterCharacterNamesStore.end(); ++iterator)
    {
        size_t pos = charName.find(iterator->name);
        if (pos != std::string::npos)
        {
            return false;
        }
    }

    return true;
}

void MySQLDataStore::loadWordFilterChat()
{
    auto startTime = Util::TimeNow();

    QueryResult* filter_chat_result = WorldDatabase.Query("SELECT * FROM wordfilter_chat");
    if (filter_chat_result == nullptr)
    {
        sLogger.info("MySQLDataLoads : Table `wordfilter_chat` is empty!");
        return;
    }

    sLogger.info("MySQLDataLoads : Table `wordfilter_chat` has %u columns", filter_chat_result->GetFieldCount());

    _wordFilterChatStore.clear();

    uint32_t filter_chat_count = 0;
    do
    {
        Field* fields = filter_chat_result->Fetch();

        MySQLStructure::WordFilterChat wfChat;
        wfChat.word = fields[0].GetString();
        wfChat.wordReplace = fields[1].GetString();
        if (wfChat.wordReplace.empty())
        {
            wfChat.blockMessage = true;
        }
        else
        {
            wfChat.blockMessage = false;
        }

        _wordFilterChatStore.push_back(wfChat);

        ++filter_chat_count;

    } while (filter_chat_result->NextRow());

    delete filter_chat_result;

    sLogger.info("MySQLDataLoads : Loaded %u rows from `wordfilter_chat` table in %u ms!", filter_chat_count, static_cast<uint32_t>(Util::GetTimeDifferenceToNow(startTime)));
}

//////////////////////////////////////////////////////////////////////////////////////////
// locales
void MySQLDataStore::loadLocalesCreature()
{
    auto startTime = Util::TimeNow();
    //                                                0         1          2      3
    QueryResult* result = WorldDatabase.Query("SELECT id, language_code, name, subname FROM locales_creature");
    if (result == nullptr)
    {
        sLogger.info("MySQLDataLoads : Table `locales_creature` is empty!");
        return;
    }

    sLogger.info("MySQLDataLoads : Table `locales_creature` has %u columns", result->GetFieldCount());

    _localesCreatureStore.rehash(result->GetRowCount());

    uint32_t load_count = 0;
    uint32_t i = 0;
    do
    {
        ++i;
        Field* fields = result->Fetch();

        MySQLStructure::LocalesCreature& localCreature = _localesCreatureStore[i];

        localCreature.entry = fields[0].GetUInt32();
        std::string locString = fields[1].GetString();
        localCreature.languageCode = Util::getLanguagesIdFromString(locString);
        localCreature.name = strdup(fields[2].GetString());
        localCreature.subName = strdup(fields[3].GetString());

        ++load_count;

    } while (result->NextRow());

    delete result;

    sLogger.info("MySQLDataLoads : Loaded %u rows from `locales_creature` table in %u ms!", load_count, static_cast<uint32_t>(Util::GetTimeDifferenceToNow(startTime)));
}

MySQLStructure::LocalesCreature const* MySQLDataStore::getLocalizedCreature(uint32_t entry, uint32_t sessionLocale)
{
    for (LocalesCreatureContainer::const_iterator itr = _localesCreatureStore.begin(); itr != _localesCreatureStore.end(); ++itr)
    {
        if (itr->second.entry == entry)
        {
            if (itr->second.languageCode == sessionLocale)
            {
                return &itr->second;
            }
        }
    }
    return nullptr;
}

void MySQLDataStore::loadLocalesGameobject()
{
    auto startTime = Util::TimeNow();
    //                                                  0         1          2
    QueryResult* result = WorldDatabase.Query("SELECT entry, language_code, name FROM locales_gameobject");
    if (result == nullptr)
    {
        sLogger.info("MySQLDataLoads : Table `locales_gameobject` is empty!");
        return;
    }

    sLogger.info("MySQLDataLoads : Table `locales_gameobject` has %u columns", result->GetFieldCount());

    _localesGameobjectStore.rehash(result->GetRowCount());

    uint32_t load_count = 0;
    uint32_t i = 0;
    do
    {
        ++i;
        Field* fields = result->Fetch();

        MySQLStructure::LocalesGameobject& localGameobject = _localesGameobjectStore[i];

        localGameobject.entry = fields[0].GetUInt32();
        std::string locString = fields[1].GetString();
        localGameobject.languageCode = Util::getLanguagesIdFromString(locString);
        localGameobject.name = strdup(fields[2].GetString());

        ++load_count;

    } while (result->NextRow());

    delete result;

    sLogger.info("MySQLDataLoads : Loaded %u rows from `locales_gameobject` table in %u ms!", load_count, static_cast<uint32_t>(Util::GetTimeDifferenceToNow(startTime)));
}

MySQLStructure::LocalesGameobject const* MySQLDataStore::getLocalizedGameobject(uint32_t entry, uint32_t sessionLocale)
{
    for (LocalesGameobjectContainer::const_iterator itr = _localesGameobjectStore.begin(); itr != _localesGameobjectStore.end(); ++itr)
    {
        if (itr->second.entry == entry)
        {
            if (itr->second.languageCode == sessionLocale)
            {
                return &itr->second;
            }
        }
    }
    return nullptr;
}

void MySQLDataStore::loadLocalesGossipMenuOption()
{
    auto startTime = Util::TimeNow();
    //                                                   0         1             2
    QueryResult* result = WorldDatabase.Query("SELECT entry, language_code, option_text FROM locales_gossip_menu_option");
    if (result == nullptr)
    {
        sLogger.info("MySQLDataLoads : Table `locales_gossip_menu_option` is empty!");
        return;
    }

    sLogger.info("MySQLDataLoads : Table `locales_gossip_menu_option` has %u columns", result->GetFieldCount());

    _localesGossipMenuOptionStore.rehash(result->GetRowCount());

    uint32_t load_count = 0;
    uint32_t i = 0;
    do
    {
        ++i;
        Field* fields = result->Fetch();

        MySQLStructure::LocalesGossipMenuOption& localGossipMenuOption = _localesGossipMenuOptionStore[1];

        localGossipMenuOption.entry = fields[0].GetUInt32();
        std::string locString = fields[1].GetString();
        localGossipMenuOption.languageCode = Util::getLanguagesIdFromString(locString);
        localGossipMenuOption.name = strdup(fields[2].GetString());

        ++load_count;

    } while (result->NextRow());

    delete result;

    sLogger.info("MySQLDataLoads : Loaded %u rows from `locales_gossip_menu_option` table in %u ms!", load_count, static_cast<uint32_t>(Util::GetTimeDifferenceToNow(startTime)));
}

MySQLStructure::LocalesGossipMenuOption const* MySQLDataStore::getLocalizedGossipMenuOption(uint32_t entry, uint32_t sessionLocale)
{
    for (LocalesGossipMenuOptionContainer::const_iterator itr = _localesGossipMenuOptionStore.begin(); itr != _localesGossipMenuOptionStore.end(); ++itr)
    {
        if (itr->second.entry == entry)
        {
            if (itr->second.languageCode == sessionLocale)
            {
                return &itr->second;
            }
        }
    }
    return nullptr;
}

void MySQLDataStore::loadLocalesItem()
{
    auto startTime = Util::TimeNow();
    //                                                  0         1          2         3
    QueryResult* result = WorldDatabase.Query("SELECT entry, language_code, name, description FROM locales_item");
    if (result == nullptr)
    {
        sLogger.info("MySQLDataLoads : Table `locales_item` is empty!");
        return;
    }

    sLogger.info("MySQLDataLoads : Table `locales_item` has %u columns", result->GetFieldCount());

    _localesItemStore.rehash(result->GetRowCount());

    uint32_t load_count = 0;
    uint32_t i = 0;
    do
    {
        ++i;
        Field* fields = result->Fetch();

        MySQLStructure::LocalesItem& localItem = _localesItemStore[i];

        localItem.entry = fields[0].GetUInt32();
        std::string locString = fields[1].GetString();
        localItem.languageCode = Util::getLanguagesIdFromString(locString);
        localItem.name = strdup(fields[2].GetString());
        localItem.description = strdup(fields[3].GetString());

        ++load_count;

    } while (result->NextRow());

    delete result;

    sLogger.info("MySQLDataLoads : Loaded %u rows from `locales_item` table in %u ms!", load_count, static_cast<uint32_t>(Util::GetTimeDifferenceToNow(startTime)));
}

MySQLStructure::LocalesItem const* MySQLDataStore::getLocalizedItem(uint32_t entry, uint32_t sessionLocale)
{
    for (LocalesItemContainer::const_iterator itr = _localesItemStore.begin(); itr != _localesItemStore.end(); ++itr)
    {
        if (itr->second.entry == entry)
        {
            if (itr->second.languageCode == sessionLocale)
            {
                return &itr->second;
            }
        }
    }
    return nullptr;
}

MySQLStructure::RecallStruct const* MySQLDataStore::getRecallByName(const std::string name)
{
    std::string searchName(name);
    AscEmu::Util::Strings::toLowerCase(searchName);

    for (auto itr : _recallStore)
    {
        std::string recallName(itr->name);
        AscEmu::Util::Strings::toLowerCase(recallName);
        if (recallName == searchName)
            return itr;
    }

    return nullptr;
}

void MySQLDataStore::loadLocalesItemPages()
{
    auto startTime = Util::TimeNow();
    //                                                 0         1           2
    QueryResult* result = WorldDatabase.Query("SELECT entry, language_code, text FROM locales_item_pages");
    if (result == nullptr)
    {
        sLogger.info("MySQLDataLoads : Table `locales_item_pages` is empty!");
        return;
    }

    sLogger.info("MySQLDataLoads : Table `locales_item_pages` has %u columns", result->GetFieldCount());

    _localesItemPagesStore.rehash(result->GetRowCount());

    uint32_t load_count = 0;
    uint32_t i = 0;
    do
    {
        ++i;
        Field* fields = result->Fetch();

        MySQLStructure::LocalesItemPages& localesItemPages = _localesItemPagesStore[i];

        localesItemPages.entry = fields[0].GetUInt32();
        std::string locString = fields[1].GetString();
        localesItemPages.languageCode = Util::getLanguagesIdFromString(locString);
        localesItemPages.text = strdup(fields[2].GetString());

        ++load_count;

    } while (result->NextRow());

    delete result;

    sLogger.info("MySQLDataLoads : Loaded %u rows from `locales_item_pages` table in %u ms!", load_count, static_cast<uint32_t>(Util::GetTimeDifferenceToNow(startTime)));
}

MySQLStructure::LocalesItemPages const* MySQLDataStore::getLocalizedItemPages(uint32_t entry, uint32_t sessionLocale)
{
    for (LocalesItemPagesContainer::const_iterator itr = _localesItemPagesStore.begin(); itr != _localesItemPagesStore.end(); ++itr)
    {
        if (itr->second.entry == entry)
        {
            if (itr->second.languageCode == sessionLocale)
            {
                return &itr->second;
            }
        }
    }
    return nullptr;
}

void MySQLDataStore::loadLocalesNpcScriptText()
{
    auto startTime = Util::TimeNow();
    //                                                  0         1          2
    QueryResult* result = WorldDatabase.Query("SELECT entry, language_code, text FROM locales_npc_script_text");
    if (result == nullptr)
    {
        sLogger.info("MySQLDataLoads : Table `locales_npc_script_text` is empty!");
        return;
    }

    sLogger.info("MySQLDataLoads : Table `locales_npc_script_text` has %u columns", result->GetFieldCount());

    _localesNpcScriptTextStore.rehash(result->GetRowCount());

    uint32_t load_count = 0;
    uint32_t i = 0;
    do
    {
        ++i;
        Field* fields = result->Fetch();

        MySQLStructure::LocalesNpcScriptText& localNpcScriptText = _localesNpcScriptTextStore[i];

        localNpcScriptText.entry = fields[0].GetUInt32();
        std::string locString = fields[1].GetString();
        localNpcScriptText.languageCode = Util::getLanguagesIdFromString(locString);
        localNpcScriptText.text = strdup(fields[2].GetString());

        ++load_count;

    } while (result->NextRow());

    delete result;

    sLogger.info("MySQLDataLoads : Loaded %u rows from `locales_npc_script_text` table in %u ms!", load_count, static_cast<uint32_t>(Util::GetTimeDifferenceToNow(startTime)));
}

MySQLStructure::LocalesNpcScriptText const* MySQLDataStore::getLocalizedNpcScriptText(uint32_t entry, uint32_t sessionLocale)
{
    for (LocalesNpcScriptTextContainer::const_iterator itr = _localesNpcScriptTextStore.begin(); itr != _localesNpcScriptTextStore.end(); ++itr)
    {
        if (itr->second.entry == entry)
        {
            if (itr->second.languageCode == sessionLocale)
            {
                return &itr->second;
            }
        }
    }
    return nullptr;
}

void MySQLDataStore::loadLocalesNpcText()
{
    auto startTime = Util::TimeNow();
    //                                                  0         1           2       3       4       5       6       7       8       9       10      11     12      13      14      15      16      17
    QueryResult* result = WorldDatabase.Query("SELECT entry, language_code, text0, text0_1, text1, text1_1, text2, text2_1, text3, text3_1, text4, text4_1, text5, text5_1, text6, text6_1, text7, text7_1 FROM locales_npc_gossip_texts");
    if (result == nullptr)
    {
        sLogger.info("MySQLDataLoads : Table `locales_npc_gossip_texts` is empty!");
        return;
    }

    sLogger.info("MySQLDataLoads : Table `locales_npc_gossip_texts` has %u columns", result->GetFieldCount());

    _localesNpcGossipTextStore.rehash(result->GetRowCount());

    uint32_t load_count = 0;
    uint32_t i = 0;
    do
    {
        ++i;
        Field* fields = result->Fetch();

        MySQLStructure::LocalesNpcGossipText& localNpcGossipText = _localesNpcGossipTextStore[i];

        localNpcGossipText.entry = fields[0].GetUInt32();
        std::string locString = fields[1].GetString();
        localNpcGossipText.languageCode = Util::getLanguagesIdFromString(locString);

        for (uint8 j = 0; j < 8; ++j)
        {
            localNpcGossipText.texts[j][0] = strdup(fields[2 + (2 * j)].GetString());
            localNpcGossipText.texts[j][1] = strdup(fields[3 + (2 * j)].GetString());
        }

        ++load_count;

    } while (result->NextRow());

    delete result;

    sLogger.info("MySQLDataLoads : Loaded %u rows from `locales_npc_gossip_texts` table in %u ms!", load_count, static_cast<uint32_t>(Util::GetTimeDifferenceToNow(startTime)));
}

MySQLStructure::LocalesNpcGossipText const* MySQLDataStore::getLocalizedNpcGossipText(uint32_t entry, uint32_t sessionLocale) const
{
    for (LocalesNpcGossipTextContainer::const_iterator itr = _localesNpcGossipTextStore.begin(); itr != _localesNpcGossipTextStore.end(); ++itr)
    {
        if (itr->second.entry == entry)
        {
            if (itr->second.languageCode == sessionLocale)
            {
                return &itr->second;
            }
        }
    }
    return nullptr;
}

void MySQLDataStore::loadLocalesQuest()
{
    auto startTime = Util::TimeNow();
    //                                                  0         1           2       3         4            5                 6           7           8                9              10             11
    QueryResult* result = WorldDatabase.Query("SELECT entry, language_code, Title, Details, Objectives, CompletionText, IncompleteText, EndText, ObjectiveText1, ObjectiveText2, ObjectiveText3, ObjectiveText4 FROM locales_quest");
    if (result == nullptr)
    {
        sLogger.info("MySQLDataLoads : Table `locales_quest` is empty!");
        return;
    }

    sLogger.info("MySQLDataLoads : Table `locales_quest` has %u columns", result->GetFieldCount());

    _localesQuestStore.rehash(result->GetRowCount());

    uint32_t load_count = 0;
    uint32_t i = 0;
    do
    {
        ++i;
        Field* fields = result->Fetch();

        MySQLStructure::LocalesQuest& localQuest = _localesQuestStore[i];

        localQuest.entry = fields[0].GetUInt32();
        std::string locString = fields[1].GetString();
        localQuest.languageCode = Util::getLanguagesIdFromString(locString);
        localQuest.title = strdup(fields[2].GetString());
        localQuest.details = strdup(fields[3].GetString());
        localQuest.objectives = strdup(fields[4].GetString());
        localQuest.completionText = strdup(fields[5].GetString());
        localQuest.incompleteText = strdup(fields[6].GetString());
        localQuest.endText = strdup(fields[7].GetString());
        localQuest.objectiveText[0] = strdup(fields[8].GetString());
        localQuest.objectiveText[1] = strdup(fields[9].GetString());
        localQuest.objectiveText[2] = strdup(fields[10].GetString());
        localQuest.objectiveText[3] = strdup(fields[11].GetString());

        ++load_count;

    } while (result->NextRow());

    delete result;

    sLogger.info("MySQLDataLoads : Loaded %u rows from `locales_quest` table in %u ms!", load_count, static_cast<uint32_t>(Util::GetTimeDifferenceToNow(startTime)));
}

MySQLStructure::LocalesQuest const* MySQLDataStore::getLocalizedQuest(uint32_t entry, uint32_t sessionLocale)
{
    for (LocalesQuestContainer::const_iterator itr = _localesQuestStore.begin(); itr != _localesQuestStore.end(); ++itr)
    {
        if (itr->second.entry == entry)
        {
            if (itr->second.languageCode == sessionLocale)
            {
                return &itr->second;
            }
        }
    }
    return nullptr;
}

void MySQLDataStore::loadLocalesWorldbroadcast()
{
    auto startTime = Util::TimeNow();
    //                                                  0         1          2
    QueryResult* result = WorldDatabase.Query("SELECT entry, language_code, text FROM locales_worldbroadcast");
    if (result == nullptr)
    {
        sLogger.info("MySQLDataLoads : Table `locales_worldbroadcast` is empty!");
        return;
    }

    sLogger.info("MySQLDataLoads : Table `locales_worldbroadcast` has %u columns", result->GetFieldCount());

    _localesWorldbroadcastStore.rehash(result->GetRowCount());

    uint32_t load_count = 0;
    uint32_t i = 0;
    do
    {
        ++i;
        Field* fields = result->Fetch();

        MySQLStructure::LocalesWorldbroadcast& localWorldbroadcast = _localesWorldbroadcastStore[i];

        localWorldbroadcast.entry = fields[0].GetUInt32();
        std::string locString = fields[1].GetString();
        localWorldbroadcast.languageCode = Util::getLanguagesIdFromString(locString);
        localWorldbroadcast.text = strdup(fields[2].GetString());

        ++load_count;

    } while (result->NextRow());

    delete result;

    sLogger.info("MySQLDataLoads : Loaded %u rows from `locales_worldbroadcast` table in %u ms!", load_count, static_cast<uint32_t>(Util::GetTimeDifferenceToNow(startTime)));
}

MySQLStructure::LocalesWorldbroadcast const* MySQLDataStore::getLocalizedWorldbroadcast(uint32_t entry, uint32_t sessionLocale)
{
    for (LocalesWorldbroadcastContainer::const_iterator itr = _localesWorldbroadcastStore.begin(); itr != _localesWorldbroadcastStore.end(); ++itr)
    {
        if (itr->second.entry == entry)
        {
            if (itr->second.languageCode == sessionLocale)
            {
                return &itr->second;
            }
        }
    }
    return nullptr;
}

void MySQLDataStore::loadLocalesWorldmapInfo()
{
    auto startTime = Util::TimeNow();
    //                                                  0           1         2
    QueryResult* result = WorldDatabase.Query("SELECT entry, language_code, text FROM locales_worldmap_info");
    if (result == nullptr)
    {
        sLogger.info("MySQLDataLoads : Table `locales_worldmap_info` is empty!");
        return;
    }

    sLogger.info("MySQLDataLoads : Table `locales_worldmap_info` has %u columns", result->GetFieldCount());

    _localesWorldmapInfoStore.rehash(result->GetRowCount());

    uint32_t load_count = 0;
    uint32_t i = 0;
    do
    {
        ++i;
        Field* fields = result->Fetch();

        MySQLStructure::LocalesWorldmapInfo& localWorldmapInfo = _localesWorldmapInfoStore[i];

        localWorldmapInfo.entry = fields[0].GetUInt32();
        std::string locString = fields[1].GetString();
        localWorldmapInfo.languageCode = Util::getLanguagesIdFromString(locString);
        localWorldmapInfo.text = strdup(fields[2].GetString());

        ++load_count;

    } while (result->NextRow());

    delete result;

    sLogger.info("MySQLDataLoads : Loaded %u rows from `locales_worldmap_info` table in %u ms!", load_count, static_cast<uint32_t>(Util::GetTimeDifferenceToNow(startTime)));
}

MySQLStructure::LocalesWorldmapInfo const* MySQLDataStore::getLocalizedWorldmapInfo(uint32_t entry, uint32_t sessionLocale)
{
    for (LocalesWorldmapInfoContainer::const_iterator itr = _localesWorldmapInfoStore.begin(); itr != _localesWorldmapInfoStore.end(); ++itr)
    {
        if (itr->second.entry == entry)
        {
            if (itr->second.languageCode == sessionLocale)
            {
                return &itr->second;
            }
        }
    }
    return nullptr;
}

void MySQLDataStore::loadLocalesWorldStringTable()
{
    auto startTime = Util::TimeNow();
    //                                                  0           1         2
    QueryResult* result = WorldDatabase.Query("SELECT entry, language_code, text FROM locales_worldstring_table");
    if (result == nullptr)
    {
        sLogger.info("MySQLDataLoads : Table `locales_worldstring_table` is empty!");
        return;
    }

    sLogger.info("MySQLDataLoads : Table `locales_worldstring_table` has %u columns", result->GetFieldCount());

    _localesWorldStringTableStore.rehash(result->GetRowCount());

    uint32_t load_count = 0;
    uint32_t i = 0;
    do
    {
        ++i;
        Field* fields = result->Fetch();

        MySQLStructure::LocalesWorldStringTable& localWorldStringTable = _localesWorldStringTableStore[i];

        localWorldStringTable.entry = fields[0].GetUInt32();
        std::string locString = fields[1].GetString();
        localWorldStringTable.languageCode = Util::getLanguagesIdFromString(locString);
        localWorldStringTable.text = strdup(fields[2].GetString());

        ++load_count;

    } while (result->NextRow());

    delete result;

    sLogger.info("MySQLDataLoads : Loaded %u rows from `locales_worldstring_table` table in %u ms!", load_count, static_cast<uint32_t>(Util::GetTimeDifferenceToNow(startTime)));
}

MySQLStructure::LocalesWorldStringTable const* MySQLDataStore::getLocalizedWorldStringTable(uint32_t entry, uint32_t sessionLocale)
{
    for (LocalesWorldStringTableContainer::const_iterator itr = _localesWorldStringTableStore.begin(); itr != _localesWorldStringTableStore.end(); ++itr)
    {
        if (itr->second.entry == entry)
        {
            if (itr->second.languageCode == sessionLocale)
            {
                return &itr->second;
            }
        }
    }
    return nullptr;
}

std::string MySQLDataStore::getLocaleGossipMenuOptionOrElse(uint32_t entry, uint32_t sessionLocale)
{
    const auto wst = sMySQLStore.getGossipMenuOption(entry);
    const auto lpi = (sessionLocale > 0) ? sMySQLStore.getLocalizedGossipMenuOption(entry, sessionLocale) : nullptr;
    if (lpi != nullptr)
        return lpi->name;

    if (wst)
        return wst->text;

    std::stringstream errorMsg;
    errorMsg << "GossipMenuItem ID " << entry << "not available in database";
    return errorMsg.str();
}

std::string MySQLDataStore::getLocaleGossipTitleOrElse(uint32_t entry, uint32_t sessionLocale)
{
    const auto wst = sMySQLStore.getQuestProperties(entry);
    const auto lpi = (sessionLocale > 0) ? sMySQLStore.getLocalizedQuest(entry, sessionLocale) : nullptr;
    if (lpi != nullptr)
        return lpi->title;

    if (wst)
        return wst->title;

    std::stringstream errorMsg;
    errorMsg << "Quest ID " << entry << "not available in database";
    return errorMsg.str();
}

//\brief Data loaded but never used!    Zyres 2017/07/16 not used
//void MySQLDataStore::loadDefaultPetSpellsTable()
//{
//    auto startTime = Util::TimeNow();
//    //                                                  0      1
//    QueryResult* result = WorldDatabase.Query("SELECT entry, spell FROM petdefaultspells");
//    if (result == nullptr)
//    {
//        sLogger.info("MySQLDataLoads : Table `petdefaultspells` is empty!");
//        return;
//    }
//
//    sLogger.info("MySQLDataLoads : Table `petdefaultspells` has %u columns", result->GetFieldCount());
//
//    uint32_t load_count = 0;
//    do
//    {
//        Field* fields = result->Fetch();
//        uint32 entry = fields[0].GetUInt32();
//        uint32 spell = fields[1].GetUInt32();
//        const auto spellInfo = sSpellMgr.getSpellInfo(spell);
//
//        if (spell && entry && spellInfo)
//        {
//            PetDefaultSpellsMap::iterator itr = _defaultPetSpellsStore.find(entry);
//            if (itr != _defaultPetSpellsStore.end())
//            {
//                itr->second.insert(spellInfo);
//            }
//            else
//            {
//                std::set<SpellInfo const*> spellInfoSet;
//                spellInfoSet.insert(spellInfo);
//                _defaultPetSpellsStore[entry] = spellInfoSet;
//            }
//        }
//    } while (result->NextRow());
//
//    delete result;
//
//    sLogger.info("MySQLDataLoads : Loaded %u rows from `petdefaultspells` table in %u ms!", load_count, static_cast<uint32_t>(Util::GetTimeDifferenceToNow(startTime)));
//}

//\brief This function is never called!     Zyres 2017/07/16 not used
//std::set<SpellInfo const*>* MySQLDataStore::getDefaultPetSpellsByEntry(uint32_t entry)
//{
//    PetDefaultSpellsMap::iterator itr = _defaultPetSpellsStore.find(entry);
//    if (itr == _defaultPetSpellsStore.end())
//    {
//        return nullptr;
//    }
//
//    return &(itr->second);
//}

void MySQLDataStore::loadProfessionDiscoveriesTable()
{
    auto startTime = Util::TimeNow();
    //                                                   0           1              2          3
    QueryResult* result = WorldDatabase.Query("SELECT SpellId, SpellToDiscover, SkillValue, Chance FROM professiondiscoveries");
    if (result == nullptr)
    {
        sLogger.info("MySQLDataLoads : Table `professiondiscoveries` is empty!");
        return;
    }

    sLogger.info("MySQLDataLoads : Table `professiondiscoveries` has %u columns", result->GetFieldCount());

    uint32_t load_count = 0;
    do
    {
        Field* fields = result->Fetch();
        MySQLStructure::ProfessionDiscovery* professionDiscovery = new MySQLStructure::ProfessionDiscovery;
        professionDiscovery->SpellId = fields[0].GetUInt32();
        professionDiscovery->SpellToDiscover = fields[1].GetUInt32();
        professionDiscovery->SkillValue = fields[2].GetUInt32();
        professionDiscovery->Chance = fields[3].GetFloat();
        _professionDiscoveryStore.insert(professionDiscovery);

        ++load_count;

    } while (result->NextRow());

    delete result;

    sLogger.info("MySQLDataLoads : Loaded %u rows from `professiondiscoveries` table in %u ms!", load_count, static_cast<uint32_t>(Util::GetTimeDifferenceToNow(startTime)));
}

void MySQLDataStore::loadTransportDataTable()
{
    auto startTime = Util::TimeNow();
    //                                                  0      1
    QueryResult* result = WorldDatabase.Query("SELECT entry, name FROM transport_data WHERE min_build <= %u AND max_build >= %u", getAEVersion(), getAEVersion());
    if (result == nullptr)
    {
        sLogger.info("MySQLDataLoads : Table `transport_data` is empty!");
        return;
    }

    sLogger.info("MySQLDataLoads : Table `transport_data` has %u columns", result->GetFieldCount());

    uint32_t load_count = 0;
    do
    {
        Field* fields = result->Fetch();
        uint32_t entry = fields[0].GetUInt32();

        GameObjectProperties const* gameobject_info = sMySQLStore.getGameObjectProperties(entry);
        if (gameobject_info == nullptr)
        {
            sLogger.failure("Transport entry: %u, will not be loaded, gameobject_properties missing", entry);
            continue;
        }

        if (gameobject_info->type != GAMEOBJECT_TYPE_MO_TRANSPORT)
        {
            sLogger.failure("Transport entry: %u, will not be loaded, gameobject_properties type wrong", entry);
            continue;
        }

        MySQLStructure::TransportData& transportData = _transportDataStore[entry];
        transportData.entry = entry;
        transportData.name = fields[1].GetString();

        ++load_count;

    } while (result->NextRow());

    delete result;

    sLogger.info("MySQLDataLoads : Loaded %u rows from `transport_data` table in %u ms!", load_count, static_cast<uint32_t>(Util::GetTimeDifferenceToNow(startTime)));
}

void MySQLDataStore::loadTransportEntrys()
{
    auto startTime = Util::TimeNow();
    //                                                  
    QueryResult* result = WorldDatabase.Query("SELECT entry FROM gameobject_properties WHERE type = 15 AND  build <= %u ORDER BY entry ASC", getAEVersion());
    if (result == nullptr)
    {
        sLogger.info("MySQLDataLoads : Loaded 0 transport templates. DB table `gameobject_properties` has no transports!");
        return;
    }

    uint32_t load_count = 0;
    do
    {
        Field* fields = result->Fetch();
        uint32_t entry = fields[0].GetUInt32();

        MySQLStructure::TransportEntrys& transportEntrys = _transportEntryStore[entry];
        transportEntrys.entry = entry;

        ++load_count;

    } while (result->NextRow());

    delete result;

    sLogger.info("MySQLDataLoads : Loaded %u rows from `transport_entrys` table in %u ms!", load_count, static_cast<uint32_t>(Util::GetTimeDifferenceToNow(startTime)));
}

void MySQLDataStore::loadTransportMaps()
{
    auto startTime = Util::TimeNow();
    //                                                  
    QueryResult* result = WorldDatabase.Query("SELECT parameter_6 FROM gameobject_properties WHERE type = 15 AND  build <= %u ORDER BY entry ASC", getAEVersion());
    if (result == nullptr)
    {
        sLogger.info("MySQLDataLoads : Loaded 0 transport maps. DB table `gameobject_properties` has no transports!");
        return;
    }

    uint32_t load_count = 0;
    do
    {
        Field* fields = result->Fetch();
        uint32_t mapId = fields[0].GetUInt32();

        _transportMapStore.push_back(mapId);

        ++load_count;

    } while (result->NextRow());

    delete result;

    sLogger.info("MySQLDataLoads : Loaded %u maps from `transport_maps` table in %u ms!", load_count, static_cast<uint32_t>(Util::GetTimeDifferenceToNow(startTime)));
}

void MySQLDataStore::loadGossipMenuItemsTable()
{
    auto startTime = Util::TimeNow();

    //                                                      0          1
    QueryResult* result = WorldDatabase.Query("SELECT gossip_menu, text_id FROM gossip_menu ORDER BY gossip_menu");
    if (result == nullptr)
    {
        sLogger.info("MySQLDataLoads : Table `gossip_menu` is empty!");
        return;
    }

    sLogger.info("MySQLDataLoads : Table `gossip_menu` has %u columns", result->GetFieldCount());

    uint32_t load_count = 0;
    do
    {
        Field* fields = result->Fetch();
        uint32_t entry = fields[0].GetUInt32();

        MySQLStructure::GossipMenuInit& gMenuItem = _gossipMenuInitStore[entry];
        gMenuItem.gossipMenu = entry;
        gMenuItem.textId = fields[1].GetUInt32();

        ++load_count;
    } while (result->NextRow());

    delete result;

    sLogger.info("MySQLDataLoads : Loaded %u rows from `gossip_menu` table in %u ms!", load_count, static_cast<uint32_t>(Util::GetTimeDifferenceToNow(startTime)));

    _gossipMenuItemsStores.clear();

    //                                                      0       1            2        3            4                5               6               7                 8                9                10                11                 12
    QueryResult* resultItems = WorldDatabase.Query("SELECT id, item_order, menu_option, icon, on_choose_action, on_choose_data, on_choose_data2, on_choose_data3, on_choose_data4, next_gossip_menu, next_gossip_text, requirement_type, requirement_data FROM gossip_menu_items ORDER BY id, item_order");
    if (resultItems == nullptr)
    {
        sLogger.info("MySQLDataLoads : Table `gossip_menu_items` is empty!");
        return;
    }

    sLogger.info("MySQLDataLoads : Table `gossip_menu_items` has %u columns", resultItems->GetFieldCount());

    load_count = 0;
    do
    {
        Field* fields = resultItems->Fetch();

        MySQLStructure::GossipMenuItems gMenuItem;

        gMenuItem.gossipMenu = fields[0].GetUInt32();
        gMenuItem.itemOrder = fields[1].GetUInt32();
        gMenuItem.menuOptionText = fields[2].GetUInt32();
        gMenuItem.icon = fields[3].GetUInt8();
        gMenuItem.onChooseAction = fields[4].GetUInt8();
        gMenuItem.onChooseData = fields[5].GetUInt32();
        gMenuItem.onChooseData2 = fields[6].GetUInt32();
        gMenuItem.onChooseData3 = fields[7].GetUInt32();
        gMenuItem.onChooseData4 = fields[8].GetUInt32();
        gMenuItem.nextGossipMenu = fields[9].GetUInt32();
        gMenuItem.nextGossipMenuText = fields[10].GetUInt32();
        gMenuItem.requirementType = fields[11].GetUInt8();
        gMenuItem.requirementData = fields[12].GetUInt32();

        _gossipMenuItemsStores.emplace(GossipMenuItemsContainer::value_type(gMenuItem.gossipMenu, gMenuItem));
        ++load_count;
    } while (resultItems->NextRow());

    delete resultItems;

    sLogger.info("MySQLDataLoads : Loaded %u rows from `gossip_menu_items` table in %u ms!", load_count, static_cast<uint32_t>(Util::GetTimeDifferenceToNow(startTime)));
}

void MySQLDataStore::loadCreatureSpawns()
{
    auto startTime = Util::TimeNow();
    uint32_t count = 0;

    QueryResult* creature_spawn_result = sMySQLStore.getWorldDBQuery("SELECT * FROM creature_spawns WHERE min_build <= %u AND max_build >= %u AND event_entry = 0", getAEVersion(), getAEVersion());
    if (creature_spawn_result)
    {
        uint32 creature_spawn_fields = creature_spawn_result->GetFieldCount();
        if (creature_spawn_fields != CREATURE_SPAWNS_FIELDCOUNT + 2 + 2)
        {
            sLogger.failure("Table `creature_spawns` has %u columns, but needs %u columns! Skipped!", creature_spawn_fields, CREATURE_SPAWNS_FIELDCOUNT);
            return;
        }
        else
        {
            do
            {
                Field* fields = creature_spawn_result->Fetch();
                MySQLStructure::CreatureSpawn* cspawn = new MySQLStructure::CreatureSpawn;
                cspawn->id = fields[0].GetUInt32();

                uint32 creature_entry = fields[3].GetUInt32();
                auto creature_properties = sMySQLStore.getCreatureProperties(creature_entry);
                if (creature_properties == nullptr)
                {
                    sLogger.debugFlag(AscEmu::Logging::LF_DB_TABLES, "Creature spawn ID: %u has invalid entry: %u which is not in creature_properties table! Skipped loading.", cspawn->id, creature_entry);
                    continue;
                }

                cspawn->entry = creature_entry;
                cspawn->mapId = fields[4].GetUInt32();
                cspawn->x = fields[5].GetFloat();
                cspawn->y = fields[6].GetFloat();
                cspawn->z = fields[7].GetFloat();
                cspawn->o = fields[8].GetFloat();
                cspawn->movetype = fields[9].GetUInt8();
                cspawn->displayid = fields[10].GetUInt32();
                if (cspawn->displayid != 0 && !creature_properties->isTriggerNpc)
                {
                    const auto* creature_display = sObjectMgr.getCreatureDisplayInfoData(cspawn->displayid);
                    if (!creature_display)
                    {
                        sLogger.debugFlag(AscEmu::Logging::LF_DB_TABLES, "Table creature_spawns includes invalid displayid %u for npc entry: %u, spawn_id: %u. Set to a random modelid!", cspawn->displayid, cspawn->entry, cspawn->id);
                        cspawn->displayid = creature_properties->getRandomModelId();
                    }
                }
                else
                {
                    cspawn->displayid = creature_properties->getRandomModelId();
                }

                cspawn->factionid = fields[11].GetUInt32();
                cspawn->flags = fields[12].GetUInt32();
                cspawn->bytes0 = fields[13].GetUInt32();
                cspawn->bytes1 = fields[14].GetUInt32();
                cspawn->bytes2 = fields[15].GetUInt32();
                cspawn->emote_state = fields[16].GetUInt32();
                //cspawn->respawnNpcLink = fields[17].GetUInt32();
                cspawn->channel_spell = fields[18].GetUInt16();
                cspawn->channel_target_go = fields[19].GetUInt32();
                cspawn->channel_target_creature = fields[20].GetUInt32();
                cspawn->stand_state = fields[21].GetUInt16();
                cspawn->death_state = fields[22].GetUInt32();
                cspawn->MountedDisplayID = fields[23].GetUInt32();

                cspawn->Item1SlotEntry = fields[24].GetUInt32();
                cspawn->Item2SlotEntry = fields[25].GetUInt32();
                cspawn->Item3SlotEntry = fields[26].GetUInt32();

                cspawn->Item1SlotDisplay = sMySQLStore.getItemDisplayIdForEntry(cspawn->Item1SlotEntry);
                cspawn->Item2SlotDisplay = sMySQLStore.getItemDisplayIdForEntry(cspawn->Item2SlotEntry);
                cspawn->Item3SlotDisplay = sMySQLStore.getItemDisplayIdForEntry(cspawn->Item3SlotEntry);

                cspawn->CanFly = fields[27].GetUInt32();

                cspawn->phase = fields[28].GetUInt32();
                if (cspawn->phase == 0)
                    cspawn->phase = 0xFFFFFFFF;

                cspawn->wander_distance = fields[30].GetUInt32();
                cspawn->waypoint_id = fields[31].GetUInt32();

                cspawn->table = "creature_spawns";

                //\todo add flag to declare a spawn as static. E.g. gameobject_spawns
                /*if (!stricmp((*tableiterator).c_str(), "creature_staticspawns"))
                {
                    staticSpawns.CreatureSpawns.push_back(cspawn);
                    ++CreatureSpawnCount;
                }*/

                _creatureSpawnsStore[cspawn->mapId].push_back(cspawn);
                ++count;

            } while (creature_spawn_result->NextRow());
        }

        delete creature_spawn_result;
    }

    sLogger.info("MySQLDataLoads : Loaded %u rows from `creature_spawns` table in %u ms!", count, static_cast<uint32_t>(Util::GetTimeDifferenceToNow(startTime)));
}

void MySQLDataStore::loadGameobjectSpawns()
{
    auto startTime = Util::TimeNow();
    uint32_t count = 0;

    QueryResult* gobject_spawn_result = WorldDatabase.Query("SELECT * FROM gameobject_spawns WHERE min_build <= %u AND max_build >= %u AND event_entry = 0", VERSION_STRING, VERSION_STRING);
    if (gobject_spawn_result)
    {
        uint32 gobject_spawn_fields = gobject_spawn_result->GetFieldCount();
        if (gobject_spawn_fields != GO_SPAWNS_FIELDCOUNT + 1 + 2)
        {
            sLogger.failure("Table `gameobject_spawns` has %u columns, but needs %u columns! Skipped!", gobject_spawn_fields, GO_SPAWNS_FIELDCOUNT);
            return;
        }
        else
        {
            do
            {
                Field* fields = gobject_spawn_result->Fetch();
                MySQLStructure::GameobjectSpawn* go_spawn = new MySQLStructure::GameobjectSpawn;
                go_spawn->id = fields[0].GetUInt32();

                uint32 gameobject_entry = fields[3].GetUInt32();
                auto gameobject_info = sMySQLStore.getGameObjectProperties(gameobject_entry);
                if (gameobject_info == nullptr)
                {
                    sLogger.debugFlag(AscEmu::Logging::LF_DB_TABLES, "Gameobject spawn ID: %u has invalid entry: %u which is not in gameobject_properties table! Skipped loading.", go_spawn->id, gameobject_entry);
                    continue;
                }

#if VERSION_STRING == TBC
                //\ brief: the following 3 go types crashing tbc
                switch (gameobject_info->type)
                {
                    //case GAMEOBJECT_TYPE_TRANSPORT:
                case GAMEOBJECT_TYPE_MAP_OBJECT:
                case GAMEOBJECT_TYPE_MO_TRANSPORT:
                {
                    delete go_spawn;
                    continue;
                }
                }
#endif

                go_spawn->entry = gameobject_entry;
                go_spawn->map = fields[4].GetUInt32();
                go_spawn->position_x = fields[5].GetFloat();
                go_spawn->position_y = fields[6].GetFloat();
                go_spawn->position_z = fields[7].GetFloat();
                go_spawn->orientation = fields[8].GetFloat();
                go_spawn->rotation_0 = fields[9].GetFloat();
                go_spawn->rotation_1 = fields[10].GetFloat();
                go_spawn->rotation_2 = fields[11].GetFloat();
                go_spawn->rotation_3 = fields[12].GetFloat();
                go_spawn->state = fields[13].GetUInt32();
                go_spawn->flags = fields[14].GetUInt32();
                go_spawn->faction = fields[15].GetUInt32();
                go_spawn->scale = fields[16].GetFloat();
                //gspawn->stateNpcLink = fields[17].GetUInt32();
                go_spawn->phase = fields[18].GetUInt32();

                if (go_spawn->phase == 0)
                    go_spawn->phase = 0xFFFFFFFF;

                go_spawn->overrides = fields[19].GetUInt32();

                go_spawn->table = "gameobject_spawns";

                _gameobjectSpawnsStore[go_spawn->map].push_back(go_spawn);
                ++count;
            } while (gobject_spawn_result->NextRow());
        }

        delete gobject_spawn_result;
    }

    sLogger.info("MySQLDataLoads : Loaded %u rows from `gameobject_spawns` table in %u ms!", count, static_cast<uint32_t>(Util::GetTimeDifferenceToNow(startTime)));
}

void MySQLDataStore::loadRecallTable()
{
    auto startTime = Util::TimeNow();
    uint32_t count = 0;

    _recallStore.clear();

    QueryResult* recall_result = sMySQLStore.getWorldDBQuery("SELECT id, name, MapId, positionX, positionY, positionZ, Orientation FROM recall WHERE min_build <= %u AND max_build >= %u", VERSION_STRING, VERSION_STRING);
    if (recall_result)
    {
        do
        {
            Field* fields = recall_result->Fetch();
            MySQLStructure::RecallStruct* teleCoords = new MySQLStructure::RecallStruct;

            teleCoords->name = fields[1].GetString();
            teleCoords->mapId = fields[2].GetUInt32();
            teleCoords->location.x = fields[3].GetFloat();
            teleCoords->location.y = fields[4].GetFloat();
            teleCoords->location.z = fields[5].GetFloat();
            teleCoords->location.o = fields[6].GetFloat();

            _recallStore.push_back(teleCoords);

            ++count;
        } while (recall_result->NextRow());

        delete recall_result;
    }

    sLogger.info("MySQLDataLoads : Loaded %u rows from `recall` table in %u ms!", count, static_cast<uint32_t>(Util::GetTimeDifferenceToNow(startTime)));
}

void MySQLDataStore::loadCreatureAIScriptsTable()
{
    auto startTime = Util::TimeNow();

    _creatureAIScriptStore.clear();

    QueryResult* result = WorldDatabase.Query("SELECT * FROM creature_ai_scripts WHERE min_build <= %u AND max_build >= %u ORDER BY entry, event", VERSION_STRING, VERSION_STRING);
    if (result == nullptr)
    {
        sLogger.info("MySQLDataLoads : Table `creature_ai_scripts` is empty!");
        return;
    }

    sLogger.info("MySQLDataLoads : Table `creature_ai_scripts` has %u columns", result->GetFieldCount());

    uint32_t load_count = 0;
    do
    {
        Field* fields = result->Fetch();
        MySQLStructure::CreatureAIScripts* ai_script = new MySQLStructure::CreatureAIScripts;

        uint32_t creature_entry = fields[2].GetUInt32();
        uint32_t spellId = fields[9].GetUInt32();
        uint32_t textId = fields[17].GetUInt32();

        if (!getCreatureProperties(creature_entry))
        {
            sLogger.debugFlag(AscEmu::Logging::LF_DB_TABLES, "Table `creature_ai_scripts` includes invalid creature entry %u <skipped>", creature_entry);
            continue;
        }
           
        SpellInfo const* spell = sSpellMgr.getSpellInfo(spellId);
        if (spell == nullptr && spellId != 0)
        {
            sLogger.debugFlag(AscEmu::Logging::LF_DB_TABLES, "Table `creature_ai_scripts` includes invalid spellId for creature entry %u <skipped>", spellId, creature_entry);
            continue;
        }

        if (!sMySQLStore.getNpcScriptText(textId) && textId != 0)
        {
            sLogger.debugFlag(AscEmu::Logging::LF_DB_TABLES, "Table `creature_ai_scripts` includes invalid textId for creature entry %u <skipped>", textId, creature_entry);
            continue;
        }

        ai_script->entry = creature_entry;
        ai_script->difficulty = fields[3].GetUInt8();
        ai_script->phase = fields[4].GetUInt8();
        ai_script->event = fields[5].GetUInt8();
        ai_script->action = fields[6].GetUInt8();
        ai_script->maxCount = fields[7].GetUInt8();
        ai_script->chance = fields[8].GetFloat();
        ai_script->spellId = spellId;
        ai_script->spell_type = fields[10].GetUInt8();
        ai_script->triggered = fields[11].GetBool();
        ai_script->target = fields[12].GetUInt8();
        ai_script->cooldownMin = fields[13].GetUInt32();
        ai_script->cooldownMax = fields[14].GetUInt32();
        ai_script->minHealth = fields[15].GetFloat();
        ai_script->maxHealth = fields[16].GetFloat();
        ai_script->textId = textId;
        ai_script->misc1 = fields[18].GetUInt32();

        _creatureAIScriptStore.emplace(creature_entry, ai_script);

        ++load_count;
    } while (result->NextRow());

    delete result;

    sLogger.info("MySQLDataLoads : Loaded %u rows from `creature_ai_scripts` table in %u ms!", load_count, static_cast<uint32_t>(Util::GetTimeDifferenceToNow(startTime)));
}

std::vector<MySQLStructure::CreatureAIScripts>* MySQLDataStore::getCreatureAiScripts(uint32_t entry)
{
    auto result = new std::vector <MySQLStructure::CreatureAIScripts>;

    result->clear();

    for (auto itr : _creatureAIScriptStore)
    {
        if (itr.first == entry)
            result->push_back(*itr.second);
    }

    return result;
}

void MySQLDataStore::loadSpawnGroupIds()
{
    auto startTime = Util::TimeNow();

    _spawnGroupDataStore.clear();

    QueryResult* result = WorldDatabase.Query("SELECT * FROM spawn_group_id ORDER BY groupId");
    if (result == nullptr)
    {
        sLogger.info("MySQLDataLoads : Table `spawn_group_id` is empty!");
        return;
    }

    sLogger.info("MySQLDataLoads : Table `spawn_group_id` has %u columns", result->GetFieldCount());

    uint32_t load_count = 0;
    do
    {
        Field* fields = result->Fetch();
        uint32_t groupId = fields[0].GetUInt8();

        SpawnGroupTemplateData& spawnGroup = _spawnGroupDataStore[groupId];

        spawnGroup.groupId = groupId;
        spawnGroup.name = fields[1].GetString();
        spawnGroup.mapId = 0xFFFFFFFF;
        uint32_t flags = fields[2].GetUInt8();
        if (flags & ~SPAWNGROUP_FLAGS_ALL)
        {
            flags &= SPAWNGROUP_FLAGS_ALL;
            sLogger.failure("Invalid spawn group flag %u on group ID %u (%s), reduced to valid flag %u.", flags, groupId, spawnGroup.name.c_str(), uint32_t(spawnGroup.groupFlags));
        }
        if (flags & SPAWNGROUP_FLAG_SYSTEM && flags & SPAWNGROUP_FLAG_MANUAL_SPAWN)
        {
            flags &= ~SPAWNGROUP_FLAG_MANUAL_SPAWN;
            sLogger.failure("System spawn group %u (%s) has invalid manual spawn flag. Ignored.", groupId, spawnGroup.name.c_str());
        }
        spawnGroup.groupFlags = SpawnGroupFlags(flags);
        spawnGroup.spawnFlags = SpawnFlags(fields[3].GetUInt8());
        spawnGroup.bossId = fields[4].GetUInt32();

        ++load_count;
    } while (result->NextRow());

    delete result;

    sLogger.info("MySQLDataLoads : Loaded %u rows from `spawn_group_id` table in %u ms!", load_count, static_cast<uint32_t>(Util::GetTimeDifferenceToNow(startTime)));
}

void MySQLDataStore::loadCreatureGroupSpawns()
{
    auto startTime = Util::TimeNow();

    _spawnGroupMapStore.clear();

    QueryResult* result = WorldDatabase.Query("SELECT * FROM creature_group_spawn ORDER BY groupId");
    if (result == nullptr)
    {
        sLogger.info("MySQLDataLoads : Table `creature_group_spawn` is empty!");
        return;
    }

    sLogger.info("MySQLDataLoads : Table `creature_group_spawn` has %u columns", result->GetFieldCount());

    uint32_t load_count = 0;
    do
    {
        Field* fields = result->Fetch();
        uint32_t groupId = fields[0].GetUInt8();
        uint32_t spawnId = fields[1].GetUInt32();
        MySQLStructure::CreatureSpawn spawn;
        bool data = false;

        auto it = _spawnGroupDataStore.find(groupId);
        if (it == _spawnGroupDataStore.end())
        {
            sLogger.failure("Spawn group %u assigned to spawn ID (%u), but group does not exist!", groupId, spawnId);
            continue;
        }

        for (const auto creatureSpawnMap : sMySQLStore._creatureSpawnsStore)
        {
            for (const auto creatureSpawn : creatureSpawnMap)
            {
                if (creatureSpawn->id == spawnId)
                {
                    data = true;

                    SpawnGroupTemplateData& groupTemplate = it->second;
                    if (groupTemplate.mapId == 0xFFFFFFFF)
                        groupTemplate.mapId = creatureSpawn->mapId;

                    else if (groupTemplate.mapId != creatureSpawn->mapId && !(groupTemplate.groupFlags & SPAWNGROUP_FLAG_SYSTEM))
                    {
                        sLogger.failure("Spawn group %u has map ID %u, but spawn (%u) has map id %u - spawn NOT added to group!", groupId, groupTemplate.mapId, spawnId, creatureSpawn->mapId);
                        continue;
                    }

                    groupTemplate.spawns.insert(std::make_pair(spawnId, nullptr));
                    _spawnGroupMapStore.emplace(spawnId, &groupTemplate);

                    ++load_count;
                }
            }
        }

        if (!data)
        {
            sLogger.failure("Spawn data with ID (%u) not found, but is listed as a member of spawn group %u!", spawnId, groupId);
            continue;
        } 
    } while (result->NextRow());

    delete result;

    sLogger.info("MySQLDataLoads : Loaded %u rows from `creature_group_spawn` table in %u ms!", load_count, static_cast<uint32_t>(Util::GetTimeDifferenceToNow(startTime)));
}

SpawnGroupTemplateData* MySQLDataStore::getSpawnGroupDataByGroup(uint32_t groupId)
{
    for (auto spawnData : _spawnGroupMapStore)
    {
        if (spawnData.second->groupId == groupId)
            return spawnData.second;
    }

    return nullptr;
}

SpawnGroupTemplateData* MySQLDataStore::getSpawnGroupDataBySpawn(uint32_t spawnId)
{
    for (auto spawnData : _spawnGroupMapStore)
    {
        if (spawnData.first == spawnId)
            return spawnData.second;
    }

    return nullptr;
}

std::vector<Creature*> const MySQLDataStore::getSpawnGroupDataByBoss(uint32_t bossId)
{
    std::vector<Creature*> data;

    for (auto spawnData : _spawnGroupMapStore)
    {
        if (spawnData.second->bossId == bossId)
            data.push_back(spawnData.second->spawns[spawnData.first]);
    }

    return data;
}
