/*
Copyright (c) 2014-2017 AscEmu Team <http://www.ascemu.org/>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

namespace MySQLStructure
{
    //////////////////////////////////////////////////////////////////////////////////////////
    /* WORLD DB Structures
    All table names
    SELECT TABLE_NAME
    FROM INFORMATION_SCHEMA.TABLES
    WHERE TABLE_TYPE = 'BASE TABLE' AND TABLE_SCHEMA='asc_world';
    */

    //achievement_reward

    //ai_agents
    //ai_threattospellid

    //areatriggers

    //auctionhouse

    //\TODO table banned_phrases not used
    //banned_phrases

    //battlemasters

    //creature_difficulty
    //creature_formations
    //creature_initial_equip
    //creature_properties
    //creature_quest_finisher
    //creature_quest_starter
    //creature_spawns
    //creature_staticspawns
    //creature_timed_emotes
    //creature_waypoints
    //creature_waypoints_manual
    //display_bounding_boxes

    //event_creature_spawns
    //event_gameobject_spawns
    //event_properties

    //event_scripts

    //fishing

    //gameobject_properties
    //gameobject_quest_finisher
    //gameobject_quest_item_binding
    //gameobject_quest_pickup_binding
    //gameobject_quest_starter
    //gameobject_spawns
    //gameobject_staticspawns
    //gameobject_teleports

    //gossip_menu_option
    struct GossipMenuOption
    {
        uint32_t id;
        std::string text;
    };

    //graveyards

    //instance_bosses

    //item_pages
    struct ItemPage
    {
        uint32_t id;
        std::string text;
        uint32_t nextPage;
    };

    //item_properties
    //item_quest_association
    //item_randomprop_groups
    //item_randomsuffix_groups
    //itemset_linked_itemsetbonus

    //lfg_dungeon_rewards

    //locales_creature
    //locales_gameobject
    //locales_gossip_menu_option
    //locales_item
    //locales_item_pages
    //locales_npc_monstersay
    //locales_npc_script_text
    //locales_npc_text
    //locales_quest
    //locales_worldbroadcast
    //locales_worldmap_info
    //locales_worldstring_table

    //loot_creatures
    //loot_fishing
    //loot_gameobjects
    //loot_items
    //loot_pickpocketing
    //loot_skinning

    //\TODO table map_checkpoint not used
    //map_checkpoint

    //npc_gossip_textid

    //npc_monstersay
    //npc_script_text
    //npc_text

    //pet_level_abilities

    //petdefaultspells

    //player_xp_for_level
    //playercreateinfo
    //playercreateinfo_bars
    //playercreateinfo_items
    //playercreateinfo_skills
    //playercreateinfo_spells

    //points_of_interest

    //professiondiscoveries

    //quest_poi
    //quest_poi_points
    //quest_properties

    //\TODO table recall called by execute recall commands. Load it on startup.
    //recall

    //reputation_creature_onkill
    //reputation_faction_onkill
    //reputation_instance_onkill

    //spell_area
    //spell_coef_flags
    //spell_coef_override
    //spell_custom_assign
    //spell_disable
    //spell_disable_trainers
    //spell_effects_override
    //spell_proc
    //spell_ranks
    //spell_teleport_coords
    // Defined in Spells/TeleportCoords.h struct TeleportCoords

    //spellclickspells
    //spelloverride
    //spelltargetconstraints
    //totemdisplayids

    //trainer_defs
    //trainer_spells
    //trainerspelloverride

    //transport_creatures
    //transport_data

    //vehicle_accessories

    //vendor_restrictions
    //vendors

    //weather
    //weather_season

    //\brief loaded on server startup. Related Class "WordFilter"
    //\TODO switch to std::string in class WordFilter.
    // Rewrite WordFilter, split character name and chat word filter in two seperated functions/classes.
    //wordfilter_character_names
    struct WordFilterCharacterNames
    {
        std::string name;
        std::string nameReplace;
    };

    //wordfilter_chat
    struct WordFilterChat
    {
        std::string word;
        std::string wordReplace;
        bool blockMessage;
    };

    //\brief loaded on server startup. Not needed after server startup
    //world_db_version

    //\TODO table world_instance_entrance not used
    //world_instance_entrance

    //worldbroadcast
    struct WorldBroadCast
    {
        uint32_t id;
        uint32_t interval;
        uint32_t randomInterval;
        uint32_t nextUpdate;        // serverside not in sql table
        std::string text;
    };

    //worldmap_info

    //worldstate_templates

    //worldstring_tables

    //\brief Data used in AIInterface.cpp (summoned id in function FindFriends)
    //zoneguards
    struct ZoneGuards
    {
        uint32_t zoneId;
        uint32_t hordeEntry;
        uint32_t allianceEntry;
    };
    

    //////////////////////////////////////////////////////////////////////////////////////////
    /* CHARACTERS DB Structures
    All table names
    SELECT TABLE_NAME
    FROM INFORMATION_SCHEMA.TABLES
    WHERE TABLE_TYPE = 'BASE TABLE' AND TABLE_SCHEMA='asc_char';
    */

    //account_data
    //account_forced_permissions
    //arenateams
    //auctions
    //banned_char_log
    //banned_names
    //calendar_events
    //calendar_invites
    //character_achievement
    //character_achievement_progress
    //character_db_version
    //characters
    //charters
    //clientaddons
    //command_overrides
    //corpses
    //equipmentsets
    //event_save
    //gm_survey
    //gm_survey_answers
    //gm_tickets
    //groups
    //guild_bankitems
    //guild_banklogs
    //guild_banktabs
    //guild_data
    //guild_logs
    //guild_ranks
    //guilds
    //instanceids
    //instances
    //lag_reports
    //lfg_data
    //mailbox
    //playerbugreports
    //playercooldowns
    //playerdeletedspells
    //playeritems
    //playerpets
    //playerpetspells
    //playerreputations
    //playerskills
    //playerspells
    //playersummons
    //playersummonspells
    //questlog
    //server_settings
    //social_friends
    //social_ignores
    //tutorials

}
