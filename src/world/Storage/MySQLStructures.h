/*
Copyright (c) 2014-2017 AscEmu Team <http://www.ascemu.org/>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

// related to table areatriggers
enum AreaTriggerType
{
    ATTYPE_NULL = 0,
    ATTYPE_INSTANCE = 1,
    ATTYPE_QUESTTRIGGER = 2,
    ATTYPE_INN = 3,
    ATTYPE_TELEPORT = 4,
    ATTYPE_SPELL = 5,
    ATTYPE_BATTLEGROUND = 6
};

// related to table worldmap_info
enum WorldMapInfoFlag
{
    WMI_INSTANCE_ENABLED = 0x001,
    WMI_INSTANCE_WELCOME = 0x002,
    WMI_INSTANCE_ARENA = 0x004,
    WMI_INSTANCE_XPACK_01 = 0x008, // TBC
    WMI_INSTANCE_XPACK_02 = 0x010, // WotLK
    WMI_INSTANCE_HAS_NORMAL_10MEN = 0x020,
    WMI_INSTANCE_HAS_NORMAL_25MEN = 0x040,
    WMI_INSTANCE_HAS_HEROIC_10MEN = 0x080,
    WMI_INSTANCE_HAS_HEROIC_25MEN = 0x100
};

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
    struct AreaTrigger
    {
        uint32_t id;
        uint8_t type;
        uint32_t mapId;
        uint32_t pendingScreen;
        std::string name;
        float x;
        float y;
        float z;
        float o;
        uint32_t requiredHonorRank;
        uint32_t requiredLevel;
    };

    //auctionhouse

    //battlemasters

    //creature_difficulty
    struct CreatureDifficulty
    {
        uint32_t id;
        uint32_t difficultyEntry1;
        uint32_t difficultyEntry2;
        uint32_t difficultyEntry3;
    };

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

    //\TODO rename table to fishing_zones
    //fishing
    struct FishingZones
    {
        uint32_t zoneId;
        uint32_t minSkill;
        uint32_t maxSkill;
    };

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

    //npc_gossip_textid

    //npc_monstersay
    //npc_script_text
    //npc_text

    //pet_level_abilities
    struct PetLevelAbilities
    {
        uint32_t level;
        uint32_t health;
        uint32_t armor;
        uint32_t strength;
        uint32_t agility;
        uint32_t stamina;
        uint32_t intellect;
        uint32_t spirit;
    };

    //petdefaultspells

    //player_xp_for_level
    //playercreateinfo
    //playercreateinfo_bars
    //playercreateinfo_items
    //playercreateinfo_skills
    //playercreateinfo_spells

    //points_of_interest
    struct PointsOfInterest
    {
        uint32_t id;
        float x;
        float y;
        uint32_t icon;
        uint32_t flags;
        uint32_t data;
        std::string iconName;
    };

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
    struct TotemDisplayIds
    {
        uint32_t displayId;
        uint32_t draeneiId;
        uint32_t trollId;
        uint32_t orcId;
#if VERSION_STRING == Cata
        uint32_t taurenId;
        uint32_t dwarfId;
        uint32_t goblinId;
#endif
    };

    //trainer_defs
    //trainer_spells
    //trainerspelloverride

    //transport_creatures
    //transport_data

    //vehicle_accessories

    //vendor_restrictions
    struct VendorRestrictions
    {
        uint32_t entry;
        int32_t racemask;
        int32_t classmask;
        uint32_t reqrepfaction;
        uint32_t reqrepvalue;
        uint32_t canbuyattextid;
        uint32_t cannotbuyattextid;
        uint32_t flags;
    };

    //vendors

    //weather

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
    struct MapInfo
    {
        uint32_t mapid;
        uint32_t screenid;
        uint32_t type;
        uint32_t playerlimit;
        uint32_t minlevel;
        uint32_t minlevel_heroic;
        float repopx;
        float repopy;
        float repopz;
        uint32_t repopmapid;
        std::string name;
        uint32_t flags;
        uint32_t cooldown;
        uint32_t lvl_mod_a;
        uint32_t required_quest_A;
        uint32_t required_quest_H;
        uint32_t required_item;
        uint32_t heroic_key_1;
        uint32_t heroic_key_2;
        float update_distance;
        uint32_t checkpoint_id;

        bool HasFlag(uint32_t flag) const
        {
            if ((flags & flag) != 0)
                return true;
            else
                return false;
        }

        bool HasDifficulty(uint32_t difficulty) const
        {
            if (difficulty > uint32_t(TOTAL_RAID_MODES))
                return false;

            return HasFlag(uint32_t(WMI_INSTANCE_HAS_NORMAL_10MEN) << difficulty);
        }
    };

    //worldstate_templates

    //worldstring_tables
    struct WorldStringTable
    {
        uint32_t id;
        std::string text;
    };

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
