/*
 * AscEmu Framework based on ArcEmu MMORPG Server
 * Copyright (C) 2014-2015 AscEmu Team <http://www.ascemu.org>
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

#include "StdAfx.h"
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

void GameEventMgr::LoadFromDB()
{
    // Loading event_names
    {
        const char* loadAllEventsQuery = "SELECT entry, UNIX_TIMESTAMP(start_time), UNIX_TIMESTAMP(end_time), occurence,\
                                          length, holiday, description, world_event, announce\
                                          FROM event_names WHERE entry > 0";
        QueryResult* result = WorldDatabase.Query(loadAllEventsQuery);
        if (!result)
        {
            //mGameEvent.clear();
            Log.Error("GameEventMgr", "Query failed: %s", loadAllEventsQuery);
            return;
        }

        uint32 count = 0;
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

            if (gameEvent.isValid())
            {
                mGameEvents.insert(std::make_pair(dbResult.entry, new GameEvent(dbResult)));
                Log.Success("GameEventMgr", "%s, Entry: %u, State: %u, Holiday: %u loaded", dbResult.description.c_str(), dbResult.entry, dbResult.world_event, dbResult.holiday_id);
                ++count;
            }
            else
            {
                Log.Debug("GameEventMgr", "%s game event Entry: %u isn't a world event and has length = 0, thus it can't be used.", dbResult.description.c_str(), dbResult.entry);
            }
        } while (result->NextRow());

        delete result;
        Log.Success("GameEventMgr", "%u events loaded from table event_names", count);
    }
    // Loading event_saves from CharacterDB
    Log.Notice("GameEventMgr", "Start loading event_save");
    {
        const char* loadEventSaveQuery = "SELECT eventEntry, state, next_start FROM event_save";
        bool success = false;
        QueryResult* result = CharacterDatabase.Query(&success, loadEventSaveQuery);

        if (!success)
        {
            Log.Error("GameEventMgr", "Query failed: %s", loadEventSaveQuery);
            return;
        }

        int count = 0;
        if (result)
        {
            do
            {
                Field* field = result->Fetch();
                uint32 event_id = field[0].GetUInt8();

                auto gameEvent = GetEventById(event_id);
                if (gameEvent == nullptr)
                {
                    Log.Error("GameEventMgr", "Could not find event for event_save entry %u", event_id);
                    continue;
                }

                gameEvent->state = (GameEventState)(field[1].GetUInt8());
                gameEvent->nextstart = time_t(field[2].GetUInt32());

                ++count;

            } while (result->NextRow());
        }

        Log.Success("GameEventMgr", "Loaded %u saved events loaded from table event_saves", count);
    }
    // Loading event_creature from WorldDB
    Log.Notice("GameEventMgr", "Start loading game event creature spawns");
    {
        const char* loadEventCreatureSpawnsQuery = "SELECT eventEntry, id, entry, map, position_x, position_y, position_z, \
                                                    orientation, movetype, displayid, faction, flags, bytes0, bytes1, bytes2, \
                                                    emote_state, npc_respawn_link, channel_spell, channel_target_sqlid, \
                                                    channel_target_sqlid_creature, standstate, death_state, mountdisplayid, \
                                                    slot1item, slot2item, slot3item, CanFly, phase, waypoint_group \
                                                    FROM event_creature_spawns";
        bool success = false;
        QueryResult* result = WorldDatabase.Query(&success, loadEventCreatureSpawnsQuery);
        if (!success)
        {
            Log.Error("GameEventMgr", "Query failed: %s", loadEventCreatureSpawnsQuery);
            return;
        }

        uint32 count = 0;
        if (result)
        {
            do
            {
                Field* field = result->Fetch();

                uint32 event_id = field[0].GetUInt32();

                auto gameEvent = GetEventById(event_id);
                if (gameEvent == nullptr)
                {
                    Log.Error("GameEventMgr", "Could not find event for event_creature_spawns entry %u", event_id);
                    continue;
                }

                EventCreatureSpawnsQueryResult dbResult;
                dbResult.eventEntry = field[0].GetUInt32();
                dbResult.id = field[1].GetUInt32();
                dbResult.entry = field[2].GetUInt32();
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

                ++count;

                //mNPCGuidList.insert(NPCGuidList::value_type(event_id, id));

            } while (result->NextRow());
        }
        Log.Success("GameEventMgr", "%u creature spawns for %u events from table event_creature_spawns loaded.", count, mGameEvents.size());
    }
    // Loading event_gameobject from WorldDB
    Log.Notice("GameEventMgr", "Start loading game event gameobject spawns");
    {
        const char* loadEventGameobjectSpawnsQuery = "SELECT eventEntry, id, entry, map, position_x, position_y, \
                                                      position_z, facing, orientation1, orientation2, orientation3, \
                                                      orientation4, state, flags, faction, scale, stateNpcLink, phase, \
                                                      overrides FROM event_gameobject_spawns";
        bool success = false;
        QueryResult* result = WorldDatabase.Query(&success, loadEventGameobjectSpawnsQuery);
        if (!success)
        {
            Log.Error("GameEventMgr", "Query failed: %s", loadEventGameobjectSpawnsQuery);
            return;
        }

        uint32 count = 0;
        if (result)
        {
            do
            {
                Field* field = result->Fetch();
                uint32 event_id = field[0].GetUInt32();

                auto gameEvent = GetEventById(event_id);
                if (gameEvent == nullptr)
                {
                    Log.Error("GameEventMgr", "Could not find event for event_gameobject_spawns entry %u", event_id);
                    continue;
                }

                EventGameObjectSpawnsQueryResult dbResult;
                dbResult.eventEntry = field[0].GetUInt32();
                dbResult.id = field[1].GetUInt32();
                dbResult.entry = field[2].GetUInt32();
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
                dbResult.stateNpcLink = field[16].GetFloat();
                dbResult.phase = field[17].GetUInt32();
                dbResult.overrides = field[18].GetUInt32();

                gameEvent->gameobject_data.push_back(dbResult);

                ++count;

                //mGOBGuidList.insert(GOBGuidList::value_type(event_id, id));

            } while (result->NextRow());
        }
        Log.Success("GameEventMgr", "%u gameobject spawns for %u events from table event_gameobject_spawns loaded.", count, mGameEvents.size());
    }
}

void GameEventMgr::GameEventMgrThread::Update()
{
    //Log.Notice("GameEventMgr", "Tick!");
    auto now = time(0);
    for each (auto gameEventPair in sGameEventMgr.mGameEvents)
    {
        GameEvent* gameEvent = gameEventPair.second;

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

void GameEventMgr::GameEventMgrThread::OnShutdown()
{
    Log.Notice("GameEventMgr", "Shutdown!");
    ThreadState.SetVal(THREADSTATE_TERMINATE);
}

void GameEventMgr::GameEventMgrThread::CleanupEntities()
{
    // DO NOT USE THIS FUNCTION UNLESS YOU KNOW WHAT YOU ARE DOING
    ARCEMU_ASSERT(FALSE);
    Log.Success("GameEventMgr", "Cleaning up entity remnants");
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
    Log.Success("GameEventMgr", "Entity remnants cleaned up, starting main thread");
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

bool GameEventMgr::GameEventMgrThread::run()
{
    Log.Success("GameEventMgr", "Started.");

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

        ThreadState.SetVal(THREADSTATE_BUSY);
        Update();

        if (GetThreadState() == THREADSTATE_TERMINATE)
            break;

        ThreadState.SetVal(THREADSTATE_SLEEPING);
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
