/*
* AscEmu Framework based on ArcEmu MMORPG Server
* Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program. If not, see <http://www.gnu.org/licenses/>.
*/
#pragma once

#include "CThreads.h"
#include <set>
#include <map>
#include <ctime>

#include "../../shared/Singleton.h"
#include <string>

class GameEvent;

#define max_ge_check_delay TIME_DAY  // 1 day in seconds

enum GameEventState
{
    GAMEEVENT_INACTIVE          = 0,    // Event not active
    GAMEEVENT_ACTIVE            = 1,    // Event active
    GAMEEVENT_PREPARING         = 2,    // Game event starting soon
    GAMEEVENT_INACTIVE_FORCED   = 3,    // Deactivated by GM command or script
    GAMEEVENT_ACTIVE_FORCED     = 4     // Activated by GM command or script
};

// values based on Holidays.dbc
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
    // HOLIDAY_LOVE_IS_IN_THE_AIR    = 335, unused/duplicated
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
    HOLIDAY_KALU_AK_FISHING_DERBY       = 424
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

struct GameEventData
{
    GameEventData(time_t pstart = 0, time_t pend = 0, time_t pnextstart = 0, uint32 poccurence = 0, uint32 plength = 0, HolidayIds pholiday_id = HOLIDAY_NONE,
                  GameEventState pstate = GAMEEVENT_INACTIVE, uint8 pannounce = 0)
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

    uint32 event_id;
    time_t start;
    time_t end;
    time_t nextstart;                   // after this time the follow-up events count this phase completed
    uint32 occurence;
    uint32 length;
    HolidayIds holiday_id;
    std::set<uint16 /*gameevent id*/> prerequisite_events;
    std::string description;
    GameEventState state;               // state of the game event, these are saved into the game_event table on change!
    GameEventConditionMap conditions;   // conditions to finish
    uint8 announce;

    bool isValid() const { return length > 0 && end > time(0); }
};

typedef std::map<uint32, GameEvent*> GameEvents;
typedef std::set<uint16> ActiveEvents;

typedef std::map<uint32, uint32> NPCGuidList;
typedef std::map<uint32, uint32> GOBGuidList;

class GameEventMgr : public Singleton < GameEventMgr >
{
    public:

        class GameEventMgrThread : public CThread, public Singleton < GameEventMgrThread >
        {
            public:

                bool m_IsActive = false;
                bool Pause(int timeout = 1500);
                void Resume();
                bool runThread();
                void onShutdown();
                void Update();
                void CleanupEntities();
                void SpawnActiveEvents();
        };

        GameEventMgr();
        ~GameEventMgr();

        ActiveEvents const& GetActiveEventList() const { return mActiveEvents; }
        void StartArenaEvents();
        void LoadFromDB();
        GameEvent* GetEventById(uint32 pEventId);

        GameEvents mGameEvents;
        ActiveEvents mActiveEvents;
        NPCGuidList mNPCGuidList;
        GOBGuidList mGOBGuidList;
};

#define sGameEventMgr GameEventMgr::getSingleton()
#define sGameEventMgrThread GameEventMgr::GameEventMgrThread::getSingleton()
