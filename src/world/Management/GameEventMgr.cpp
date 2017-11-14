/*
Copyright (c) 2014-2017 AscEmu Team <http://www.ascemu.org/>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "StdAfx.h"
#include "Singleton.h"
#include "GameEventMgr.h"
#include "Log.hpp"
#include "Server/World.h"
#include "Server/World.Legacy.h"
#include "Server/MainServerDefines.h"
#include "GameEvent.h"
#include "Storage/MySQLDataStore.hpp"
#include "CrashHandler.h"

initialiseSingleton(GameEventMgr);
initialiseSingleton(GameEventMgr::GameEventMgrThread);

GameEventMgr::GameEventMgr()
{
    mGameEvents.clear();
}
GameEventMgr::~GameEventMgr()
{

}

GameEvent* GameEventMgr::GetEventById(uint32 pEventId)
{
    auto rEvent = mGameEvents.find(pEventId);
    if (rEvent == mGameEvents.end())
        return nullptr;
    else
        return rEvent->second;
}

void GameEventMgr::StartArenaEvents()
{
    for (auto i = 57; i <= 60; ++i)
    {
        auto gameEvent = GetEventById(i);
        if (gameEvent == nullptr)
        {
            LOG_ERROR("Missing arena event (id: %u)", i);
            continue;
        }

        if (i - 52 == worldConfig.arena.arenaSeason && worldConfig.arena.arenaProgress == 1)
            gameEvent->SetState(GAMEEVENT_ACTIVE_FORCED);
        else
            gameEvent->SetState(GAMEEVENT_INACTIVE_FORCED);
    }
}

void GameEventMgr::LoadFromDB()
{
    // Clean event_saves from CharacterDB
    LogNotice("GameEventMgr : Start cleaning event_save");
    {
        const char* cleanEventSaveQuery = "DELETE FROM event_save WHERE state<>4";
        CharacterDatabase.Execute(cleanEventSaveQuery);
    }
    // Loading event_properties
    {
        const char* loadAllEventsQuery = "SELECT entry, UNIX_TIMESTAMP(start_time), UNIX_TIMESTAMP(end_time), occurence,\
                                          length, holiday, description, world_event, announce\
                                          FROM event_properties WHERE entry > 0";
        QueryResult* result = WorldDatabase.Query(loadAllEventsQuery);
        if (!result)
        {
            //mGameEvent.clear();
            LOG_ERROR("Query failed: %s", loadAllEventsQuery);
            return;
        }

        uint32 pCount = 0;
        do
        {
            Field* field = result->Fetch();

            EventNamesQueryResult dbResult;
            dbResult.entry = field[0].GetUInt32();
            dbResult.start_time = field[1].GetUInt32();
            dbResult.end_time = field[2].GetUInt32();
            dbResult.occurence = field[3].GetUInt32();
            dbResult.length = field[4].GetUInt32();
            dbResult.holiday_id = HolidayIds(field[5].GetUInt32());
            dbResult.description = field[6].GetString();
            dbResult.world_event = GameEventState(field[7].GetUInt8());
            dbResult.announce = field[8].GetUInt8();

            GameEvent gameEvent = GameEvent(dbResult);

            //if (gameEvent.isValid())
            //{
                mGameEvents.insert(std::make_pair(dbResult.entry, new GameEvent(dbResult)));
                LogDebugFlag(LF_DB_TABLES, "GameEventMgr : %s, Entry: %u, State: %u, Holiday: %u loaded", dbResult.description.c_str(), dbResult.entry, dbResult.world_event, dbResult.holiday_id);
                ++pCount;
            //}
            //else
            //{
            //    LOG_DEBUG("%s game event Entry: %u isn't a world event and has length = 0, thus it can't be used.", dbResult.description.c_str(), dbResult.entry);
            //}
        } while (result->NextRow());
        delete result;
        LogDetail("GameEventMgr : %u events loaded from table event_properties", pCount);
    }
    // Loading event_saves from CharacterDB
    LogNotice("GameEventMgr : Start loading event_save");
    {
        const char* loadEventSaveQuery = "SELECT event_entry, state, next_start FROM event_save";
        bool success = false;
        QueryResult* result = CharacterDatabase.Query(&success, loadEventSaveQuery);

        if (!success)
        {
            LOG_ERROR("Query failed: %s", loadEventSaveQuery);
            return;
        }

        uint32 pCount = 0;
        if (result)
        {
            do
            {
                Field* field = result->Fetch();
                uint32 event_id = field[0].GetUInt8();

                auto gameEvent = GetEventById(event_id);
                if (gameEvent == nullptr)
                {
                    LOG_ERROR("Could not find event for event_save entry %u", event_id);
                    continue;
                }

                gameEvent->state = (GameEventState)(field[1].GetUInt8());
                gameEvent->nextstart = time_t(field[2].GetUInt32());

                ++pCount;

            } while (result->NextRow());
            delete result;
        }

        LogDetail("GameEventMgr : Loaded %u saved events loaded from table event_saves", pCount);
    }
    // Loading event_creature from WorldDB
    LogNotice("GameEventMgr : Start loading game event creature spawns");
    {
        const char* loadEventCreatureSpawnsQuery = "SELECT event_entry, id, entry, map, position_x, position_y, position_z, \
                                                    orientation, movetype, displayid, faction, flags, bytes0, bytes1, bytes2, \
                                                    emote_state, npc_respawn_link, channel_spell, channel_target_sqlid, \
                                                    channel_target_sqlid_creature, standstate, death_state, mountdisplayid, \
                                                    slot1item, slot2item, slot3item, CanFly, phase, waypoint_group \
                                                    FROM event_creature_spawns";
        bool success = false;
        QueryResult* result = WorldDatabase.Query(&success, loadEventCreatureSpawnsQuery);
        if (!success)
        {
            LOG_ERROR("Query failed: %s", loadEventCreatureSpawnsQuery);
            return;
        }

        uint32 pCount = 0;
        if (result)
        {
            do
            {
                Field* field = result->Fetch();

                uint32 event_id = field[0].GetUInt32();

                auto gameEvent = GetEventById(event_id);
                if (gameEvent == nullptr)
                {
                    LOG_ERROR("Could not find event for event_creature_spawns entry %u", event_id);
                    continue;
                }

                EventCreatureSpawnsQueryResult dbResult;
                dbResult.event_entry = field[0].GetUInt32();
                dbResult.id = field[1].GetUInt32();
                dbResult.entry = field[2].GetUInt32();
                auto creature_properties = sMySQLStore.getCreatureProperties(dbResult.entry);
                if (creature_properties == nullptr)
                {
                    LOG_ERROR("Could not create CreatureSpawn for invalid entry %u (missing in table creature_properties)", dbResult.entry);
                    continue;
                }
                dbResult.map_id = field[3].GetUInt16();
                dbResult.position_x = field[4].GetFloat();
                dbResult.position_y = field[5].GetFloat();
                dbResult.position_z = field[6].GetFloat();
                dbResult.orientation = field[7].GetFloat();
                dbResult.movetype = field[8].GetUInt8();
                dbResult.displayid = field[9].GetUInt32();
                dbResult.faction = field[10].GetUInt32();
                dbResult.flags = field[11].GetUInt32();
                dbResult.bytes0 = field[12].GetUInt32();
                dbResult.bytes1 = field[13].GetUInt32();
                dbResult.bytes2 = field[14].GetUInt32();
                dbResult.emote_state = field[15].GetUInt16();
                dbResult.npc_respawn_link = field[16].GetUInt32();
                dbResult.channel_spell = field[17].GetUInt32();
                dbResult.channel_target_sqlid = field[18].GetUInt32();
                dbResult.channel_target_sqlid_creature = field[19].GetUInt32();
                dbResult.standstate = field[20].GetUInt8();
                dbResult.death_state = field[21].GetUInt8();
                dbResult.mountdisplayid = field[22].GetUInt32();
                dbResult.slot1item = field[23].GetUInt32();
                dbResult.slot2item = field[24].GetUInt32();
                dbResult.slot3item = field[25].GetUInt32();
                dbResult.CanFly = field[26].GetUInt16();
                dbResult.phase = field[27].GetUInt32();
                dbResult.waypoint_group = field[28].GetUInt32();

                gameEvent->npc_data.push_back(dbResult);

                ++pCount;

                //mNPCGuidList.insert(NPCGuidList::value_type(event_id, id));

            } while (result->NextRow());
            delete result;
        }
        LogDetail("GameEventMgr : %u creature spawns for %u events from table event_creature_spawns loaded.", pCount, mGameEvents.size());
    }
    // Loading event_gameobject from WorldDB
    LogNotice("GameEventMgr : Start loading game event gameobject spawns");
    {
        const char* loadEventGameobjectSpawnsQuery = "SELECT event_entry, id, entry, map, position_x, position_y, \
                                                      position_z, facing, orientation1, orientation2, orientation3, \
                                                      orientation4, state, flags, faction, scale, respawnNpcLink, phase, \
                                                      overrides FROM event_gameobject_spawns";
        bool success = false;
        QueryResult* result = WorldDatabase.Query(&success, loadEventGameobjectSpawnsQuery);
        if (!success)
        {
            LOG_ERROR("Query failed: %s", loadEventGameobjectSpawnsQuery);
            return;
        }

        uint32 pCount = 0;
        if (result)
        {
            do
            {
                Field* field = result->Fetch();
                uint32 event_id = field[0].GetUInt32();

                auto gameEvent = GetEventById(event_id);
                if (gameEvent == nullptr)
                {
                    LOG_ERROR("ould not find event for event_gameobject_spawns entry %u", event_id);
                    continue;
                }

                EventGameObjectSpawnsQueryResult dbResult;
                dbResult.event_entry = field[0].GetUInt32();
                dbResult.id = field[1].GetUInt32();
                dbResult.entry = field[2].GetUInt32();
                auto gameobject_info = sMySQLStore.getGameObjectProperties(dbResult.entry);
                if (gameobject_info == nullptr)
                {
                    LOG_ERROR("Could not create GameobjectSpawn for invalid entry %u (missing in table gameobject_properties)", dbResult.entry);
                    continue;
                }
                dbResult.map_id = field[3].GetUInt32();
                dbResult.position_x = field[4].GetFloat();
                dbResult.position_y = field[5].GetFloat();
                dbResult.position_z = field[6].GetFloat();
                dbResult.facing = field[7].GetFloat();
                dbResult.orientation1 = field[8].GetFloat();
                dbResult.orientation2 = field[9].GetFloat();
                dbResult.orientation3 = field[10].GetFloat();
                dbResult.orientation4 = field[11].GetFloat();
                dbResult.state = field[12].GetUInt32();
                dbResult.flags = field[13].GetUInt32();
                dbResult.faction = field[14].GetUInt32();
                dbResult.scale = field[15].GetFloat();
                dbResult.stateNpcLink = field[16].GetUInt32();
                dbResult.phase = field[17].GetUInt32();
                dbResult.overrides = field[18].GetUInt32();

                gameEvent->gameobject_data.push_back(dbResult);

                ++pCount;

                //mGOBGuidList.insert(GOBGuidList::value_type(event_id, id));

            } while (result->NextRow());
            delete result;
        }
        LogDetail("GameEventMgr : %u gameobject spawns for %u events from table event_gameobject_spawns loaded.", pCount, mGameEvents.size());
    }

    StartArenaEvents();
}

void GameEventMgr::GameEventMgrThread::Update()
{
    //LogNotice("GameEventMgr : Tick!");
    auto now = time(0);

    for (auto gameEventPair : sGameEventMgr.mGameEvents)
    {
        GameEvent* gameEvent = gameEventPair.second;

        // Don't alter manual events
        if (!gameEvent->isValid())
            continue;

        auto startTime = time_t(gameEvent->start);
        if (startTime < now && now < gameEvent->end)
        {
            if ((now - startTime) % (gameEvent->occurence * 60) < gameEvent->length * 60)
            {
                // Event should start
                if (gameEvent->state != GAMEEVENT_INACTIVE_FORCED)
                {
                    gameEvent->StartEvent();
                    continue;
                }
            }
            continue;
        }

        // Event should stop
        if (gameEvent->state != GAMEEVENT_ACTIVE_FORCED)
        {
            gameEvent->StopEvent();
        }
    }
}

void GameEventMgr::GameEventMgrThread::onShutdown()
{
    LogNotice("GameEventMgr : Shutdown!");
    ThreadState = THREADSTATE_TERMINATE;
}

void GameEventMgr::GameEventMgrThread::CleanupEntities()
{
    // DO NOT USE THIS FUNCTION UNLESS YOU KNOW WHAT YOU ARE DOING
    ARCEMU_ASSERT(FALSE);
    LogNotice("GameEventMgr : Cleaning up entity remnants");
    // Better solution: don't have creatures save here in the first place
    for (auto gameEventPair : sGameEventMgr.mGameEvents)
    {
        auto gameEvent = gameEventPair.second;
        for (auto npc : gameEvent->npc_data)
        {
            const char* cleanCreaturesQuery = "DELETE FROM creature_spawns WHERE entry=%u";
            WorldDatabase.Execute(cleanCreaturesQuery, npc.entry);
        }
        for (auto gameobject : gameEvent->gameobject_data)
        {
            const char* cleanGameObjectsQuery = "DELETE FROM gameobject_spawns WHERE entry=%u";
            WorldDatabase.Execute(cleanGameObjectsQuery, gameobject.entry);
        }
    }
    LogDetail("GameEventMgr : Entity remnants cleaned up, starting main thread");
}

void GameEventMgr::GameEventMgrThread::SpawnActiveEvents()
{
    for (auto gameEventPair : sGameEventMgr.mGameEvents)
    {
        if (gameEventPair.second->GetState() == GAMEEVENT_ACTIVE || gameEventPair.second->GetState() == GAMEEVENT_ACTIVE_FORCED)
        {
            gameEventPair.second->SpawnAllEntities();
        }
    }
}

bool GameEventMgr::GameEventMgrThread::runThread()
{
    LogNotice("GameEventMgr : Started.");

    // Do NOT uncomment this unless you know what you're doing, debug code
    //CleanupEntities();

    SpawnActiveEvents();

    THREAD_TRY_EXECUTION

    while (GetThreadState() != THREADSTATE_TERMINATE)
    {
        while (GetThreadState() == THREADSTATE_PAUSED)
        {
            m_IsActive = false;
            Arcemu::Sleep(200);
        }

        m_IsActive = true;

        if (GetThreadState() == THREADSTATE_TERMINATE)
            break;

        ThreadState = THREADSTATE_BUSY;
        Update();

        if (GetThreadState() == THREADSTATE_TERMINATE)
            break;

        ThreadState = THREADSTATE_SLEEPING;
        Arcemu::Sleep(1 * 1000); // 1 second
    }

    THREAD_HANDLE_CRASH
        return true;
}

bool GameEventMgr::GameEventMgrThread::Pause(int timeout /*= 1500*/)
{
    sGameEventMgrThread.SetThreadState(THREADSTATE_PAUSED);

    for (auto i = 0; i < timeout / 100; ++i)
    {
        Arcemu::Sleep(100);
        if (!sGameEventMgrThread.m_IsActive)
        {
            return true;
        }
    }

    return false;
}

void GameEventMgr::GameEventMgrThread::Resume()
{
    sGameEventMgrThread.SetThreadState(THREADSTATE_SLEEPING);
}
