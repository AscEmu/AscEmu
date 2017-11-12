/*
Copyright (c) 2014-2017 AscEmu Team <http://www.ascemu.org/>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "Server/Definitions.h"
#include <stdint.h>
#include <string>

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

    //\brief loaded in AuctionHouse.cpp
    //auctionhouse

    //battlemasters
    struct Battlemasters
    {
        uint32_t creatureEntry;
        uint32_t battlegroundId;
    };

    //creature_difficulty
    struct CreatureDifficulty
    {
        uint32_t id;
        uint32_t difficultyEntry1;
        uint32_t difficultyEntry2;
        uint32_t difficultyEntry3;
    };

    //creature_formations
    struct CreatureFormation
    {
        //uint32_t spawnId
        uint32_t targetSpawnId;
        float followAngle;
        float followDistance;
    };

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
    struct DisplayBoundingBoxes
    {
        uint32_t displayid;
        float low[3];
        float high[3];
        float boundradius;
    };

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

    //gossip_menu
    struct GossipMenuInit
    {
        uint32_t gossipMenu;
        uint32_t textId;
    };

    //gossip_menu_items
    struct GossipMenuItems
    {
        uint32_t gossipMenu;
        uint32_t itemOrder;
        uint32_t menuOptionText;
        uint32_t icon;
        uint32_t pointOfInterest;
        uint32_t nextGossipMenu;
        uint32_t nextGossipMenuText;
    };

    //gossip_menu_option
    struct GossipMenuOption
    {
        uint32_t id;
        std::string text;
    };

    //graveyards
    struct Graveyards
    {
        uint32_t id;
        float position_x;
        float position_y;
        float position_z;
        float orientation;          //\todo: orientation always 0 in db.
        uint32_t zoneId;            //\todo: not used.. always 0 in db.
        uint32_t adjacentZoneId;
        uint32_t mapId;
        uint32_t factionId;         //\todo: this is not faction... it is team!
        //std::string name;         //\todo: not loaded from db!
    };

    //instance_bosses

    //item_pages
    struct ItemPage
    {
        uint32_t id;
        std::string text;
        uint32_t nextPage;
    };

    //\TODO structure defined in ItemPrototype.h (legacy file)
    //item_properties

    //\TODO table loaded by QuestMgr.
    //item_quest_association

    //\TODO table loaded by LootMgr.
    //item_randomprop_groups

    //\TODO table loaded by LootMgr.
    //item_randomsuffix_groups

    //itemset_linked_itemsetbonus
    struct ItemSetLinkedItemSetBonus
    {
        int32_t itemset;
        uint32_t itemset_bonus;
    };

    //lfg_dungeon_rewards

    //locales_creature
    struct LocalesCreature
    {
        uint32_t entry;
        uint32_t languageCode;
        char* name;
        char* subName;
    };

    //locales_gameobject
    struct LocalesGameobject
    {
        uint32_t entry;
        uint32_t languageCode;
        char* name;
    };

    //locales_gossip_menu_option
    struct LocalesGossipMenuOption
    {
        uint32_t entry;
        uint32_t languageCode;
        char* name;
    };

    //locales_item
    struct LocalesItem
    {
        uint32_t entry;
        uint32_t languageCode;
        char* name;
        char* description;
    };

    //locales_item_pages
    struct LocalesItemPages
    {
        uint32_t entry;
        uint32_t languageCode;
        char* text;
    };

    //locales_npc_monstersay
    struct LocalesNPCMonstersay
    {
        uint32_t entry;
        uint32_t type;
        uint32_t languageCode;
        char* monstername;
        char* text0;
        char* text1;
        char* text2;
        char* text3;
        char* text4;
    };

    //locales_npc_script_text
    struct LocalesNpcScriptText
    {
        uint32_t entry;
        uint32_t languageCode;
        char* text;
    };

    //locales_npc_text
    struct LocalesNpcText
    {
        uint32_t entry;
        uint32_t languageCode;
        char* texts[8][2];
    };

    //locales_quest
    struct LocalesQuest
    {
        uint32_t entry;
        uint32_t languageCode;
        char* title;
        char* details;
        char* objectives;
        char* completionText;
        char* incompleteText;
        char* endText;
        char* objectiveText[4];
    };

    //locales_worldbroadcast
    struct LocalesWorldbroadcast
    {
        uint32_t entry;
        uint32_t languageCode;
        char* text;
    };

    //locales_worldmap_info
    struct LocalesWorldmapInfo
    {
        uint32_t entry;
        uint32_t languageCode;
        char* text;
    };

    //locales_worldstring_table
    struct LocalesWorldStringTable
    {
        uint32_t entry;
        uint32_t languageCode;
        char* text;
    };

    //loot_creatures
    //loot_fishing
    //loot_gameobjects
    //loot_items
    //loot_pickpocketing
    //loot_skinning

    //\brief No structure!
    //npc_gossip_textid

    //npc_monstersay
    struct NpcMonsterSay
    {
        float chance;
        uint32_t language;
        uint32_t type;
        const char* monsterName;

        uint32_t textCount;
        const char** texts;
    };

    //npc_script_text
    struct NpcScriptText
    {
        uint32_t id;                // unique id \todo remove this and use creature_entry + text_id as key
        std::string text;
        uint32_t creature_entry;    // creature entry ID
        uint32_t text_id;           // text_id started with 0
        uint32_t type;
        uint32_t language;
        float probability;          // chance/percent
        uint32_t emote;             // emote id on say
        uint32_t duration;
        uint32_t sound;             // the sound on say
        uint32_t broadcast_id;
    };

    //npc_text
    struct NpcText_Emote
    {
        uint32_t delay;
        uint32_t emote;
    };

    #define GOSSIP_EMOTE_COUNT 3
    struct NpcText_Texts
    {
        float probability;
        std::string texts[2];
        uint32_t language;
        NpcText_Emote gossipEmotes[GOSSIP_EMOTE_COUNT];
    };

    struct NpcText
    {
        uint32_t entry;
        NpcText_Texts textHolder[8];
    };

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

    //\brief No structure!
    //petdefaultspells      Zyres 2017/07/16 not used

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
    struct ProfessionDiscovery
    {
        uint32_t SpellId;
        uint32_t SpellToDiscover;
        uint32_t SkillValue;
        float Chance;
    };

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

    //transport_creatures
    struct TransportCreatures
    {
        uint32_t guid;
        uint32_t entry;
        uint32_t transportEntry;
        float transportOffsetX;
        float transportOffsetY;
        float transportOffsetZ;
        float transportOffsetO;
        uint32_t animation;
    };

    //transport_data
    struct TransportData
    {
        uint32_t entry;
        std::string name;
        uint32_t period;
    };

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

        bool hasFlag(uint32_t flag) const
        {
            return ((flag & flags) != 0);
        }

        bool hasDifficulty(uint32_t difficulty) const
        {
            if (difficulty > uint32_t(TOTAL_RAID_MODES))
            {
                return false;
            }

            return hasFlag(uint32_t(WMI_INSTANCE_HAS_NORMAL_10MEN) << difficulty);
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
    //account_permissions
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
