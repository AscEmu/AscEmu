/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <cstdint>
#include <map>
#include <memory>
#include <set>
#include <string>

class GameEvent;

enum GameEventState : uint8_t
{
    GAMEEVENT_INACTIVE          = 0,    // Event not active
    GAMEEVENT_ACTIVE            = 1,    // Event active
    GAMEEVENT_PREPARING         = 2,    // Game event starting soon
    GAMEEVENT_INACTIVE_FORCED   = 3,    // Deactivated by GM command or script
    GAMEEVENT_ACTIVE_FORCED     = 4     // Activated by GM command or script
};

// Values based on Holidays.dbc
enum HolidayIds : uint32_t
{
    HOLIDAY_NONE                        = 0,
    HOLIDAY_FIREWORKS_SPECTACULAR       = 62,
    HOLIDAY_FEAST_OF_WINTER_VEIL        = 141,
    HOLIDAY_NOBLEGARDEN                 = 181,
    HOLIDAY_CHILDRENS_WEEK              = 201,
    HOLIDAY_CALL_TO_ARMS_AV             = 283,
    HOLIDAY_CALL_TO_ARMS_WS             = 284,
    HOLIDAY_CALL_TO_ARMS_AB             = 285,
    HOLIDAY_FISHING_EXTRAVAGANZA        = 301,
    HOLIDAY_HARVEST_FESTIVAL            = 321,
    HOLIDAY_HALLOWS_END                 = 324,
    HOLIDAY_LUNAR_FESTIVAL              = 327,
    // HOLIDAY_LOVE_IS_IN_THE_AIR       = 335, unused/duplicated
    HOLIDAY_FIRE_FESTIVAL               = 341,
    HOLIDAY_CALL_TO_ARMS_EY             = 353,
    HOLIDAY_BREWFEST                    = 372,
    HOLIDAY_DARKMOON_FAIRE_ELWYNN       = 374,
    HOLIDAY_DARKMOON_FAIRE_THUNDER      = 375,
    HOLIDAY_DARKMOON_FAIRE_SHATTRATH    = 376,
    HOLIDAY_PIRATES_DAY                 = 398,
    HOLIDAY_CALL_TO_ARMS_SA             = 400,
    HOLIDAY_PILGRIMS_BOUNTY             = 404,
    HOLIDAY_WOTLK_LAUNCH                = 406,
    HOLIDAY_DAY_OF_DEAD                 = 409,
    HOLIDAY_CALL_TO_ARMS_IC             = 420,
    HOLIDAY_LOVE_IS_IN_THE_AIR          = 423,
    HOLIDAY_KALU_AK_FISHING_DERBY       = 424,
    // Cataclysm
    HOLIDAY_CALL_TO_ARMS_BFG            = 435,
    HOLIDAY_CALL_TO_ARMS_TP             = 436,
    HOLIDAY_RATED_BG_15_VS_15           = 442,
    HOLIDAY_RATED_BG_25_VS_25           = 443,
    HOLIDAY_ANNIVERSARY_7_YEARS         = 467,
};

struct GameEventFinishCondition
{
    float reqNum;                   // required number // use float, since some events use percent
    float done;                     // done number
    uint32_t max_world_state;       // max resource count world state update id
    uint32_t done_world_state;      // done resource count world state update id
};

typedef std::map<uint32_t /*condition id*/, GameEventFinishCondition> GameEventConditionMap;

struct EventNamesQueryResult
{
    uint32_t entry;
    time_t start_time;
    time_t end_time;
    uint32_t occurence;
    uint32_t length;
    HolidayIds holiday_id;
    std::string description;
    GameEventState world_event;
    uint8_t announce;
};

struct EventCreatureSpawnsQueryResult
{
    uint32_t event_entry;
    uint32_t id;
    uint32_t entry;
    uint16_t map_id;
    float position_x;
    float position_y;
    float position_z;
    float orientation;
    uint8_t movetype;
    uint32_t displayid;
    uint32_t faction;
    uint32_t flags;
    uint8_t pvp_flagged;
    uint32_t bytes0;
    uint16_t emote_state;
    uint32_t npc_respawn_link;
    uint32_t channel_spell;
    uint32_t channel_target_sqlid;
    uint32_t channel_target_sqlid_creature;
    uint8_t standstate;
    uint8_t death_state;
    uint32_t mountdisplayid;
    uint8_t sheath_state;
    uint32_t slot1item;
    uint32_t slot2item;
    uint32_t slot3item;
    uint16_t CanFly;
    uint32_t phase;
    uint32_t waypoint_group;
};

struct EventGameObjectSpawnsQueryResult
{
    uint32_t event_entry;
    uint32_t id;
    uint32_t entry;
    uint32_t map_id;
    uint32_t phase;
    float position_x;
    float position_y;
    float position_z;
    float facing;
    float orientation1;
    float orientation2;
    float orientation3;
    float orientation4;
    uint32_t spawnTimesecs;
    uint32_t state;
};

typedef std::map<uint32_t, std::unique_ptr<GameEvent>> GameEvents;
typedef std::set<uint16_t> ActiveEvents;

typedef std::map<uint32_t, uint32_t> NPCGuidList;
typedef std::map<uint32_t, uint32_t> GOBGuidList;
