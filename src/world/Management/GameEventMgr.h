/*
Copyright (c) 2014-2020 AscEmu Team <http://www.ascemu.org>
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
    uint32_t max_world_state;     // max resource count world state update id
    uint32_t done_world_state;    // done resource count world state update id
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
    uint32_t bytes0;
    uint32_t bytes1;
    uint32_t bytes2;
    uint16_t emote_state;
    uint32_t npc_respawn_link;
    uint32_t channel_spell;
    uint32_t channel_target_sqlid;
    uint32_t channel_target_sqlid_creature;
    uint8_t standstate;
    uint8_t death_state;
    uint32_t mountdisplayid;
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
    float position_x;
    float position_y;
    float position_z;
    float facing;
    float orientation1;
    float orientation2;
    float orientation3;
    float orientation4;
    uint32_t state;
    uint32_t flags;
    uint32_t faction;
    float scale;
    uint32_t stateNpcLink;
    uint32_t phase;
    uint32_t overrides;
};

struct GameEventData
{
    GameEventData(time_t pstart = 0, time_t pend = 0, time_t pnextstart = 0, uint32_t poccurence = 0, uint32_t plength = 0, HolidayIds pholiday_id = HOLIDAY_NONE,
                  GameEventState pstate = GAMEEVENT_INACTIVE, uint8_t pannounce = 0)
    {
        start = pstart;
        end = pend;
        nextstart = pnextstart;
        occurence = poccurence;
        length = plength;
        holiday_id = pholiday_id;
        state = pstate;
        announce = pannounce;
    };

    GameEventData(EventNamesQueryResult result)
    {
        start = result.start_time;
        end = result.end_time;
        nextstart = 0;                  // after this time the follow-up events count this phase completed
        occurence = result.occurence;
        length = result.length;
        holiday_id = result.holiday_id;
        state = result.world_event;     // state of the game event, these are saved into the game_event table on change!
        announce = result.announce;
    }

    ~GameEventData()
    {
    };

    uint32_t event_id;
    time_t start;
    time_t end;
    time_t nextstart;                   // after this time the follow-up events count this phase completed
    uint32_t occurence;
    uint32_t length;
    HolidayIds holiday_id;
    std::set<uint16_t /*gameevent id*/> prerequisite_events;
    std::string description;
    GameEventState state;               // state of the game event, these are saved into the game_event table on change!
    GameEventConditionMap conditions;   // conditions to finish
    uint8_t announce;

    bool isValid() const { return length > 0 && end > time(0); }
};

typedef std::map<uint32_t, GameEvent*> GameEvents;
typedef std::set<uint16_t> ActiveEvents;

typedef std::map<uint32_t, uint32_t> NPCGuidList;
typedef std::map<uint32_t, uint32_t> GOBGuidList;

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
        GameEvent* GetEventById(uint32_t pEventId);

        GameEvents mGameEvents;
        ActiveEvents mActiveEvents;
        NPCGuidList mNPCGuidList;
        GOBGuidList mGOBGuidList;
};

#define sGameEventMgr GameEventMgr::getInstance()
#define sGameEventMgrThread GameEventMgr::GameEventMgrThread::getInstance()
