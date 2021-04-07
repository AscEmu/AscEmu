/*
Copyright (c) 2014-2021 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "Threading/AEThread.h"
#include <set>
#include <map>
#include <ctime>
#include <string>
#include "CommonTypes.hpp"

class GameEvent;

enum GameEventState
{
    GAMEEVENT_INACTIVE          = 0,    // Event not active
    GAMEEVENT_ACTIVE            = 1,    // Event active
    GAMEEVENT_PREPARING         = 2,    // Game event starting soon
    GAMEEVENT_INACTIVE_FORCED   = 3,    // Deactivated by GM command or script
    GAMEEVENT_ACTIVE_FORCED     = 4     // Activated by GM command or script
};

// Values based on Holidays.dbc
enum HolidayIds
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
    float reqNum;               // required number // use float, since some events use percent
    float done;                 // done number
    uint32 max_world_state;     // max resource count world state update id
    uint32 done_world_state;    // done resource count world state update id
};

typedef std::map<uint32 /*condition id*/, GameEventFinishCondition> GameEventConditionMap;

struct EventNamesQueryResult
{
    uint32 entry;
    time_t start_time;
    time_t end_time;
    uint32 occurence;
    uint32 length;
    HolidayIds holiday_id;
    std::string description;
    GameEventState world_event;
    uint8 announce;
};

struct EventCreatureSpawnsQueryResult
{
    uint32 event_entry;
    uint32 id;
    uint32 entry;
    uint16 map_id;
    float position_x;
    float position_y;
    float position_z;
    float orientation;
    uint8 movetype;
    uint32 displayid;
    uint32 faction;
    uint32 flags;
    uint32 bytes0;
    uint32 bytes1;
    uint32 bytes2;
    uint16 emote_state;
    uint32 npc_respawn_link;
    uint32 channel_spell;
    uint32 channel_target_sqlid;
    uint32 channel_target_sqlid_creature;
    uint8 standstate;
    uint8 death_state;
    uint32 mountdisplayid;
    uint32 slot1item;
    uint32 slot2item;
    uint32 slot3item;
    uint16 CanFly;
    uint32 phase;
    uint32 waypoint_group;
};

struct EventGameObjectSpawnsQueryResult
{
    uint32 event_entry;
    uint32 id;
    uint32 entry;
    uint32 map_id;
    float position_x;
    float position_y;
    float position_z;
    float facing;
    float orientation1;
    float orientation2;
    float orientation3;
    float orientation4;
    uint32 state;
    uint32 flags;
    uint32 faction;
    float scale;
    uint32 stateNpcLink;
    uint32 phase;
    uint32 overrides;
};

typedef std::map<uint32, GameEvent*> GameEvents;
typedef std::set<uint16> ActiveEvents;

typedef std::map<uint32, uint32> NPCGuidList;
typedef std::map<uint32, uint32> GOBGuidList;

class GameEventMgr
{
    private:

        GameEventMgr() = default;
        ~GameEventMgr() = default;

    public:

        class GameEventMgrThread
        {
            private:

                GameEventMgrThread() = default;
                ~GameEventMgrThread() = default;

            public:

                static GameEventMgrThread& getInstance();
                void initialize();
                void finalize();

                GameEventMgrThread(GameEventMgrThread&&) = delete;
                GameEventMgrThread(GameEventMgrThread const&) = delete;
                GameEventMgrThread& operator=(GameEventMgrThread&&) = delete;
                GameEventMgrThread& operator=(GameEventMgrThread const&) = delete;

                bool m_IsActive = false;

                void Update();

                std::unique_ptr<AscEmu::Threading::AEThread> m_reloadThread;
                uint32_t m_reloadTime;
        };

        static GameEventMgr& getInstance();
        void initialize();

        GameEventMgr(GameEventMgr&&) = delete;
        GameEventMgr(GameEventMgr const&) = delete;
        GameEventMgr& operator=(GameEventMgr&&) = delete;
        GameEventMgr& operator=(GameEventMgr const&) = delete;

        ActiveEvents const& GetActiveEventList() const { return mActiveEvents; }
        void StartArenaEvents();
        void LoadFromDB();
        GameEvent* GetEventById(uint32 pEventId);

        GameEvents mGameEvents;
        ActiveEvents mActiveEvents;
        NPCGuidList mNPCGuidList;
        GOBGuidList mGOBGuidList;
};

#define sGameEventMgr GameEventMgr::getInstance()
#define sGameEventMgrThread GameEventMgr::GameEventMgrThread::getInstance()
