/*
Copyright (c) 2014-2016 AscEmu Team <http://www.ascemu.org/>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "StdAfx.h"

initialiseSingleton(MySQLDataStore);

MySQLDataStore::MySQLDataStore() {}
MySQLDataStore::~MySQLDataStore() {}

void MySQLDataStore::LoadItemPagesTable()
{
    uint32 start_time = getMSTime();

    QueryResult* itempages_result = WorldDatabase.Query("SELECT entry, text, next_page FROM itempages");
    if (itempages_result == nullptr)
    {
        Log.Notice("MySQLDataLoads", "Table `itempages` is empty!");
        return;
    }

    Log.Notice("MySQLDataLoads", "Table `itempages` has %u columns", itempages_result->GetFieldCount());

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

    Log.Success("MySQLDataLoads", "Loaded %u pages from `itempages` table in %u ms!", itempages_count, getMSTime() - start_time);
}

ItemPage const* MySQLDataStore::GetItemPage(uint32 entry)
{
    ItemPageContainer::const_iterator itr = _itemPagesStore.find(entry);
    if (itr != _itemPagesStore.end())
        return &(itr->second);

    return nullptr;
}

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
        uint32 page_id = fields[97].GetUInt32();
        if (page_id != 0)
        {
            ItemPage const* item_page = GetItemPage(page_id);
            if (item_page == nullptr)
            {
                Log.Error("MySQLDataLoads", "Table `items` entry: %u includes invalid pageId %u! pageId is set to 0.", entry, page_id);
                itemProto.PageId = 0;
            }
            else
            {
                itemProto.PageId = page_id;
            }
        }
        else
        {
            itemProto.PageId = page_id;
        }

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

        //lowercase
        std::string lower_case_name = itemProto.Name;
        std::transform(lower_case_name.begin(), lower_case_name.end(), lower_case_name.begin(), ::tolower);
        itemProto.lowercase_name = lower_case_name;

        //forced pet entries (hacky stuff ->spells)
        switch (itemProto.ItemId)
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
                itemProto.ForcedPetId = 17252;
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
                itemProto.ForcedPetId = 416;
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
                itemProto.ForcedPetId = 1860;
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
                itemProto.ForcedPetId = 1863;
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
                itemProto.ForcedPetId = 417;
                break;

            case 21283:
            case 3144:
            case 21282:
            case 9214:
            case 21281:
            case 22891:
                // Player
                itemProto.ForcedPetId = 0;
                break;

            default:
                itemProto.ForcedPetId = -1;
                break;
        }


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

void MySQLDataStore::LoadCreatureNamesTable()
{
    uint32 start_time = getMSTime();

    //                                                                 0     1       2         3       4      5      6      7        8          9             10
    QueryResult* creature_names_result = WorldDatabase.Query("SELECT entry, name, subname, info_str, flags1, type, family, rank, encounter, killcredit1, killcredit2, "
    //                                                            11                12              13                 14               15              16          17
                                                            "male_displayid, female_displayid, male_displayid2, female_displayid2, unknown_float1, unknown_float2, leader, "
    //                                                          18          19         20           21          22          23           24
                                                            "questitem1, questitem2, questitem3, questitem4, questitem5, questitem6, waypointid FROM creature_names");

    if (creature_names_result == nullptr)
    {
        Log.Notice("MySQLDataLoads", "Table `creature_names` is empty!");
        return;
    }

    Log.Notice("MySQLDataLoads", "Table `creature_names` has %u columns", creature_names_result->GetFieldCount());

    _creatureNamesStore.rehash(creature_names_result->GetRowCount());

    uint32 creature_names_count = 0;
    do
    {
        Field* fields = creature_names_result->Fetch();

        uint32 entry = fields[0].GetUInt32();

        CreatureInfo& creatureInfo = _creatureNamesStore[entry];

        creatureInfo.Id = entry;
        creatureInfo.Name = fields[1].GetString();
        creatureInfo.SubName = fields[2].GetString();
        creatureInfo.info_str = fields[3].GetString();
        creatureInfo.Flags1 = fields[4].GetUInt32();
        creatureInfo.Type = fields[5].GetUInt32();
        creatureInfo.Family = fields[6].GetUInt32();
        creatureInfo.Rank = fields[7].GetUInt32();
        creatureInfo.Encounter = fields[8].GetUInt32();

        for (uint8 i = 0; i < 2; ++i)
        {
            creatureInfo.killcredit[i] = fields[9 + i].GetUInt32();
        }

        creatureInfo.Male_DisplayID = fields[11].GetUInt32();
        creatureInfo.Female_DisplayID = fields[12].GetUInt32();
        creatureInfo.Male_DisplayID2 = fields[13].GetUInt32();
        creatureInfo.Female_DisplayID2 = fields[14].GetUInt32();
        creatureInfo.unkfloat1 = fields[15].GetFloat();
        creatureInfo.unkfloat2 = fields[16].GetFloat();
        creatureInfo.Leader = fields[17].GetUInt8();

        for (uint8 i = 0; i < 6; ++i)
        {
            creatureInfo.QuestItems[i] = fields[18 + i].GetUInt32();
        }

        creatureInfo.waypointid = fields[24].GetUInt32();

        //lowercase
        std::string lower_case_name = creatureInfo.Name;
        std::transform(lower_case_name.begin(), lower_case_name.end(), lower_case_name.begin(), ::tolower);
        creatureInfo.lowercase_name = lower_case_name;

        //monster say
        for (uint8 i = 0; i < NUM_MONSTER_SAY_EVENTS; i++)
            creatureInfo.MonsterSay[i] = objmgr.HasMonsterSay(creatureInfo.Id, MONSTER_SAY_EVENTS(i));


        ++creature_names_count;
    } while (creature_names_result->NextRow());

    delete creature_names_result;

    Log.Success("MySQLDataLoads", "Loaded %u creature info from `creature_names` table in %u ms!", creature_names_count, getMSTime() - start_time);
}

CreatureInfo const* MySQLDataStore::GetCreatureInfo(uint32 entry)
{
    CreatureInfoContainer::const_iterator itr = _creatureNamesStore.find(entry);
    if (itr != _creatureNamesStore.end())
        return &(itr->second);

    return nullptr;
}

void MySQLDataStore::LoadCreatureProtoTable()
{
    uint32 start_time = getMSTime();

    //                                                                 0       1          2        3         4          5        6     7       8          9           10
    QueryResult* creature_proto_result = WorldDatabase.Query("SELECT entry, minlevel, maxlevel, faction, minhealth, maxhealth, mana, scale, npcflags, attacktime, attacktype, "
    //                                                          11          12         13            14                 15                16            17        18
                                                            "mindamage, maxdamage, can_ranged, rangedattacktime, rangedmindamage, rangedmaxdamage, respawntime, armor, "
    //                                                            19           20           21            22          23           24            25             26
                                                            "resistance1, resistance2, resistance3, resistance4, resistance5, resistance6, combat_reach, bounding_radius, "
    //                                                         27    28     29         30                 31         32        33          34            35     36      37 
                                                            "auras, boss, money, invisibility_type, walk_speed, run_speed, fly_speed, extra_a9_flags, spell1, spell2, spell3, "
    //                                                          38      39      40      41      42        43           44               45            46         47           48
                                                            "spell4, spell5, spell6, spell7, spell8, spell_flags, modImmunities, isTrainingDummy, guardtype, summonguard, spelldataid, "
    //                                                          49         50
                                                            "vehicleid, rooted FROM creature_proto");

    if (creature_proto_result == nullptr)
    {
        Log.Notice("MySQLDataLoads", "Table `creature_proto` is empty!");
        return;
    }

    Log.Notice("MySQLDataLoads", "Table `creature_proto` has %u columns", creature_proto_result->GetFieldCount());

    _creatureNamesStore.rehash(creature_proto_result->GetRowCount());

    uint32 creature_proto_count = 0;
    do
    {
        Field* fields = creature_proto_result->Fetch();

        uint32 entry = fields[0].GetUInt32();

        if (GetCreatureInfo(entry) == nullptr)
        {
            Log.Error("MySQLDataLoads", "Table `creature_proto` includes entry: %u which is not in table `creature_names`! Skipped dataload for this entry.", entry);
            continue;
        }

        CreatureProto& creatureProto = _creatureProtoStore[entry];

        creatureProto.Id = entry;
        creatureProto.MinLevel = fields[1].GetUInt32();
        creatureProto.MaxLevel = fields[2].GetUInt32();
        creatureProto.Faction = fields[3].GetUInt32();
        if (fields[4].GetUInt32() != 0)
        {
            creatureProto.MinHealth = fields[4].GetUInt32();
        }
        else
        {
            Log.Error("MySQLDataLoads", "Table `creature_proto` MinHealth = 0 is not a valid value! Default set to 1 for entry: %u.", entry);
            creatureProto.MinHealth = 1;
        }

        if (fields[5].GetUInt32() != 0)
        {
            creatureProto.MaxHealth = fields[5].GetUInt32();
        }
        else
        {
            Log.Error("MySQLDataLoads", "Table `creature_proto` MaxHealth = 0 is not a valid value! Default set to 1 for entry: %u.", entry);
            creatureProto.MaxHealth = 1;
        }

        creatureProto.Mana = fields[6].GetUInt32();
        creatureProto.Scale = fields[7].GetFloat();
        creatureProto.NPCFLags = fields[8].GetUInt32();
        creatureProto.AttackTime = fields[9].GetUInt32();
        creatureProto.AttackType = fields[10].GetUInt32();
        if (fields[10].GetUInt32() <= SCHOOL_ARCANE)
        {
            creatureProto.AttackType = fields[10].GetUInt32();
        }
        else
        {
            Log.Error("MySQLDataLoads", "Table `creature_proto` AttackType: %u is not a valid value! Default set to 0 for entry: %u.", fields[10].GetUInt32(), entry);
            creatureProto.AttackType = SCHOOL_NORMAL;
        }

        creatureProto.MinDamage = fields[11].GetFloat();
        creatureProto.MaxDamage = fields[12].GetFloat();
        creatureProto.CanRanged = fields[13].GetUInt32();
        creatureProto.RangedAttackTime = fields[14].GetUInt32();
        creatureProto.RangedMinDamage = fields[15].GetFloat();
        creatureProto.RangedMaxDamage = fields[16].GetFloat();
        creatureProto.RespawnTime = fields[17].GetUInt32();
        for (uint8 i = 0; i < SCHOOL_COUNT; ++i)
        {
            creatureProto.Resistances[i] = fields[18 + i].GetUInt32();
        }

        creatureProto.CombatReach = fields[25].GetFloat();
        creatureProto.BoundingRadius = fields[26].GetFloat();
        creatureProto.aura_string = fields[27].GetString();
        creatureProto.isBoss = fields[28].GetBool();
        creatureProto.money = fields[29].GetUInt32();
        creatureProto.invisibility_type = fields[30].GetUInt32();
        creatureProto.walk_speed = fields[31].GetFloat();
        creatureProto.run_speed = fields[32].GetFloat();
        creatureProto.fly_speed = fields[33].GetFloat();
        creatureProto.extra_a9_flags = fields[34].GetUInt32();

        for (uint8 i = 0; i < creatureMaxProtoSpells; ++i)
        {
            // Process spell fields
            creatureProto.AISpells[i] = fields[35 + i].GetUInt32();
            if (creatureProto.AISpells[i] != 0)
            {
                SpellEntry* sp = dbcSpell.LookupEntryForced(creatureProto.AISpells[i]);
                if (sp == nullptr)
                {
                    uint8 spell_number = i;
                    Log.Error("MySQLDataStore", "your creature_proto table column spell%u spell: %u for creature entry: %u", spell_number + 1, creatureProto.AISpells[i], entry);
                    continue;
                }
                else
                {
                    if ((sp->Attributes & ATTRIBUTES_PASSIVE) == 0)
                        creatureProto.castable_spells.push_back(sp->Id);
                    else
                        creatureProto.start_auras.insert(sp->Id);
                }
            }
        }

        creatureProto.AISpellsFlags = fields[43].GetUInt32();
        creatureProto.modImmunities = fields[44].GetUInt32();
        creatureProto.isTrainingDummy = fields[45].GetBool();
        creatureProto.guardtype = fields[46].GetUInt32();
        creatureProto.summonguard = fields[47].GetUInt32();
        creatureProto.spelldataid = fields[48].GetUInt32();
        // process creature spells from creaturespelldata.dbc
        if (creatureProto.spelldataid != 0)
        {
            auto creature_spell_data = sCreatureSpellDataStore.LookupEntry(creatureProto.spelldataid);
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
                    creatureProto.castable_spells.push_back(sp->Id);
                else
                    creatureProto.start_auras.insert(sp->Id);
            }
        }

        creatureProto.vehicleid = fields[49].GetUInt32();
        creatureProto.rooted = fields[50].GetBool();

        //process aura string
        if (creatureProto.aura_string.size() != 0)
        {
            std::string auras = creatureProto.aura_string;
            std::vector<std::string> split_auras = StrSplit(auras, " ");
            for (std::vector<std::string>::iterator it = split_auras.begin(); it != split_auras.end(); ++it)
            {
                uint32 id = atol((*it).c_str());
                if (id)
                    creatureProto.start_auras.insert(id);
            }
        }

        //AI stuff
        creatureProto.m_canFlee = false;
        creatureProto.m_canRangedAttack = false;
        creatureProto.m_canCallForHelp = false;
        creatureProto.m_fleeHealth = 0.0f;
        creatureProto.m_fleeDuration = 0;

        //Itemslot
        creatureProto.itemslot_1 = 0;
        creatureProto.itemslot_2 = 0;
        creatureProto.itemslot_3 = 0;

        ++creature_proto_count;
    } while (creature_proto_result->NextRow());

    delete creature_proto_result;

    Log.Success("MySQLDataLoads", "Loaded %u creature proto data from `creature_proto` table in %u ms!", creature_proto_count, getMSTime() - start_time);
}

CreatureProto const* MySQLDataStore::GetCreatureProto(uint32 entry)
{
    CreatureProtoContainer::const_iterator itr = _creatureProtoStore.find(entry);
    if (itr != _creatureProtoStore.end())
        return &(itr->second);

    return nullptr;
}

void MySQLDataStore::LoadGameObjectNamesTable()
{
    uint32 start_time = getMSTime();

    //                                                                  0       1        2        3         4              5          6          7            8             9
    QueryResult* gameobject_names_result = WorldDatabase.Query("SELECT entry, type, display_id, name, category_name, cast_bar_text, UnkStr, parameter_0, parameter_1, parameter_2, "
    //                                                                10           11          12           13           14            15           16           17           18
                                                                "parameter_3, parameter_4, parameter_5, parameter_6, parameter_7, parameter_8, parameter_9, parameter_10, parameter_11, "
    //                                                                19            20            21            22           23            24            25            26
                                                                "parameter_12, parameter_13, parameter_14, parameter_15, parameter_16, parameter_17, parameter_18, parameter_19, "
    //                                                                27            28            29            30        31        32          33          34         35
                                                                "parameter_20, parameter_21, parameter_22, parameter_23, size, QuestItem1, QuestItem2, QuestItem3, QuestItem4, "
    //                                                                36          37 
                                                                "QuestItem5, QuestItem6 FROM gameobject_names");

    if (gameobject_names_result == nullptr)
    {
        Log.Notice("MySQLDataLoads", "Table `creature_proto` is empty!");
        return;
    }

    Log.Notice("MySQLDataLoads", "Table `creature_proto` has %u columns", gameobject_names_result->GetFieldCount());

    _creatureNamesStore.rehash(gameobject_names_result->GetRowCount());

    uint32 gameobject_names_count = 0;
    do
    {
        Field* fields = gameobject_names_result->Fetch();

        uint32 entry = fields[0].GetUInt32();

        GameObjectInfo& gameobjecInfo = _gameobjectNamesStore[entry];

        gameobjecInfo.entry = entry;
        gameobjecInfo.type = fields[1].GetUInt32();
        gameobjecInfo.display_id = fields[2].GetUInt32();
        gameobjecInfo.name = fields[3].GetString();
        gameobjecInfo.category_name = fields[4].GetString();
        gameobjecInfo.cast_bar_text = fields[5].GetString();
        gameobjecInfo.Unkstr = fields[6].GetString();

        gameobjecInfo.raw.parameter_0 = fields[7].GetUInt32();
        gameobjecInfo.raw.parameter_1 = fields[8].GetUInt32();
        gameobjecInfo.raw.parameter_2 = fields[9].GetUInt32();
        gameobjecInfo.raw.parameter_3 = fields[10].GetUInt32();
        gameobjecInfo.raw.parameter_4 = fields[11].GetUInt32();
        gameobjecInfo.raw.parameter_5 = fields[12].GetUInt32();
        gameobjecInfo.raw.parameter_6 = fields[13].GetUInt32();
        gameobjecInfo.raw.parameter_7 = fields[14].GetUInt32();
        gameobjecInfo.raw.parameter_8 = fields[15].GetUInt32();
        gameobjecInfo.raw.parameter_9 = fields[16].GetUInt32();
        gameobjecInfo.raw.parameter_10 = fields[17].GetUInt32();
        gameobjecInfo.raw.parameter_11 = fields[18].GetUInt32();
        gameobjecInfo.raw.parameter_12 = fields[19].GetUInt32();
        gameobjecInfo.raw.parameter_13 = fields[20].GetUInt32();
        gameobjecInfo.raw.parameter_14 = fields[21].GetUInt32();
        gameobjecInfo.raw.parameter_15 = fields[22].GetUInt32();
        gameobjecInfo.raw.parameter_16 = fields[23].GetUInt32();
        gameobjecInfo.raw.parameter_17 = fields[24].GetUInt32();
        gameobjecInfo.raw.parameter_18 = fields[25].GetUInt32();
        gameobjecInfo.raw.parameter_19 = fields[26].GetUInt32();
        gameobjecInfo.raw.parameter_20 = fields[27].GetUInt32();
        gameobjecInfo.raw.parameter_21 = fields[28].GetUInt32();
        gameobjecInfo.raw.parameter_22 = fields[29].GetUInt32();
        gameobjecInfo.raw.parameter_23 = fields[30].GetUInt32();

        gameobjecInfo.size = fields[31].GetFloat();

        for (uint8 i = 0; i < 6; ++i)
        {
            uint32 quest_item_entry = fields[32 + i].GetUInt32();
            if (quest_item_entry != 0)
            {
                auto quest_item_proto = GetItemProto(quest_item_entry);
                if (quest_item_proto == nullptr)
                {
                    Log.Error("MySQLDataLoads", "Table `gameobject_names` questitem%u : %u is not a valid item! Default set to 0 for entry: %u.", i, quest_item_entry, entry);
                    gameobjecInfo.QuestItems[i] = 0;
                }
                else
                {
                    gameobjecInfo.QuestItems[i] = quest_item_entry;
                }
            }
        }


        ++gameobject_names_count;
    } while (gameobject_names_result->NextRow());

    delete gameobject_names_result;

    Log.Success("MySQLDataLoads", "Loaded %u gameobject data from `gameobject_names` table in %u ms!", gameobject_names_count, getMSTime() - start_time);
}

GameObjectInfo const* MySQLDataStore::GetGameObjectInfo(uint32 entry)
{
    GameObjectNamesContainer::const_iterator itr = _gameobjectNamesStore.find(entry);
    if (itr != _gameobjectNamesStore.end())
        return &(itr->second);

    return nullptr;
}

//quests
void MySQLDataStore::LoadQuestsTable()
{
    uint32 start_time = getMSTime();

    //QueryResult* quest_result = WorldDatabase.Query("SELECT * FROM quest");

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
                                                    "incompleteemote, iscompletedbyspelleffect, RewXPId FROM quests");

    if (quest_result == nullptr)
    {
        Log.Notice("MySQLDataLoads", "Table `quests` is empty!");
        return;
    }

    Log.Notice("MySQLDataLoads", "Table `quests` has %u columns", quest_result->GetFieldCount());

    _questStore.rehash(quest_result->GetRowCount());

    uint32 quest_count = 0;
    do
    {
        Field* fields = quest_result->Fetch();

        uint32 entry = fields[0].GetUInt32();

        Quest& questInfo = _questStore[entry];

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

        questInfo.GetRewardItemCount();

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

    Log.Success("MySQLDataLoads", "Loaded %u quest data from `quests` table in %u ms!", quest_count, getMSTime() - start_time);
}

Quest const* MySQLDataStore::GetQuest(uint32 entry)
{
    QuestContainer::const_iterator itr = _questStore.find(entry);
    if (itr != _questStore.end())
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

            GameObjectInfo const* gameobject_info = sMySQLStore.GetGameObjectInfo(entry);
            if (gameobject_info == nullptr)
            {
                Log.Error("MySQLDataLoads", "Table `gameobject_quest_item_binding` includes data for invalid entry: %u. Skipped!", entry);
                continue;
            }

            uint32 quest_entry = fields[1].GetUInt32();
            Quest const* quest = sMySQLStore.GetQuest(quest_entry);
            if (quest == nullptr)
            {
                Log.Error("MySQLDataLoads", "Table `gameobject_quest_item_binding` includes data for invalid quest : %u. Skipped!", quest_entry);
                continue;
            }
            else
            {
                const_cast<GameObjectInfo*>(gameobject_info)->itemMap[quest].insert(std::make_pair(fields[2].GetUInt32(), fields[3].GetUInt32()));
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

            GameObjectInfo const* gameobject_info = sMySQLStore.GetGameObjectInfo(entry);
            if (gameobject_info == nullptr)
            {
                Log.Error("MySQLDataLoads", "Table `gameobject_quest_pickup_binding` includes data for invalid entry: %u. Skipped!", entry);
                continue;
            }

            uint32 quest_entry = fields[1].GetUInt32();
            Quest const* quest = sMySQLStore.GetQuest(quest_entry);
            if (quest == nullptr)
            {
                Log.Error("MySQLDataLoads", "Table `gameobject_quest_pickup_binding` includes data for invalid quest : %u. Skipped!", quest_entry);
                continue;
            }
            else
            {
                uint32 required_count = fields[2].GetUInt32();
                const_cast<GameObjectInfo*>(gameobject_info)->goMap.insert(std::make_pair(quest, required_count));
            }
                

            ++gameobject_quest_pickup_count;
        } while (gameobject_quest_pickup_result->NextRow());

        delete gameobject_quest_pickup_result;
    }

    Log.Success("MySQLDataLoads", "Loaded %u data from `gameobject_quest_pickup_binding` table in %u ms!", gameobject_quest_pickup_count, getMSTime() - start_time);
}
