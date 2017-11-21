/*
Copyright (c) 2014-2017 AscEmu Team <http://www.ascemu.org/>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "StdAfx.h"
#include "Storage/MySQLDataStore.hpp"
#include "Server/MainServerDefines.h"
#include "Config/Config.h"
#include "Spell/Customization/SpellCustomizations.hpp"

initialiseSingleton(MySQLDataStore);

SERVER_DECL std::set<std::string> CreatureSpawnsTables;
SERVER_DECL std::set<std::string> GameObjectSpawnsTables;
SERVER_DECL std::set<std::string> GameObjectPropertiesTables;
SERVER_DECL std::set<std::string> CreaturePropertiesTables;
SERVER_DECL std::set<std::string> ItemPropertiesTables;
SERVER_DECL std::set<std::string> QuestPropertiesTables;

MySQLDataStore::MySQLDataStore() {}

MySQLDataStore::~MySQLDataStore()
{
    for (int i = 0; i < NUM_MONSTER_SAY_EVENTS; ++i)
    {
        for (NpcMonstersayContainer::iterator itr = _npcMonstersayContainer[i].begin(); itr != _npcMonstersayContainer[i].end(); ++itr)
        {
            MySQLStructure::NpcMonsterSay* npcMonsterSay = itr->second;
            for (uint32_t j = 0; j < npcMonsterSay->textCount; ++j)
            {
                free((char*)npcMonsterSay->texts[j]);
            }

            delete[] npcMonsterSay->texts;
            free((char*)npcMonsterSay->monsterName);
            delete npcMonsterSay;
        }

        _npcMonstersayContainer[i].clear();
    }

    for (auto&& professionDiscovery : _professionDiscoveryStore)
    {
        delete professionDiscovery;
    }

}

void MySQLDataStore::loadAdditionalTableConfig()
{
    // init basic tables
    CreatureSpawnsTables.insert(std::string("creature_spawns"));
    CreatureSpawnsTables.insert(std::string("creature_staticspawns"));
    GameObjectSpawnsTables.insert(std::string("gameobject_spawns"));
    GameObjectSpawnsTables.insert(std::string("gameobject_staticspawns"));
    GameObjectPropertiesTables.insert(std::string("gameobject_properties"));
    CreaturePropertiesTables.insert(std::string("creature_properties"));
    ItemPropertiesTables.insert(std::string("item_properties"));
    QuestPropertiesTables.insert(std::string("quest_properties"));

    // get config
    std::string strData = worldConfig.startup.additionalTableLoads;
    if (strData.empty())
        return;

    std::vector<std::string> strs = Util::SplitStringBySeperator(strData, ",");
    if (strs.empty())
        return;

    for (std::vector<std::string>::iterator itr = strs.begin(); itr != strs.end(); ++itr)
    {
        std::stringstream additionTableStream((*itr));
        std::string additional_table;
        std::string target_table;

        additionTableStream >> additional_table;
        additionTableStream >> target_table;

        if (additional_table.empty() || target_table.empty())
            continue;

        if (target_table.compare("creature_spawns") == 0)
            CreatureSpawnsTables.insert(additional_table);

        if (target_table.compare("gameobject_spawns") == 0)
            GameObjectSpawnsTables.insert(additional_table);

        if (target_table.compare("gameobject_properties") == 0)
            GameObjectPropertiesTables.insert(additional_table);

        if (target_table.compare("creature_properties") == 0)
            CreaturePropertiesTables.insert(additional_table);

        if (target_table.compare("item_properties") == 0)
            ItemPropertiesTables.insert(additional_table);

        if (target_table.compare("quest_properties") == 0)
            QuestPropertiesTables.insert(additional_table);
    }
}

void MySQLDataStore::loadItemPagesTable()
{
    auto startTime = Util::TimeNow();

    QueryResult* itempages_result = WorldDatabase.Query("SELECT entry, text, next_page FROM item_pages");
    if (itempages_result == nullptr)
    {
        LogNotice("MySQLDataLoads : able `item_pages` is empty!");
        return;
    }

    LogNotice("MySQLDataLoads : Table `item_pages` has %u columns", itempages_result->GetFieldCount());

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

    LogDetail("MySQLDataLoads : Loaded %u pages from `item_pages` table in %u ms!", itempages_count, Util::GetTimeDifferenceToNow(startTime));
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
    uint32_t basic_field_count = 0;

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
            LogNotice("MySQLDataLoads : Table `%s` is empty!", table_name.c_str());
            return;
        }

        uint32_t row_count = 0;
        if (table_name.compare("item_properties") == 0)
        {
            basic_field_count = item_result->GetFieldCount();
        }
        else
        {
            row_count = static_cast<uint32_t>(_itemPropertiesStore.size());
        }

        if (basic_field_count != item_result->GetFieldCount())
        {
            LOG_ERROR("Additional item_properties table `%s` has %u columns, but needs %u columns! Skipped!", table_name.c_str(), item_result->GetFieldCount(), basic_field_count);
            continue;
        }

        LogNotice("MySQLDataLoads : Table `%s` has %u columns", table_name.c_str(), item_result->GetFieldCount());

        _itemPropertiesStore.rehash(row_count + item_result->GetRowCount());

        do
        {
            Field* fields = item_result->Fetch();

            uint32_t entry = fields[0].GetUInt32();

            ItemProperties& itemProperties = _itemPropertiesStore[entry];

            itemProperties.ItemId = entry;
            itemProperties.Class = fields[1].GetUInt32();
            itemProperties.SubClass = fields[2].GetUInt16();
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

            for (uint8_t i = 0; i < itemProperties.itemstatscount; ++i)
            {
                itemProperties.Stats[i].Type = fields[27 + i * 2].GetUInt32();
                itemProperties.Stats[i].Value = fields[28 + i * 2].GetUInt32();
            }

            itemProperties.ScalingStatsEntry = fields[47].GetUInt32();
            itemProperties.ScalingStatsFlag = fields[48].GetUInt32();

            for (uint8_t i = 0; i < MAX_ITEM_PROTO_DAMAGES; ++i)
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

            for (uint8_t i = 0; i < MAX_ITEM_PROTO_SPELLS; ++i)
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
            uint32_t page_id = fields[97].GetUInt32();
            if (page_id != 0)
            {
                MySQLStructure::ItemPage const* item_page = getItemPage(page_id);
                if (item_page == nullptr)
                {
                    LOG_ERROR("Table `%s` entry: %u includes invalid pageId %u! pageId is set to 0.", table_name.c_str(), entry, page_id);
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

            for (uint8_t i = 0; i < MAX_ITEM_PROTO_SOCKETS; ++i)
            {
                itemProperties.Sockets[i].SocketColor = uint32_t(fields[113 + i * 2].GetUInt8());
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
            Util::StringToLowerCase(lower_case_name);
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

    LogDetail("MySQLDataLoads : Loaded %u item_properties in %u ms!", item_count, Util::GetTimeDifferenceToNow(startTime));
}

ItemProperties const* MySQLDataStore::getItemProperties(uint32_t entry)
{
    ItemPropertiesContainer::const_iterator itr = _itemPropertiesStore.find(entry);
    if (itr != _itemPropertiesStore.end())
        return &(itr->second);

    return nullptr;
}

void MySQLDataStore::loadCreaturePropertiesTable()
{
    auto startTime = Util::TimeNow();
    uint32_t creature_properties_count = 0;
    uint32_t basic_field_count = 0;

    std::set<std::string>::iterator tableiterator;
    for (tableiterator = CreaturePropertiesTables.begin(); tableiterator != CreaturePropertiesTables.end(); ++tableiterator)
    {
        std::string table_name = *tableiterator;
        //                                                                 0          1           2             3                 4               5                  6
        QueryResult* creature_properties_result = WorldDatabase.Query("SELECT entry, killcredit1, killcredit2, male_displayid, female_displayid, male_displayid2, female_displayid2, "
        //                                                         7      8         9         10       11     12     13       14            15              16           17
                                                                "name, subname, info_str, type_flags, type, family, rank, encounter, base_attack_mod, range_attack_mod, leader, "
        //                                                          18        19        20        21         22      23     24      25          26           27
                                                                "minlevel, maxlevel, faction, minhealth, maxhealth, mana, scale, npcflags, attacktime, attack_school, "
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
            LogNotice("MySQLDataLoads : Table `%s` is empty!", table_name.c_str());
            return;
        }

        uint32_t row_count = 0;
        if (table_name.compare("creature_properties") == 0)
        {
            basic_field_count = creature_properties_result->GetFieldCount();
        }
        else
        {
            row_count = static_cast<uint32_t>(_creaturePropertiesStore.size());
        }

        if (basic_field_count != creature_properties_result->GetFieldCount())
        {
            LOG_ERROR("Additional creature_properties table `%s` has %u columns, but needs %u columns! Skipped!", table_name.c_str(), creature_properties_result->GetFieldCount());
            delete creature_properties_result;
            continue;
        }

        LogNotice("MySQLDataLoads : Table `%s` has %u columns", table_name.c_str(), creature_properties_result->GetFieldCount());

        _creaturePropertiesStore.rehash(row_count + creature_properties_result->GetRowCount());

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
                DBC::Structures::CreatureDisplayInfoEntry const* creature_display = sCreatureDisplayInfoStore.LookupEntry(creatureProperties.Male_DisplayID);
                if (creature_display == nullptr)
                {
                    LogError("Table %s includes invalid Male_DisplayID %u for npc entry: %u. Set to 0!", (*tableiterator).c_str(), creatureProperties.Male_DisplayID, entry);
                    creatureProperties.Male_DisplayID = 0;
                }
            }
            creatureProperties.Female_DisplayID = fields[4].GetUInt32();
            if (creatureProperties.Female_DisplayID != 0)
            {
                DBC::Structures::CreatureDisplayInfoEntry const* creature_display = sCreatureDisplayInfoStore.LookupEntry(creatureProperties.Female_DisplayID);
                if (creature_display == nullptr)
                {
                    LogError("Table %s includes invalid Female_DisplayID %u for npc entry: %u. Set to 0!", (*tableiterator).c_str(), creatureProperties.Female_DisplayID, entry);
                    creatureProperties.Female_DisplayID = 0;
                }
            }
            creatureProperties.Male_DisplayID2 = fields[5].GetUInt32();
            if (creatureProperties.Male_DisplayID2 != 0)
            {
                DBC::Structures::CreatureDisplayInfoEntry const* creature_display = sCreatureDisplayInfoStore.LookupEntry(creatureProperties.Male_DisplayID2);
                if (creature_display == nullptr)
                {
                    LogError("Table %s includes invalid Male_DisplayID2 %u for npc entry: %u. Set to 0!", (*tableiterator).c_str(), creatureProperties.Male_DisplayID2, entry);
                    creatureProperties.Male_DisplayID2 = 0;
                }
            }
            creatureProperties.Female_DisplayID2 = fields[6].GetUInt32();
            if (creatureProperties.Female_DisplayID2 != 0)
            {
                DBC::Structures::CreatureDisplayInfoEntry const* creature_display = sCreatureDisplayInfoStore.LookupEntry(creatureProperties.Female_DisplayID2);
                if (creature_display == nullptr)
                {
                    LogError("Table %s includes invalid Female_DisplayID2 %u for npc entry: %u. Set to 0!", (*tableiterator).c_str(), creatureProperties.Female_DisplayID2, entry);
                    creatureProperties.Female_DisplayID2 = 0;
                }
            }

            creatureProperties.Name = fields[7].GetString();

            //lowercase
            std::string lower_case_name = creatureProperties.Name;
            Util::StringToLowerCase(lower_case_name);
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
                LOG_ERROR("Table `%s` MinHealth = 0 is not a valid value! Default set to 1 for entry: %u.", table_name.c_str(), entry);
                creatureProperties.MinHealth = 1;
            }

            if (fields[22].GetUInt32() != 0)
            {
                creatureProperties.MaxHealth = fields[22].GetUInt32();
            }
            else
            {
                LOG_ERROR("Table `%s` MaxHealth = 0 is not a valid value! Default set to 1 for entry: %u.", table_name.c_str(), entry);
                creatureProperties.MaxHealth = 1;
            }

            creatureProperties.Mana = fields[23].GetUInt32();
            creatureProperties.Scale = fields[24].GetFloat();
            creatureProperties.NPCFLags = fields[25].GetUInt32();
            creatureProperties.AttackTime = fields[26].GetUInt32();
            creatureProperties.attackSchool = fields[27].GetUInt32();
            if (fields[27].GetUInt32() <= SCHOOL_ARCANE)
            {
                creatureProperties.attackSchool = fields[27].GetUInt32();
            }
            else
            {
                LOG_ERROR("Table `%s` AttackType: %u is not a valid value! Default set to 0 for entry: %u.", table_name.c_str(), fields[10].GetUInt32(), entry);
                creatureProperties.attackSchool = SCHOOL_NORMAL;
            }

            creatureProperties.MinDamage = fields[28].GetFloat();
            creatureProperties.MaxDamage = fields[29].GetFloat();
            creatureProperties.CanRanged = fields[30].GetUInt32();
            creatureProperties.RangedAttackTime = fields[31].GetUInt32();
            creatureProperties.RangedMinDamage = fields[32].GetFloat();
            creatureProperties.RangedMaxDamage = fields[33].GetFloat();
            creatureProperties.RespawnTime = fields[34].GetUInt32();
            for (uint8_t i = 0; i < SCHOOL_COUNT; ++i)
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

            for (uint8_t i = 0; i < creatureMaxProtoSpells; ++i)
            {
                // Process spell fields
                creatureProperties.AISpells[i] = fields[52 + i].GetUInt32();
                if (creatureProperties.AISpells[i] != 0)
                {
                    SpellInfo* sp = sSpellCustomizations.GetSpellInfo(creatureProperties.AISpells[i]);
                    if (sp == nullptr)
                    {
                        uint8_t spell_number = i;
                        LOG_ERROR("spell %u in table %s column spell%u for creature entry: %u is not a valid spell!", creatureProperties.AISpells[i], table_name.c_str(), spell_number + 1, entry);
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

                    SpellInfo* sp = sSpellCustomizations.GetSpellInfo(creature_spell_data->Spells[i]);
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

            //process aura string
            if (creatureProperties.aura_string.size() != 0)
            {
                std::string auras = creatureProperties.aura_string;
                std::vector<std::string> split_auras = Util::SplitStringBySeperator(auras, " ");
                for (std::vector<std::string>::iterator it = split_auras.begin(); it != split_auras.end(); ++it)
                {
                    uint32_t id = atol((*it).c_str());
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

            /*for (uint8_t i = 0; i < NUM_MONSTER_SAY_EVENTS; ++i)
            {
                creatureProperties.MonsterSay[i] = nullptr;
            }*/

            ++creature_properties_count;
        } while (creature_properties_result->NextRow());

        delete creature_properties_result;
    }

    LogDetail("MySQLDataLoads : Loaded %u creature proto data in %u ms!", creature_properties_count, Util::GetTimeDifferenceToNow(startTime));
}

CreatureProperties const* MySQLDataStore::getCreatureProperties(uint32_t entry)
{
    CreaturePropertiesContainer::const_iterator itr = _creaturePropertiesStore.find(entry);
    if (itr != _creaturePropertiesStore.end())
        return &(itr->second);

    return nullptr;
}

void MySQLDataStore::loadGameObjectPropertiesTable()
{
    auto startTime = Util::TimeNow();
    uint32_t gameobject_properties_count = 0;
    uint32_t basic_field_count = 0;

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
            LogNotice("MySQLDataLoads : Table `%s` is empty!", table_name.c_str());
            return;
        }

        uint32_t row_count = 0;
        if (table_name.compare("gameobject_properties") == 0)
        {
            basic_field_count = gameobject_properties_result->GetFieldCount();
        }
        else
        {
            row_count = static_cast<uint32_t>(_gameobjectPropertiesStore.size());
        }

        if (basic_field_count != gameobject_properties_result->GetFieldCount())
        {
            LOG_ERROR("Additional gameobject_properties table `%s` has %u columns, but needs %u columns! Skipped!", table_name.c_str(), gameobject_properties_result->GetFieldCount());
            delete gameobject_properties_result;
            continue;
        }

        LogNotice("MySQLDataLoads : Table `%s` has %u columns", table_name.c_str(), gameobject_properties_result->GetFieldCount());

        _gameobjectPropertiesStore.rehash(row_count + gameobject_properties_result->GetRowCount());

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
                        LOG_ERROR("Table `%s` questitem%u : %u is not a valid item! Default set to 0 for entry: %u.", table_name.c_str(), i, quest_item_entry, entry);
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

    LogDetail("MySQLDataLoads : Loaded %u gameobject data in %u ms!", gameobject_properties_count, Util::GetTimeDifferenceToNow(startTime));
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
    uint32_t basic_field_count = 0;

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
        //                                                     27               28           29          30           31          32         33           34         35
                                                        "ObjectiveText3, ObjectiveText4, ReqItemId1, ReqItemId2, ReqItemId3, ReqItemId4, ReqItemId5, ReqItemId6, ReqItemCount1, "
        //                                                     36             37            38              39             40              41                 42
                                                        "ReqItemCount2, ReqItemCount3, ReqItemCount4, ReqItemCount5, ReqItemCount6, ReqKillMobOrGOId1, ReqKillMobOrGOId2, "
        //                                                     43                   44                    45                  46                      47                  48
                                                        "ReqKillMobOrGOId3, ReqKillMobOrGOId4, ReqKillMobOrGOCount1, ReqKillMobOrGOCount2, ReqKillMobOrGOCount3, ReqKillMobOrGOCount4, "
        //                                                     49                 50              51              52              53           54           55           56
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
            LogNotice("MySQLDataLoads : Table `%s` is empty!", table_name.c_str());
            return;
        }

        uint32_t row_count = 0;
        if (table_name.compare("quest_properties") == 0)
        {
            basic_field_count = quest_result->GetFieldCount();
        }
        else
        {
            row_count = static_cast<uint32_t>(_questPropertiesStore.size());
        }

        if (basic_field_count != quest_result->GetFieldCount())
        {
            LOG_ERROR("Additional quest_properties table `%s` has %u columns, but needs %u columns! Skipped!", table_name.c_str(), quest_result->GetFieldCount());
            delete quest_result;
            continue;
        }

        LogNotice("MySQLDataLoads : Table `%s` has %u columns", table_name.c_str(), quest_result->GetFieldCount());

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
                            LogError("Quest %u has `ReqCreatureOrGOId%d` = %i but creature with entry %u does not exist in creature_properties table!",
                                     entry, i, questInfo.required_mob_or_go[i], questInfo.required_mob_or_go[i]);
                        }
                    }
                    else
                    {
                        if (!getGameObjectProperties(-questInfo.required_mob_or_go[i]))
                        {
                            LogError("Quest %u has `ReqCreatureOrGOId%d` = %i but gameobject %u does not exist in gameobject_properties table!",
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
            questInfo.bonushonor = fields[121].GetInt32();
            questInfo.bonusarenapoints = fields[122].GetInt32();
            questInfo.rewardtitleid = fields[123].GetInt32();
            questInfo.rewardtalents = fields[124].GetInt32();
            questInfo.suggestedplayers = fields[125].GetInt32();

            // emotes
            questInfo.detailemotecount = fields[126].GetInt32();

            for (uint8_t i = 0; i < 4; ++i)
            {
                questInfo.detailemote[i] = fields[127 + i].GetUInt32();
                questInfo.detailemotedelay[i] = fields[131 + i].GetUInt32();
            }

            questInfo.completionemotecount = fields[135].GetInt32();

            for (uint8_t i = 0; i < 4; ++i)
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

    LogDetail("MySQLDataLoads : Loaded %u quest_properties data in %u ms!", quest_count, Util::GetTimeDifferenceToNow(startTime));
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
                LOG_ERROR("Table `gameobject_quest_item_binding` includes data for invalid gameobject_properties entry: %u. Skipped!", entry);
                continue;
            }

            uint32_t quest_entry = fields[1].GetUInt32();
            QuestProperties const* quest = sMySQLStore.getQuestProperties(quest_entry);
            if (quest == nullptr)
            {
                LOG_ERROR("Table `gameobject_quest_item_binding` includes data for invalid quest_properties : %u. Skipped!", quest_entry);
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

    LogDetail("MySQLDataLoads : Loaded %u data from `gameobject_quest_item_binding` table in %u ms!", gameobject_quest_item_count, Util::GetTimeDifferenceToNow(startTime));
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
                LOG_ERROR("Table `gameobject_quest_pickup_binding` includes data for invalid gameobject_properties entry: %u. Skipped!", entry);
                continue;
            }

            uint32_t quest_entry = fields[1].GetUInt32();
            QuestProperties const* quest = sMySQLStore.getQuestProperties(quest_entry);
            if (quest == nullptr)
            {
                LOG_ERROR("Table `gameobject_quest_pickup_binding` includes data for invalid quest_properties : %u. Skipped!", quest_entry);
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

    LogDetail("MySQLDataLoads : Loaded %u data from `gameobject_quest_pickup_binding` table in %u ms!", gameobject_quest_pickup_count, Util::GetTimeDifferenceToNow(startTime));
}

void MySQLDataStore::loadCreatureDifficultyTable()
{
    auto startTime = Util::TimeNow();

    //                                                                         0          1            2             3
    QueryResult* creature_difficulty_result = WorldDatabase.Query("SELECT entry, difficulty_1, difficulty_2, difficulty_3 FROM creature_difficulty");

    if (creature_difficulty_result == nullptr)
    {
        LogNotice("MySQLDataLoads : Table `creature_difficulty` is empty!");
        return;
    }

    LogNotice("MySQLDataLoads : Table `creature_difficulty` has %u columns", creature_difficulty_result->GetFieldCount());

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

    LogDetail("MySQLDataLoads : Loaded %u creature difficulties info from `creature_difficulty` table in %u ms!", creature_difficulty_count, Util::GetTimeDifferenceToNow(startTime));
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
        LogNotice("MySQLDataLoads : Table `display_bounding_boxes` is empty!");
        return;
    }

    LogNotice("MySQLDataLoads : Table `display_bounding_boxes` has %u columns", display_bounding_boxes_result->GetFieldCount());

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

    LogDetail("MySQLDataLoads : Loaded %u display bounding info from `display_bounding_boxes` table in %u ms!", display_bounding_boxes_count, Util::GetTimeDifferenceToNow(startTime));
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
        LogNotice("MySQLDataLoads : Table `vendor_restrictions` is empty!");
        return;
    }

    LogNotice("MySQLDataLoads : Table `vendor_restrictions` has %u columns", vendor_restricitons_result->GetFieldCount());

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

    LogDetail("MySQLDataLoads : Loaded %u restrictions from `vendor_restrictions` table in %u ms!", vendor_restricitons_count, Util::GetTimeDifferenceToNow(startTime));
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
        LogNotice("MySQLDataLoads : Table `npc_text` is empty!");
        return;
    }

    LogNotice("MySQLDataLoads : Table `npc_text` has %u columns", npc_text_result->GetFieldCount());

    _npcTextStore.rehash(npc_text_result->GetRowCount());

    uint32_t npc_text_count = 0;
    do
    {
        Field* fields = npc_text_result->Fetch();

        uint32_t entry = fields[0].GetUInt32();

        MySQLStructure::NpcText& npcText = _npcTextStore[entry];

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
    } while (npc_text_result->NextRow());

    delete npc_text_result;

    LogDetail("MySQLDataLoads : Loaded %u rows from `npc_text` table in %u ms!", npc_text_count, Util::GetTimeDifferenceToNow(startTime));
}

MySQLStructure::NpcText const* MySQLDataStore::getNpcText(uint32_t entry)
{
    NpcTextContainer::const_iterator itr = _npcTextStore.find(entry);
    if (itr != _npcTextStore.end())
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
        LogNotice("MySQLDataLoads : Table `npc_script_text` is empty!");
        return;
    }

    LogNotice("MySQLDataLoads : Table `npc_script_text` has %u columns", npc_script_text_result->GetFieldCount());

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

    LogDetail("MySQLDataLoads : Loaded %u rows from `npc_script_text` table in %u ms!", npc_script_text_count, Util::GetTimeDifferenceToNow(startTime));
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
        LogNotice("MySQLDataLoads : Table `gossip_menu_option` is empty!");
        return;
    }

    LogNotice("MySQLDataLoads : Table `gossip_menu_option` has %u columns", gossip_menu_optiont_result->GetFieldCount());

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

    LogDetail("MySQLDataLoads : Loaded %u rows from `gossip_menu_option` table in %u ms!", gossip_menu_optiont_count, Util::GetTimeDifferenceToNow(startTime));
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
        LogNotice("MySQLDataLoads : Table `graveyards` is empty!");
        return;
    }

    LogNotice("MySQLDataLoads : Table `graveyards` has %u columns", graveyards_result->GetFieldCount());

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

    LogDetail("MySQLDataLoads : Loaded %u rows from `graveyards` table in %u ms!", graveyards_count, Util::GetTimeDifferenceToNow(startTime));
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
        LogNotice("MySQLDataLoads : Table `spell_teleport_coords` is empty!");
        return;
    }

    LogNotice("MySQLDataLoads : Table `spell_teleport_coords` has %u columns", teleport_coords_result->GetFieldCount());

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

    LogDetail("MySQLDataLoads : Loaded %u rows from `spell_teleport_coords` table in %u ms!", teleport_coords_count, Util::GetTimeDifferenceToNow(startTime));
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
        LogNotice("MySQLDataLoads : Table `fishing` is empty!");
        return;
    }

    LogNotice("MySQLDataLoads : Table `fishing` has %u columns", fishing_result->GetFieldCount());

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

    LogDetail("MySQLDataLoads : Loaded %u rows from `fishing` table in %u ms!", fishing_count, Util::GetTimeDifferenceToNow(startTime));
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
                                                            "heroic_keyid_1, heroic_keyid_2, viewingDistance, required_checkpoint FROM worldmap_info");
    if (worldmap_info_result == nullptr)
    {
        LogNotice("MySQLDataLoads : Table `worldmap_info` is empty!");
        return;
    }

    LogNotice("MySQLDataLoads : Table `worldmap_info` has %u columns", worldmap_info_result->GetFieldCount());

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

    LogDetail("MySQLDataLoads : Loaded %u rows from `worldmap_info` table in %u ms!", world_map_info_count, Util::GetTimeDifferenceToNow(startTime));
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
        LogNotice("MySQLDataLoads : Table `zoneguards` is empty!");
        return;
    }

    LogNotice("MySQLDataLoads : Table `zoneguards` has %u columns", zone_guards_result->GetFieldCount());

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

    LogDetail("MySQLDataLoads : Loaded %u rows from `zoneguards` table in %u ms!", zone_guards_count, Util::GetTimeDifferenceToNow(startTime));
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
        LogNotice("MySQLDataLoads : Table `battlemasters` is empty!");
        return;
    }

    LogNotice("MySQLDataLoads : Table `battlemasters` has %u columns", battlemasters_result->GetFieldCount());

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

    LogDetail("MySQLDataLoads : Loaded %u rows from `battlemasters` table in %u ms!", battlemasters_count, Util::GetTimeDifferenceToNow(startTime));
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

#if VERSION_STRING != Cata
    //                                                                      0         1        2       3
    QueryResult* totemdisplayids_result = WorldDatabase.Query("SELECT displayid, draeneiid, trollid, orcid FROM totemdisplayids");
#else
    //                                                                      0         1        2       3       4         5         6
    QueryResult* totemdisplayids_result = WorldDatabase.Query("SELECT displayid, draeneiid, trollid, orcid, taurenid, dwarfid, goblinid FROM totemdisplayids");
#endif
    if (totemdisplayids_result == nullptr)
    {
        LogNotice("MySQLDataLoads : Table `totemdisplayids` is empty!");
        return;
    }

    LogNotice("MySQLDataLoads : Table `totemdisplayids` has %u columns", totemdisplayids_result->GetFieldCount());

    _totemDisplayIdsStore.rehash(totemdisplayids_result->GetRowCount());

    uint32_t totemdisplayids_count = 0;
    do
    {
        Field* fields = totemdisplayids_result->Fetch();

        uint32_t entry = fields[0].GetUInt32();

        MySQLStructure::TotemDisplayIds& totemDisplayId = _totemDisplayIdsStore[entry];

        totemDisplayId.displayId = entry;
        totemDisplayId.draeneiId = fields[1].GetUInt32();
        totemDisplayId.trollId = fields[2].GetUInt32();
        totemDisplayId.orcId = fields[3].GetUInt32();
#if VERSION_STRING == Cata
        totemDisplayId.taurenId = fields[4].GetUInt32();
        totemDisplayId.dwarfId = fields[5].GetUInt32();
        totemDisplayId.goblinId = fields[6].GetUInt32();
#endif

        ++totemdisplayids_count;
    } while (totemdisplayids_result->NextRow());

    delete totemdisplayids_result;

    LogDetail("MySQLDataLoads : Loaded %u rows from `totemdisplayids` table in %u ms!", totemdisplayids_count, Util::GetTimeDifferenceToNow(startTime));
}

MySQLStructure::TotemDisplayIds const* MySQLDataStore::getTotemDisplayId(uint32_t entry)
{
    TotemDisplayIdContainer::const_iterator itr = _totemDisplayIdsStore.find(entry);
    if (itr != _totemDisplayIdsStore.end())
        return &(itr->second);

    return nullptr;
}

void MySQLDataStore::loadSpellClickSpellsTable()
{
    auto startTime = Util::TimeNow();

    //                                                                      0         1
    QueryResult* spellclickspells_result = WorldDatabase.Query("SELECT CreatureID, SpellID FROM spellclickspells");
    if (spellclickspells_result == nullptr)
    {
        LogNotice("MySQLDataLoads : Table `spellclickspells` is empty!");
        return;
    }

    LogNotice("MySQLDataLoads : Table `spellclickspells` has %u columns", spellclickspells_result->GetFieldCount());

    _spellClickSpellsStore.rehash(spellclickspells_result->GetRowCount());

    uint32_t spellclickspells_count = 0;
    do
    {
        Field* fields = spellclickspells_result->Fetch();

        uint32_t entry = fields[0].GetUInt32();

        SpellClickSpell& spellClickSpells = _spellClickSpellsStore[entry];

        spellClickSpells.CreatureID = entry;
        spellClickSpells.SpellID = fields[1].GetUInt32();

        ++spellclickspells_count;
    } while (spellclickspells_result->NextRow());

    delete spellclickspells_result;

    LogDetail("MySQLDataLoads : Loaded %u rows from `spellclickspells` table in %u ms!", spellclickspells_count, Util::GetTimeDifferenceToNow(startTime));
}

SpellClickSpell const* MySQLDataStore::getSpellClickSpell(uint32_t entry)
{
    SpellClickSpellContainer::const_iterator itr = _spellClickSpellsStore.find(entry);
    if (itr != _spellClickSpellsStore.end())
        return &(itr->second);

    return nullptr;
}

void MySQLDataStore::loadWorldStringsTable()
{
    auto startTime = Util::TimeNow();

    //                                                                     0     1
    QueryResult* worldstring_tables_result = WorldDatabase.Query("SELECT entry, text FROM worldstring_tables");
    if (worldstring_tables_result == nullptr)
    {
        LogNotice("MySQLDataLoads : Table `worldstring_tables` is empty!");
        return;
    }

    LogNotice("MySQLDataLoads : Table `worldstring_tables` has %u columns", worldstring_tables_result->GetFieldCount());

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

    LogDetail("MySQLDataLoads : Loaded %u rows from `worldstring_tables` table in %u ms!", worldstring_tables_count, Util::GetTimeDifferenceToNow(startTime));
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
        LogNotice("MySQLDataLoads : Table `points_of_interest` is empty!");
        return;
    }

    LogNotice("MySQLDataLoads : Table `points_of_interest` has %u columns", points_of_interest_result->GetFieldCount());

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

    LogDetail("MySQLDataLoads : Loaded %u rows from `points_of_interest` table in %u ms!", points_of_interest_count, Util::GetTimeDifferenceToNow(startTime));
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
        LogNotice("MySQLDataLoads : Table `itemset_linked_itemsetbonus` is empty!");
        return;
    }

    LogNotice("MySQLDataLoads : Table `itemset_linked_itemsetbonus` has %u columns", linked_set_bonus_result->GetFieldCount());

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

    LogDetail("MySQLDataLoads : Loaded %u rows from `itemset_linked_itemsetbonus` table in %u ms!", linked_set_bonus_count, Util::GetTimeDifferenceToNow(startTime));
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
        LogNotice("MySQLDataLoads : Table `creature_initial_equip` is empty!");
        return;
    }

    LogNotice("MySQLDataLoads : Table `creature_initial_equip` has %u columns", initial_equipment_result->GetFieldCount());

    uint32_t initial_equipment_count = 0;
    do
    {
        Field* fields = initial_equipment_result->Fetch();
        uint32_t entry = fields[0].GetUInt32();
        CreatureProperties const* creature_properties = sMySQLStore.getCreatureProperties(entry);
        if (creature_properties == nullptr)
        {
            LOG_ERROR("Invalid creature_entry %u in table creature_initial_equip!", entry);
            continue;
        }

        const_cast<CreatureProperties*>(creature_properties)->itemslot_1 = fields[1].GetUInt32();
        const_cast<CreatureProperties*>(creature_properties)->itemslot_2 = fields[2].GetUInt32();
        const_cast<CreatureProperties*>(creature_properties)->itemslot_3 = fields[3].GetUInt32();

        ++initial_equipment_count;

    } while (initial_equipment_result->NextRow());

    delete initial_equipment_result;

    LogDetail("MySQLDataLoads : Loaded %u rows from `creature_initial_equip` table in %u ms!", initial_equipment_count, Util::GetTimeDifferenceToNow(startTime));
}

void MySQLDataStore::loadPlayerCreateInfoTable()
{
    auto startTime = Util::TimeNow();

    //                                                                     0       1           2           3      4       5        6          7          8          9            10
    QueryResult* player_create_info_result = WorldDatabase.Query("SELECT `Index`, race, factiontemplate, class, mapID, zoneID, positionX, positionY, positionZ, orientation, displayID, "
    //                                                                11            12           13           14           15           16         17        18        19
                                                                "BaseStrength, BaseAgility, BaseStamina, BaseIntellect, BaseSpirit, BaseHealth, BaseMana, BaseRage, BaseFocus, "
    //                                                                20         21         22      23       24       25
                                                                "BaseEnergy, attackpower, mindmg, maxdmg, introid, taximask FROM playercreateinfo;");
    if (player_create_info_result == nullptr)
    {
        LogNotice("MySQLDataLoads : Table `playercreateinfo` is empty!");
        return;
    }

    LogNotice("MySQLDataLoads : Table `playercreateinfo` has %u columns", player_create_info_result->GetFieldCount());

    do
    {
        Field* fields = player_create_info_result->Fetch();
        uint32_t player_info_index = fields[0].GetUInt32();
        PlayerCreateInfo& playerCreateInfo = _playerCreateInfoStore[player_info_index];

        playerCreateInfo.race = fields[1].GetUInt8();
        playerCreateInfo.factiontemplate = fields[2].GetUInt32();
        playerCreateInfo.class_ = fields[3].GetUInt8();
        playerCreateInfo.mapId = fields[4].GetUInt32();
        playerCreateInfo.zoneId = fields[5].GetUInt32();
        playerCreateInfo.positionX = fields[6].GetFloat();
        playerCreateInfo.positionY = fields[7].GetFloat();
        playerCreateInfo.positionZ = fields[8].GetFloat();
        playerCreateInfo.orientation = fields[9].GetFloat();
        playerCreateInfo.displayId = fields[10].GetUInt16();
        playerCreateInfo.strength = fields[11].GetUInt8();
        playerCreateInfo.ability = fields[12].GetUInt8();
        playerCreateInfo.stamina = fields[13].GetUInt8();
        playerCreateInfo.intellect = fields[14].GetUInt8();
        playerCreateInfo.spirit = fields[15].GetUInt8();
        playerCreateInfo.health = fields[16].GetUInt32();
        playerCreateInfo.mana = fields[17].GetUInt32();
        playerCreateInfo.rage = fields[18].GetUInt32();
        playerCreateInfo.focus = fields[19].GetUInt32();
        playerCreateInfo.energy = fields[20].GetUInt32();
        playerCreateInfo.attackpower = fields[21].GetUInt32();
        playerCreateInfo.mindmg = fields[22].GetFloat();
        playerCreateInfo.maxdmg = fields[23].GetFloat();
        playerCreateInfo.introid = fields[24].GetUInt32();

        std::string taxiMaskStr = fields[25].GetString();
        std::vector<std::string> tokens = Util::SplitStringBySeperator(taxiMaskStr, " ");

        memset(playerCreateInfo.taximask, 0, sizeof(playerCreateInfo.taximask));
        int index;
        std::vector<std::string>::iterator iter;
        for (iter = tokens.begin(), index = 0; (index < 12) && (iter != tokens.end()); ++iter, ++index)
        {
            playerCreateInfo.taximask[index] = atol((*iter).c_str());
        }

        loadPlayerCreateInfoBarsTable(player_info_index);

    } while (player_create_info_result->NextRow());

    delete player_create_info_result;

    LogDetail("MySQLDataLoads : Loaded %u rows from `playercreateinfo` table in %u ms!", _playerCreateInfoStore.size(), Util::GetTimeDifferenceToNow(startTime));
}

void MySQLDataStore::loadPlayerCreateInfoSkillsTable()
{
    auto startTime = Util::TimeNow();

    //                                                                              0       1       2        3
    QueryResult* player_create_info_skills_result = WorldDatabase.Query("SELECT Indexid, skillid, level, maxlevel FROM playercreateinfo_skills;");

    if (player_create_info_skills_result == nullptr)
    {
        LogNotice("MySQLDataLoads : Table `playercreateinfo_skills` is empty!");
        return;
    }

    LogNotice("MySQLDataLoads : Table `playercreateinfo_skills` has %u columns", player_create_info_skills_result->GetFieldCount());

    uint32_t player_create_info_skills_count = 0;
    do
    {
        Field* fields = player_create_info_skills_result->Fetch();

        uint32_t player_info_index = fields[0].GetUInt32();
        uint32_t skill_id = fields[1].GetUInt32();

        auto player_skill = sSkillLineStore.LookupEntry(skill_id);
        if (player_skill == nullptr)
        {
            LOG_ERROR("Table `playercreateinfo_skills` includes invalid skill id %u for index %u", skill_id, player_info_index);
            continue;
        }

        PlayerCreateInfo& playerCreateInfo = _playerCreateInfoStore[player_info_index];

        CreateInfo_SkillStruct tsk;
        tsk.skillid = fields[1].GetUInt32();
        tsk.currentval = fields[2].GetUInt32();
        tsk.maxval = fields[3].GetUInt32();

        playerCreateInfo.skills.push_back(tsk);

        ++player_create_info_skills_count;

    } while (player_create_info_skills_result->NextRow());

    delete player_create_info_skills_result;

    LogDetail("MySQLDataLoads : Loaded %u rows from `playercreateinfo_skills` table in %u ms!", player_create_info_skills_count, Util::GetTimeDifferenceToNow(startTime));
}

void MySQLDataStore::loadPlayerCreateInfoSpellsTable()
{
    auto startTime = Util::TimeNow();

    //                                                                            0       1
    QueryResult* player_create_info_spells_result = WorldDatabase.Query("SELECT indexid, spellid FROM playercreateinfo_spells");

    if (player_create_info_spells_result == nullptr)
    {
        LogNotice("MySQLDataLoads : Table `playercreateinfo_spells` is empty!");
        return;
    }

    LogNotice("MySQLDataLoads : Table `playercreateinfo_spells` has %u columns", player_create_info_spells_result->GetFieldCount());

    uint32_t player_create_info_spells_count = 0;
    do
    {
        Field* fields = player_create_info_spells_result->Fetch();

        uint32_t player_info_index = fields[0].GetUInt32();
        uint32_t spell_id = fields[1].GetUInt32();

        auto player_spell = sSpellStore.LookupEntry(spell_id);
        if (player_spell == nullptr)
        {
            LOG_ERROR("Table `playercreateinfo_spells` includes invalid spell %u for index %u", spell_id, player_info_index);
            continue;
        }

        PlayerCreateInfo& playerCreateInfo = _playerCreateInfoStore[player_info_index];

        playerCreateInfo.spell_list.insert(spell_id);

        ++player_create_info_spells_count;

    } while (player_create_info_spells_result->NextRow());

    delete player_create_info_spells_result;

    LogDetail("MySQLDataLoads : Loaded %u rows from `playercreateinfo_spells` table in %u ms!", player_create_info_spells_count, Util::GetTimeDifferenceToNow(startTime));
}

void MySQLDataStore::loadPlayerCreateInfoItemsTable()
{
    auto startTime = Util::TimeNow();

    //                                                                            0        1       2        3
    QueryResult* player_create_info_items_result = WorldDatabase.Query("SELECT indexid, protoid, slotid, amount FROM playercreateinfo_items;");

    if (player_create_info_items_result == nullptr)
    {
        LogNotice("MySQLDataLoads : Table `playercreateinfo_items` is empty!");
        return;
    }

    LogNotice("MySQLDataLoads : Table `playercreateinfo_items` has %u columns", player_create_info_items_result->GetFieldCount());

    uint32_t player_create_info_items_count = 0;
    do
    {
        Field* fields = player_create_info_items_result->Fetch();

        uint32_t player_info_index = fields[0].GetUInt32();
        uint32_t item_id = fields[1].GetUInt32();

#if VERSION_STRING != Cata
        auto player_item = sMySQLStore.getItemProperties(item_id);
#else
        DB2::Structures::ItemEntry const* player_item = sItemStore.LookupEntry(item_id);
#endif
        if (player_item == nullptr)
        {
            LOG_ERROR("Table `playercreateinfo_items` includes invalid item %u for index %u", item_id, player_info_index);
            continue;
        }

        PlayerCreateInfo& playerCreateInfo = _playerCreateInfoStore[player_info_index];

        CreateInfo_ItemStruct itm;
        itm.protoid = fields[1].GetUInt32();
        itm.slot = fields[2].GetUInt8();
        itm.amount = fields[3].GetUInt32();

        playerCreateInfo.items.push_back(itm);

        ++player_create_info_items_count;

    } while (player_create_info_items_result->NextRow());

    delete player_create_info_items_result;

    LogDetail("MySQLDataLoads : Loaded %u rows from `playercreateinfo_items` table in %u ms!", player_create_info_items_count, Util::GetTimeDifferenceToNow(startTime));
}

void MySQLDataStore::loadPlayerCreateInfoBarsTable(uint32_t player_info_index)
{
    PlayerCreateInfo& playerCreateInfo = _playerCreateInfoStore[player_info_index];

    //                                                                          0     1      2        3      4     5
    QueryResult* player_create_info_bars_result = WorldDatabase.Query("SELECT race, class, button, action, type, misc FROM playercreateinfo_bars WHERE class = %u;", uint32_t(playerCreateInfo.class_));

    if (player_create_info_bars_result == nullptr)
    {
        LogNotice("MySQLDataLoads : Table `playercreateinfo_bars` has no data for class %u", uint32_t(playerCreateInfo.class_));
        return;
    }

    //LogNotice("MySQLDataLoads : Table `playercreateinfo_bars` has %u columns", player_create_info_bars_result->GetFieldCount());

    uint32_t player_create_info_bars_count = 0;
    do
    {
        Field* fields = player_create_info_bars_result->Fetch();

        CreateInfo_ActionBarStruct bar;
        bar.button = fields[2].GetUInt32();
        bar.action = fields[3].GetUInt32();
        bar.type = fields[4].GetUInt32();
        bar.misc = fields[5].GetUInt32();

        playerCreateInfo.actionbars.push_back(bar);

        ++player_create_info_bars_count;

    } while (player_create_info_bars_result->NextRow());

    delete player_create_info_bars_result;
}

PlayerCreateInfo const* MySQLDataStore::getPlayerCreateInfo(uint8_t player_race, uint8_t player_class)
{
    PlayerCreateInfoContainer::const_iterator itr;
    for (itr = _playerCreateInfoStore.begin(); itr != _playerCreateInfoStore.end(); ++itr)
    {
        if ((itr->second.race == player_race) && (itr->second.class_ == player_class))
            return &(itr->second);
    }
    return nullptr;
}


void MySQLDataStore::loadPlayerXpToLevelTable()
{
    auto startTime = Util::TimeNow();

    _playerXPperLevelStore.clear();
    _playerXPperLevelStore.resize(worldConfig.player.playerLevelCap);

    for (uint32_t level = 0; level < worldConfig.player.playerLevelCap; ++level)
        _playerXPperLevelStore[level] = 0;

    QueryResult* player_xp_to_level_result = WorldDatabase.Query("SELECT player_lvl, next_lvl_req_xp FROM player_xp_for_level");
    if (player_xp_to_level_result == nullptr)
    {
        LogNotice("MySQLDataLoads : Table `player_xp_for_level` is empty!");
        return;
    }

    LogNotice("MySQLDataLoads : Table `playercreateinfo_bars` has %u columns", player_xp_to_level_result->GetFieldCount());

    uint32_t player_xp_to_level_count = 0;
    do
    {
        Field* fields = player_xp_to_level_result->Fetch();
        uint32_t current_level = fields[0].GetUInt8();
        uint32_t current_xp = fields[1].GetUInt32();

        if (current_level >= worldConfig.player.playerLevelCap)
        {
            LOG_ERROR("Table `player_xp_for_level` includes invalid xp definitions for level %u which is higher than the defined levelcap in your config file! <skipped>", current_level);
            continue;
        }

        _playerXPperLevelStore[current_level] = current_xp;

        ++player_xp_to_level_count;

    } while (player_xp_to_level_result->NextRow());

    delete player_xp_to_level_result;

    LogDetail("MySQLDataLoads : Loaded %u rows from `player_xp_for_level` table in %u ms!", player_xp_to_level_count, Util::GetTimeDifferenceToNow(startTime));

    if (player_xp_to_level_count < (worldConfig.player.playerLevelCap - 1))
        LOG_ERROR("Table `player_xp_for_level` includes definitions for %u level, but your defined level cap is %u!", player_xp_to_level_count, worldConfig.player.playerLevelCap);
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
        LogNotice("MySQLDataLoads : Table `spelloverride` is empty!");
        return;
    }

    do
    {
        Field* fields = spelloverride_result->Fetch();
        uint32_t distinct_override_id = fields[0].GetUInt32();

        QueryResult* spellid_for_overrideid_result = WorldDatabase.Query("SELECT spellId FROM spelloverride WHERE overrideId = %u", distinct_override_id);
        std::list<SpellInfo*>* list = new std::list <SpellInfo*>;
        if (spellid_for_overrideid_result != nullptr)
        {
            do
            {
                Field* fieldsIn = spellid_for_overrideid_result->Fetch();
                uint32_t spellid = fieldsIn[0].GetUInt32();
                SpellInfo* spell = sSpellCustomizations.GetSpellInfo(spellid);
                if (spell == nullptr)
                {
                    LOG_ERROR("Table `spelloverride` includes invalid spellId %u for overrideId %u! <skipped>", spellid, distinct_override_id);
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
            _spellOverrideIdStore.insert(SpellOverrideIdMap::value_type(distinct_override_id, list));
        }

    } while (spelloverride_result->NextRow());

    delete spelloverride_result;

    LogDetail("MySQLDataLoads : %u spell overrides loaded.", _spellOverrideIdStore.size());
}

void MySQLDataStore::loadNpcGossipTextIdTable()
{
    auto startTime = Util::TimeNow();
    //                                                    0         1
    QueryResult* npc_gossip_textid_result = WorldDatabase.Query("SELECT creatureid, textid FROM npc_gossip_textid");
    if (npc_gossip_textid_result == nullptr)
    {
        LogNotice("MySQLDataLoads : Table `npc_gossip_textid` is empty!");
        return;
    }

    LogNotice("MySQLDataLoads : Table `npc_gossip_textid` has %u columns", npc_gossip_textid_result->GetFieldCount());

    uint32_t npc_gossip_textid_count = 0;
    do
    {
        Field* fields = npc_gossip_textid_result->Fetch();
        uint32_t entry = fields[0].GetUInt32();
        auto creature_properties = sMySQLStore.getCreatureProperties(entry);
        if (creature_properties == nullptr)
        {
            LOG_ERROR("Table `npc_gossip_textid` includes invalid creatureid %u! <skipped>", entry);
            continue;
        }

        uint32_t text = fields[1].GetUInt32();

        _npcGossipTextIdStore[entry] = text;

        ++npc_gossip_textid_count;

    } while (npc_gossip_textid_result->NextRow());

    delete npc_gossip_textid_result;

    LogDetail("MySQLDataLoads : Loaded %u rows from `npc_gossip_textid` table in %u ms!", npc_gossip_textid_count, Util::GetTimeDifferenceToNow(startTime));
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
        LogNotice("MySQLDataLoads : Table `pet_level_abilities` is empty!");
        return;
    }

    LogNotice("MySQLDataLoads : Table `pet_level_abilities` has %u columns", pet_level_abilities_result->GetFieldCount());

    _petLevelAbilitiesStore.rehash(pet_level_abilities_result->GetRowCount());

    uint32_t pet_level_abilities_count = 0;
    do
    {
        Field* fields = pet_level_abilities_result->Fetch();

        uint32_t entry = fields[0].GetInt32();

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

    LogDetail("MySQLDataLoads : Loaded %u rows from `pet_level_abilities` table in %u ms!", pet_level_abilities_count, Util::GetTimeDifferenceToNow(startTime));

    if (pet_level_abilities_count < worldConfig.player.playerLevelCap)
        LOG_ERROR("Table `pet_level_abilities` includes definitions for %u level, but your defined level cap is %u!", pet_level_abilities_count, worldConfig.player.playerLevelCap);
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
        LogNotice("MySQLDataLoads : Table `worldbroadcast` is empty!");
        return;
    }

    LogNotice("MySQLDataLoads : Table `worldbroadcast` has %u columns", broadcast_result->GetFieldCount());

    _worldBroadcastStore.rehash(broadcast_result->GetRowCount());

    uint32_t broadcast_count = 0;
    do
    {
        Field* fields = broadcast_result->Fetch();

        uint32_t entry = fields[0].GetInt32();

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

    LogDetail("MySQLDataLoads : Loaded %u rows from `worldbroadcast` table in %u ms!", broadcast_count, Util::GetTimeDifferenceToNow(startTime));
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
        LogNotice("MySQLDataLoads : Table `areatriggers` is empty!");
        return;
    }

    LogNotice("MySQLDataLoads : Table `areatriggers` has %u columns", area_trigger_result->GetFieldCount());

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
            LogDebugFlag(LF_DB_TABLES, "AreaTrigger : Area trigger (ID:%u) does not exist in `AreaTrigger.dbc`.", areaTrigger.id);
            continue;
        }

        DBC::Structures::MapEntry const* map_entry = sMapStore.LookupEntry(areaTrigger.mapId);
        if (!map_entry)
        {
            LogDebugFlag(LF_DB_TABLES, "AreaTrigger : Area trigger (ID:%u) target map (ID: %u) does not exist in `Map.dbc`.", areaTrigger.id, areaTrigger.mapId);
            continue;
        }

        if (areaTrigger.x == 0 && areaTrigger.y == 0 && areaTrigger.z == 0 && (areaTrigger.type == ATTYPE_INSTANCE || areaTrigger.type == ATTYPE_TELEPORT))    // check target coordinates only for teleport triggers
        {
            LogDebugFlag(LF_DB_TABLES, "AreaTrigger : Area trigger (ID:%u) target coordinates not provided.", areaTrigger.id);
            continue;
        }

        _areaTriggerStore[areaTrigger.id] = areaTrigger;
        ++areaTrigger_count;

    } while (area_trigger_result->NextRow());

    delete area_trigger_result;

    LogDetail("MySQLDataLoads : Loaded %u rows from `areatriggers` table in %u ms!", areaTrigger_count, Util::GetTimeDifferenceToNow(startTime));
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

void MySQLDataStore::loadWordFilterCharacterNames()
{
    auto startTime = Util::TimeNow();

    QueryResult* filter_character_names_result = WorldDatabase.Query("SELECT * FROM wordfilter_character_names");
    if (filter_character_names_result == nullptr)
    {
        LogNotice("MySQLDataLoads : Table `wordfilter_character_names` is empty!");
        return;
    }

    LogNotice("MySQLDataLoads : Table `wordfilter_character_names` has %u columns", filter_character_names_result->GetFieldCount());

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

    LogDetail("MySQLDataLoads : Loaded %u rows from `wordfilter_character_names` table in %u ms!", filter_character_names_count, Util::GetTimeDifferenceToNow(startTime));
}

bool MySQLDataStore::isCharacterNameAllowed(std::string charName)
{
    std::list<MySQLStructure::WordFilterCharacterNames>::const_iterator iterator;
    for (iterator = _wordFilterCharacterNamesStore.begin(); iterator != _wordFilterCharacterNamesStore.end(); ++iterator)
    {
        size_t pos = charName.find(iterator->name);
        if (pos != std::string::npos)
        {
            return true;
        }
    }

    return false;
}

void MySQLDataStore::loadWordFilterChat()
{
    auto startTime = Util::TimeNow();

    QueryResult* filter_chat_result = WorldDatabase.Query("SELECT * FROM wordfilter_chat");
    if (filter_chat_result == nullptr)
    {
        LogNotice("MySQLDataLoads : Table `wordfilter_chat` is empty!");
        return;
    }

    LogNotice("MySQLDataLoads : Table `wordfilter_chat` has %u columns", filter_chat_result->GetFieldCount());

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

    LogDetail("MySQLDataLoads : Loaded %u rows from `wordfilter_chat` table in %u ms!", filter_chat_count, Util::GetTimeDifferenceToNow(startTime));
}

void MySQLDataStore::loadCreatureFormationsTable()
{
    auto startTime = Util::TimeNow();
    //                                                                       0              1              2            3
    QueryResult* creature_formations_result = WorldDatabase.Query("SELECT spawn_id, target_spawn_id, follow_angle, follow_dist FROM creature_formations");
    if (creature_formations_result == nullptr)
    {
        LogNotice("MySQLDataLoads : Table `creature_formations` is empty!");
        return;
    }

    LogNotice("MySQLDataLoads : Table `creature_formations` has %u columns", creature_formations_result->GetFieldCount());

    _creatureFormationsStore.rehash(creature_formations_result->GetRowCount());

    uint32_t formations_count = 0;
    do
    {
        Field* fields = creature_formations_result->Fetch();

        uint32_t spawnId = fields[0].GetInt32();
        QueryResult* spawn_result = WorldDatabase.Query("SELECT id FROM creature_spawns WHERE id = %u", spawnId);
        if (spawn_result == nullptr)
        {
            LogError("Table `creature_formations` includes formation data for invalid spawn id %u. Skipped!", spawnId);
            continue;
        }

        MySQLStructure::CreatureFormation& creatureFormation = _creatureFormationsStore[spawnId];

        creatureFormation.targetSpawnId = fields[1].GetUInt32();
        creatureFormation.followAngle = fields[2].GetFloat();
        creatureFormation.followDistance = fields[3].GetFloat();

        ++formations_count;

    } while (creature_formations_result->NextRow());

    delete creature_formations_result;

    LogDetail("MySQLDataLoads : Loaded %u rows from `creature_formations` table in %u ms!", formations_count, Util::GetTimeDifferenceToNow(startTime));
}

MySQLStructure::CreatureFormation const* MySQLDataStore::getCreatureFormationBySpawnId(uint32_t spawnId)
{
    CreatureFormationsMap::const_iterator itr = _creatureFormationsStore.find(spawnId);
    if (itr != _creatureFormationsStore.end())
        return &(itr->second);

    return nullptr;
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
        LogNotice("MySQLDataLoads : Table `locales_creature` is empty!");
        return;
    }

    LogNotice("MySQLDataLoads : Table `locales_creature` has %u columns", result->GetFieldCount());

    _localesCreatureStore.rehash(result->GetRowCount());

    uint32_t load_count = 0;
    uint32_t i = 0;
    do
    {
        ++i;
        Field* fields = result->Fetch();

        MySQLStructure::LocalesCreature& localCreature = _localesCreatureStore[i];

        localCreature.entry = fields[0].GetInt32();
        std::string locString = fields[1].GetString();
        localCreature.languageCode = Util::getLanguagesIdFromString(locString);
        localCreature.name = strdup(fields[2].GetString());
        localCreature.subName = strdup(fields[3].GetString());

        ++load_count;

    } while (result->NextRow());

    delete result;

    LogDetail("MySQLDataLoads : Loaded %u rows from `locales_creature` table in %u ms!", load_count, Util::GetTimeDifferenceToNow(startTime));
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
        LogNotice("MySQLDataLoads : Table `locales_gameobject` is empty!");
        return;
    }

    LogNotice("MySQLDataLoads : Table `locales_gameobject` has %u columns", result->GetFieldCount());

    _localesGameobjectStore.rehash(result->GetRowCount());

    uint32_t load_count = 0;
    uint32_t i = 0;
    do
    {
        ++i;
        Field* fields = result->Fetch();

        MySQLStructure::LocalesGameobject& localGameobject = _localesGameobjectStore[i];

        localGameobject.entry = fields[0].GetInt32();
        std::string locString = fields[1].GetString();
        localGameobject.languageCode = Util::getLanguagesIdFromString(locString);
        localGameobject.name = strdup(fields[2].GetString());

        ++load_count;

    } while (result->NextRow());

    delete result;

    LogDetail("MySQLDataLoads : Loaded %u rows from `locales_gameobject` table in %u ms!", load_count, Util::GetTimeDifferenceToNow(startTime));
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
        LogNotice("MySQLDataLoads : Table `locales_gossip_menu_option` is empty!");
        return;
    }

    LogNotice("MySQLDataLoads : Table `locales_gossip_menu_option` has %u columns", result->GetFieldCount());

    _localesGossipMenuOptionStore.rehash(result->GetRowCount());

    uint32_t load_count = 0;
    uint32_t i = 0;
    do
    {
        ++i;
        Field* fields = result->Fetch();

        MySQLStructure::LocalesGossipMenuOption& localGossipMenuOption = _localesGossipMenuOptionStore[1];

        localGossipMenuOption.entry = fields[0].GetInt32();
        std::string locString = fields[1].GetString();
        localGossipMenuOption.languageCode = Util::getLanguagesIdFromString(locString);
        localGossipMenuOption.name = strdup(fields[2].GetString());

        ++load_count;

    } while (result->NextRow());

    delete result;

    LogDetail("MySQLDataLoads : Loaded %u rows from `locales_gossip_menu_option` table in %u ms!", load_count, Util::GetTimeDifferenceToNow(startTime));
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
        LogNotice("MySQLDataLoads : Table `locales_item` is empty!");
        return;
    }

    LogNotice("MySQLDataLoads : Table `locales_item` has %u columns", result->GetFieldCount());

    _localesItemStore.rehash(result->GetRowCount());

    uint32_t load_count = 0;
    uint32_t i = 0;
    do
    {
        ++i;
        Field* fields = result->Fetch();

        MySQLStructure::LocalesItem& localItem = _localesItemStore[i];

        localItem.entry = fields[0].GetInt32();
        std::string locString = fields[1].GetString();
        localItem.languageCode = Util::getLanguagesIdFromString(locString);
        localItem.name = strdup(fields[2].GetString());
        localItem.description = strdup(fields[3].GetString());

        ++load_count;

    } while (result->NextRow());

    delete result;

    LogDetail("MySQLDataLoads : Loaded %u rows from `locales_item` table in %u ms!", load_count, Util::GetTimeDifferenceToNow(startTime));
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

void MySQLDataStore::loadLocalesItemPages()
{
    auto startTime = Util::TimeNow();
    //                                                 0         1           2
    QueryResult* result = WorldDatabase.Query("SELECT entry, language_code, text FROM locales_item_pages");
    if (result == nullptr)
    {
        LogNotice("MySQLDataLoads : Table `locales_item_pages` is empty!");
        return;
    }

    LogNotice("MySQLDataLoads : Table `locales_item_pages` has %u columns", result->GetFieldCount());

    _localesItemPagesStore.rehash(result->GetRowCount());

    uint32_t load_count = 0;
    uint32_t i = 0;
    do
    {
        ++i;
        Field* fields = result->Fetch();

        MySQLStructure::LocalesItemPages& localesItemPages = _localesItemPagesStore[i];

        localesItemPages.entry = fields[0].GetInt32();
        std::string locString = fields[1].GetString();
        localesItemPages.languageCode = Util::getLanguagesIdFromString(locString);
        localesItemPages.text = strdup(fields[2].GetString());

        ++load_count;

    } while (result->NextRow());

    delete result;

    LogDetail("MySQLDataLoads : Loaded %u rows from `locales_item_pages` table in %u ms!", load_count, Util::GetTimeDifferenceToNow(startTime));
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

void MySQLDataStore::loadLocalesNPCMonstersay()
{
    auto startTime = Util::TimeNow();
    //                                                                   0      1          2            3         4      5      6      7      8
    QueryResult* local_monstersay_result = WorldDatabase.Query("SELECT entry, type, language_code, monstername, text0, text1, text2, text3, text4 FROM locales_npc_monstersay");
    if (local_monstersay_result == nullptr)
    {
        LogNotice("MySQLDataLoads : Table `locales_npc_monstersay` is empty!");
        return;
    }

    LogNotice("MySQLDataLoads : Table `locales_npc_monstersay` has %u columns", local_monstersay_result->GetFieldCount());

    _localesNPCMonstersayStore.rehash(local_monstersay_result->GetRowCount());

    uint32_t local_monstersay_count = 0;
    uint32_t i = 0;
    do
    {
        ++i;
        Field* fields = local_monstersay_result->Fetch();

        MySQLStructure::LocalesNPCMonstersay& localMonstersay = _localesNPCMonstersayStore[i];

        localMonstersay.entry = fields[0].GetInt32();
        localMonstersay.type = fields[1].GetUInt32();
        std::string locString = fields[2].GetString();
        localMonstersay.languageCode = Util::getLanguagesIdFromString(locString);
        localMonstersay.monstername = strdup(fields[3].GetString());
        localMonstersay.text0 = strdup(fields[4].GetString());
        localMonstersay.text1 = strdup(fields[5].GetString());
        localMonstersay.text2 = strdup(fields[6].GetString());
        localMonstersay.text3 = strdup(fields[7].GetString());
        localMonstersay.text4 = strdup(fields[8].GetString());

        ++local_monstersay_count;

    } while (local_monstersay_result->NextRow());

    delete local_monstersay_result;

    LogDetail("MySQLDataLoads : Loaded %u rows from `locales_npc_monstersay` table in %u ms!", local_monstersay_count, Util::GetTimeDifferenceToNow(startTime));
}

MySQLStructure::LocalesNPCMonstersay const* MySQLDataStore::getLocalizedMonsterSay(uint32_t entry, uint32_t sessionLocale, uint32_t event)
{
    for (LocalesNPCMonstersayContainer::const_iterator itr = _localesNPCMonstersayStore.begin(); itr != _localesNPCMonstersayStore.end(); ++itr)
    {
        if (itr->second.entry == entry)
        {
            if (itr->second.languageCode == sessionLocale)
            {
                if (itr->second.type == event)
                {
                    return &itr->second;
                }
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
        LogNotice("MySQLDataLoads : Table `locales_npc_script_text` is empty!");
        return;
    }

    LogNotice("MySQLDataLoads : Table `locales_npc_script_text` has %u columns", result->GetFieldCount());

    _localesNpcScriptTextStore.rehash(result->GetRowCount());

    uint32_t load_count = 0;
    uint32_t i = 0;
    do
    {
        ++i;
        Field* fields = result->Fetch();

        MySQLStructure::LocalesNpcScriptText& localNpcScriptText = _localesNpcScriptTextStore[i];

        localNpcScriptText.entry = fields[0].GetInt32();
        std::string locString = fields[1].GetString();
        localNpcScriptText.languageCode = Util::getLanguagesIdFromString(locString);
        localNpcScriptText.text = strdup(fields[2].GetString());

        ++load_count;

    } while (result->NextRow());

    delete result;

    LogDetail("MySQLDataLoads : Loaded %u rows from `locales_npc_script_text` table in %u ms!", load_count, Util::GetTimeDifferenceToNow(startTime));
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
    QueryResult* result = WorldDatabase.Query("SELECT entry, language_code, text0, text0_1, text1, text1_1, text2, text2_1, text3, text3_1, text4, text4_1, text5, text5_1, text6, text6_1, text7, text7_1 FROM locales_npc_text");
    if (result == nullptr)
    {
        LogNotice("MySQLDataLoads : Table `locales_npc_text` is empty!");
        return;
    }

    LogNotice("MySQLDataLoads : Table `locales_npc_text` has %u columns", result->GetFieldCount());

    _localesNpcTextStore.rehash(result->GetRowCount());

    uint32_t load_count = 0;
    uint32_t i = 0;
    do
    {
        ++i;
        Field* fields = result->Fetch();

        MySQLStructure::LocalesNpcText& localNpcText = _localesNpcTextStore[i];

        localNpcText.entry = fields[0].GetInt32();
        std::string locString = fields[1].GetString();
        localNpcText.languageCode = Util::getLanguagesIdFromString(locString);

        for (uint8 j = 0; j < 8; ++j)
        {
            localNpcText.texts[j][0] = strdup(fields[2 + (2 * j)].GetString());
            localNpcText.texts[j][1] = strdup(fields[3 + (2 * j)].GetString());
        }

        ++load_count;

    } while (result->NextRow());

    delete result;

    LogDetail("MySQLDataLoads : Loaded %u rows from `locales_npc_text` table in %u ms!", load_count, Util::GetTimeDifferenceToNow(startTime));
}

MySQLStructure::LocalesNpcText const* MySQLDataStore::getLocalizedNpcText(uint32_t entry, uint32_t sessionLocale)
{
    for (LocalesNpcTextContainer::const_iterator itr = _localesNpcTextStore.begin(); itr != _localesNpcTextStore.end(); ++itr)
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
        LogNotice("MySQLDataLoads : Table `locales_quest` is empty!");
        return;
    }

    LogNotice("MySQLDataLoads : Table `locales_quest` has %u columns", result->GetFieldCount());

    _localesQuestStore.rehash(result->GetRowCount());

    uint32_t load_count = 0;
    uint32_t i = 0;
    do
    {
        ++i;
        Field* fields = result->Fetch();

        MySQLStructure::LocalesQuest& localQuest = _localesQuestStore[i];

        localQuest.entry = fields[0].GetInt32();
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

    LogDetail("MySQLDataLoads : Loaded %u rows from `locales_quest` table in %u ms!", load_count, Util::GetTimeDifferenceToNow(startTime));
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
        LogNotice("MySQLDataLoads : Table `locales_worldbroadcast` is empty!");
        return;
    }

    LogNotice("MySQLDataLoads : Table `locales_worldbroadcast` has %u columns", result->GetFieldCount());

    _localesWorldbroadcastStore.rehash(result->GetRowCount());

    uint32_t load_count = 0;
    uint32_t i = 0;
    do
    {
        ++i;
        Field* fields = result->Fetch();

        MySQLStructure::LocalesWorldbroadcast& localWorldbroadcast = _localesWorldbroadcastStore[i];

        localWorldbroadcast.entry = fields[0].GetInt32();
        std::string locString = fields[1].GetString();
        localWorldbroadcast.languageCode = Util::getLanguagesIdFromString(locString);
        localWorldbroadcast.text = strdup(fields[2].GetString());

        ++load_count;

    } while (result->NextRow());

    delete result;

    LogDetail("MySQLDataLoads : Loaded %u rows from `locales_worldbroadcast` table in %u ms!", load_count, Util::GetTimeDifferenceToNow(startTime));
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
        LogNotice("MySQLDataLoads : Table `locales_worldmap_info` is empty!");
        return;
    }

    LogNotice("MySQLDataLoads : Table `locales_worldmap_info` has %u columns", result->GetFieldCount());

    _localesWorldmapInfoStore.rehash(result->GetRowCount());

    uint32_t load_count = 0;
    uint32_t i = 0;
    do
    {
        ++i;
        Field* fields = result->Fetch();

        MySQLStructure::LocalesWorldmapInfo& localWorldmapInfo = _localesWorldmapInfoStore[i];

        localWorldmapInfo.entry = fields[0].GetInt32();
        std::string locString = fields[1].GetString();
        localWorldmapInfo.languageCode = Util::getLanguagesIdFromString(locString);
        localWorldmapInfo.text = strdup(fields[2].GetString());

        ++load_count;

    } while (result->NextRow());

    delete result;

    LogDetail("MySQLDataLoads : Loaded %u rows from `locales_worldmap_info` table in %u ms!", load_count, Util::GetTimeDifferenceToNow(startTime));
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
        LogNotice("MySQLDataLoads : Table `locales_worldstring_table` is empty!");
        return;
    }

    LogNotice("MySQLDataLoads : Table `locales_worldstring_table` has %u columns", result->GetFieldCount());

    _localesWorldStringTableStore.rehash(result->GetRowCount());

    uint32_t load_count = 0;
    uint32_t i = 0;
    do
    {
        ++i;
        Field* fields = result->Fetch();

        MySQLStructure::LocalesWorldStringTable& localWorldStringTable = _localesWorldStringTableStore[i];

        localWorldStringTable.entry = fields[0].GetInt32();
        std::string locString = fields[1].GetString();
        localWorldStringTable.languageCode = Util::getLanguagesIdFromString(locString);
        localWorldStringTable.text = strdup(fields[2].GetString());

        ++load_count;

    } while (result->NextRow());

    delete result;

    LogDetail("MySQLDataLoads : Loaded %u rows from `locales_worldstring_table` table in %u ms!", load_count, Util::GetTimeDifferenceToNow(startTime));
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

void MySQLDataStore::loadNpcMonstersayTable()
{
    auto startTime = Util::TimeNow();
    //                                                  0      1       2        3       4       5          6      7      8      9     10
    QueryResult* result = WorldDatabase.Query("SELECT entry, event, chance, language, type, monstername, text0, text1, text2, text3, text4 FROM npc_monstersay");
    if (result == nullptr)
    {
        LogNotice("MySQLDataLoads : Table `npc_monstersay` is empty!");
        return;
    }

    LogNotice("MySQLDataLoads : Table `npc_monstersay` has %u columns", result->GetFieldCount());

    uint32_t load_count = 0;
    do
    {
        Field* fields = result->Fetch();
        uint32_t entry = fields[0].GetUInt32();
        uint32_t creatureEvent = fields[1].GetUInt32();

        if (creatureEvent >= NUM_MONSTER_SAY_EVENTS)
        {
            continue;
        }

        if (_npcMonstersayContainer[creatureEvent].find(entry) != _npcMonstersayContainer[creatureEvent].end())
        {
            LogDebugFlag(LF_DB_TABLES, "Duplicate npc_monstersay event %u for entry %u, skipping", creatureEvent, entry);
            continue;
        }

        MySQLStructure::NpcMonsterSay* npcMonsterSay = new MySQLStructure::NpcMonsterSay;
        npcMonsterSay->chance = fields[2].GetFloat();
        npcMonsterSay->language = fields[3].GetUInt32();
        npcMonsterSay->type = fields[4].GetUInt32();
        npcMonsterSay->monsterName = fields[5].GetString() ? strdup(fields[5].GetString()) : strdup("None");

        char* texts[5];
        char* text;
        uint32_t textcount = 0;

        for (int i = 0; i < 5; ++i)
        {
            text = (char*)fields[6 + i].GetString();
            if (!text)
            {
                continue;
            }

            if (strlen(fields[6 + i].GetString()) < 5)
            {
                continue;
            }

            texts[textcount] = strdup(fields[6 + i].GetString());

            if (texts[textcount][strlen(texts[textcount]) - 1] == ';')
            {
                texts[textcount][strlen(texts[textcount]) - 1] = 0;
            }

            ++textcount;
        }

        if (textcount == 0)
        {
            free(((char*)npcMonsterSay->monsterName));
            delete npcMonsterSay;
            continue;
        }

        npcMonsterSay->texts = new const char*[textcount];
        memcpy(npcMonsterSay->texts, texts, sizeof(char*) * textcount);
        npcMonsterSay->textCount = textcount;

        _npcMonstersayContainer[creatureEvent].insert(std::make_pair(entry, npcMonsterSay));

        ++load_count;
    } while (result->NextRow());

    delete result;

    LogDetail("MySQLDataLoads : Loaded %u rows from `npc_monstersay` table in %u ms!", load_count, Util::GetTimeDifferenceToNow(startTime));
}

MySQLStructure::NpcMonsterSay* MySQLDataStore::getMonstersayEventForCreature(uint32_t entry, MONSTER_SAY_EVENTS _event)
{
    if (_npcMonstersayContainer[_event].empty())
    {
        return nullptr;
    }

    NpcMonstersayContainer::iterator itr = _npcMonstersayContainer[_event].find(entry);
    if (itr != _npcMonstersayContainer[_event].end())
    {
        return itr->second;
    }

    return nullptr;
}

//\brief Data loaded but never used!    Zyres 2017/07/16 not used
//void MySQLDataStore::loadDefaultPetSpellsTable()
//{
//    auto startTime = Util::TimeNow();
//    //                                                  0      1
//    QueryResult* result = WorldDatabase.Query("SELECT entry, spell FROM petdefaultspells");
//    if (result == nullptr)
//    {
//        LogNotice("MySQLDataLoads : Table `petdefaultspells` is empty!");
//        return;
//    }
//
//    LogNotice("MySQLDataLoads : Table `petdefaultspells` has %u columns", result->GetFieldCount());
//
//    uint32_t load_count = 0;
//    do
//    {
//        Field* fields = result->Fetch();
//        uint32 entry = fields[0].GetUInt32();
//        uint32 spell = fields[1].GetUInt32();
//        SpellInfo* spellInfo = sSpellCustomizations.GetSpellInfo(spell);
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
//                std::set<SpellInfo*> spellInfoSet;
//                spellInfoSet.insert(spellInfo);
//                _defaultPetSpellsStore[entry] = spellInfoSet;
//            }
//        }
//    } while (result->NextRow());
//
//    delete result;
//
//    LogDetail("MySQLDataLoads : Loaded %u rows from `petdefaultspells` table in %u ms!", load_count, Util::GetTimeDifferenceToNow(startTime));
//}

//\brief This function is never called!     Zyres 2017/07/16 not used
//std::set<SpellInfo*>* MySQLDataStore::getDefaultPetSpellsByEntry(uint32_t entry)
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
        LogNotice("MySQLDataLoads : Table `professiondiscoveries` is empty!");
        return;
    }

    LogNotice("MySQLDataLoads : Table `professiondiscoveries` has %u columns", result->GetFieldCount());

    if (result != nullptr)
    {
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

        LogDetail("MySQLDataLoads : Loaded %u rows from `professiondiscoveries` table in %u ms!", load_count, Util::GetTimeDifferenceToNow(startTime));
    }
}

void MySQLDataStore::loadTransportCreaturesTable()
{
    auto startTime = Util::TimeNow();
    //                                                  0       1              2              3            4              5            6          7
    QueryResult* result = WorldDatabase.Query("SELECT guid, npc_entry, transport_entry, TransOffsetX, TransOffsetY, TransOffsetZ, TransOffsetO, emote FROM transport_creatures");
    if (result == nullptr)
    {
        LogNotice("MySQLDataLoads : Table `transport_creatures` is empty!");
        return;
    }

    LogNotice("MySQLDataLoads : Table `transport_creatures` has %u columns", result->GetFieldCount());

    if (result != nullptr)
    {
        uint32_t load_count = 0;
        do
        {
            Field* fields = result->Fetch();
            MySQLStructure::TransportCreatures& transportCreature = _transportCreaturesStore[load_count];
            transportCreature.guid = fields[0].GetUInt32();
            transportCreature.entry = fields[1].GetUInt32();
            transportCreature.transportEntry = fields[2].GetUInt32();
            transportCreature.transportOffsetX = fields[3].GetFloat();
            transportCreature.transportOffsetY = fields[4].GetFloat();
            transportCreature.transportOffsetZ = fields[5].GetFloat();
            transportCreature.transportOffsetO = fields[6].GetFloat();
            transportCreature.animation = fields[7].GetUInt32();

            ++load_count;

        } while (result->NextRow());

        delete result;

        LogDetail("MySQLDataLoads : Loaded %u rows from `transport_creatures` table in %u ms!", load_count, Util::GetTimeDifferenceToNow(startTime));
    }
}

void MySQLDataStore::loadTransportDataTable()
{
    auto startTime = Util::TimeNow();
    //                                                  0      1     2
    QueryResult* result = WorldDatabase.Query("SELECT entry, name, period FROM transport_data");
    if (result == nullptr)
    {
        LogNotice("MySQLDataLoads : Table `transport_data` is empty!");
        return;
    }

    LogNotice("MySQLDataLoads : Table `transport_data` has %u columns", result->GetFieldCount());

    if (result != nullptr)
    {
        uint32_t load_count = 0;
        do
        {
            Field* fields = result->Fetch();
            uint32_t entry = fields[0].GetUInt32();

            GameObjectProperties const* gameobject_info = sMySQLStore.getGameObjectProperties(entry);
            if (gameobject_info == nullptr)
            {
                LOG_ERROR("Transport entry: %u, will not be loaded, gameobject_properties missing", entry);
                continue;
            }

            if (gameobject_info->type != GAMEOBJECT_TYPE_MO_TRANSPORT)
            {
                LOG_ERROR("Transport entry: %u, will not be loaded, gameobject_properties type wrong", entry);
                continue;
            }

            MySQLStructure::TransportData& transportData = _transportDataStore[entry];
            transportData.entry = entry;
            transportData.name = fields[1].GetString();
            transportData.period = fields[2].GetUInt32();

            ++load_count;

        } while (result->NextRow());

        delete result;

        LogDetail("MySQLDataLoads : Loaded %u rows from `transport_data` table in %u ms!", load_count, Util::GetTimeDifferenceToNow(startTime));
    }
}

void MySQLDataStore::loadGossipMenuItemsTable()
{
    auto startTime = Util::TimeNow();

    //                                                      0          1
    QueryResult* result = WorldDatabase.Query("SELECT gossip_menu, text_id FROM gossip_menu ORDER BY gossip_menu");
    if (result == nullptr)
    {
        LogNotice("MySQLDataLoads : Table `gossip_menu` is empty!");
        return;
    }

    LogNotice("MySQLDataLoads : Table `gossip_menu` has %u columns", result->GetFieldCount());

    if (result != nullptr)
    {
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

        LogDetail("MySQLDataLoads : Loaded %u rows from `gossip_menu` table in %u ms!", load_count, Util::GetTimeDifferenceToNow(startTime));
    }

    _gossipMenuItemsStores.clear();

    //                                                      0       1            2        3            4                  5                6
    QueryResult* resultItems = WorldDatabase.Query("SELECT id, item_order, menu_option, icon, point_of_interest, next_gossip_menu, next_gossip_text FROM gossip_menu_items ORDER BY id, item_order");
    if (resultItems == nullptr)
    {
        LogNotice("MySQLDataLoads : Table `gossip_menu_items` is empty!");
        return;
    }

    LogNotice("MySQLDataLoads : Table `gossip_menu_items` has %u columns", resultItems->GetFieldCount());

    if (resultItems != nullptr)
    {
        uint32_t load_count = 0;
        do
        {
            Field* fields = resultItems->Fetch();

            MySQLStructure::GossipMenuItems gMenuItem;

            gMenuItem.gossipMenu = fields[0].GetUInt32();
            gMenuItem.itemOrder = fields[1].GetUInt32();
            gMenuItem.menuOptionText = fields[2].GetUInt32();
            gMenuItem.icon = fields[3].GetUInt8();
            gMenuItem.pointOfInterest = fields[4].GetUInt32();
            gMenuItem.nextGossipMenu = fields[5].GetUInt32();
            gMenuItem.nextGossipMenuText = fields[6].GetUInt32();

            _gossipMenuItemsStores.insert(GossipMenuItemsContainer::value_type(gMenuItem.gossipMenu, gMenuItem));
            ++load_count;
        } while (resultItems->NextRow());

        delete resultItems;

        LogDetail("MySQLDataLoads : Loaded %u rows from `gossip_menu_items` table in %u ms!", load_count, Util::GetTimeDifferenceToNow(startTime));
    }
}
