/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include <regex>

#include "Storage/MySQLDataStore.hpp"

#include "Chat/ChatDefines.hpp"
#include "Logging/Log.hpp"
#include "Logging/Logger.hpp"
#include "Management/ObjectMgr.hpp"
#include "Management/QuestProperties.hpp"
#include "Movement/MovementDefines.h"
#include "Objects/GameObjectProperties.hpp"
#include "Objects/Units/Creatures/Creature.h"
#include "Server/DatabaseDefinition.hpp"
#include "Server/World.h"
#include "Spell/SpellClickInfo.hpp"
#include "Spell/SpellInfo.hpp"
#include "Spell/SpellMgr.hpp"
#include "Utilities/Strings.hpp"
#include "Storage/WDB/WDBStores.hpp"
#include "WDB/WDBStructures.hpp"
#include <cstdarg>

SERVER_DECL std::vector<MySQLAdditionalTable> MySQLAdditionalTables;

MySQLDataStore& MySQLDataStore::getInstance()
{
    static MySQLDataStore mInstance;
    return mInstance;
}

void MySQLDataStore::finalize()
{
    _professionDiscoveryStore.clear();
}

static std::vector<std::string> ascemuTables = { "achievement_reward", "ai_threattospellid", "areatriggers", "auctionhouse", "battlemasters", "creature_ai_scripts", "creature_difficulty", "creature_formations", "creature_group_spawn", "creature_initial_equip", "creature_movement_override", "creature_properties", "creature_properties_movement", "creature_quest_finisher", "creature_quest_starter", "creature_script_waypoints", "creature_spawns", "creature_timed_emotes", "creature_waypoints", "display_bounding_boxes", "event_scripts", "fishing", "gameevent_properties", "gameobject_properties", "gameobject_quest_finisher", "gameobject_quest_item_binding", "gameobject_quest_pickup_binding", "gameobject_quest_starter", "gameobject_spawns", "gameobject_spawns_extra", "gameobject_spawns_overrides", "gameobject_teleports", "gossip_menu", "gossip_menu_items", "gossip_menu_option", "graveyards", "guild_rewards", "guild_xp_for_level", "instance_encounters", "item_pages", "item_properties", "item_quest_association", "item_randomprop_groups", "item_randomsuffix_groups", "itemset_linked_itemsetbonus", "lfg_dungeon_rewards", "locales_achievement_reward", "locales_creature", "locales_gameobject", "locales_gossip_menu_option", "locales_item", "locales_item_pages", "locales_npc_gossip_texts", "locales_npc_script_text", "locales_points_of_interest", "locales_quest", "locales_worldbroadcast", "locales_worldmap_info", "locales_worldstring_table", "loot_creatures", "loot_fishing", "loot_gameobjects", "loot_items", "loot_pickpocketing", "loot_skinning", "npc_gossip_properties", "npc_gossip_texts", "npc_script_text", "npc_spellclick_spells", "pet_level_abilities", "petdefaultspells", "player_classlevelstats", "player_levelstats", "player_xp_for_level", "playercreateinfo", "playercreateinfo_bars", "playercreateinfo_items", "playercreateinfo_skills", "playercreateinfo_spell_cast", "playercreateinfo_spell_learn", "points_of_interest", "professiondiscoveries", "quest_poi", "quest_poi_points", "quest_properties", "recall", "reputation_creature_onkill", "reputation_faction_onkill", "reputation_instance_onkill", "spawn_group_id", "spell_area", "spell_coefficient_override", "spell_custom_override", "spell_disable", "spell_disable_trainers", "spell_effects_override", "spell_ranks", "spell_required", "spell_teleport_coords", "spelloverride", "spelltargetconstraints", "totemdisplayids", "trainer_properties", "trainer_properties_spellset", "transport_data", "vehicle_accessories", "vehicle_seat_addon", "vendor_restrictions", "vendors", "weather", "wordfilter_character_names", "wordfilter_chat", "world_db_version", "worldbroadcast", "worldmap_info", "worldstate_templates", "worldstring_tables", "zoneguards" };

void MySQLDataStore::loadAdditionalTableConfig()
{
    // add all defined ascemu tables to addition table loading
    for (const auto& ascTables : ascemuTables)
    {
        MySQLAdditionalTable myTable;
        myTable.mainTable = ascTables;
        myTable.tableVector.push_back(ascTables);

        // get config
        const std::string strData = worldConfig.startup.additionalTableLoads;
        if (!strData.empty())
        {
            const std::vector<std::string> strs = AscEmu::Util::Strings::split(strData, ",");
            if (!strs.empty())
            {
                for (auto& str : strs)
                {
                    std::stringstream additionTableStream(str);
                    std::string additional_table;
                    std::string target_table;

                    additionTableStream >> additional_table;
                    additionTableStream >> target_table;

                    if (!additional_table.empty() || !target_table.empty())
                    {
                        if (myTable.mainTable == target_table)
                        {
                            if (auto result = WorldDatabase.Query("SELECT COUNT(*) FROM information_schema.tables WHERE table_schema = \"%s\" AND table_name = \"%s\"", WorldDatabase.GetDatabaseName().c_str(), additional_table.c_str()))
                            {
                                Field* fields = result->Fetch();

                                uint32_t count = fields[0].asUint32();
                                if (fields[0].asUint32())
                                    myTable.tableVector.push_back(additional_table);
                                else
                                    sLogger.info("MySQLDataLoads : Additional table `{}` defined in world.conf does not exist!", additional_table);
                            }
                        }
                    }
                }
            }
        }

        MySQLAdditionalTables.push_back(myTable);
    }

    for (auto additionalTable : MySQLAdditionalTables)
    {
        sLogger.debugFlag(AscEmu::Logging::DebugFlags::LF_DB_TABLES, "MySQLDataLoads : Table {} has additional tables:", additionalTable.mainTable);
        for (auto additionalTableList : additionalTable.tableVector)
            sLogger.debugFlag(AscEmu::Logging::DebugFlags::LF_DB_TABLES, "MySQLDataLoads : - {} ", additionalTableList);
    }
}

std::unique_ptr<QueryResult> MySQLDataStore::getWorldDBQuery(const char* query, ...)
{
    // fill in values
    va_list vlist;
    va_start(vlist, query);

    // Get buffer size
    va_list vlist_copy;
    va_copy(vlist_copy, vlist);
    const auto size = vsnprintf(nullptr, 0, query, vlist_copy);
    va_end(vlist_copy);

    if (size < 0)
    {
        va_end(vlist);
        return nullptr;
    }

    // save query as prepared
    std::string preparedQuery(size, '\0');
    vsnprintf(&preparedQuery[0], static_cast<size_t>(size) + 1, query, vlist);
    va_end(vlist);

    // checkout additional tables
    for (const auto& additionalTable : MySQLAdditionalTables)
    {
        // query includes table which has additional tables
        if (AscEmu::Util::Strings::contains(additionalTable.mainTable, preparedQuery))
        {
            // add origigin table name to query, replace first occurence of "FROM"
            std::string originName = ", '" + additionalTable.mainTable + "' as origin FROM";
            std::string bar = " FROM";
            size_t pos = preparedQuery.find(bar);
            size_t len = bar.length();
            preparedQuery.replace(pos, len, originName);
            //preparedQuery = std::regex_replace(preparedQuery, std::regex("FROM"), originName);

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

                        sLogger.debugFlag(AscEmu::Logging::DebugFlags::LF_DB_TABLES, "MySQLDataLoads : Added additional query '{}'", changeQuery);
                    }
                }
            }

            sLogger.debugFlag(AscEmu::Logging::DebugFlags::LF_DB_TABLES, "MySQLDataLoads : AdditionalTableLoading - Query: '{}'", completeQuery);
            return WorldDatabase.Query(completeQuery.c_str());
        }
    }

    sLogger.debugFlag(AscEmu::Logging::DebugFlags::LF_DB_TABLES, "MySQLDataLoads : Query: '{}'", preparedQuery);
    // no additional tables defined, just send our query
    return WorldDatabase.Query(preparedQuery.c_str());
}

void MySQLDataStore::loadItemPagesTable()
{
    auto startTime = Util::TimeNow();

    auto itempages_result = WorldDatabase.Query("SELECT entry, text, next_page FROM item_pages");
    if (itempages_result == nullptr)
    {
        sLogger.info("MySQLDataLoads : Table `item_pages` is empty!");
        return;
    }

    sLogger.info("MySQLDataLoads : Table `item_pages` has {} columns", itempages_result->GetFieldCount());

    _itemPagesStore.rehash(itempages_result->GetRowCount());

    uint32_t itempages_count = 0;
    do
    {
        Field* fields = itempages_result->Fetch();

        addItemPage(fields[0].asUint32(), fields[1].asCString(), fields[2].asUint32());

        ++itempages_count;
    } while (itempages_result->NextRow());

    sLogger.info("MySQLDataLoads : Loaded {} pages from `item_pages` table in {} ms!", itempages_count, static_cast<uint32_t>(Util::GetTimeDifferenceToNow(startTime)));
}

void MySQLDataStore::addItemPage(uint32_t _entry, std::string _text, uint32_t _nextPage /*= 0*/)
{
    MySQLStructure::ItemPage& itemPage = _itemPagesStore[_entry];

    itemPage.id = _entry;
    itemPage.text = _text;
    itemPage.nextPage = _nextPage;
}

MySQLStructure::ItemPage const* MySQLDataStore::getItemPage(uint32_t entry)
{
    ItemPageContainer::const_iterator itr = _itemPagesStore.find(entry);
    if (itr != _itemPagesStore.end())
        return &(itr->second);

    return nullptr;
}

uint32_t MySQLDataStore::getItemPageEntryByText(std::string _text)
{
    for (auto itemPage : _itemPagesStore)
        if (itemPage.second.text == _text)
            return itemPage.first;
    return 0;
}

void MySQLDataStore::loadItemPropertiesTable()
{
    auto startTime = Util::TimeNow();

    uint32_t item_count = 0;

    auto item_result = getWorldDBQuery("SELECT * FROM item_properties base "
        "WHERE build=(SELECT MAX(build) FROM item_properties spec WHERE base.entry = spec.entry AND build <= %u)", VERSION_STRING);


    if (item_result == nullptr)
    {
        sLogger.info("MySQLDataLoads : Table `item_properties` is empty!");
        return;
    }

    sLogger.info("MySQLDataLoads : Table `item_properties` has {} columns", item_result->GetFieldCount());

    _itemPropertiesStore.rehash(item_result->GetRowCount());

    do
    {
        Field* fields = item_result->Fetch();

        uint32_t entry = fields[0].asUint32();

        ItemProperties& itemProperties = _itemPropertiesStore[entry];

        itemProperties.ItemId = entry;
        itemProperties.Class = fields[2].asUint32();
        itemProperties.SubClass = fields[3].asUint16();
        itemProperties.unknown_bc = fields[4].asUint32(true);
        itemProperties.Name = fields[5].asCString();
        itemProperties.DisplayInfoID = fields[6].asUint32();
        itemProperties.Quality = fields[7].asUint32();
        itemProperties.Flags = fields[8].asUint32();
        itemProperties.Flags2 = fields[9].asUint32();
        itemProperties.BuyPrice = fields[10].asUint32();
        itemProperties.SellPrice = fields[11].asUint32();

        itemProperties.InventoryType = fields[12].asUint32();
        itemProperties.AllowableClass = fields[13].asUint32(true);
        itemProperties.AllowableRace = fields[14].asUint32(true);
        itemProperties.ItemLevel = fields[15].asUint32();
        itemProperties.RequiredLevel = fields[16].asUint32();
        itemProperties.RequiredSkill = fields[17].asUint16();
        itemProperties.RequiredSkillRank = fields[18].asUint32();
        itemProperties.RequiredSpell = fields[19].asUint32();
        itemProperties.RequiredPlayerRank1 = fields[20].asUint32();
        itemProperties.RequiredPlayerRank2 = fields[21].asUint32();
        itemProperties.RequiredFaction = fields[22].asUint32();
        itemProperties.RequiredFactionStanding = fields[23].asUint32();
        itemProperties.Unique = fields[24].asUint32();
        itemProperties.MaxCount = fields[25].asUint32();
        itemProperties.ContainerSlots = fields[26].asUint32();

        itemProperties.ScalingStatsEntry = fields[27].asUint32();
        itemProperties.ScalingStatsFlag = fields[28].asUint32();

        for (uint8_t i = 0; i < MAX_ITEM_PROTO_DAMAGES; ++i)
        {
            itemProperties.Damage[i].Min = fields[29 + i * 3].asFloat();
            itemProperties.Damage[i].Max = fields[30 + i * 3].asFloat();
            itemProperties.Damage[i].Type = fields[31 + i * 3].asUint32();
        }

        itemProperties.Armor = fields[35].asUint32();
        itemProperties.Delay = fields[36].asUint32();
        itemProperties.AmmoType = fields[37].asUint32();
        itemProperties.Range = fields[38].asFloat();

        itemProperties.Bonding = fields[39].asUint32();
        itemProperties.Description = fields[40].asCString();
        uint32_t page_id = fields[41].asUint32();
        if (page_id != 0)
        {
            MySQLStructure::ItemPage const* item_page = getItemPage(page_id);
            if (item_page == nullptr)
            {
                sLogger.failure("Table `item_properties` entry: {} includes invalid pageId {}! pageId is set to 0.", entry, page_id);
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

        itemProperties.PageLanguage = fields[42].asUint32();
        itemProperties.PageMaterial = fields[43].asUint32();
        itemProperties.QuestId = fields[44].asUint32();
        itemProperties.LockId = fields[45].asUint32();
        itemProperties.LockMaterial = fields[46].asUint32(true);
        itemProperties.SheathID = fields[47].asUint32();
        itemProperties.RandomPropId = fields[48].asUint32();
        itemProperties.RandomSuffixId = fields[49].asUint32();
        itemProperties.Block = fields[50].asUint32();
        itemProperties.ItemSet = fields[51].asInt32();
        itemProperties.MaxDurability = fields[52].asUint32();
        itemProperties.ZoneNameID = fields[53].asUint32();
        itemProperties.MapID = fields[54].asUint32();
        itemProperties.BagFamily = fields[55].asUint32();
        itemProperties.TotemCategory = fields[56].asUint32();

        for (uint8_t i = 0; i < MAX_ITEM_PROTO_SOCKETS; ++i)
        {
            itemProperties.Sockets[i].SocketColor = uint32_t(fields[57 + i * 2].asUint8());
            itemProperties.Sockets[i].Unk = fields[58 + i * 2].asUint32();
        }

        itemProperties.SocketBonus = fields[63].asUint32();
        itemProperties.GemProperties = fields[64].asUint32();
        itemProperties.DisenchantReqSkill = fields[65].asInt32();
        itemProperties.ArmorDamageModifier = fields[66].asFloat();
        itemProperties.ExistingDuration = fields[67].asUint32();
        itemProperties.ItemLimitCategory = fields[68].asUint32();
        itemProperties.HolidayId = fields[69].asUint32();
        itemProperties.FoodType = fields[70].asUint32();

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

    sLogger.info("MySQLDataLoads : Loaded {} item_properties in {} ms!", item_count, static_cast<uint32_t>(Util::GetTimeDifferenceToNow(startTime)));
}

void MySQLDataStore::loadItemPropertiesSpellsTable()
{
    auto startTime = Util::TimeNow();

    uint32_t spell_count = 0;

    auto item_result = getWorldDBQuery("SELECT * FROM item_properties_spells base "
        "WHERE build=(SELECT MAX(build) FROM item_properties_spells spec WHERE base.entry = spec.entry AND build <= %u)", VERSION_STRING);

    if (item_result == nullptr)
    {
        sLogger.info("MySQLDataLoads : Table `item_properties_spells` is empty!");
        return;
    }

    sLogger.info("MySQLDataLoads : Table `item_properties_spells` has {} columns", item_result->GetFieldCount());

    struct LoadItemSpell
    {
        uint32_t trigger;
        int32_t charges;
        int32_t cooldown;
        uint32_t category;
        int32_t categoryCooldown;
    };

    std::map<uint32_t, std::unordered_map<uint32_t, LoadItemSpell>> tempSpellStore;
    do
    {
        Field* fields = item_result->Fetch();

        uint32_t entry = fields[0].asUint32();
        uint32_t spellid = fields[2].asUint32();

        LoadItemSpell spellData;
        spellData.trigger = fields[3].asUint32();
        spellData.charges = fields[4].asInt32();
        spellData.cooldown = fields[5].asInt32();
        spellData.category = fields[6].asUint32();
        spellData.categoryCooldown = fields[7].asInt32();

        tempSpellStore[entry][spellid] = spellData;

        ++spell_count;
    } while (item_result->NextRow());

    sLogger.info("MySQLDataLoads : Loaded {} item_properties_spells in {} ms!", spell_count, static_cast<uint32_t>(Util::GetTimeDifferenceToNow(startTime)));


    startTime = Util::TimeNow();
    uint32_t assignedCount = 0;

    for (const auto& [entry, bySpellId] : tempSpellStore)
    {
        ItemProperties& ip = _itemPropertiesStore[entry];

        size_t i = 0;
        for (const auto& [spellId, data] : bySpellId)
        {
            if (i >= MAX_ITEM_PROTO_SPELLS)
                break;

            ip.Spells[i].Id = spellId;
            ip.Spells[i].Trigger = data.trigger;
            ip.Spells[i].Charges = data.charges;
            ip.Spells[i].Cooldown = data.cooldown;
            ip.Spells[i].Category = data.category;
            ip.Spells[i].CategoryCooldown = data.categoryCooldown;

            ++i;
            ++assignedCount;
        }
    }

    sLogger.info("MySQLDataLoads : assigned {} spells to item_properties in {} ms!", assignedCount, static_cast<uint32_t>(Util::GetTimeDifferenceToNow(startTime)));
}

void MySQLDataStore::loadItemPropertiesStatsTable()
{
    auto startTime = Util::TimeNow();

    uint32_t stat_count = 0;

    auto item_result = getWorldDBQuery("SELECT * FROM item_properties_stats base "
        "WHERE build=(SELECT MAX(build) FROM item_properties_stats spec WHERE base.entry = spec.entry AND build <= %u)", VERSION_STRING);


    if (item_result == nullptr)
    {
        sLogger.info("MySQLDataLoads : Table `item_properties_stats` is empty!");
        return;
    }

    sLogger.info("MySQLDataLoads : Table `item_properties_stats` has {} columns", item_result->GetFieldCount());

    std::map<uint32_t, std::map<uint32_t, int32_t>> tempStatsStore;
    do
    {
        Field* fields = item_result->Fetch();

        uint32_t entry = fields[0].asUint32();
        uint32_t type = fields[2].asUint32();
        int32_t value = fields[3].asInt32();

        tempStatsStore[entry][type] = value;

        ++stat_count;
    } while (item_result->NextRow());

    sLogger.info("MySQLDataLoads : Loaded {} item_properties_stats in {} ms!", stat_count, static_cast<uint32_t>(Util::GetTimeDifferenceToNow(startTime)));


    startTime = Util::TimeNow();
    uint32_t assignedCount = 0;

    for (const auto& statEntry : tempStatsStore)
    {
        uint32_t entry = statEntry.first;
        const auto& typeMap = statEntry.second;

        ItemProperties& itemProperties = _itemPropertiesStore[entry];

        for (const auto& kv : typeMap)
        {
            uint32_t type = kv.first;
            int32_t value = kv.second;

            itemProperties.addStat(type, value);

            ++assignedCount;
        }
    }

    sLogger.info("MySQLDataLoads : assigned {} stats to item_properties in {} ms!", assignedCount, static_cast<uint32_t>(Util::GetTimeDifferenceToNow(startTime)));
}

ItemProperties const* MySQLDataStore::getItemProperties(uint32_t entry)
{
    ItemPropertiesContainer::const_iterator itr = _itemPropertiesStore.find(entry);
    if (itr != _itemPropertiesStore.end())
        return &(itr->second);

    return nullptr;
}

MySQLDataStore::ItemPropertiesContainer const* MySQLDataStore::getItemPropertiesStore() { return &_itemPropertiesStore; }

std::string MySQLDataStore::getItemLinkByProto(ItemProperties const* iProto, uint32_t language/* = 0*/)
{
    char buffer[256];
    std::string colour;

    switch (iProto->Quality)
    {
    case ITEM_QUALITY_NORMAL: // white
        colour = "cffffffff";
        break;
    case ITEM_QUALITY_UNCOMMON: // green
        colour = "cff1eff00";
        break;
    case ITEM_QUALITY_RARE: // blue
        colour = "cff0070dd";
        break;
    case ITEM_QUALITY_EPIC: // purple
        colour = "cffa335ee";
        break;
    case ITEM_QUALITY_LEGENDARY: // orange
        colour = "cffff8000";
        break;
    case ITEM_QUALITY_ARTIFACT:
    case ITEM_QUALITY_HEIRLOOM: // gold
        colour = "c00fce080";
        break;
    case ITEM_QUALITY_POOR: // gray
    default:
        colour = "cff9d9d9d";
    }

    // try to get localized version
    char* lit = (language > 0) ? sMySQLStore.getLocalizedItemName(iProto->ItemId, language) : nullptr;
    if (lit)
        snprintf(buffer, 256, "|%s|Hitem:%u:0:0:0:0:0:0:0|h[%s]|h|r", colour.c_str(), iProto->ItemId, lit);
    else
        snprintf(buffer, 256, "|%s|Hitem:%u:0:0:0:0:0:0:0|h[%s]|h|r", colour.c_str(), iProto->ItemId, iProto->Name.c_str());

    const char* ItemLink = buffer;

    return ItemLink;
}

void MySQLDataStore::loadCreaturePropertiesTable()
{
    auto startTime = Util::TimeNow();
    uint32_t creature_properties_count = 0;

    //                                                         0          1           2             3                 4               5                  6
    auto creature_properties_result = getWorldDBQuery("SELECT entry, killcredit1, killcredit2, male_displayid, female_displayid, male_displayid2, female_displayid2, "
        //7      8         9         10       11     12     13       14            15              16           17
        "name, subname, icon_name, type_flags, type, family, `rank`, encounter, base_attack_mod, range_attack_mod, leader, "
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

    sLogger.info("MySQLDataLoads : Table creature_properties has {} columns", creature_properties_result->GetFieldCount());

    _creaturePropertiesStore.rehash(creature_properties_result->GetRowCount());

    do
    {
        Field* fields = creature_properties_result->Fetch();

        uint32_t entry = fields[0].asUint32();

        CreatureProperties& creatureProperties = _creaturePropertiesStore[entry];

        creatureProperties.Id = entry;
        creatureProperties.killcredit[0] = fields[1].asUint32();
        creatureProperties.killcredit[1] = fields[2].asUint32();
        creatureProperties.Male_DisplayID = fields[3].asUint32();
        if (creatureProperties.Male_DisplayID != 0)
        {
            const auto* creature_display = sObjectMgr.getCreatureDisplayInfoData(creatureProperties.Male_DisplayID);
            if (creature_display == nullptr)
            {
                sLogger.debugFlag(AscEmu::Logging::LF_DB_TABLES, "Table creature_properties includes invalid Male_DisplayID {} for npc entry: {}. Set to 0!", creatureProperties.Male_DisplayID, entry);
                creatureProperties.Male_DisplayID = 0;
            }
        }
        creatureProperties.Female_DisplayID = fields[4].asUint32();
        if (creatureProperties.Female_DisplayID != 0)
        {
            const auto* creature_display = sObjectMgr.getCreatureDisplayInfoData(creatureProperties.Female_DisplayID);
            if (creature_display == nullptr)
            {
                sLogger.debugFlag(AscEmu::Logging::LF_DB_TABLES, "Table creature_properties includes invalid Female_DisplayID {} for npc entry: {}. Set to 0!", creatureProperties.Female_DisplayID, entry);
                creatureProperties.Female_DisplayID = 0;
            }
        }
        creatureProperties.Male_DisplayID2 = fields[5].asUint32();
        if (creatureProperties.Male_DisplayID2 != 0)
        {
            const auto* creature_display = sObjectMgr.getCreatureDisplayInfoData(creatureProperties.Male_DisplayID2);
            if (creature_display == nullptr)
            {
                sLogger.debugFlag(AscEmu::Logging::LF_DB_TABLES, "Table creature_properties includes invalid Male_DisplayID2 {} for npc entry: {}. Set to 0!", creatureProperties.Male_DisplayID2, entry);
                creatureProperties.Male_DisplayID2 = 0;
            }
        }
        creatureProperties.Female_DisplayID2 = fields[6].asUint32();
        if (creatureProperties.Female_DisplayID2 != 0)
        {
            const auto* creature_display = sObjectMgr.getCreatureDisplayInfoData(creatureProperties.Female_DisplayID2);
            if (creature_display == nullptr)
            {
                sLogger.debugFlag(AscEmu::Logging::LF_DB_TABLES, "Table creature_properties includes invalid Female_DisplayID2 {} for npc entry: {}. Set to 0!", creatureProperties.Female_DisplayID2, entry);
                creatureProperties.Female_DisplayID2 = 0;
            }
        }

        creatureProperties.Name = fields[7].asCString();

        //lowercase
        std::string lower_case_name = creatureProperties.Name;
        AscEmu::Util::Strings::toLowerCase(lower_case_name);
        creatureProperties.lowercase_name = lower_case_name;

        creatureProperties.SubName = fields[8].asCString();
        creatureProperties.icon_name = fields[9].asCString();
        creatureProperties.typeFlags = fields[10].asUint32();
        creatureProperties.Type = fields[11].asUint32();
        creatureProperties.Family = fields[12].asUint32();
        creatureProperties.Rank = fields[13].asUint32();
        creatureProperties.Encounter = fields[14].asUint32();
        creatureProperties.baseAttackMod = fields[15].asFloat();
        creatureProperties.rangeAttackMod = fields[16].asFloat();
        creatureProperties.Leader = fields[17].asUint8();
        creatureProperties.MinLevel = fields[18].asUint32();
        creatureProperties.MaxLevel = fields[19].asUint32();
        creatureProperties.Faction = fields[20].asUint32();
        if (fields[21].asUint32() != 0)
        {
            creatureProperties.MinHealth = fields[21].asUint32();
        }
        else
        {
            sLogger.debugFlag(AscEmu::Logging::LF_DB_TABLES, "Table `creature_properties` MinHealth = 0 is not a valid value! Default set to 1 for entry: {}.", entry);
            creatureProperties.MinHealth = 1;
        }

        if (fields[22].asUint32() != 0)
        {
            creatureProperties.MaxHealth = fields[22].asUint32();
        }
        else
        {
            sLogger.debugFlag(AscEmu::Logging::LF_DB_TABLES, "Table `creature_properties` MaxHealth = 0 is not a valid value! Default set to 1 for entry: {}.", entry);
            creatureProperties.MaxHealth = 1;
        }

        creatureProperties.Mana = fields[23].asUint32();
        creatureProperties.Scale = fields[24].asFloat();
        creatureProperties.NPCFLags = fields[25].asUint32();

        if (fields[26].asUint32() != 0)
        {
            creatureProperties.AttackTime = fields[26].asUint32();
        }
        else
        {
            sLogger.debugFlag(AscEmu::Logging::LF_DB_TABLES, "Table `creature_properties` attacktime = 0 is not a valid value! Default set to 2000 for entry: {}.", entry);
            creatureProperties.AttackTime = 2000;
        }

        if (fields[27].asUint8() <= SCHOOL_ARCANE)
        {
            creatureProperties.attackSchool = fields[27].asUint8();
        }
        else
        {
            sLogger.debugFlag(AscEmu::Logging::LF_DB_TABLES, "Table `creature_properties` AttackType: {} is not a valid value! Default set to 0 for entry: {}.", fields[10].asUint32(), entry);
            creatureProperties.attackSchool = SCHOOL_NORMAL;
        }

        if (fields[28].asUint32() != 0)
        {
            creatureProperties.MinDamage = fields[28].asFloat();
        }
        else
        {
            sLogger.debugFlag(AscEmu::Logging::LF_DB_TABLES, "Table `creature_properties` mindamage = 0 is not a valid value! Default set to 5 for entry: {}.", entry);
            creatureProperties.MinDamage = 5;
        }

        if (fields[29].asUint32() != 0 || fields[29].asFloat() > creatureProperties.MinDamage)
        {
            creatureProperties.MaxDamage = fields[29].asFloat();
        }
        else
        {
            sLogger.debugFlag(AscEmu::Logging::LF_DB_TABLES, "Table `creature_properties` maxdamage = 0 or is lower than mindamage! Default set to mindamage + 5 for entry: {}.", entry);
            creatureProperties.MaxDamage = creatureProperties.MinDamage + 5;
        }

        creatureProperties.CanRanged = fields[30].asUint32();
        creatureProperties.RangedAttackTime = fields[31].asUint32();
        creatureProperties.RangedMinDamage = fields[32].asFloat();
        creatureProperties.RangedMaxDamage = fields[33].asFloat();
        creatureProperties.RespawnTime = fields[34].asUint32();
        for (uint8_t i = 0; i < TOTAL_SPELL_SCHOOLS; ++i)
        {
            creatureProperties.Resistances[i] = fields[35 + i].asUint32();
        }

        creatureProperties.CombatReach = fields[42].asFloat();
        creatureProperties.BoundingRadius = fields[43].asFloat();
        creatureProperties.aura_string = fields[44].asCString();
        creatureProperties.isBoss = fields[45].asBool();
        creatureProperties.money = fields[46].asUint32();
        creatureProperties.isTriggerNpc = fields[47].asBool();
        creatureProperties.walk_speed = fields[48].asFloat();
        creatureProperties.run_speed = fields[49].asFloat();
        creatureProperties.fly_speed = fields[50].asFloat();
        creatureProperties.extra_a9_flags = fields[51].asUint32();

        for (uint8_t i = 0; i < creatureMaxProtoSpells; ++i)
        {
            // Process spell fields
            creatureProperties.AISpells[i] = fields[52 + i].asUint32();
            if (creatureProperties.AISpells[i] != 0)
            {
                SpellInfo const* sp = sSpellMgr.getSpellInfo(creatureProperties.AISpells[i]);
                if (sp == nullptr)
                {
                    uint8_t spell_number = i;
                    sLogger.debugFlag(AscEmu::Logging::LF_DB_TABLES, "spell {} in table creature_properties column spell{} for creature entry: {} is not a valid spell!", creatureProperties.AISpells[i], spell_number + 1, entry);
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

        creatureProperties.AISpellsFlags = fields[60].asUint32();
        creatureProperties.modImmunities = fields[61].asUint32();
        creatureProperties.isTrainingDummy = fields[62].asBool();
        creatureProperties.guardtype = fields[63].asUint32();
        creatureProperties.summonguard = fields[64].asUint32();
        creatureProperties.spelldataid = fields[65].asUint32();
        // process creature spells from creaturespelldata.dbc
        if (creatureProperties.spelldataid != 0)
        {
            auto creature_spell_data = sCreatureSpellDataStore.lookupEntry(creatureProperties.spelldataid);
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

        creatureProperties.vehicleid = fields[66].asUint32();
        creatureProperties.rooted = fields[67].asBool();

        for (uint8_t i = 0; i < 6; ++i)
            creatureProperties.QuestItems[i] = fields[68 + i].asUint32();

        creatureProperties.waypointid = fields[74].asUint32();

        creatureProperties.gossipId = fields[75].asUint32();
        std::string origin = fields[76].asCString();

        if (origin == "creature_properties_copy")
            sLogger.info("MySQLDataLoads : Loaded {} creature proto from table {}", creatureProperties.Id, origin);

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
                uint32_t id = std::stoul((*it).c_str());
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

    sLogger.info("MySQLDataLoads : Loaded {} creature proto data in {} ms!", creature_properties_count, static_cast<uint32_t>(Util::GetTimeDifferenceToNow(startTime)));
}

void MySQLDataStore::loadCreaturePropertiesMovementTable()
{
    auto startTime = Util::TimeNow();
    uint32_t creature_properties_movement_count = 0;

    //                                                                      0          1           2             3                 4               5                  6
    auto creature_properties_movement_result = WorldDatabase.Query("SELECT CreatureId, Ground, Swim, Flight, Rooted, Chase, Random, InteractionPauseTimer FROM creature_properties_movement");

    if (creature_properties_movement_result == nullptr)
    {
        sLogger.info("MySQLDataLoads : Table creature_properties_movement is empty!");
        return;
    }

    uint32_t row_count = 0;   
    row_count = static_cast<uint32_t>(_creaturePropertiesMovementStore.size());

    sLogger.info("MySQLDataLoads : Table creature_properties_movement has {} columns", creature_properties_movement_result->GetFieldCount());

    _creaturePropertiesMovementStore.rehash(row_count + creature_properties_movement_result->GetRowCount());
    do
    {
        Field* fields = creature_properties_movement_result->Fetch();

        uint32_t entry = fields[0].asUint32();

        CreaturePropertiesMovement& creaturePropertiesMovement = _creaturePropertiesMovementStore[entry];

        creaturePropertiesMovement.Id = entry;
        creaturePropertiesMovement.MovementType = IDLE_MOTION_TYPE;
        creaturePropertiesMovement.Movement.Ground = static_cast<CreatureGroundMovementType>(fields[1].asUint8());
        creaturePropertiesMovement.Movement.Swim = fields[2].asBool();
        creaturePropertiesMovement.Movement.Flight = static_cast<CreatureFlightMovementType>(fields[3].asUint8());
        creaturePropertiesMovement.Movement.Rooted = fields[4].asBool();
        creaturePropertiesMovement.Movement.Chase = static_cast<CreatureChaseMovementType>(fields[5].asUint8());
        creaturePropertiesMovement.Movement.Random = static_cast<CreatureRandomMovementType>(fields[6].asUint8());

        ++creature_properties_movement_count;
        } while (creature_properties_movement_result->NextRow());

    sLogger.info("MySQLDataLoads : Loaded {} creature movement data in {} ms!", creature_properties_movement_count, static_cast<uint32_t>(Util::GetTimeDifferenceToNow(startTime)));
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

    //                                                           0      1        2        3         4              5          6          7            8             9
    auto gameobject_properties_result = getWorldDBQuery("SELECT entry, type, display_id, name, category_name, cast_bar_text, UnkStr, parameter_0, parameter_1, parameter_2, "
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

    sLogger.info("MySQLDataLoads : Table `gameobject_properties` has {} columns", gameobject_properties_result->GetFieldCount());

    _gameobjectPropertiesStore.rehash(gameobject_properties_result->GetRowCount());

    do
    {
        Field* fields = gameobject_properties_result->Fetch();

        uint32_t entry = fields[0].asUint32();

        GameObjectProperties& gameobjecProperties = _gameobjectPropertiesStore[entry];

        gameobjecProperties.entry = entry;
        gameobjecProperties.type = fields[1].asUint32();
        gameobjecProperties.display_id = fields[2].asUint32();
        gameobjecProperties.name = fields[3].asCString();
        gameobjecProperties.category_name = fields[4].asCString();
        gameobjecProperties.cast_bar_text = fields[5].asCString();
        gameobjecProperties.Unkstr = fields[6].asCString();

        gameobjecProperties.raw.parameter_0 = fields[7].asUint32();
        gameobjecProperties.raw.parameter_1 = fields[8].asUint32();
        gameobjecProperties.raw.parameter_2 = fields[9].asUint32();
        gameobjecProperties.raw.parameter_3 = fields[10].asUint32();
        gameobjecProperties.raw.parameter_4 = fields[11].asUint32();
        gameobjecProperties.raw.parameter_5 = fields[12].asUint32();
        gameobjecProperties.raw.parameter_6 = fields[13].asUint32();
        gameobjecProperties.raw.parameter_7 = fields[14].asUint32();
        gameobjecProperties.raw.parameter_8 = fields[15].asUint32();
        gameobjecProperties.raw.parameter_9 = fields[16].asUint32();
        gameobjecProperties.raw.parameter_10 = fields[17].asUint32();
        gameobjecProperties.raw.parameter_11 = fields[18].asUint32();
        gameobjecProperties.raw.parameter_12 = fields[19].asUint32();
        gameobjecProperties.raw.parameter_13 = fields[20].asUint32();
        gameobjecProperties.raw.parameter_14 = fields[21].asUint32();
        gameobjecProperties.raw.parameter_15 = fields[22].asUint32();
        gameobjecProperties.raw.parameter_16 = fields[23].asUint32();
        gameobjecProperties.raw.parameter_17 = fields[24].asUint32();
        gameobjecProperties.raw.parameter_18 = fields[25].asUint32();
        gameobjecProperties.raw.parameter_19 = fields[26].asUint32();
        gameobjecProperties.raw.parameter_20 = fields[27].asUint32();
        gameobjecProperties.raw.parameter_21 = fields[28].asUint32();
        gameobjecProperties.raw.parameter_22 = fields[29].asUint32();
        gameobjecProperties.raw.parameter_23 = fields[30].asUint32();

        gameobjecProperties.size = fields[31].asFloat();

        for (uint8_t i = 0; i < 6; ++i)
        {
            uint32_t quest_item_entry = fields[32 + i].asUint32();
            if (quest_item_entry != 0)
            {
                auto quest_item_proto = getItemProperties(quest_item_entry);
                if (quest_item_proto == nullptr)
                {
                    sLogger.failure("Table `gameobject_properties` questitem{} : {} is not a valid item! Default set to 0 for entry: {}.", i, quest_item_entry, entry);
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

    sLogger.info("MySQLDataLoads : Loaded {} gameobject data in {} ms!", gameobject_properties_count, static_cast<uint32_t>(Util::GetTimeDifferenceToNow(startTime)));
}

GameObjectProperties const* MySQLDataStore::getGameObjectProperties(uint32_t entry)
{
    GameObjectPropertiesContainer::const_iterator itr = _gameobjectPropertiesStore.find(entry);
    if (itr != _gameobjectPropertiesStore.end())
        return &(itr->second);

    return nullptr;
}

MySQLDataStore::GameObjectPropertiesContainer const* MySQLDataStore::getGameObjectPropertiesStore() { return &_gameobjectPropertiesStore; }

void MySQLDataStore::loadGameObjectSpawnsExtraTable()
{
    auto startTime = Util::TimeNow();

    auto result = getWorldDBQuery("SELECT id, parent_rotation0, parent_rotation1, parent_rotation2, parent_rotation3 FROM gameobject_spawns_extra WHERE min_build <= %u AND max_build >= %u", VERSION_STRING, VERSION_STRING);
    if (!result)
    {
        sLogger.info("Loaded 0 gameobjectSpawnsExtra definitions. DB table `gameobject_spawns_extra` is empty.");
        return;
    }

    uint32_t count = 0;
    do
    {
        Field* fields = result->Fetch();

        uint32_t spawnId = fields[0].asUint32();

        MySQLStructure::GameObjectSpawnExtra& gameObjectAddon = _gameObjectSpawnExtraStore[spawnId];
        gameObjectAddon.parentRotation = QuaternionData(fields[1].asFloat(), fields[2].asFloat(), fields[3].asFloat(), fields[4].asFloat());

        if (!gameObjectAddon.parentRotation.isUnit())
        {
            sLogger.failure("GameObject (spawnId: {}) has invalid parent rotation in `gameobject_spawns_extra`, set to default", spawnId);
            gameObjectAddon.parentRotation = QuaternionData();
        }

        ++count;
    } while (result->NextRow());

    sLogger.info("Loaded {} gameobject overrides in {} ms", count, static_cast<uint32_t>(Util::GetTimeDifferenceToNow(startTime)));
}

MySQLStructure::GameObjectSpawnExtra const* MySQLDataStore::getGameObjectExtra(uint32_t lowguid) const
{
    GameObjectSpawnExtraContainer::const_iterator itr = _gameObjectSpawnExtraStore.find(lowguid);
    if (itr != _gameObjectSpawnExtraStore.end())
        return &(itr->second);

    return nullptr;
}

void MySQLDataStore::loadGameObjectSpawnsOverrideTable()
{
    auto startTime = Util::TimeNow();

    auto result = getWorldDBQuery("SELECT id, scale, faction, flags FROM gameobject_spawns_overrides WHERE min_build <= %u AND max_build >= %u", VERSION_STRING, VERSION_STRING);
    if (!result)
    {
        sLogger.info("Loaded 0 gameobject overrides. DB table `gameobject_spawn_overrides` is empty.");
        return;
    }

    uint32_t count = 0;
    do
    {
        Field* fields = result->Fetch();

        uint32_t spawnId = fields[0].asUint32();

        MySQLStructure::GameObjectSpawnOverrides& gameObjectOverride = _gameObjectSpawnOverrideStore[spawnId];
        gameObjectOverride.scale = fields[1].asFloat();
        gameObjectOverride.faction = fields[2].asUint16();
        gameObjectOverride.flags = fields[3].asUint32();

        if (gameObjectOverride.faction && !sFactionTemplateStore.lookupEntry(gameObjectOverride.faction))
            sLogger.failure("GameObject (SpawnId: {}) has invalid faction ({}) defined in `gameobject_spawns_overrides`.", spawnId, gameObjectOverride.faction);

        ++count;
    } while (result->NextRow());

    sLogger.info("Loaded {} gameobject overrides in {} ms", count, static_cast<uint32_t>(Util::GetTimeDifferenceToNow(startTime)));
}

MySQLStructure::GameObjectSpawnOverrides const* MySQLDataStore::getGameObjectOverride(uint32_t lowguid) const
{
    auto itr = _gameObjectSpawnOverrideStore.find(lowguid);
    return itr != _gameObjectSpawnOverrideStore.end() ? &itr->second : nullptr;
}

//quests
void MySQLDataStore::loadQuestPropertiesTable()
{
    auto startTime = Util::TimeNow();
    uint32_t quest_count = 0;


              //                                  0       1     2      3       4          5        6          7              8                 9
    auto quest_result = getWorldDBQuery("SELECT entry, ZoneId, sort, flags, MinLevel, questlevel, Type, RequiredRaces, RequiredClass, RequiredTradeskill, "
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

    sLogger.info("MySQLDataLoads : Table `quest_properties` has {} columns", quest_result->GetFieldCount());

    _questPropertiesStore.rehash(row_count + quest_result->GetRowCount());

    do
    {
        Field* fields = quest_result->Fetch();

        uint32_t entry = fields[0].asUint32();

        QuestProperties& questInfo = _questPropertiesStore[entry];

        questInfo.id = entry;
        questInfo.zone_id = fields[1].asUint32();
        questInfo.quest_sort = fields[2].asUint32();
        questInfo.quest_flags = fields[3].asUint32();
        questInfo.min_level = fields[4].asUint32();
        questInfo.questlevel = fields[5].asInt32();
        questInfo.type = fields[6].asUint32();
        questInfo.required_races = fields[7].asUint32();
        questInfo.required_class = fields[8].asUint32();
        questInfo.required_tradeskill = fields[9].asUint16();
        questInfo.required_tradeskill_value = fields[10].asUint32();
        questInfo.required_rep_faction = fields[11].asUint32();
        questInfo.required_rep_value = fields[12].asUint32();

        questInfo.time = fields[13].asUint32();
        questInfo.special_flags = fields[14].asUint32();

        questInfo.previous_quest_id = fields[15].asUint32();
        questInfo.next_quest_id = fields[16].asUint32();

        questInfo.srcitem = fields[17].asUint32();
        questInfo.srcitemcount = fields[18].asUint32();

        questInfo.title = fields[19].asCString();
        questInfo.details = fields[20].asCString();
        questInfo.objectives = fields[21].asCString();
        questInfo.completiontext = fields[22].asCString();
        questInfo.incompletetext = fields[23].asCString();
        questInfo.endtext = fields[24].asCString();

        for (uint8_t i = 0; i < 4; ++i)
        {
            questInfo.objectivetexts[i] = fields[25 + i].asCString();
        }

        for (uint8_t i = 0; i < MAX_REQUIRED_QUEST_ITEM; ++i)
        {
            questInfo.required_item[i] = fields[29 + i].asUint32();
            questInfo.required_itemcount[i] = fields[35 + i].asUint32();
        }

        for (uint8_t i = 0; i < 4; ++i)
        {
            questInfo.required_mob_or_go[i] = fields[41 + i].asInt32();
            if (questInfo.required_mob_or_go[i] != 0)
            {
                if (questInfo.required_mob_or_go[i] > 0)
                {
                    if (!getCreatureProperties(questInfo.required_mob_or_go[i]))
                    {
                        sLogger.debugFlag(AscEmu::Logging::LF_DB_TABLES, "Quest {} has `ReqCreatureOrGOId{}` = {} but creature with entry {} does not exist in creature_properties table!",
                            entry, i, questInfo.required_mob_or_go[i], questInfo.required_mob_or_go[i]);
                    }
                }
                else
                {
                    if (!getGameObjectProperties(-questInfo.required_mob_or_go[i]))
                    {
                        sLogger.debugFlag(AscEmu::Logging::LF_DB_TABLES, "Quest {} has `ReqCreatureOrGOId{}` = {} but gameobject {} does not exist in gameobject_properties table!",
                            entry, i, questInfo.required_mob_or_go[i], -questInfo.required_mob_or_go[i]);
                    }
                }
            }

            questInfo.required_mob_or_go_count[i] = fields[45 + i].asUint32();
        }

        for (uint8_t i = 0; i < 4; ++i)
        {
            questInfo.required_spell[i] = fields[49 + i].asUint32();
            questInfo.required_emote[i] = fields[53 + i].asUint32();
        }

        for (uint8_t i = 0; i < 6; ++i)
        {
            questInfo.reward_choiceitem[i] = fields[57 + i].asUint32();
            questInfo.reward_choiceitemcount[i] = fields[63 + i].asUint32();
        }

        for (uint8_t i = 0; i < 4; ++i)
        {
            questInfo.reward_item[i] = fields[69 + i].asUint32();
            questInfo.reward_itemcount[i] = fields[73 + i].asUint32();
        }

        for (uint8_t i = 0; i < 6; ++i)
        {
            questInfo.reward_repfaction[i] = fields[77 + i].asUint32();
            questInfo.reward_repvalue[i] = fields[83 + i].asInt32();
        }

        questInfo.reward_replimit = fields[89].asUint32();

        questInfo.reward_money = fields[90].asInt32();
        questInfo.reward_xp = fields[91].asUint32();
        questInfo.reward_spell = fields[92].asUint32();
        questInfo.effect_on_player = fields[93].asUint32();

        questInfo.MailTemplateId = fields[94].asUint32();
        questInfo.MailDelaySecs = fields[95].asUint32();
        questInfo.MailSendItem = fields[96].asUint32();

        questInfo.point_mapid = fields[97].asUint32();
        questInfo.point_x = fields[98].asFloat();
        questInfo.point_y = fields[99].asFloat();
        questInfo.point_opt = fields[100].asUint32();

        questInfo.rew_money_at_max_level = fields[101].asUint32();

        for (uint8_t i = 0; i < 4; ++i)
        {
            questInfo.required_triggers[i] = fields[102 + i].asUint32();
        }

        questInfo.x_or_y_quest_string = fields[106].asCString();

        for (uint8_t i = 0; i < 4; ++i)
        {
            questInfo.required_quests[i] = fields[107 + i].asUint32();
        }

        questInfo.remove_quests = fields[111].asCString();

        for (uint8_t i = 0; i < 4; ++i)
        {
            questInfo.receive_items[i] = fields[112 + i].asUint32();
            questInfo.receive_itemcount[i] = fields[116 + i].asUint32();
        }

        questInfo.is_repeatable = fields[120].asInt32();
        questInfo.bonushonor = fields[121].asUint32();
        questInfo.bonusarenapoints = fields[122].asUint32();
        questInfo.rewardtitleid = fields[123].asUint32();
        questInfo.rewardtalents = fields[124].asUint32();
        questInfo.suggestedplayers = fields[125].asUint32();

        // emotes
        questInfo.detailemotecount = fields[126].asUint32();

        for (uint8_t i = 0; i < 4; ++i)
        {
            questInfo.detailemote[i] = fields[127 + i].asUint32();
            questInfo.detailemotedelay[i] = fields[131 + i].asUint32();
        }

        questInfo.completionemotecount = fields[135].asUint32();

        for (uint8_t i = 0; i < 4; ++i)
        {
            questInfo.completionemote[i] = fields[136 + i].asUint32();
            questInfo.completionemotedelay[i] = fields[140 + i].asUint32();
        }

        questInfo.completeemote = fields[144].asUint32();
        questInfo.incompleteemote = fields[145].asUint32();
        questInfo.iscompletedbyspelleffect = fields[146].asUint32();
        questInfo.RewXPId = fields[147].asUint32();

        ++quest_count;
    } while (quest_result->NextRow());

    sLogger.info("MySQLDataLoads : Loaded {} quest_properties data in {} ms!", quest_count, static_cast<uint32_t>(Util::GetTimeDifferenceToNow(startTime)));
}

QuestProperties const* MySQLDataStore::getQuestProperties(uint32_t entry)
{
    QuestPropertiesContainer::const_iterator itr = _questPropertiesStore.find(entry);
    if (itr != _questPropertiesStore.end())
        return &(itr->second);

    return nullptr;
}

MySQLDataStore::QuestPropertiesContainer const* MySQLDataStore::getQuestPropertiesStore() { return &_questPropertiesStore; }

void MySQLDataStore::loadGameObjectQuestItemBindingTable()
{
    auto startTime = Util::TimeNow();

    //                                                                0      1     2        3
    auto gameobject_quest_item_result = WorldDatabase.Query("SELECT entry, quest, item, item_count FROM gameobject_quest_item_binding");

    uint32_t gameobject_quest_item_count = 0;

    if (gameobject_quest_item_result != nullptr)
    {
        do
        {
            Field* fields = gameobject_quest_item_result->Fetch();
            uint32_t entry = fields[0].asUint32();

            GameObjectProperties const* gameobject_properties = sMySQLStore.getGameObjectProperties(entry);
            if (gameobject_properties == nullptr)
            {
                sLogger.debugFlag(AscEmu::Logging::LF_DB_TABLES, "Table `gameobject_quest_item_binding` includes data for invalid gameobject_properties entry: {}. Skipped!", entry);
                continue;
            }

            uint32_t quest_entry = fields[1].asUint32();
            QuestProperties const* quest = sMySQLStore.getQuestProperties(quest_entry);
            if (quest == nullptr)
            {
                sLogger.debugFlag(AscEmu::Logging::LF_DB_TABLES, "Table `gameobject_quest_item_binding` includes data for invalid quest_properties : {}. Skipped!", quest_entry);
                continue;
            }
            else
            {
                const_cast<GameObjectProperties*>(gameobject_properties)->itemMap[quest].insert(std::make_pair(fields[2].asUint32(), fields[3].asUint32()));
            }

            ++gameobject_quest_item_count;
        } while (gameobject_quest_item_result->NextRow());
    }

    sLogger.info("MySQLDataLoads : Loaded {} data from `gameobject_quest_item_binding` table in {} ms!", gameobject_quest_item_count, static_cast<uint32_t>(Util::GetTimeDifferenceToNow(startTime)));
}

void MySQLDataStore::loadGameObjectQuestPickupBindingTable()
{
    auto startTime = Util::TimeNow();

    //                                                                  0      1           2
    auto gameobject_quest_pickup_result = WorldDatabase.Query("SELECT entry, quest, required_count FROM gameobject_quest_pickup_binding");

    uint32_t gameobject_quest_pickup_count = 0;

    if (gameobject_quest_pickup_result != nullptr)
    {
        do
        {
            Field* fields = gameobject_quest_pickup_result->Fetch();
            uint32_t entry = fields[0].asUint32();

            GameObjectProperties const* gameobject_properties = sMySQLStore.getGameObjectProperties(entry);
            if (gameobject_properties == nullptr)
            {
                sLogger.debugFlag(AscEmu::Logging::LF_DB_TABLES, "Table `gameobject_quest_pickup_binding` includes data for invalid gameobject_properties entry: {}. Skipped!", entry);
                continue;
            }

            uint32_t quest_entry = fields[1].asUint32();
            QuestProperties const* quest = sMySQLStore.getQuestProperties(quest_entry);
            if (quest == nullptr)
            {
                sLogger.debugFlag(AscEmu::Logging::LF_DB_TABLES, "Table `gameobject_quest_pickup_binding` includes data for invalid quest_properties : {}. Skipped!", quest_entry);
                continue;
            }
            else
            {
                uint32_t required_count = fields[2].asUint32();
                const_cast<GameObjectProperties*>(gameobject_properties)->goMap.insert(std::make_pair(quest, required_count));
            }


            ++gameobject_quest_pickup_count;
        } while (gameobject_quest_pickup_result->NextRow());
    }

    sLogger.info("MySQLDataLoads : Loaded {} data from `gameobject_quest_pickup_binding` table in {} ms!", gameobject_quest_pickup_count, static_cast<uint32_t>(Util::GetTimeDifferenceToNow(startTime)));
}

void MySQLDataStore::loadCreatureDifficultyTable()
{
    auto startTime = Util::TimeNow();

    //                                                             0          1            2             3
    auto creature_difficulty_result = WorldDatabase.Query("SELECT entry, difficulty_1, difficulty_2, difficulty_3 FROM creature_difficulty");

    if (creature_difficulty_result == nullptr)
    {
        sLogger.info("MySQLDataLoads : Table `creature_difficulty` is empty!");
        return;
    }

    sLogger.info("MySQLDataLoads : Table `creature_difficulty` has {} columns", creature_difficulty_result->GetFieldCount());

    _creatureDifficultyStore.rehash(creature_difficulty_result->GetRowCount());

    uint32_t creature_difficulty_count = 0;
    do
    {
        Field* fields = creature_difficulty_result->Fetch();

        uint32_t entry = fields[0].asUint32();

        MySQLStructure::CreatureDifficulty& creatureDifficulty = _creatureDifficultyStore[entry];

        creatureDifficulty.id = entry;

        creatureDifficulty.difficultyEntry1 = fields[1].asUint32();
        creatureDifficulty.difficultyEntry2 = fields[2].asUint32();
        creatureDifficulty.difficultyEntry3 = fields[3].asUint32();


        ++creature_difficulty_count;
    } while (creature_difficulty_result->NextRow());

    sLogger.info("MySQLDataLoads : Loaded {} creature difficulties info from `creature_difficulty` table in {} ms!", creature_difficulty_count, static_cast<uint32_t>(Util::GetTimeDifferenceToNow(startTime)));
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

    //                                                                       0       1    2     3      4      5      6         7
    //auto display_bounding_boxes_result = WorldDatabase.Query("SELECT displayid, lowx, lowy, lowz, highx, highy, highz, boundradius FROM display_bounding_boxes");
    auto display_bounding_boxes_result = WorldDatabase.Query("SELECT displayid, highz FROM display_bounding_boxes");

    if (display_bounding_boxes_result == nullptr)
    {
        sLogger.info("MySQLDataLoads : Table `display_bounding_boxes` is empty!");
        return;
    }

    sLogger.info("MySQLDataLoads : Table `display_bounding_boxes` has {} columns", display_bounding_boxes_result->GetFieldCount());

    _displayBoundingBoxesStore.rehash(display_bounding_boxes_result->GetRowCount());

    uint32_t display_bounding_boxes_count = 0;
    do
    {
        Field* fields = display_bounding_boxes_result->Fetch();

        uint32_t entry = fields[0].asUint32();

        MySQLStructure::DisplayBoundingBoxes& displayBounding = _displayBoundingBoxesStore[entry];

        displayBounding.displayid = entry;

        //for (uint8_t i = 0; i < 3; i++)
        //{
        //    displayBounding.low[i] = fields[1 + i].GetFloat();
        //    displayBounding.high[i] = fields[4 + i].GetFloat();
        //}

        //displayBounding.boundradius = fields[7].GetFloat();

        // highz is the only value used in Unit::EventModelChange()
        displayBounding.high[2] = fields[1].asFloat();


        ++display_bounding_boxes_count;
    } while (display_bounding_boxes_result->NextRow());

    sLogger.info("MySQLDataLoads : Loaded {} display bounding info from `display_bounding_boxes` table in {} ms!", display_bounding_boxes_count, static_cast<uint32_t>(Util::GetTimeDifferenceToNow(startTime)));
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

    //                                                              0       1          2            3              4
    auto vendor_restricitons_result = WorldDatabase.Query("SELECT entry, racemask, classmask, reqrepfaction, reqrepfactionvalue, "
    //                                                                    5                 6           7
                                                                  "canbuyattextid, cannotbuyattextid, flags FROM vendor_restrictions");

    if (vendor_restricitons_result == nullptr)
    {
        sLogger.info("MySQLDataLoads : Table `vendor_restrictions` is empty!");
        return;
    }

    sLogger.info("MySQLDataLoads : Table `vendor_restrictions` has {} columns", vendor_restricitons_result->GetFieldCount());

    _vendorRestrictionsStore.rehash(vendor_restricitons_result->GetRowCount());

    uint32_t vendor_restricitons_count = 0;
    do
    {
        Field* fields = vendor_restricitons_result->Fetch();

        uint32_t entry = fields[0].asUint32();

        MySQLStructure::VendorRestrictions& vendorRestriction = _vendorRestrictionsStore[entry];

        vendorRestriction.entry = entry;
        vendorRestriction.racemask = fields[1].asInt32();
        vendorRestriction.classmask = fields[2].asInt32();
        vendorRestriction.reqrepfaction = fields[3].asUint32();
        vendorRestriction.reqrepvalue = fields[4].asUint32();
        vendorRestriction.canbuyattextid = fields[5].asUint32();
        vendorRestriction.cannotbuyattextid = fields[6].asUint32();
        vendorRestriction.flags = fields[7].asUint32();

        ++vendor_restricitons_count;
    } while (vendor_restricitons_result->NextRow());

    sLogger.info("MySQLDataLoads : Loaded {} restrictions from `vendor_restrictions` table in {} ms!", vendor_restricitons_count, static_cast<uint32_t>(Util::GetTimeDifferenceToNow(startTime)));
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

    //                                                          0
    auto npc_gossip_text_result = WorldDatabase.Query("SELECT entry, "
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

    sLogger.info("MySQLDataLoads : Table `npc_gossip_texts` has {} columns", npc_gossip_text_result->GetFieldCount());

    _npcGossipTextStore.rehash(npc_gossip_text_result->GetRowCount());

    uint32_t npc_text_count = 0;
    do
    {
        Field* fields = npc_gossip_text_result->Fetch();

        uint32_t entry = fields[0].asUint32();

        MySQLStructure::NpcGossipText& npcText = _npcGossipTextStore[entry];

        npcText.entry = entry;
        for (uint8_t i = 0; i < 8; ++i)
        {
            npcText.textHolder[i].probability = fields[1].asFloat();

            for (uint8_t j = 0; j < 2; ++j)
            {
                npcText.textHolder[i].texts[j] = fields[2 + j].asCString();
            }

            npcText.textHolder[i].language = fields[4].asUint32();

            for (uint8_t k = 0; k < GOSSIP_EMOTE_COUNT; ++k)
            {
                npcText.textHolder[i].gossipEmotes[k].delay = fields[5 + k * 2].asUint32();
                npcText.textHolder[i].gossipEmotes[k].emote = fields[6 + k * 2].asUint32();
            }
        }


        ++npc_text_count;
    } while (npc_gossip_text_result->NextRow());

    sLogger.info("MySQLDataLoads : Loaded {} rows from `npc_gossip_texts` table in {} ms!", npc_text_count, static_cast<uint32_t>(Util::GetTimeDifferenceToNow(startTime)));
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

    //                                                          0      1           2       3     4       5          6         7       8        9         10
    auto npc_script_text_result = WorldDatabase.Query("SELECT entry, text, creature_entry, id, type, language, probability, emote, duration, sound, broadcast_id FROM npc_script_text");

    if (npc_script_text_result == nullptr)
    {
        sLogger.info("MySQLDataLoads : Table `npc_script_text` is empty!");
        return;
    }

    sLogger.info("MySQLDataLoads : Table `npc_script_text` has {} columns", npc_script_text_result->GetFieldCount());

    _npcScriptTextStore.rehash(npc_script_text_result->GetRowCount());

    uint32_t npc_script_text_count = 0;
    do
    {
        Field* fields = npc_script_text_result->Fetch();

        uint32_t entry = fields[0].asUint32();

        MySQLStructure::NpcScriptText& npcScriptText = _npcScriptTextStore[entry];

        npcScriptText.id = entry;
        npcScriptText.text = fields[1].asCString();
        npcScriptText.creature_entry = fields[2].asUint32();
        npcScriptText.text_id = fields[3].asUint32();
        npcScriptText.type = fields[4].asUint8();
        npcScriptText.language = Languages(fields[5].asUint32());
        npcScriptText.probability = fields[6].asFloat();
        npcScriptText.emote = EmoteType(fields[7].asUint32());
        npcScriptText.duration = fields[8].asUint32();
        npcScriptText.sound = fields[9].asUint32();
        npcScriptText.broadcast_id = fields[10].asUint32();

        // Store Sorted by CreatureId with a vector of all Texts for that creature
        _npcScriptTextStoreById[npcScriptText.creature_entry].push_back(npcScriptText);

        ++npc_script_text_count;
    } while (npc_script_text_result->NextRow());

    sLogger.info("MySQLDataLoads : Loaded {} rows from `npc_script_text` table in {} ms!", npc_script_text_count, static_cast<uint32_t>(Util::GetTimeDifferenceToNow(startTime)));
}

MySQLStructure::NpcScriptText const* MySQLDataStore::getNpcScriptText(uint32_t entry)
{
    NpcScriptTextContainer::const_iterator itr = _npcScriptTextStore.find(entry);
    if (itr != _npcScriptTextStore.end())
        return &(itr->second);

    return nullptr;
}

MySQLStructure::NpcScriptText const* MySQLDataStore::getNpcScriptTextById(uint32_t entry, uint8_t index)
{
    if (!entry)
        return nullptr;

    NpcScriptTextByIdContainer::const_iterator list = _npcScriptTextStoreById.find(entry);
    if (list != _npcScriptTextStoreById.end())
    {
        std::vector<MySQLStructure::NpcScriptText> const& textList = list->second;
        for (const auto& text : textList)
        {
            if (text.text_id == index)
                return &text;
        }
    }

    return nullptr;
}

void MySQLDataStore::loadGossipMenuOptionTable()
{
    auto startTime = Util::TimeNow();

    //                                                              0         1
    auto gossip_menu_optiont_result = WorldDatabase.Query("SELECT entry, option_text FROM gossip_menu_option");

    if (gossip_menu_optiont_result == nullptr)
    {
        sLogger.info("MySQLDataLoads : Table `gossip_menu_option` is empty!");
        return;
    }

    sLogger.info("MySQLDataLoads : Table `gossip_menu_option` has {} columns", gossip_menu_optiont_result->GetFieldCount());

    _gossipMenuOptionStore.rehash(gossip_menu_optiont_result->GetRowCount());

    uint32_t gossip_menu_optiont_count = 0;
    do
    {
        Field* fields = gossip_menu_optiont_result->Fetch();

        uint32_t entry = fields[0].asUint32();

        MySQLStructure::GossipMenuOption& gossipMenuOptionText = _gossipMenuOptionStore[entry];

        gossipMenuOptionText.id = entry;
        gossipMenuOptionText.text = fields[1].asCString();

        ++gossip_menu_optiont_count;
    } while (gossip_menu_optiont_result->NextRow());

    sLogger.info("MySQLDataLoads : Loaded {} rows from `gossip_menu_option` table in {} ms!", gossip_menu_optiont_count, static_cast<uint32_t>(Util::GetTimeDifferenceToNow(startTime)));
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

    //                                                   0         1         2           3            4         5          6           7       8
    auto graveyards_result = WorldDatabase.Query("SELECT id, position_x, position_y, position_z, orientation, zoneid, adjacentzoneid, mapid, faction FROM graveyards");
    if (graveyards_result == nullptr)
    {
        sLogger.info("MySQLDataLoads : Table `graveyards` is empty!");
        return;
    }

    sLogger.info("MySQLDataLoads : Table `graveyards` has {} columns", graveyards_result->GetFieldCount());

    _graveyardsStore.rehash(graveyards_result->GetRowCount());

    uint32_t graveyards_count = 0;
    do
    {
        Field* fields = graveyards_result->Fetch();

        uint32_t entry = fields[0].asUint32();

        MySQLStructure::Graveyards& graveyardTeleport = _graveyardsStore[entry];

        graveyardTeleport.id = entry;
        graveyardTeleport.position_x = fields[1].asFloat();
        graveyardTeleport.position_y = fields[2].asFloat();
        graveyardTeleport.position_z = fields[3].asFloat();
        graveyardTeleport.orientation = fields[4].asFloat();
        graveyardTeleport.zoneId = fields[5].asUint32();
        graveyardTeleport.adjacentZoneId = fields[6].asUint32();
        graveyardTeleport.mapId = fields[7].asUint32();
        graveyardTeleport.factionId = fields[8].asUint32();

        ++graveyards_count;
    } while (graveyards_result->NextRow());

    sLogger.info("MySQLDataLoads : Loaded {} rows from `graveyards` table in {} ms!", graveyards_count, static_cast<uint32_t>(Util::GetTimeDifferenceToNow(startTime)));
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

    //                                                        0     1         2           3           4
    auto teleport_coords_result = WorldDatabase.Query("SELECT id, mapId, position_x, position_y, position_z FROM spell_teleport_coords");
    if (teleport_coords_result == nullptr)
    {
        sLogger.info("MySQLDataLoads : Table `spell_teleport_coords` is empty!");
        return;
    }

    sLogger.info("MySQLDataLoads : Table `spell_teleport_coords` has {} columns", teleport_coords_result->GetFieldCount());

    _teleportCoordsStore.rehash(teleport_coords_result->GetRowCount());

    uint32_t teleport_coords_count = 0;
    do
    {
        Field* fields = teleport_coords_result->Fetch();

        uint32_t entry = fields[0].asUint32();

        TeleportCoords& teleportCoords = _teleportCoordsStore[entry];

        teleportCoords.id = entry;
        teleportCoords.mapId = fields[1].asUint32();
        teleportCoords.x = fields[2].asFloat();
        teleportCoords.y = fields[3].asFloat();
        teleportCoords.z = fields[4].asFloat();

        ++teleport_coords_count;
    } while (teleport_coords_result->NextRow());

    sLogger.info("MySQLDataLoads : Loaded {} rows from `spell_teleport_coords` table in {} ms!", teleport_coords_count, static_cast<uint32_t>(Util::GetTimeDifferenceToNow(startTime)));
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

    //                                                  0      1         2
    auto fishing_result = WorldDatabase.Query("SELECT zone, MinSkill, MaxSkill FROM fishing");
    if (fishing_result == nullptr)
    {
        sLogger.info("MySQLDataLoads : Table `fishing` is empty!");
        return;
    }

    sLogger.info("MySQLDataLoads : Table `fishing` has {} columns", fishing_result->GetFieldCount());

    _fishingZonesStore.rehash(fishing_result->GetRowCount());

    uint32_t fishing_count = 0;
    do
    {
        Field* fields = fishing_result->Fetch();

        uint32_t entry = fields[0].asUint32();

        MySQLStructure::FishingZones& fishingZone = _fishingZonesStore[entry];

        fishingZone.zoneId = entry;
        fishingZone.minSkill = fields[1].asUint32();
        fishingZone.maxSkill = fields[2].asUint32();

        ++fishing_count;
    } while (fishing_result->NextRow());

    sLogger.info("MySQLDataLoads : Loaded {} rows from `fishing` table in {} ms!", fishing_count, static_cast<uint32_t>(Util::GetTimeDifferenceToNow(startTime)));
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

    //                                                       0        1       2       3           4             5          6        7      8          9
    auto worldmap_info_result = WorldDatabase.Query("SELECT entry, screenid, type, maxplayers, minlevel, minlevel_heroic, repopx, repopy, repopz, repopentry, "
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

    sLogger.info("MySQLDataLoads : Table `worldmap_info` has {} columns", worldmap_info_result->GetFieldCount());

    _worldMapInfoStore.rehash(worldmap_info_result->GetRowCount());

    uint32_t world_map_info_count = 0;
    do
    {
        Field* fields = worldmap_info_result->Fetch();

        uint32_t entry = fields[0].asUint32();

        MySQLStructure::MapInfo& mapInfo = _worldMapInfoStore[entry];

        mapInfo.mapid = entry;
        mapInfo.screenid = fields[1].asUint32();
        mapInfo.type = fields[2].asUint32();
        mapInfo.playerlimit = fields[3].asUint32();
        mapInfo.minlevel = fields[4].asUint32();
        mapInfo.minlevel_heroic = fields[5].asUint32();
        mapInfo.repopx = fields[6].asFloat();
        mapInfo.repopy = fields[7].asFloat();
        mapInfo.repopz = fields[8].asFloat();
        mapInfo.repopmapid = fields[9].asUint32();
        mapInfo.name = fields[10].asCString();
        mapInfo.flags = fields[11].asUint32();
        mapInfo.cooldown = fields[12].asUint32();
        mapInfo.lvl_mod_a = fields[13].asUint32();
        mapInfo.required_quest_A = fields[14].asUint32();
        mapInfo.required_quest_H = fields[15].asUint32();
        mapInfo.required_item = fields[16].asUint32();
        mapInfo.heroic_key_1 = fields[17].asUint32();
        mapInfo.heroic_key_2 = fields[18].asUint32();
        mapInfo.update_distance = fields[19].asFloat();
        mapInfo.checkpoint_id = fields[20].asUint32();

        ++world_map_info_count;
    } while (worldmap_info_result->NextRow());

    sLogger.info("MySQLDataLoads : Loaded {} rows from `worldmap_info` table in {} ms!", world_map_info_count, static_cast<uint32_t>(Util::GetTimeDifferenceToNow(startTime)));
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

    //                                                     0         1              2
    auto zone_guards_result = WorldDatabase.Query("SELECT zone, horde_entry, alliance_entry FROM zoneguards");
    if (zone_guards_result == nullptr)
    {
        sLogger.info("MySQLDataLoads : Table `zoneguards` is empty!");
        return;
    }

    sLogger.info("MySQLDataLoads : Table `zoneguards` has {} columns", zone_guards_result->GetFieldCount());

    _zoneGuardsStore.rehash(zone_guards_result->GetRowCount());

    uint32_t zone_guards_count = 0;
    do
    {
        Field* fields = zone_guards_result->Fetch();

        uint32_t entry = fields[0].asUint32();

        MySQLStructure::ZoneGuards& zoneGuard = _zoneGuardsStore[entry];

        zoneGuard.zoneId = entry;
        zoneGuard.hordeEntry = fields[1].asUint32();
        zoneGuard.allianceEntry = fields[2].asUint32();

        ++zone_guards_count;
    } while (zone_guards_result->NextRow());

    sLogger.info("MySQLDataLoads : Loaded {} rows from `zoneguards` table in {} ms!", zone_guards_count, static_cast<uint32_t>(Util::GetTimeDifferenceToNow(startTime)));
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

    //                                                            0                1
    auto battlemasters_result = WorldDatabase.Query("SELECT creature_entry, battleground_id FROM battlemasters");
    if (battlemasters_result == nullptr)
    {
        sLogger.info("MySQLDataLoads : Table `battlemasters` is empty!");
        return;
    }

    sLogger.info("MySQLDataLoads : Table `battlemasters` has {} columns", battlemasters_result->GetFieldCount());

    _battleMastersStore.rehash(battlemasters_result->GetRowCount());

    uint32_t battlemasters_count = 0;
    do
    {
        Field* fields = battlemasters_result->Fetch();

        uint32_t entry = fields[0].asUint32();

        MySQLStructure::Battlemasters& bgMaster = _battleMastersStore[entry];

        bgMaster.creatureEntry = entry;
        bgMaster.battlegroundId = fields[1].asUint32();

        ++battlemasters_count;
    } while (battlemasters_result->NextRow());

    sLogger.info("MySQLDataLoads : Loaded {} rows from `battlemasters` table in {} ms!", battlemasters_count, static_cast<uint32_t>(Util::GetTimeDifferenceToNow(startTime)));
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

    //                                                          0     1        2
    auto totemdisplayids_result = WorldDatabase.Query("SELECT race, totem, displayid FROM totemdisplayids base "
        "WHERE build=(SELECT MAX(build) FROM totemdisplayids spec WHERE base.race = spec.race AND base.totem = spec.totem AND build <= %u)", VERSION_STRING);

    if (totemdisplayids_result == nullptr)
    {
        sLogger.info("MySQLDataLoads : Table `totemdisplayids` is empty!");
        return;
    }

    sLogger.info("MySQLDataLoads : Table `totemdisplayids` has {} columns", totemdisplayids_result->GetFieldCount());

    uint32_t totemdisplayids_count = 0;
    do
    {
        Field* fields = totemdisplayids_result->Fetch();

        MySQLStructure::TotemDisplayIds totemDisplayId;

        totemDisplayId._race = static_cast<uint8_t>(fields[0].asUint32());
        totemDisplayId.display_id = fields[1].asUint32();
        totemDisplayId.race_specific_id = fields[2].asUint32();

        _totemDisplayIdsStore.push_back(totemDisplayId);

        ++totemdisplayids_count;
    } while (totemdisplayids_result->NextRow());

    sLogger.info("MySQLDataLoads : Loaded {} rows from `totemdisplayids` table in {} ms!", static_cast<uint32_t>(_totemDisplayIdsStore.size()), static_cast<uint32_t>(Util::GetTimeDifferenceToNow(startTime)));
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
    auto spellclickspells_result = WorldDatabase.Query("SELECT npc_entry, spell_id, cast_flags, user_type FROM npc_spellclick_spells");
    if (spellclickspells_result == nullptr)
    {
        sLogger.info("MySQLDataLoads : Table `spellclickspells` is empty!");
        return;
    }

    sLogger.info("MySQLDataLoads : Table `spellclickspells` has {} columns", spellclickspells_result->GetFieldCount());

    uint32_t spellclickspells_count = 0;
    do
    {
        Field* fields = spellclickspells_result->Fetch();

        uint32_t npc_entry = fields[0].asUint32();
        CreatureProperties const* cInfo = sMySQLStore.getCreatureProperties(npc_entry);
        if (!cInfo)
        {
            sLogger.debugFlag(AscEmu::Logging::LF_DB_TABLES, "Table npc_spellclick_spells references unknown creature_properties {}. Skipping entry.", npc_entry);
            continue;
        }

        uint32_t spellid = fields[1].asUint32();
        SpellInfo const* spellinfo = sSpellMgr.getSpellInfo(spellid);
        if (!spellinfo)
        {
            sLogger.debugFlag(AscEmu::Logging::LF_DB_TABLES, "Table npc_spellclick_spells creature: {} references unknown spellid {}. Skipping entry.", npc_entry, spellid);
            continue;
        }

        uint8_t userType = fields[3].asUint8();
        if (userType >= SPELL_CLICK_USER_MAX)
        {
            sLogger.debugFlag(AscEmu::Logging::LF_DB_TABLES, "Table npc_spellclick_spells creature: {} references unknown user type {}. Skipping entry.", npc_entry, uint32_t(userType));
            continue;
        }
        uint8_t castFlags = fields[2].asUint8();

        SpellClickInfo info;
        info.spellId = spellid;
        info.castFlags = castFlags;
        info.userType = SpellClickUserTypes(userType);
        _spellClickInfoStore.insert(SpellClickInfoContainer::value_type(npc_entry, info));
    } while (spellclickspells_result->NextRow());

    sLogger.info("MySQLDataLoads : Loaded {} rows from `spellclickspells` table in {} ms!", spellclickspells_count, static_cast<uint32_t>(Util::GetTimeDifferenceToNow(startTime)));
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

    //                                                             0     1
    auto worldstring_tables_result = WorldDatabase.Query("SELECT entry, text FROM worldstring_tables");
    if (worldstring_tables_result == nullptr)
    {
        sLogger.info("MySQLDataLoads : Table `worldstring_tables` is empty!");
        return;
    }

    sLogger.info("MySQLDataLoads : Table `worldstring_tables` has {} columns", worldstring_tables_result->GetFieldCount());

    _worldStringsStore.rehash(worldstring_tables_result->GetRowCount());

    uint32_t worldstring_tables_count = 0;
    do
    {
        Field* fields = worldstring_tables_result->Fetch();

        uint32_t entry = fields[0].asUint32();

        MySQLStructure::WorldStringTable& worldString = _worldStringsStore[entry];

        worldString.id = entry;
        worldString.text = fields[1].asCString();

        ++worldstring_tables_count;
    } while (worldstring_tables_result->NextRow());

    sLogger.info("MySQLDataLoads : Loaded {} rows from `worldstring_tables` table in {} ms!", worldstring_tables_count, static_cast<uint32_t>(Util::GetTimeDifferenceToNow(startTime)));
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

    //                                                              0   1  2    3     4     5        6
    auto points_of_interest_result = WorldDatabase.Query("SELECT entry, x, y, icon, flags, data, icon_name FROM points_of_interest");
    if (points_of_interest_result == nullptr)
    {
        sLogger.info("MySQLDataLoads : Table `points_of_interest` is empty!");
        return;
    }

    sLogger.info("MySQLDataLoads : Table `points_of_interest` has {} columns", points_of_interest_result->GetFieldCount());

    _pointsOfInterestStore.rehash(points_of_interest_result->GetRowCount());

    uint32_t points_of_interest_count = 0;
    do
    {
        Field* fields = points_of_interest_result->Fetch();

        uint32_t entry = fields[0].asUint32();

        MySQLStructure::PointsOfInterest& pointOfInterest = _pointsOfInterestStore[entry];

        pointOfInterest.id = entry;
        pointOfInterest.x = fields[1].asFloat();
        pointOfInterest.y = fields[2].asFloat();
        pointOfInterest.icon = fields[3].asUint32();
        pointOfInterest.flags = fields[4].asUint32();
        pointOfInterest.data = fields[5].asUint32();
        pointOfInterest.iconName = fields[6].asCString();

        ++points_of_interest_count;
    } while (points_of_interest_result->NextRow());

    sLogger.info("MySQLDataLoads : Loaded {} rows from `points_of_interest` table in {} ms!", points_of_interest_count, static_cast<uint32_t>(Util::GetTimeDifferenceToNow(startTime)));
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

    //                                                           0            1
    auto linked_set_bonus_result = WorldDatabase.Query("SELECT itemset, itemset_bonus FROM itemset_linked_itemsetbonus");
    if (linked_set_bonus_result == nullptr)
    {
        sLogger.info("MySQLDataLoads : Table `itemset_linked_itemsetbonus` is empty!");
        return;
    }

    sLogger.info("MySQLDataLoads : Table `itemset_linked_itemsetbonus` has {} columns", linked_set_bonus_result->GetFieldCount());

    _definedItemSetBonusStore.rehash(linked_set_bonus_result->GetRowCount());

    uint32_t linked_set_bonus_count = 0;
    do
    {
        Field* fields = linked_set_bonus_result->Fetch();

        int32_t entry = fields[0].asInt32();

        MySQLStructure::ItemSetLinkedItemSetBonus& itemSetLinkedItemSetBonus = _definedItemSetBonusStore[entry];

        itemSetLinkedItemSetBonus.itemset = entry;
        itemSetLinkedItemSetBonus.itemset_bonus  = fields[1].asUint32();

        ++linked_set_bonus_count;

    } while (linked_set_bonus_result->NextRow());

    sLogger.info("MySQLDataLoads : Loaded {} rows from `itemset_linked_itemsetbonus` table in {} ms!", linked_set_bonus_count, static_cast<uint32_t>(Util::GetTimeDifferenceToNow(startTime)));
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

    //                                                                0              1           2          3
    auto initial_equipment_result = WorldDatabase.Query("SELECT creature_entry, itemslot_1, itemslot_2, itemslot_3 FROM creature_initial_equip;");
    if (initial_equipment_result == nullptr)
    {
        sLogger.info("MySQLDataLoads : Table `creature_initial_equip` is empty!");
        return;
    }

    sLogger.info("MySQLDataLoads : Table `creature_initial_equip` has {} columns", initial_equipment_result->GetFieldCount());

    uint32_t initial_equipment_count = 0;
    do
    {
        Field* fields = initial_equipment_result->Fetch();
        uint32_t entry = fields[0].asUint32();
        CreatureProperties const* creature_properties = sMySQLStore.getCreatureProperties(entry);
        if (creature_properties == nullptr)
        {
            //sLogger.debug("Invalid creature_entry {} in table creature_initial_equip!", entry);
            continue;
        }

        uint32_t itemId = fields[1].asUint32();
        if (sMySQLStore.getItemProperties(itemId) || sItemStore.lookupEntry(itemId))
            const_cast<CreatureProperties*>(creature_properties)->itemslot_1 = itemId;
        else
            sLogger.debugFlag(AscEmu::Logging::LF_DB_TABLES, "MySQLDataLoads : Table `creature_initial_equip` has unknown itemslot_1 {} for creature {}", itemId, entry);

        itemId = fields[2].asUint32();
        if (sMySQLStore.getItemProperties(itemId) || sItemStore.lookupEntry(itemId))
            const_cast<CreatureProperties*>(creature_properties)->itemslot_2 = itemId;
        else
            sLogger.debugFlag(AscEmu::Logging::LF_DB_TABLES, "MySQLDataLoads : Table `creature_initial_equip` has unknown itemslot_2 {} for creature {}", itemId, entry);

        itemId = fields[3].asUint32();
        if (sMySQLStore.getItemProperties(itemId) || sItemStore.lookupEntry(itemId))
            const_cast<CreatureProperties*>(creature_properties)->itemslot_3 = itemId;
        else
            sLogger.debugFlag(AscEmu::Logging::LF_DB_TABLES, "MySQLDataLoads : Table `creature_initial_equip` has unknown itemslot_3 {} for creature {}", itemId, entry);

        ++initial_equipment_count;

    } while (initial_equipment_result->NextRow());

    sLogger.info("MySQLDataLoads : Loaded {} rows from `creature_initial_equip` table in {} ms!", initial_equipment_count, static_cast<uint32_t>(Util::GetTimeDifferenceToNow(startTime)));
}

void MySQLDataStore::loadPlayerCreateInfoTable()
{
    auto startTime = Util::TimeNow();

    //                                                             1     2      3      4          5          6         7           8
    auto player_create_info_result = WorldDatabase.Query("SELECT race, class, mapID, zoneID, positionX, positionY, positionZ, orientation FROM playercreateinfo pi "

        "WHERE build=(SELECT MAX(build) FROM playercreateinfo buildspecific WHERE pi.race = buildspecific.race AND pi.class = buildspecific.class AND build <= %u)", VERSION_STRING);
    if (player_create_info_result == nullptr)
    {
        sLogger.info("MySQLDataLoads : Table `playercreateinfo` is empty!");
        return;
    }

    sLogger.info("MySQLDataLoads : Table `playercreateinfo` has {} columns", player_create_info_result->GetFieldCount());

    uint32_t player_create_info_count = 0;
    do
    {
        Field* fields = player_create_info_result->Fetch();

        uint8_t _race = fields[0].asUint8();
        uint8_t _class = fields[1].asUint8();
        _playerCreateInfoStoreNew[_race][_class] = std::make_unique<PlayerCreateInfo>();
        auto* playerCreateInfo = _playerCreateInfoStoreNew[_race][_class].get();

        playerCreateInfo->mapId = fields[2].asUint32();
        playerCreateInfo->zoneId = fields[3].asUint32();
        playerCreateInfo->positionX = fields[4].asFloat();
        playerCreateInfo->positionY = fields[5].asFloat();
        playerCreateInfo->positionZ = fields[6].asFloat();
        playerCreateInfo->orientation = fields[7].asFloat();

        player_create_info_count++;

    } while (player_create_info_result->NextRow());

    sLogger.info("MySQLDataLoads : Loaded {} rows from `playercreateinfo` table in {} ms!", player_create_info_count, static_cast<uint32_t>(Util::GetTimeDifferenceToNow(startTime)));
}


void MySQLDataStore::loadPlayerCreateInfoBars()
{

    //                                                                 0     1      2        3      4     5
    auto player_create_info_bars_result = WorldDatabase.Query("SELECT race, class, button, action, type, misc FROM playercreateinfo_bars WHERE build = %u", VERSION_STRING);

    if (player_create_info_bars_result == nullptr)
    {
        sLogger.info("MySQLDataLoads : Table `playercreateinfo_bars` has no data");
        return;
    }

    uint32_t player_create_info_bars_count = 0;
    do
    {
        Field* fields = player_create_info_bars_result->Fetch();

        uint8_t _race = fields[0].asUint8();
        uint8_t _class = fields[1].asUint8();

        if (auto& playerCreateInfo = _playerCreateInfoStoreNew[_race][_class])
        {
            CreateInfo_ActionBarStruct bar{};
            bar.button = fields[2].asUint8();
            bar.action = fields[3].asUint32();
            bar.type = fields[4].asUint8();
            bar.misc = fields[5].asUint8();

            playerCreateInfo->actionbars.push_back(bar);

            ++player_create_info_bars_count;
        }

    } while (player_create_info_bars_result->NextRow());
}

void MySQLDataStore::loadPlayerCreateInfoItems()
{
    auto startTime = Util::TimeNow();

    //                                                                   0     1       2       3       4
    auto player_create_info_items_result = WorldDatabase.Query("SELECT race, class, protoid, slotid, amount FROM playercreateinfo_items WHERE build = %u", VERSION_STRING);

    if (player_create_info_items_result == nullptr)
    {
        sLogger.info("MySQLDataLoads : Table `playercreateinfo_items` is empty!");
        return;
    }

    sLogger.info("MySQLDataLoads : Table `playercreateinfo_items` has {} columns", player_create_info_items_result->GetFieldCount());

    uint32_t player_create_info_items_count = 0;
    do
    {
        Field* fields = player_create_info_items_result->Fetch();

        uint8_t _race = fields[0].asUint8();
        uint8_t _class = fields[1].asUint8();
        uint32_t item_id = fields[2].asUint32();

#if VERSION_STRING < Cata
        auto player_item = sMySQLStore.getItemProperties(item_id);
#else
        WDB::Structures::ItemEntry const* player_item = sItemStore.lookupEntry(item_id);
#endif
        if (player_item == nullptr)
        {
            sLogger.failure("Table `old_playercreateinfo_items` includes invalid item {}", item_id);
            continue;
        }

        if (auto& playerCreateInfo = _playerCreateInfoStoreNew[_race][_class])
        {
            CreateInfo_ItemStruct itm{};
            itm.id = item_id;
            itm.slot = fields[3].asUint8();
            itm.amount = fields[4].asUint32();

            playerCreateInfo->items.push_back(itm);

            ++player_create_info_items_count;
        }

    } while (player_create_info_items_result->NextRow());

    sLogger.info("MySQLDataLoads : Loaded {} rows from `playercreateinfo_items` table in {} ms!", player_create_info_items_count, static_cast<uint32_t>(Util::GetTimeDifferenceToNow(startTime)));
}

void MySQLDataStore::loadPlayerCreateInfoSkills()
{
    auto startTime = Util::TimeNow();

    //                                                                      0         1         2       3
    auto player_create_info_skills_result = WorldDatabase.Query("SELECT raceMask, classMask, skillid, level FROM playercreateinfo_skills WHERE min_build <= %u AND max_build >= %u", getAEVersion(), getAEVersion());

    if (player_create_info_skills_result == nullptr)
    {
        sLogger.info("MySQLDataLoads : Table `playercreateinfo_skills` is empty!");
        return;
    }

    sLogger.info("MySQLDataLoads : Table `playercreateinfo_skills` has {} columns", player_create_info_skills_result->GetFieldCount());

    uint32_t player_create_info_skills_count = 0;
    do
    {
        Field* fields = player_create_info_skills_result->Fetch();

        uint32_t raceMask = fields[0].asUint32();
        uint32_t classMask = fields[1].asUint32();
        auto skill_id = fields[2].asUint16();

        auto player_skill = sSkillLineStore.lookupEntry(skill_id);
        if (player_skill == nullptr)
        {
            sLogger.failure("Table `playercreateinfo_skills` includes invalid skill id {}", skill_id);
            continue;
        }

        CreateInfo_SkillStruct tsk{};
        tsk.skillid = skill_id;
        tsk.currentval = fields[3].asUint16();

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

    sLogger.info("MySQLDataLoads : Loaded {} rows from `playercreateinfo_skills` table in {} ms!", player_create_info_skills_count, static_cast<uint32_t>(Util::GetTimeDifferenceToNow(startTime)));
}

void MySQLDataStore::loadPlayerCreateInfoSpellLearn()
{
    auto startTime = Util::TimeNow();

    //                                                                     0         1         2
    auto player_create_info_spells_result = WorldDatabase.Query("SELECT raceMask, classMask, spellid FROM playercreateinfo_spell_learn WHERE min_build <= %u AND max_build >= %u", getAEVersion(), getAEVersion());

    if (player_create_info_spells_result == nullptr)
    {
        sLogger.info("MySQLDataLoads : Table `playercreateinfo_spell_learn` is empty!");
        return;
    }

    sLogger.info("MySQLDataLoads : Table `playercreateinfo_spell_learn` has {} columns", player_create_info_spells_result->GetFieldCount());

    uint32_t player_create_info_spells_count = 0;
    do
    {
        Field* fields = player_create_info_spells_result->Fetch();

        uint32_t raceMask = fields[0].asUint32();
        uint32_t classMask = fields[1].asUint32();
        uint32_t spell_id = fields[2].asUint32();

        auto player_spell = sSpellStore.lookupEntry(spell_id);
        if (player_spell == nullptr)
        {
            sLogger.failure("Table `playercreateinfo_spell_learn` includes invalid spell {}", spell_id);
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

    sLogger.info("MySQLDataLoads : Loaded {} rows from `playercreateinfo_spell_learn` table in {} ms!", player_create_info_spells_count, static_cast<uint32_t>(Util::GetTimeDifferenceToNow(startTime)));
}

void MySQLDataStore::loadPlayerCreateInfoSpellCast()
{
    auto startTime = Util::TimeNow();

    //                                                                      0         1         2
    auto player_create_info_spells_result = WorldDatabase.Query("SELECT raceMask, classMask, spellid FROM playercreateinfo_spell_cast WHERE build = %u", VERSION_STRING);

    if (player_create_info_spells_result == nullptr)
    {
        sLogger.info("MySQLDataLoads : Table `playercreateinfo_spell_cast` is empty!");
        return;
    }

    sLogger.info("MySQLDataLoads : Table `playercreateinfo_spell_cast` has {} columns", player_create_info_spells_result->GetFieldCount());

    uint32_t player_create_info_spells_count = 0;
    do
    {
        Field* fields = player_create_info_spells_result->Fetch();

        uint32_t raceMask = fields[0].asUint32();
        uint32_t classMask = fields[1].asUint32();
        uint32_t spell_id = fields[2].asUint32();

        auto player_spell = sSpellStore.lookupEntry(spell_id);
        if (player_spell == nullptr)
        {
            sLogger.failure("Table `playercreateinfo_spell_cast` includes invalid spell {}", spell_id);
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

    sLogger.info("MySQLDataLoads : Loaded {} rows from `playercreateinfo_spell_cast` table in {} ms!", player_create_info_spells_count, static_cast<uint32_t>(Util::GetTimeDifferenceToNow(startTime)));
}

void MySQLDataStore::loadPlayerCreateInfoLevelstats()
{
    auto startTime = Util::TimeNow();

    //                                                           0     1      2          3           4            5             6             7
    auto player_levelstats_result = WorldDatabase.Query("SELECT race, class, level, BaseStrength, BaseAgility, BaseStamina, BaseIntellect, BaseSpirit FROM player_levelstats WHERE build = %u", VERSION_STRING);

    if (player_levelstats_result == nullptr)
    {
        sLogger.info("MySQLDataLoads : Table `player_levelstats` is empty!");
        return;
    }

    sLogger.info("MySQLDataLoads : Table `player_levelstats` has {} columns", player_levelstats_result->GetFieldCount());

    uint32_t player_levelstats_count = 0;
    do
    {
        Field* fields = player_levelstats_result->Fetch();

        uint32_t _race = fields[0].asUint32();
        uint32_t _class = fields[1].asUint32();
        uint32_t level = fields[2].asUint32();


        if (auto& playerCreateInfo = _playerCreateInfoStoreNew[_race][_class])
        {
            CreateInfo_Levelstats lvl{};
            lvl.strength = fields[3].asUint32();
            lvl.agility = fields[4].asUint32();
            lvl.stamina = fields[5].asUint32();
            lvl.intellect = fields[6].asUint32();
            lvl.spirit = fields[7].asUint32();

            playerCreateInfo->level_stats.insert(std::make_pair(level, lvl));

            ++player_levelstats_count;
        }

    } while (player_levelstats_result->NextRow());

    sLogger.info("MySQLDataLoads : Loaded {} rows from `player_levelstats` table in {} ms!", player_levelstats_count, static_cast<uint32_t>(Util::GetTimeDifferenceToNow(startTime)));

    //Zyres: load required and missing levelstats
    for (uint8_t _race = 0; _race < DBC_NUM_RACES; ++_race)
    {
        if (!sChrRacesStore.lookupEntry(_race))
            continue;

        for (uint8_t _class = 0; _class < MAX_PLAYER_CLASSES; ++_class)
        {
            if (!sChrClassesStore.lookupEntry(_class))
                continue;

            const auto& info = _playerCreateInfoStoreNew[_race][_class];
            if (!info)
                continue;

            for (uint8_t level = 1; level < DBC_STAT_LEVEL_CAP; ++level)
            {
                if (info->level_stats[level].strength == 0)
                {
                    sLogger.debugFlag(AscEmu::Logging::LF_DB_TABLES, "Race {} Class {} Level {} does not have stats data. Using stats data of level {}.", _race, _class, level + 1, level);
                    info->level_stats[level] = info->level_stats[level - 1U];
                }
            }
        }
    }
}

void MySQLDataStore::loadPlayerCreateInfoClassLevelstats()
{
    auto startTime = Util::TimeNow();

    // Zyres: load highest gamebuild version from table, otherwise we will have dead new characters
    //                                                                 0      1        2          3
    auto player_classlevelstats_result = WorldDatabase.Query("SELECT class, level, BaseHealth, BaseMana FROM player_classlevelstats base "
        "WHERE build=(SELECT MAX(build) FROM player_classlevelstats buildspecific WHERE base.class = buildspecific.class AND base.level = buildspecific.level AND build <= %u)", VERSION_STRING);

    if (player_classlevelstats_result)
    {
        sLogger.info("MySQLDataLoads : Table `player_classlevelstats` has {} columns", player_classlevelstats_result->GetFieldCount());

        uint32_t player_classlevelstats_count = 0;
        do
        {
            Field* fields = player_classlevelstats_result->Fetch();

            uint32_t _class = fields[0].asUint32();
            uint32_t level = fields[1].asUint32();

            CreateInfo_ClassLevelStats lvl{};
            lvl.health = fields[2].asUint32();
            lvl.mana = fields[3].asUint32();

            _playerClassLevelStatsStore[_class].insert(std::make_pair(level, lvl));

            ++player_classlevelstats_count;

        } while (player_classlevelstats_result->NextRow());

        sLogger.info("MySQLDataLoads : Loaded {} rows from `player_classlevelstats` table in {} ms!", player_classlevelstats_count, static_cast<uint32_t>(Util::GetTimeDifferenceToNow(startTime)));
    }
    else
    {
        sLogger.info("MySQLDataLoads : Table `player_classlevelstats` is empty!");
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

            WDB::Structures::GtOCTBaseHPByClassEntry const* hp = sGtOCTBaseHPByClassStore.lookupEntry((player_class - 1) * DBC_STAT_LEVEL_CAP + level - 1);
            WDB::Structures::GtOCTBaseMPByClassEntry const* mp = sGtOCTBaseMPByClassStore.lookupEntry((player_class - 1) * DBC_STAT_LEVEL_CAP + level - 1);

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

    sLogger.info("MySQLDataLoads : Loaded {} missing classlevelstats from dbc!", player_classlevelstats_count);

#endif
}


PlayerCreateInfo const* MySQLDataStore::getPlayerCreateInfo(uint8_t player_race, uint8_t player_class)
{
    return _playerCreateInfoStoreNew[player_race][player_class].get();
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

    auto player_xp_to_level_result = WorldDatabase.Query("SELECT player_lvl, next_lvl_req_xp FROM player_xp_for_level base "
        "WHERE build=(SELECT MAX(build) FROM player_xp_for_level spec WHERE base.player_lvl = spec.player_lvl AND build <= %u)", VERSION_STRING);

    if (player_xp_to_level_result == nullptr)
    {
        sLogger.info("MySQLDataLoads : Table `player_xp_for_level` is empty!");
        return;
    }

    sLogger.info("MySQLDataLoads : Table `player_xp_for_level` has {} columns", player_xp_to_level_result->GetFieldCount());

    uint32_t player_xp_to_level_count = 0;
    do
    {
        Field* fields = player_xp_to_level_result->Fetch();
        uint32_t current_level = fields[0].asUint8();
        uint32_t current_xp = fields[1].asUint32();

        if (current_level >= worldConfig.player.playerLevelCap)
        {
            sLogger.debugFlag(AscEmu::Logging::LF_DB_TABLES, "Table `player_xp_for_level` includes invalid xp definitions for level {} which is higher than the defined levelcap in your config file! <skipped>", current_level);
            continue;
        }

        _playerXPperLevelStore[current_level] = current_xp;

        ++player_xp_to_level_count;

    } while (player_xp_to_level_result->NextRow());

    sLogger.info("MySQLDataLoads : Loaded {} rows from `player_xp_for_level` table in {} ms!", player_xp_to_level_count, static_cast<uint32_t>(Util::GetTimeDifferenceToNow(startTime)));

    if (player_xp_to_level_count < (worldConfig.player.playerLevelCap - 1))
        sLogger.failure("Table `player_xp_for_level` includes definitions for {} level, but your defined level cap is {}!", player_xp_to_level_count, worldConfig.player.playerLevelCap);
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
    auto spelloverride_result = WorldDatabase.Query("SELECT DISTINCT overrideId FROM spelloverride");
    if (spelloverride_result == nullptr)
    {
        sLogger.info("MySQLDataLoads : Table `spelloverride` is empty!");
        return;
    }

    do
    {
        Field* fields = spelloverride_result->Fetch();
        uint32_t distinct_override_id = fields[0].asUint32();

        auto spellid_for_overrideid_result = WorldDatabase.Query("SELECT spellId FROM spelloverride WHERE overrideId = %u", distinct_override_id);
        auto list = std::make_unique<std::list<SpellInfo const*>>();
        if (spellid_for_overrideid_result != nullptr)
        {
            do
            {
                Field* fieldsIn = spellid_for_overrideid_result->Fetch();
                uint32_t spellid = fieldsIn[0].asUint32();
                SpellInfo const* spell = sSpellMgr.getSpellInfo(spellid);
                if (spell == nullptr)
                {
                    sLogger.debugFlag(AscEmu::Logging::LF_DB_TABLES, "Table `spelloverride` includes invalid spellId {} for overrideId {}! <skipped>", spellid, distinct_override_id);
                    continue;
                }

                list->push_back(spell);

            } while (spellid_for_overrideid_result->NextRow());
        }

        if (!list->empty())
        {
            _spellOverrideIdStore.emplace(distinct_override_id, std::move(list));
        }

    } while (spelloverride_result->NextRow());

    sLogger.info("MySQLDataLoads : {} spell overrides loaded.", static_cast<uint32_t>(_spellOverrideIdStore.size()));
}

void MySQLDataStore::loadNpcGossipTextIdTable()
{
    auto startTime = Util::TimeNow();
    //                                                    0         1
    auto npc_gossip_properties_result = WorldDatabase.Query("SELECT creatureid, textid FROM npc_gossip_properties");
    if (npc_gossip_properties_result == nullptr)
    {
        sLogger.info("MySQLDataLoads : Table `npc_gossip_properties` is empty!");
        return;
    }

    sLogger.info("MySQLDataLoads : Table `npc_gossip_properties` has {} columns", npc_gossip_properties_result->GetFieldCount());

    uint32_t npc_gossip_properties_count = 0;
    do
    {
        Field* fields = npc_gossip_properties_result->Fetch();
        uint32_t entry = fields[0].asUint32();
        auto creature_properties = sMySQLStore.getCreatureProperties(entry);
        if (creature_properties == nullptr)
        {
            sLogger.debugFlag(AscEmu::Logging::LF_DB_TABLES, "Table `npc_gossip_properties` includes invalid creatureid {}! <skipped>", entry);
            continue;
        }

        uint32_t text = fields[1].asUint32();

        _npcGossipTextIdStore[entry] = text;

        ++npc_gossip_properties_count;

    } while (npc_gossip_properties_result->NextRow());

    sLogger.info("MySQLDataLoads : Loaded {} rows from `npc_gossip_properties` table in {} ms!", npc_gossip_properties_count, static_cast<uint32_t>(Util::GetTimeDifferenceToNow(startTime)));
}

uint32_t MySQLDataStore::getGossipTextIdForNpc(uint32_t entry)
{
    return _npcGossipTextIdStore[entry];
}

void MySQLDataStore::loadPetLevelAbilitiesTable()
{
    auto startTime = Util::TimeNow();
    //                                                              0       1      2        3        4        5         6         7
    auto pet_level_abilities_result = WorldDatabase.Query("SELECT level, health, armor, strength, agility, stamina, intellect, spirit FROM pet_level_abilities");
    if (pet_level_abilities_result == nullptr)
    {
        sLogger.info("MySQLDataLoads : Table `pet_level_abilities` is empty!");
        return;
    }

    sLogger.info("MySQLDataLoads : Table `pet_level_abilities` has {} columns", pet_level_abilities_result->GetFieldCount());

    _petLevelAbilitiesStore.rehash(pet_level_abilities_result->GetRowCount());

    uint32_t pet_level_abilities_count = 0;
    do
    {
        Field* fields = pet_level_abilities_result->Fetch();

        uint32_t entry = fields[0].asUint32();

        MySQLStructure::PetLevelAbilities& petAbilities = _petLevelAbilitiesStore[entry];

        petAbilities.level = entry;
        petAbilities.health = fields[1].asUint32();
        petAbilities.armor = fields[2].asUint32();
        petAbilities.strength = fields[3].asUint32();
        petAbilities.agility = fields[4].asUint32();
        petAbilities.stamina = fields[5].asUint32();
        petAbilities.intellect = fields[6].asUint32();
        petAbilities.spirit = fields[7].asUint32();

        ++pet_level_abilities_count;

    } while (pet_level_abilities_result->NextRow());

    sLogger.info("MySQLDataLoads : Loaded {} rows from `pet_level_abilities` table in {} ms!", pet_level_abilities_count, static_cast<uint32_t>(Util::GetTimeDifferenceToNow(startTime)));

    if (pet_level_abilities_count < worldConfig.player.playerLevelCap)
        sLogger.failure("Table `pet_level_abilities` includes definitions for {} level, but your defined level cap is {}!", pet_level_abilities_count, worldConfig.player.playerLevelCap);
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

    auto broadcast_result = getWorldDBQuery("SELECT * FROM worldbroadcast");
    if (broadcast_result == nullptr)
    {
        sLogger.info("MySQLDataLoads : Table `worldbroadcast` is empty!");
        return;
    }

    sLogger.info("MySQLDataLoads : Table `worldbroadcast` has {} columns", broadcast_result->GetFieldCount());

    _worldBroadcastStore.rehash(broadcast_result->GetRowCount());

    uint32_t broadcast_count = 0;
    do
    {
        Field* fields = broadcast_result->Fetch();

        uint32_t entry = fields[0].asUint32();

        MySQLStructure::WorldBroadCast& broadcast = _worldBroadcastStore[entry];

        broadcast.id = entry;

        uint32_t interval = fields[1].asUint32();
        broadcast.interval = interval * 60;
        uint32_t random_interval = fields[2].asUint32();
        broadcast.randomInterval = random_interval * 60;
        broadcast.nextUpdate = broadcast.interval + (uint32_t)UNIXTIME;
        broadcast.text = fields[3].asCString();

        ++broadcast_count;

    } while (broadcast_result->NextRow());

    sLogger.info("MySQLDataLoads : Loaded {} rows from `worldbroadcast` table in {} ms!", broadcast_count, static_cast<uint32_t>(Util::GetTimeDifferenceToNow(startTime)));
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
    //                                                       0      1    2     3       4       5           6          7             8               9                  10
    auto area_trigger_result = WorldDatabase.Query("SELECT entry, type, map, screen, name, position_x, position_y, position_z, orientation, required_honor_rank, required_level FROM areatriggers");
    if (area_trigger_result == nullptr)
    {
        sLogger.info("MySQLDataLoads : Table `areatriggers` is empty!");
        return;
    }

    sLogger.info("MySQLDataLoads : Table `areatriggers` has {} columns", area_trigger_result->GetFieldCount());

    _areaTriggerStore.rehash(area_trigger_result->GetRowCount());

    uint32_t areaTrigger_count = 0;
    do
    {
        Field* fields = area_trigger_result->Fetch();

        MySQLStructure::AreaTrigger areaTrigger;
        areaTrigger.id = fields[0].asUint32();
        areaTrigger.type = fields[1].asUint8();
        areaTrigger.mapId = fields[2].asUint16();
        areaTrigger.pendingScreen = fields[3].asUint32();
        areaTrigger.name = fields[4].asCString();
        areaTrigger.x = fields[5].asFloat();
        areaTrigger.y = fields[6].asFloat();
        areaTrigger.z = fields[7].asFloat();
        areaTrigger.o = fields[8].asFloat();
        areaTrigger.requiredHonorRank = fields[9].asUint32();
        areaTrigger.requiredLevel = fields[10].asUint32();

        WDB::Structures::AreaTriggerEntry const* area_trigger_entry = sAreaTriggerStore.lookupEntry(areaTrigger.id);
        if (!area_trigger_entry)
        {
            sLogger.debugFlag(AscEmu::Logging::LF_DB_TABLES, "AreaTrigger : Area trigger (ID:{}) does not exist in `AreaTrigger.dbc`.", areaTrigger.id);
            continue;
        }

        WDB::Structures::MapEntry const* map_entry = sMapStore.lookupEntry(areaTrigger.mapId);
        if (!map_entry)
        {
            sLogger.debugFlag(AscEmu::Logging::LF_DB_TABLES, "AreaTrigger : Area trigger (ID:{}) target map (ID: {}) does not exist in `Map.dbc`.", areaTrigger.id, areaTrigger.mapId);
            continue;
        }

        if (areaTrigger.x == 0 && areaTrigger.y == 0 && areaTrigger.z == 0 && (areaTrigger.type == ATTYPE_INSTANCE || areaTrigger.type == ATTYPE_TELEPORT))    // check target coordinates only for teleport triggers
        {
            sLogger.debugFlag(AscEmu::Logging::LF_DB_TABLES, "AreaTrigger : Area trigger (ID:{}) target coordinates not provided.", areaTrigger.id);
            continue;
        }

        _areaTriggerStore[areaTrigger.id] = areaTrigger;
        ++areaTrigger_count;

    } while (area_trigger_result->NextRow());

    sLogger.info("MySQLDataLoads : Loaded {} rows from `areatriggers` table in {} ms!", areaTrigger_count, static_cast<uint32_t>(Util::GetTimeDifferenceToNow(startTime)));
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
            WDB::Structures::AreaTriggerEntry const* area_trigger_entry = sAreaTriggerStore.lookupEntry(itr->first);
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
    WDB::Structures::MapEntry const* mapEntry = sMapStore.lookupEntry(mapId);
    if (!mapEntry || mapEntry->parent_map < 0)
        return nullptr;

    if (mapEntry->isInstanceMap())
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
            WDB::Structures::AreaTriggerEntry const* atEntry = sAreaTriggerStore.lookupEntry(itr->first);
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

    if (mapEntry->isInstanceMap())
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
            WDB::Structures::AreaTriggerEntry const* atEntry = sAreaTriggerStore.lookupEntry(itr->first);
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

    auto filter_character_names_result = WorldDatabase.Query("SELECT * FROM wordfilter_character_names");
    if (filter_character_names_result == nullptr)
    {
        sLogger.info("MySQLDataLoads : Table `wordfilter_character_names` is empty!");
        return;
    }

    sLogger.info("MySQLDataLoads : Table `wordfilter_character_names` has {} columns", filter_character_names_result->GetFieldCount());

    _wordFilterCharacterNamesStore.clear();

    uint32_t filter_character_names_count = 0;
    do
    {
        Field* fields = filter_character_names_result->Fetch();

        MySQLStructure::WordFilterCharacterNames wfCharacterNames;
        wfCharacterNames.name = fields[0].asCString();
        wfCharacterNames.nameReplace = fields[1].asCString();
        if (wfCharacterNames.nameReplace.empty())
        {
            wfCharacterNames.nameReplace = "?%$?%$";
        }

        _wordFilterCharacterNamesStore.push_back(wfCharacterNames);

        ++filter_character_names_count;

    } while (filter_character_names_result->NextRow());

    sLogger.info("MySQLDataLoads : Loaded {} rows from `wordfilter_character_names` table in {} ms!", filter_character_names_count, static_cast<uint32_t>(Util::GetTimeDifferenceToNow(startTime)));
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

    auto filter_chat_result = WorldDatabase.Query("SELECT * FROM wordfilter_chat");
    if (filter_chat_result == nullptr)
    {
        sLogger.info("MySQLDataLoads : Table `wordfilter_chat` is empty!");
        return;
    }

    sLogger.info("MySQLDataLoads : Table `wordfilter_chat` has {} columns", filter_chat_result->GetFieldCount());

    _wordFilterChatStore.clear();

    uint32_t filter_chat_count = 0;
    do
    {
        Field* fields = filter_chat_result->Fetch();

        MySQLStructure::WordFilterChat wfChat;
        wfChat.word = fields[0].asCString();
        wfChat.wordReplace = fields[1].asCString();
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

    sLogger.info("MySQLDataLoads : Loaded {} rows from `wordfilter_chat` table in {} ms!", filter_chat_count, static_cast<uint32_t>(Util::GetTimeDifferenceToNow(startTime)));
}

//////////////////////////////////////////////////////////////////////////////////////////
// locales

void MySQLDataStore::loadLocalesAchievementReward()
{
    auto startTime = Util::TimeNow();
    //                                          0       1          2           3       4
    auto result = WorldDatabase.Query("SELECT entry, gender, language_code, subject, text FROM locales_achievement_reward");
    if (result == nullptr)
    {
        sLogger.info("MySQLDataLoads : Table `locales_achievement_reward` is empty!");
        return;
    }

    sLogger.info("MySQLDataLoads : Table `locales_achievement_reward` has {} columns", result->GetFieldCount());

    uint32_t load_count = 0;
    uint32_t i = 0;
    do
    {
        ++i;
        Field* fields = result->Fetch();

        MySQLStructure::LocalesAchievementReward localAchievementReward;

        localAchievementReward.entry = fields[0].asUint32();
        localAchievementReward.gender = fields[1].asUint32();
        std::string locString = fields[2].asCString();
        localAchievementReward.languageCode = Util::getLanguagesIdFromString(locString);
        localAchievementReward.subject = strdup(fields[3].asCString());
        localAchievementReward.text = strdup(fields[4].asCString());

        _localesAchievementRewardStore.push_back(localAchievementReward);

        ++load_count;

    } while (result->NextRow());

    sLogger.info("MySQLDataLoads : Loaded {} rows from `locales_achievement_reward` table in {} ms!", load_count, static_cast<uint32_t>(Util::GetTimeDifferenceToNow(startTime)));
}

MySQLStructure::LocalesAchievementReward const* MySQLDataStore::getLocalizedAchievementReward(uint32_t entry, uint32_t gender, uint32_t sessionLocale)
{
    for (const auto& localesAchievementReward : _localesAchievementRewardStore)
    {
        if (localesAchievementReward.entry == entry && localesAchievementReward.languageCode == sessionLocale)
        {
            if (localesAchievementReward.gender == gender || localesAchievementReward.gender == 2)
            {
                return &localesAchievementReward;
            }
        }
    }
    return nullptr;
}

void MySQLDataStore::loadLocalesCreature()
{
    auto startTime = Util::TimeNow();
    //                                        0         1          2      3
    auto result = WorldDatabase.Query("SELECT id, language_code, name, subname FROM locales_creature base "
        "WHERE build=(SELECT MAX(build) FROM locales_creature buildspecific WHERE base.id = buildspecific.id AND build <= %u)", VERSION_STRING);
    if (result == nullptr)
    {
        sLogger.info("MySQLDataLoads : Table `locales_creature` is empty!");
        return;
    }

    sLogger.info("MySQLDataLoads : Table `locales_creature` has {} columns", result->GetFieldCount());

    _localesCreatureStore.rehash(result->GetRowCount());

    uint32_t load_count = 0;
    uint32_t i = 0;
    do
    {
        ++i;
        Field* fields = result->Fetch();

        MySQLStructure::LocalesCreature& localCreature = _localesCreatureStore[i];

        localCreature.entry = fields[0].asUint32();
        std::string locString = fields[1].asCString();
        localCreature.languageCode = Util::getLanguagesIdFromString(locString);
        localCreature.name = strdup(fields[2].asCString());
        localCreature.subName = strdup(fields[3].asCString());

        ++load_count;

    } while (result->NextRow());

    sLogger.info("MySQLDataLoads : Loaded {} rows from `locales_creature` table in {} ms!", load_count, static_cast<uint32_t>(Util::GetTimeDifferenceToNow(startTime)));
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
    //                                          0         1          2
    auto result = WorldDatabase.Query("SELECT entry, language_code, name FROM locales_gameobject");
    if (result == nullptr)
    {
        sLogger.info("MySQLDataLoads : Table `locales_gameobject` is empty!");
        return;
    }

    sLogger.info("MySQLDataLoads : Table `locales_gameobject` has {} columns", result->GetFieldCount());

    _localesGameobjectStore.rehash(result->GetRowCount());

    uint32_t load_count = 0;
    uint32_t i = 0;
    do
    {
        ++i;
        Field* fields = result->Fetch();

        MySQLStructure::LocalesGameobject& localGameobject = _localesGameobjectStore[i];

        localGameobject.entry = fields[0].asUint32();
        std::string locString = fields[1].asCString();
        localGameobject.languageCode = Util::getLanguagesIdFromString(locString);
        localGameobject.name = strdup(fields[2].asCString());

        ++load_count;

    } while (result->NextRow());

    sLogger.info("MySQLDataLoads : Loaded {} rows from `locales_gameobject` table in {} ms!", load_count, static_cast<uint32_t>(Util::GetTimeDifferenceToNow(startTime)));
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
    //                                          0         1             2
    auto result = WorldDatabase.Query("SELECT entry, language_code, option_text FROM locales_gossip_menu_option");
    if (result == nullptr)
    {
        sLogger.info("MySQLDataLoads : Table `locales_gossip_menu_option` is empty!");
        return;
    }

    sLogger.info("MySQLDataLoads : Table `locales_gossip_menu_option` has {} columns", result->GetFieldCount());

    _localesGossipMenuOptionStore.rehash(result->GetRowCount());

    uint32_t load_count = 0;
    uint32_t i = 0;
    do
    {
        ++i;
        Field* fields = result->Fetch();

        MySQLStructure::LocalesGossipMenuOption& localGossipMenuOption = _localesGossipMenuOptionStore[1];

        localGossipMenuOption.entry = fields[0].asUint32();
        std::string locString = fields[1].asCString();
        localGossipMenuOption.languageCode = Util::getLanguagesIdFromString(locString);
        localGossipMenuOption.name = strdup(fields[2].asCString());

        ++load_count;

    } while (result->NextRow());

    sLogger.info("MySQLDataLoads : Loaded {} rows from `locales_gossip_menu_option` table in {} ms!", load_count, static_cast<uint32_t>(Util::GetTimeDifferenceToNow(startTime)));
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
    //                                          0         1          2         3
    auto result = WorldDatabase.Query("SELECT entry, language_code, name, description FROM locales_item");
    if (result == nullptr)
    {
        sLogger.info("MySQLDataLoads : Table `locales_item` is empty!");
        return;
    }

    sLogger.info("MySQLDataLoads : Table `locales_item` has {} columns", result->GetFieldCount());

    _localesItemStore.rehash(result->GetRowCount());

    uint32_t load_count = 0;
    uint32_t i = 0;
    do
    {
        ++i;
        Field* fields = result->Fetch();

        MySQLStructure::LocalesItem& localItem = _localesItemStore[i];

        localItem.entry = fields[0].asUint32();
        std::string locString = fields[1].asCString();
        localItem.languageCode = Util::getLanguagesIdFromString(locString);
        localItem.name = strdup(fields[2].asCString());
        localItem.description = strdup(fields[3].asCString());

        ++load_count;

    } while (result->NextRow());

    sLogger.info("MySQLDataLoads : Loaded {} rows from `locales_item` table in {} ms!", load_count, static_cast<uint32_t>(Util::GetTimeDifferenceToNow(startTime)));
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

char* MySQLDataStore::getLocalizedItemName(uint32_t entry, uint32_t sessionLocale)
{
    return getLocalizedItem(entry, sessionLocale)->name;
}

MySQLStructure::RecallStruct const* MySQLDataStore::getRecallByName(std::string const& name) const
{
    std::string searchName(name);
    AscEmu::Util::Strings::toLowerCase(searchName);

    for (const auto& itr : _recallStore)
    {
        std::string recallName(itr->name);
        AscEmu::Util::Strings::toLowerCase(recallName);
        if (recallName == searchName)
            return itr.get();
    }

    return nullptr;
}

void MySQLDataStore::loadLocalesItemPages()
{
    auto startTime = Util::TimeNow();
    //                                          0         1           2
    auto result = WorldDatabase.Query("SELECT entry, language_code, text FROM locales_item_pages");
    if (result == nullptr)
    {
        sLogger.info("MySQLDataLoads : Table `locales_item_pages` is empty!");
        return;
    }

    sLogger.info("MySQLDataLoads : Table `locales_item_pages` has {} columns", result->GetFieldCount());

    _localesItemPagesStore.rehash(result->GetRowCount());

    uint32_t load_count = 0;
    uint32_t i = 0;
    do
    {
        ++i;
        Field* fields = result->Fetch();

        MySQLStructure::LocalesItemPages& localesItemPages = _localesItemPagesStore[i];

        localesItemPages.entry = fields[0].asUint32();
        std::string locString = fields[1].asCString();
        localesItemPages.languageCode = Util::getLanguagesIdFromString(locString);
        localesItemPages.text = strdup(fields[2].asCString());

        ++load_count;

    } while (result->NextRow());

    sLogger.info("MySQLDataLoads : Loaded {} rows from `locales_item_pages` table in {} ms!", load_count, static_cast<uint32_t>(Util::GetTimeDifferenceToNow(startTime)));
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
    //                                          0         1          2
    auto result = WorldDatabase.Query("SELECT entry, language_code, text FROM locales_npc_script_text");
    if (result == nullptr)
    {
        sLogger.info("MySQLDataLoads : Table `locales_npc_script_text` is empty!");
        return;
    }

    sLogger.info("MySQLDataLoads : Table `locales_npc_script_text` has {} columns", result->GetFieldCount());

    _localesNpcScriptTextStore.rehash(result->GetRowCount());

    uint32_t load_count = 0;
    uint32_t i = 0;
    do
    {
        ++i;
        Field* fields = result->Fetch();

        MySQLStructure::LocalesNpcScriptText& localNpcScriptText = _localesNpcScriptTextStore[i];

        localNpcScriptText.entry = fields[0].asUint32();
        std::string locString = fields[1].asCString();
        localNpcScriptText.languageCode = Util::getLanguagesIdFromString(locString);
        localNpcScriptText.text = strdup(fields[2].asCString());

        ++load_count;

    } while (result->NextRow());

    sLogger.info("MySQLDataLoads : Loaded {} rows from `locales_npc_script_text` table in {} ms!", load_count, static_cast<uint32_t>(Util::GetTimeDifferenceToNow(startTime)));
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
    //                                          0         1           2       3       4       5       6       7       8       9       10      11     12      13      14      15      16      17
    auto result = WorldDatabase.Query("SELECT entry, language_code, text0, text0_1, text1, text1_1, text2, text2_1, text3, text3_1, text4, text4_1, text5, text5_1, text6, text6_1, text7, text7_1 FROM locales_npc_gossip_texts");
    if (result == nullptr)
    {
        sLogger.info("MySQLDataLoads : Table `locales_npc_gossip_texts` is empty!");
        return;
    }

    sLogger.info("MySQLDataLoads : Table `locales_npc_gossip_texts` has {} columns", result->GetFieldCount());

    _localesNpcGossipTextStore.rehash(result->GetRowCount());

    uint32_t load_count = 0;
    uint32_t i = 0;
    do
    {
        ++i;
        Field* fields = result->Fetch();

        MySQLStructure::LocalesNpcGossipText& localNpcGossipText = _localesNpcGossipTextStore[i];

        localNpcGossipText.entry = fields[0].asUint32();
        std::string locString = fields[1].asCString();
        localNpcGossipText.languageCode = Util::getLanguagesIdFromString(locString);

        for (uint8_t j = 0; j < 8; ++j)
        {
            localNpcGossipText.texts[j][0] = strdup(fields[2 + (2 * j)].asCString());
            localNpcGossipText.texts[j][1] = strdup(fields[3 + (2 * j)].asCString());
        }

        ++load_count;

    } while (result->NextRow());

    sLogger.info("MySQLDataLoads : Loaded {} rows from `locales_npc_gossip_texts` table in {} ms!", load_count, static_cast<uint32_t>(Util::GetTimeDifferenceToNow(startTime)));
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

void MySQLDataStore::loadLocalesPointsOfInterest()
{
    auto startTime = Util::TimeNow();
    //                                          0         1             2
    auto result = WorldDatabase.Query("SELECT entry, language_code, icon_name FROM locales_points_of_interest");
    if (result == nullptr)
    {
        sLogger.info("MySQLDataLoads : Table `locales_points_of_interest` is empty!");
        return;
    }

    sLogger.info("MySQLDataLoads : Table `locales_points_of_interest` has {} columns", result->GetFieldCount());

    _localesPointsOfInterestStore.rehash(result->GetRowCount());

    uint32_t load_count = 0;
    uint32_t i = 0;
    do
    {
        ++i;
        Field* fields = result->Fetch();

        MySQLStructure::LocalesPointsOfInterest& localPointsOfInterest = _localesPointsOfInterestStore[i];

        localPointsOfInterest.entry = fields[0].asUint32();
        std::string locString = fields[1].asCString();
        localPointsOfInterest.languageCode = Util::getLanguagesIdFromString(locString);
        localPointsOfInterest.iconName = strdup(fields[2].asCString());

        ++load_count;

    } while (result->NextRow());

    sLogger.info("MySQLDataLoads : Loaded {} rows from `locales_points_of_interest` table in {} ms!", load_count, static_cast<uint32_t>(Util::GetTimeDifferenceToNow(startTime)));
}

MySQLStructure::LocalesPointsOfInterest const* MySQLDataStore::getLocalizedPointsOfInterest(uint32_t entry, uint32_t sessionLocale)
{
    for (LocalesPointsOfInterestContainer::const_iterator itr = _localesPointsOfInterestStore.begin(); itr != _localesPointsOfInterestStore.end(); ++itr)
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
    //                                          0         1           2       3         4            5                 6           7           8                9              10             11
    auto result = WorldDatabase.Query("SELECT entry, language_code, Title, Details, Objectives, CompletionText, IncompleteText, EndText, ObjectiveText1, ObjectiveText2, ObjectiveText3, ObjectiveText4 FROM locales_quest");
    if (result == nullptr)
    {
        sLogger.info("MySQLDataLoads : Table `locales_quest` is empty!");
        return;
    }

    sLogger.info("MySQLDataLoads : Table `locales_quest` has {} columns", result->GetFieldCount());

    _localesQuestStore.rehash(result->GetRowCount());

    uint32_t load_count = 0;
    uint32_t i = 0;
    do
    {
        ++i;
        Field* fields = result->Fetch();

        MySQLStructure::LocalesQuest& localQuest = _localesQuestStore[i];

        localQuest.entry = fields[0].asUint32();
        std::string locString = fields[1].asCString();
        localQuest.languageCode = Util::getLanguagesIdFromString(locString);
        localQuest.title = strdup(fields[2].asCString());
        localQuest.details = strdup(fields[3].asCString());
        localQuest.objectives = strdup(fields[4].asCString());
        localQuest.completionText = strdup(fields[5].asCString());
        localQuest.incompleteText = strdup(fields[6].asCString());
        localQuest.endText = strdup(fields[7].asCString());
        localQuest.objectiveText[0] = strdup(fields[8].asCString());
        localQuest.objectiveText[1] = strdup(fields[9].asCString());
        localQuest.objectiveText[2] = strdup(fields[10].asCString());
        localQuest.objectiveText[3] = strdup(fields[11].asCString());

        ++load_count;

    } while (result->NextRow());

    sLogger.info("MySQLDataLoads : Loaded {} rows from `locales_quest` table in {} ms!", load_count, static_cast<uint32_t>(Util::GetTimeDifferenceToNow(startTime)));
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
    //                                          0         1          2
    auto result = WorldDatabase.Query("SELECT entry, language_code, text FROM locales_worldbroadcast");
    if (result == nullptr)
    {
        sLogger.info("MySQLDataLoads : Table `locales_worldbroadcast` is empty!");
        return;
    }

    sLogger.info("MySQLDataLoads : Table `locales_worldbroadcast` has {} columns", result->GetFieldCount());

    _localesWorldbroadcastStore.rehash(result->GetRowCount());

    uint32_t load_count = 0;
    uint32_t i = 0;
    do
    {
        ++i;
        Field* fields = result->Fetch();

        MySQLStructure::LocalesWorldbroadcast& localWorldbroadcast = _localesWorldbroadcastStore[i];

        localWorldbroadcast.entry = fields[0].asUint32();
        std::string locString = fields[1].asCString();
        localWorldbroadcast.languageCode = Util::getLanguagesIdFromString(locString);
        localWorldbroadcast.text = strdup(fields[2].asCString());

        ++load_count;

    } while (result->NextRow());

    sLogger.info("MySQLDataLoads : Loaded {} rows from `locales_worldbroadcast` table in {} ms!", load_count, static_cast<uint32_t>(Util::GetTimeDifferenceToNow(startTime)));
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
    //                                         0           1         2
    auto result = WorldDatabase.Query("SELECT entry, language_code, text FROM locales_worldmap_info");
    if (result == nullptr)
    {
        sLogger.info("MySQLDataLoads : Table `locales_worldmap_info` is empty!");
        return;
    }

    sLogger.info("MySQLDataLoads : Table `locales_worldmap_info` has {} columns", result->GetFieldCount());

    _localesWorldmapInfoStore.rehash(result->GetRowCount());

    uint32_t load_count = 0;
    uint32_t i = 0;
    do
    {
        ++i;
        Field* fields = result->Fetch();

        MySQLStructure::LocalesWorldmapInfo& localWorldmapInfo = _localesWorldmapInfoStore[i];

        localWorldmapInfo.entry = fields[0].asUint32();
        std::string locString = fields[1].asCString();
        localWorldmapInfo.languageCode = Util::getLanguagesIdFromString(locString);
        localWorldmapInfo.text = strdup(fields[2].asCString());

        ++load_count;

    } while (result->NextRow());

    sLogger.info("MySQLDataLoads : Loaded {} rows from `locales_worldmap_info` table in {} ms!", load_count, static_cast<uint32_t>(Util::GetTimeDifferenceToNow(startTime)));
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
    //                                         0           1         2
    auto result = WorldDatabase.Query("SELECT entry, language_code, text FROM locales_worldstring_table");
    if (result == nullptr)
    {
        sLogger.info("MySQLDataLoads : Table `locales_worldstring_table` is empty!");
        return;
    }

    sLogger.info("MySQLDataLoads : Table `locales_worldstring_table` has {} columns", result->GetFieldCount());

    _localesWorldStringTableStore.rehash(result->GetRowCount());

    uint32_t load_count = 0;
    uint32_t i = 0;
    do
    {
        ++i;
        Field* fields = result->Fetch();

        MySQLStructure::LocalesWorldStringTable& localWorldStringTable = _localesWorldStringTableStore[i];

        localWorldStringTable.entry = fields[0].asUint32();
        std::string locString = fields[1].asCString();
        localWorldStringTable.languageCode = Util::getLanguagesIdFromString(locString);
        localWorldStringTable.text = strdup(fields[2].asCString());

        ++load_count;

    } while (result->NextRow());

    sLogger.info("MySQLDataLoads : Loaded {} rows from `locales_worldstring_table` table in {} ms!", load_count, static_cast<uint32_t>(Util::GetTimeDifferenceToNow(startTime)));
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
//    //                                          0      1
//    auto result = WorldDatabase.Query("SELECT entry, spell FROM petdefaultspells");
//    if (result == nullptr)
//    {
//        sLogger.info("MySQLDataLoads : Table `petdefaultspells` is empty!");
//        return;
//    }
//
//    sLogger.info("MySQLDataLoads : Table `petdefaultspells` has {} columns", result->GetFieldCount());
//
//    uint32_t load_count = 0;
//    do
//    {
//        Field* fields = result->Fetch();
//        uint32_t entry = fields[0].GetUInt32();
//        uint32_t spell = fields[1].GetUInt32();
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
//    sLogger.info("MySQLDataLoads : Loaded {} rows from `petdefaultspells` table in {} ms!", load_count, static_cast<uint32_t>(Util::GetTimeDifferenceToNow(startTime)));
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
    //                                           0           1              2          3
    auto result = WorldDatabase.Query("SELECT SpellId, SpellToDiscover, SkillValue, Chance FROM professiondiscoveries");
    if (result == nullptr)
    {
        sLogger.info("MySQLDataLoads : Table `professiondiscoveries` is empty!");
        return;
    }

    sLogger.info("MySQLDataLoads : Table `professiondiscoveries` has {} columns", result->GetFieldCount());

    uint32_t load_count = 0;
    do
    {
        Field* fields = result->Fetch();
        auto* professionDiscovery = _professionDiscoveryStore.emplace(std::make_unique<MySQLStructure::ProfessionDiscovery>()).first->get();
        professionDiscovery->SpellId = fields[0].asUint32();
        professionDiscovery->SpellToDiscover = fields[1].asUint32();
        professionDiscovery->SkillValue = fields[2].asUint32();
        professionDiscovery->Chance = fields[3].asFloat();

        ++load_count;

    } while (result->NextRow());

    sLogger.info("MySQLDataLoads : Loaded {} rows from `professiondiscoveries` table in {} ms!", load_count, static_cast<uint32_t>(Util::GetTimeDifferenceToNow(startTime)));
}

void MySQLDataStore::loadTransportDataTable()
{
    auto startTime = Util::TimeNow();
    //                                          0      1
    auto result = WorldDatabase.Query("SELECT entry, name FROM transport_data WHERE min_build <= %u AND max_build >= %u", getAEVersion(), getAEVersion());
    if (result == nullptr)
    {
        sLogger.info("MySQLDataLoads : Table `transport_data` is empty!");
        return;
    }

    sLogger.info("MySQLDataLoads : Table `transport_data` has {} columns", result->GetFieldCount());

    uint32_t load_count = 0;
    do
    {
        Field* fields = result->Fetch();
        uint32_t entry = fields[0].asUint32();

        GameObjectProperties const* gameobject_info = sMySQLStore.getGameObjectProperties(entry);
        if (gameobject_info == nullptr)
        {
            sLogger.failure("Transport entry: {}, will not be loaded, gameobject_properties missing", entry);
            continue;
        }

        if (gameobject_info->type != GAMEOBJECT_TYPE_MO_TRANSPORT)
        {
            sLogger.failure("Transport entry: {}, will not be loaded, gameobject_properties type wrong", entry);
            continue;
        }

        MySQLStructure::TransportData& transportData = _transportDataStore[entry];
        transportData.entry = entry;
        transportData.name = fields[1].asCString();

        ++load_count;

    } while (result->NextRow());

    sLogger.info("MySQLDataLoads : Loaded {} rows from `transport_data` table in {} ms!", load_count, static_cast<uint32_t>(Util::GetTimeDifferenceToNow(startTime)));
}

void MySQLDataStore::loadTransportEntrys()
{
    auto startTime = Util::TimeNow();
    //                                                  
    auto result = WorldDatabase.Query("SELECT entry FROM gameobject_properties WHERE type = 15 AND  build <= %u ORDER BY entry ASC", getAEVersion());
    if (result == nullptr)
    {
        sLogger.info("MySQLDataLoads : Loaded 0 transport templates. DB table `gameobject_properties` has no transports!");
        return;
    }

    uint32_t load_count = 0;
    do
    {
        Field* fields = result->Fetch();
        uint32_t entry = fields[0].asUint32();

        MySQLStructure::TransportEntrys& transportEntrys = _transportEntryStore[entry];
        transportEntrys.entry = entry;

        ++load_count;

    } while (result->NextRow());

    sLogger.info("MySQLDataLoads : Loaded {} rows from `transport_entrys` table in {} ms!", load_count, static_cast<uint32_t>(Util::GetTimeDifferenceToNow(startTime)));
}

void MySQLDataStore::loadTransportMaps()
{
    auto startTime = Util::TimeNow();
    //                                                  
    auto result = WorldDatabase.Query("SELECT parameter_6 FROM gameobject_properties WHERE type = 15 AND  build <= %u ORDER BY entry ASC", getAEVersion());
    if (result == nullptr)
    {
        sLogger.info("MySQLDataLoads : Loaded 0 transport maps. DB table `gameobject_properties` has no transports!");
        return;
    }

    uint32_t load_count = 0;
    do
    {
        Field* fields = result->Fetch();
        uint32_t mapId = fields[0].asUint32();

        _transportMapStore.push_back(mapId);

        ++load_count;

    } while (result->NextRow());

    sLogger.info("MySQLDataLoads : Loaded {} maps from `transport_maps` table in {} ms!", load_count, static_cast<uint32_t>(Util::GetTimeDifferenceToNow(startTime)));
}

void MySQLDataStore::loadGossipMenuItemsTable()
{
    auto startTime = Util::TimeNow();

    //                                             0          1
    auto result = WorldDatabase.Query("SELECT gossip_menu, text_id FROM gossip_menu ORDER BY gossip_menu");
    if (result == nullptr)
    {
        sLogger.info("MySQLDataLoads : Table `gossip_menu` is empty!");
        return;
    }

    sLogger.info("MySQLDataLoads : Table `gossip_menu` has {} columns", result->GetFieldCount());

    uint32_t load_count = 0;
    do
    {
        Field* fields = result->Fetch();
        uint32_t entry = fields[0].asUint32();

        MySQLStructure::GossipMenuInit& gMenuItem = _gossipMenuInitStore[entry];
        gMenuItem.gossipMenu = entry;
        gMenuItem.textId = fields[1].asUint32();

        ++load_count;
    } while (result->NextRow());

    sLogger.info("MySQLDataLoads : Loaded {} rows from `gossip_menu` table in {} ms!", load_count, static_cast<uint32_t>(Util::GetTimeDifferenceToNow(startTime)));

    _gossipMenuItemsStores.clear();

    //                                             0       1            2        3            4                5               6               7                 8                9                10                11                 12
    auto resultItems = WorldDatabase.Query("SELECT id, item_order, menu_option, icon, on_choose_action, on_choose_data, on_choose_data2, on_choose_data3, on_choose_data4, next_gossip_menu, next_gossip_text, requirement_type, requirement_data FROM gossip_menu_items ORDER BY id, item_order");
    if (resultItems == nullptr)
    {
        sLogger.info("MySQLDataLoads : Table `gossip_menu_items` is empty!");
        return;
    }

    sLogger.info("MySQLDataLoads : Table `gossip_menu_items` has {} columns", resultItems->GetFieldCount());

    load_count = 0;
    do
    {
        Field* fields = resultItems->Fetch();

        MySQLStructure::GossipMenuItems gMenuItem;

        gMenuItem.gossipMenu = fields[0].asUint32();
        gMenuItem.itemOrder = fields[1].asUint32();
        gMenuItem.menuOptionText = fields[2].asUint32();
        gMenuItem.icon = fields[3].asUint8();
        gMenuItem.onChooseAction = fields[4].asUint8();
        gMenuItem.onChooseData = fields[5].asUint32();
        gMenuItem.onChooseData2 = fields[6].asUint32();
        gMenuItem.onChooseData3 = fields[7].asUint32();
        gMenuItem.onChooseData4 = fields[8].asUint32();
        gMenuItem.nextGossipMenu = fields[9].asUint32();
        gMenuItem.nextGossipMenuText = fields[10].asUint32();
        gMenuItem.requirementType = fields[11].asUint8();
        gMenuItem.requirementData = fields[12].asUint32();

        _gossipMenuItemsStores.emplace(GossipMenuItemsContainer::value_type(gMenuItem.gossipMenu, gMenuItem));
        ++load_count;
    } while (resultItems->NextRow());

    sLogger.info("MySQLDataLoads : Loaded {} rows from `gossip_menu_items` table in {} ms!", load_count, static_cast<uint32_t>(Util::GetTimeDifferenceToNow(startTime)));
}

void MySQLDataStore::loadCreatureSpawns()
{
    auto startTime = Util::TimeNow();
    uint32_t count = 0;

    auto creature_spawn_result = getWorldDBQuery("SELECT * FROM creature_spawns WHERE min_build <= %u AND max_build >= %u AND event_entry = 0", getAEVersion(), getAEVersion());
    if (creature_spawn_result)
    {
        uint32_t creature_spawn_fields = creature_spawn_result->GetFieldCount();
        if (creature_spawn_fields != CREATURE_SPAWNS_FIELDCOUNT + 1) // + 1 for additional table loading 'origin'
        {
            sLogger.failure("Table `creature_spawns` has {} columns, but needs {} columns! Skipped!", creature_spawn_fields, CREATURE_SPAWNS_FIELDCOUNT);
            return;
        }
        else
        {
            do
            {
                Field* fields = creature_spawn_result->Fetch();
                MySQLStructure::CreatureSpawn* cspawn = new MySQLStructure::CreatureSpawn;
                cspawn->id = fields[0].asUint32();

                uint32_t creature_entry = fields[3].asUint32();
                auto creature_properties = sMySQLStore.getCreatureProperties(creature_entry);
                if (creature_properties == nullptr)
                {
                    sLogger.debugFlag(AscEmu::Logging::LF_DB_TABLES, "Creature spawn ID: {} has invalid entry: {} which is not in creature_properties table! Skipped loading.", cspawn->id, creature_entry);
                    delete cspawn;
                    continue;
                }

                cspawn->entry = creature_entry;
                cspawn->mapId = fields[4].asUint32();
                cspawn->x = fields[5].asFloat();
                cspawn->y = fields[6].asFloat();
                cspawn->z = fields[7].asFloat();
                cspawn->o = fields[8].asFloat();
                cspawn->movetype = fields[9].asUint8();
                cspawn->displayid = fields[10].asUint32();
                if (cspawn->displayid != 0 && !creature_properties->isTriggerNpc)
                {
                    const auto* creature_display = sObjectMgr.getCreatureDisplayInfoData(cspawn->displayid);
                    if (!creature_display)
                    {
                        sLogger.debugFlag(AscEmu::Logging::LF_DB_TABLES, "Table creature_spawns includes invalid displayid {} for npc entry: {}, spawn_id: {}. Set to a random modelid!", cspawn->displayid, cspawn->entry, cspawn->id);
                        cspawn->displayid = creature_properties->getRandomModelId();
                    }
                }
                else
                {
                    cspawn->displayid = creature_properties->getRandomModelId();
                }

                cspawn->factionid = fields[11].asUint32();
                cspawn->flags = fields[12].asUint32();
                cspawn->pvp_flagged = fields[13].asUint8();
                cspawn->bytes0 = fields[14].asUint32();
                cspawn->emote_state = fields[15].asUint32();
                //cspawn->respawnNpcLink = fields[16].GetUInt32();
                cspawn->channel_spell = fields[17].asUint32();
                cspawn->channel_target_go = fields[18].asUint32();
                cspawn->channel_target_creature = fields[19].asUint32();
                cspawn->stand_state = fields[20].asUint8();
                cspawn->death_state = fields[21].asUint32();
                cspawn->MountedDisplayID = fields[22].asUint32();
                cspawn->sheath_state = fields[23].asUint8();

                cspawn->Item1SlotEntry = fields[24].asUint32();
                cspawn->Item2SlotEntry = fields[25].asUint32();
                cspawn->Item3SlotEntry = fields[26].asUint32();

                cspawn->CanFly = fields[27].asUint32();

                cspawn->phase = fields[28].asUint32();
                if (cspawn->phase == 0)
                    cspawn->phase = 0xFFFFFFFF;

                cspawn->wander_distance = fields[30].asUint32();
                cspawn->waypoint_id = fields[31].asUint32();

                cspawn->origine = fields[32].asCString();

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
    }

    sLogger.info("MySQLDataLoads : Loaded {} rows from `creature_spawns` table in {} ms!", count, static_cast<uint32_t>(Util::GetTimeDifferenceToNow(startTime)));
}

void MySQLDataStore::loadGameobjectSpawns()
{
    auto startTime = Util::TimeNow();
    uint32_t count = 0;

    auto gobject_spawn_result = getWorldDBQuery("SELECT * FROM gameobject_spawns WHERE min_build <= %u AND max_build >= %u AND event_entry = 0", VERSION_STRING, VERSION_STRING);
    if (gobject_spawn_result)
    {
        uint32_t gobject_spawn_fields = gobject_spawn_result->GetFieldCount();
        if (gobject_spawn_fields != GO_SPAWNS_FIELDCOUNT + 1) // + 1 for additional table loading 'origin'
        {
            sLogger.failure("Table `gameobject_spawns` has {} columns, but needs {} columns! Skipped!", gobject_spawn_fields, GO_SPAWNS_FIELDCOUNT);
            return;
        }
        else
        {
            do
            {
                Field* fields = gobject_spawn_result->Fetch();
                uint32_t spawnId = fields[0].asUint32();
                uint32_t gameobject_entry = fields[3].asUint32();
                
                auto gameobject_info = sMySQLStore.getGameObjectProperties(gameobject_entry);
                if (gameobject_info == nullptr)
                {
                    sLogger.debugFlag(AscEmu::Logging::LF_DB_TABLES, "Gameobject spawn ID: {} has invalid entry: {} which is not in gameobject_properties table! Skipped loading.", spawnId, gameobject_entry);
                    continue;
                }

                MySQLStructure::GameobjectSpawn* go_spawn = new MySQLStructure::GameobjectSpawn;
                go_spawn->id = spawnId;
                go_spawn->entry = gameobject_entry;
                go_spawn->map = fields[4].asUint32();
                go_spawn->phase = fields[5].asUint32();
                go_spawn->spawnPoint = LocationVector(fields[6].asFloat(), fields[7].asFloat(), fields[8].asFloat(), fields[9].asFloat());
                go_spawn->rotation.x = fields[10].asFloat();
                go_spawn->rotation.y = fields[11].asFloat();
                go_spawn->rotation.z = fields[12].asFloat();
                go_spawn->rotation.w = fields[13].asFloat();
                go_spawn->spawntimesecs = fields[14].asUint32();
                go_spawn->state = GameObject_State(fields[15].asUint32());
                //event_entry = 16
                go_spawn->origine = fields[17].asCString();

                if (go_spawn->phase == 0)
                    go_spawn->phase = 0xFFFFFFFF;

                _gameobjectSpawnsStore[go_spawn->map].push_back(go_spawn);
                ++count;
            } while (gobject_spawn_result->NextRow());
        }
    }

    sLogger.info("MySQLDataLoads : Loaded {} rows from `gameobject_spawns` table in {} ms!", count, static_cast<uint32_t>(Util::GetTimeDifferenceToNow(startTime)));
}

void MySQLDataStore::loadRecallTable()
{
    auto startTime = Util::TimeNow();
    uint32_t count = 0;

    _recallStore.clear();

    auto recall_result = getWorldDBQuery("SELECT id, name, MapId, positionX, positionY, positionZ, Orientation FROM recall WHERE min_build <= %u AND max_build >= %u", VERSION_STRING, VERSION_STRING);
    if (recall_result)
    {
        do
        {
            Field* fields = recall_result->Fetch();
            const auto& teleCoords = _recallStore.emplace_back(std::make_unique<MySQLStructure::RecallStruct>());

            teleCoords->name = fields[1].asCString();
            teleCoords->mapId = fields[2].asUint32();
            teleCoords->location.x = fields[3].asFloat();
            teleCoords->location.y = fields[4].asFloat();
            teleCoords->location.z = fields[5].asFloat();
            teleCoords->location.o = fields[6].asFloat();

            ++count;
        } while (recall_result->NextRow());
    }

    sLogger.info("MySQLDataLoads : Loaded {} rows from `recall` table in {} ms!", count, static_cast<uint32_t>(Util::GetTimeDifferenceToNow(startTime)));
}

void MySQLDataStore::loadCreatureAIScriptsTable()
{
    auto startTime = Util::TimeNow();

    _creatureAIScriptStore.clear();

    auto result = WorldDatabase.Query("SELECT * FROM creature_ai_scripts WHERE min_build <= %u AND max_build >= %u ORDER BY entry, event", VERSION_STRING, VERSION_STRING);
    if (result == nullptr)
    {
        sLogger.info("MySQLDataLoads : Table `creature_ai_scripts` is empty!");
        return;
    }

    sLogger.info("MySQLDataLoads : Table `creature_ai_scripts` has {} columns", result->GetFieldCount());

    uint32_t load_count = 0;
    do
    {
        Field* fields = result->Fetch();

        uint32_t creature_entry = fields[2].asUint32();
        uint32_t spellId = fields[9].asUint32();
        uint32_t textId = fields[17].asUint32();

        if (getCreatureProperties(creature_entry) == nullptr)
        {
            sLogger.debugFlag(AscEmu::Logging::LF_DB_TABLES, "Table `creature_ai_scripts` includes invalid creature entry {} <skipped>", creature_entry);
            continue;
        }

        if (spellId != 0 && sSpellMgr.getSpellInfo(spellId) == nullptr)
        {
            sLogger.debugFlag(AscEmu::Logging::LF_DB_TABLES, "Table `creature_ai_scripts` includes invalid spellId for creature entry {} <skipped>", spellId, creature_entry);
            continue;
        }

        if (textId != 0 && sMySQLStore.getNpcScriptText(textId) == nullptr)
        {
            sLogger.debugFlag(AscEmu::Logging::LF_DB_TABLES, "Table `creature_ai_scripts` includes invalid textId for creature entry {} <skipped>", textId, creature_entry);
            continue;
        }

        const auto& ai_script = _creatureAIScriptStore.emplace(creature_entry, std::make_unique<MySQLStructure::CreatureAIScripts>())->second;
        ai_script->entry = creature_entry;
        ai_script->difficulty = fields[3].asUint8();
        ai_script->phase = fields[4].asUint8();
        ai_script->event = fields[5].asUint8();
        ai_script->action = fields[6].asUint8();
        ai_script->maxCount = fields[7].asUint8();
        ai_script->chance = fields[8].asFloat();
        ai_script->spellId = spellId;
        ai_script->spell_type = fields[10].asUint8();
        ai_script->triggered = fields[11].asBool();
        ai_script->target = fields[12].asUint8();
        ai_script->cooldownMin = fields[13].asUint32();
        ai_script->cooldownMax = fields[14].asUint32();
        ai_script->minHealth = fields[15].asFloat();
        ai_script->maxHealth = fields[16].asFloat();
        ai_script->textId = textId;
        ai_script->misc1 = fields[18].asUint32();

        ++load_count;
    } while (result->NextRow());

    sLogger.info("MySQLDataLoads : Loaded {} rows from `creature_ai_scripts` table in {} ms!", load_count, static_cast<uint32_t>(Util::GetTimeDifferenceToNow(startTime)));
}

std::unique_ptr<std::vector<MySQLStructure::CreatureAIScripts>> MySQLDataStore::getCreatureAiScripts(uint32_t entry)
{
    auto result = std::make_unique<std::vector<MySQLStructure::CreatureAIScripts>>();

    result->clear();

    for (const auto& itr : _creatureAIScriptStore)
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

    auto result = WorldDatabase.Query("SELECT * FROM spawn_group_id ORDER BY groupId");
    if (result == nullptr)
    {
        sLogger.info("MySQLDataLoads : Table `spawn_group_id` is empty!");
        return;
    }

    sLogger.info("MySQLDataLoads : Table `spawn_group_id` has {} columns", result->GetFieldCount());

    uint32_t load_count = 0;
    do
    {
        Field* fields = result->Fetch();
        uint32_t groupId = fields[0].asUint8();

        SpawnGroupTemplateData& spawnGroup = _spawnGroupDataStore[groupId];

        spawnGroup.groupId = groupId;
        spawnGroup.name = fields[1].asCString();
        spawnGroup.mapId = 0xFFFFFFFF;
        uint32_t flags = fields[2].asUint8();
        if (flags & ~SPAWNGROUP_FLAGS_ALL)
        {
            flags &= SPAWNGROUP_FLAGS_ALL;
            sLogger.failure("Invalid spawn group flag {} on group ID {} ({}), reduced to valid flag {}.", flags, groupId, spawnGroup.name, uint32_t(spawnGroup.groupFlags));
        }
        if (flags & SPAWNGROUP_FLAG_SYSTEM && flags & SPAWNGROUP_FLAG_MANUAL_SPAWN)
        {
            flags &= ~SPAWNGROUP_FLAG_MANUAL_SPAWN;
            sLogger.failure("System spawn group {} ({}) has invalid manual spawn flag. Ignored.", groupId, spawnGroup.name);
        }
        spawnGroup.groupFlags = SpawnGroupFlags(flags);
        spawnGroup.spawnFlags = SpawnFlags(fields[3].asUint8());
        spawnGroup.bossId = fields[4].asUint32();

        ++load_count;
    } while (result->NextRow());

    sLogger.info("MySQLDataLoads : Loaded {} rows from `spawn_group_id` table in {} ms!", load_count, static_cast<uint32_t>(Util::GetTimeDifferenceToNow(startTime)));
}

void MySQLDataStore::loadCreatureGroupSpawns()
{
    auto startTime = Util::TimeNow();

    _spawnGroupMapStore.clear();

    auto result = WorldDatabase.Query("SELECT * FROM creature_group_spawn ORDER BY groupId");
    if (result == nullptr)
    {
        sLogger.info("MySQLDataLoads : Table `creature_group_spawn` is empty!");
        return;
    }

    sLogger.info("MySQLDataLoads : Table `creature_group_spawn` has {} columns", result->GetFieldCount());

    uint32_t load_count = 0;
    do
    {
        Field* fields = result->Fetch();
        uint32_t groupId = fields[0].asUint8();
        uint32_t spawnId = fields[1].asUint32();
        bool data = false;

        auto it = _spawnGroupDataStore.find(groupId);
        if (it == _spawnGroupDataStore.end())
        {
            sLogger.failure("Spawn group {} assigned to spawn ID ({}), but group does not exist!", groupId, spawnId);
            continue;
        }

        for (const auto& creatureSpawnMap : sMySQLStore._creatureSpawnsStore)
        {
            for (const auto& creatureSpawn : creatureSpawnMap)
            {
                if (creatureSpawn->id == spawnId)
                {
                    data = true;

                    SpawnGroupTemplateData& groupTemplate = it->second;
                    if (groupTemplate.mapId == 0xFFFFFFFF)
                        groupTemplate.mapId = creatureSpawn->mapId;

                    else if (groupTemplate.mapId != creatureSpawn->mapId && !(groupTemplate.groupFlags & SPAWNGROUP_FLAG_SYSTEM))
                    {
                        sLogger.debugFlag(AscEmu::Logging::LF_DB_TABLES, "Spawn group {} has map ID {}, but spawn ({}) has map id {} - spawn NOT added to group!", groupId, groupTemplate.mapId, spawnId, creatureSpawn->mapId);
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
            sLogger.debugFlag(AscEmu::Logging::LF_DB_TABLES, "Spawn data with ID ({}) not found, but is listed as a member of spawn group {}!", spawnId, groupId);
            continue;
        } 
    } while (result->NextRow());

    sLogger.info("MySQLDataLoads : Loaded {} rows from `creature_group_spawn` table in {} ms!", load_count, static_cast<uint32_t>(Util::GetTimeDifferenceToNow(startTime)));
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

void MySQLDataStore::loadCreatureSplineChains()
{
    auto startTime = Util::TimeNow();

    _splineChainsStore.clear();

    auto resultMeta = WorldDatabase.Query("SELECT entry, chainId, splineId, expectedDuration, msUntilNext, velocity FROM script_spline_chain_meta ORDER BY entry asc, chainId asc, splineId asc");
    if (resultMeta == nullptr)
    {
        sLogger.info("MySQLDataLoads : Table `script_spline_chain_meta` is empty!");
        return;
    }

    auto resultWp = WorldDatabase.Query("SELECT entry, chainId, splineId, wpId, x, y, z FROM script_spline_chain_waypoints ORDER BY entry asc, chainId asc, splineId asc, wpId asc");
    if (resultMeta == nullptr)
    {
        sLogger.info("MySQLDataLoads : Table `script_spline_chain_waypoints` is empty!");
        return;
    }

    uint32_t chainCount = 0;
    uint32_t splineCount = 0;
    uint32_t wpCount = 0;
    do
    {
        Field* fieldsMeta = resultMeta->Fetch();
        uint32_t entry = fieldsMeta[0].asUint32();
        uint16_t chainId = fieldsMeta[1].asUint16();
        uint8_t splineId = fieldsMeta[2].asUint8();
        std::vector<SplineChainLink>& chain = _splineChainsStore[{entry, chainId}];

        if (splineId != chain.size())
        {
            sLogger.warning("Creature #{}: Chain {} has orphaned spline {}, skipped.", entry, chainId, splineId);
            continue;
        }

        uint32_t expectedDuration = fieldsMeta[3].asUint32();
        uint32_t msUntilNext = fieldsMeta[4].asUint32();
        float velocity = fieldsMeta[5].asFloat();
        chain.emplace_back(expectedDuration, msUntilNext, velocity);

        if (splineId == 0)
            ++chainCount;
        ++splineCount;
    } while (resultMeta->NextRow());

    do
    {
        Field* fieldsWP = resultWp->Fetch();
        uint32_t entry = fieldsWP[0].asUint32();
        uint16_t chainId = fieldsWP[1].asUint16();
        uint8_t splineId = fieldsWP[2].asUint8(), wpId = fieldsWP[3].asUint8();
        float posX = fieldsWP[4].asFloat(), posY = fieldsWP[5].asFloat(), posZ = fieldsWP[6].asFloat();
        auto it = _splineChainsStore.find({ entry,chainId });
        if (it == _splineChainsStore.end())
        {
            sLogger.warning("Creature #{} has waypoint data for spline chain {}. No such chain exists - entry skipped.", entry, chainId);
            continue;
        }
        std::vector<SplineChainLink>& chain = it->second;
        if (splineId >= chain.size())
        {
            sLogger.warning("Creature #{} has waypoint data for spline ({},{}). The specified chain does not have a spline with this index - entry skipped.", entry, chainId, splineId);
            continue;
        }
        SplineChainLink& spline = chain[splineId];
        if (wpId != spline.Points.size())
        {
            sLogger.warning("Creature #{} has orphaned waypoint data in spline ({},{}) at index {}. Skipped.", entry, chainId, splineId, wpId);
            continue;
        }
        spline.Points.emplace_back(posX, posY, posZ);
        ++wpCount;
    } while (resultWp->NextRow());

    sLogger.info("MySQLDataLoads : Loaded spline chain data for {} chains, consisting of {} splines with {} waypoints in {} ms!", chainCount, splineCount, wpCount, static_cast<uint32_t>(Util::GetTimeDifferenceToNow(startTime)));
}

std::vector<SplineChainLink> const* MySQLDataStore::getSplineChain(uint32_t entry, uint16_t chainId) const
{
    auto it = _splineChainsStore.find({ entry, chainId });
    if (it == _splineChainsStore.end())
        return nullptr;

    return &it->second;
}

std::vector<SplineChainLink> const* MySQLDataStore::getSplineChain(Creature const* pCreature, uint16_t id) const
{
    return getSplineChain(pCreature->getEntry(), id);
}
