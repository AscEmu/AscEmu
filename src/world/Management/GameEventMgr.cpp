/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "GameEventMgr.hpp"
#include "GameEvent.hpp"
#include "GameEventDefines.hpp"
#include "Server/World.h"
#include "Storage/MySQLDataStore.hpp"
#include "Debugging/CrashHandler.h"
#include "Logging/Logger.hpp"
#include "Server/DatabaseDefinition.hpp"
#include "Objects/GameObjectProperties.hpp"

GameEventMgr& GameEventMgr::getInstance()
{
    static GameEventMgr mInstance;
    return mInstance;
}

void GameEventMgr::initialize()
{
    mGameEvents.clear();
}

GameEvent* GameEventMgr::GetEventById(uint32_t pEventId)
{
    auto rEvent = mGameEvents.find(pEventId);
    if (rEvent == mGameEvents.end())
        return nullptr;
    else
        return rEvent->second.get();
}

void GameEventMgr::StartArenaEvents()
{
    for (auto i = 57; i <= 60; ++i)
    {
        auto gameEvent = GetEventById(i);
        if (gameEvent == nullptr)
        {
            sLogger.debugFlag(AscEmu::Logging::LF_DB_TABLES, "Missing arena event (id: {})", i);
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
    sLogger.info("GameEventMgr : Start cleaning gameevent_save");
    {
        const char* cleanEventSaveQuery = "DELETE FROM gameevent_save WHERE state<>4";
        CharacterDatabase.Execute(cleanEventSaveQuery);
    }
    // Loading event_properties
    {
        auto result = WorldDatabase.Query("SELECT entry, UNIX_TIMESTAMP(start_time), UNIX_TIMESTAMP(end_time), occurence, "
                                          "length, holiday, description, world_event, announce "
                                          "FROM gameevent_properties WHERE entry > 0 AND min_build <= %u AND max_build >= %u", getAEVersion(), getAEVersion());
        if (!result)
        {
            //mGameEvent.clear();
            sLogger.failure("GameEventMgr : gameevent_properties can not be read or does not include any version specific events!");
            return;
        }

        uint32_t pCount = 0;
        do
        {
            Field* field = result->Fetch();

            EventNamesQueryResult dbResult;
            dbResult.entry = field[0].asUint32();
            dbResult.start_time = field[1].asUint32();
            dbResult.end_time = field[2].asUint32();
            dbResult.occurence = field[3].asUint32();
            dbResult.length = field[4].asUint32();
            dbResult.holiday_id = HolidayIds(field[5].asUint32());
            dbResult.description = field[6].asCString();
            dbResult.world_event = GameEventState(field[7].asUint8());
            dbResult.announce = field[8].asUint8();

            //if (gameEvent.isValid())
            //{
                mGameEvents.emplace(dbResult.entry, std::make_unique<GameEvent>(dbResult));
                sLogger.debug("GameEventMgr : {}, Entry: {}, State: {}, Holiday: {} loaded", dbResult.description, dbResult.entry, dbResult.world_event, dbResult.holiday_id);
                ++pCount;
            //}
            //else
            //{
            //    sLogger.debug("{} game event Entry: {} isn't a world event and has length = 0, thus it can't be used.", dbResult.description, dbResult.entry);
            //}
        } while (result->NextRow());
        sLogger.info("GameEventMgr : {} events loaded from table event_properties", pCount);
    }
    // Loading event_saves from CharacterDB
    sLogger.info("GameEventMgr : Start loading gameevent_save");
    {
        const char* loadEventSaveQuery = "SELECT event_entry, state, next_start FROM gameevent_save";
        bool success = false;
        auto result = CharacterDatabase.Query(&success, loadEventSaveQuery);

        if (!success)
        {
            sLogger.failure("Query failed: {}", loadEventSaveQuery);
            return;
        }

        uint32_t pCount = 0;
        if (result)
        {
            do
            {
                Field* field = result->Fetch();
                uint32_t event_id = field[0].asUint8();

                auto gameEvent = GetEventById(event_id);
                if (gameEvent == nullptr)
                {
                    CharacterDatabase.Query("DELETE FROM gameevent_save WHERE event_entry=%u", event_id);
                    sLogger.info("Deleted invalid gameevent_save with entry {}", event_id);
                    continue;
                }

                gameEvent->state = (GameEventState)(field[1].asUint8());
                gameEvent->nextstart = time_t(field[2].asUint32());

                ++pCount;

            } while (result->NextRow());
        }

        sLogger.info("GameEventMgr : Loaded {} saved events loaded from table gameevent_saves", pCount);
    }
    // Loading event_creature from WorldDB
    sLogger.info("GameEventMgr : Start loading game event creature spawns");
    {
        const char* loadEventCreatureSpawnsQuery = "SELECT id, entry, map, position_x, position_y, position_z, \
                                                    orientation, movetype, displayid, faction, flags, pvp_flagged, bytes0, \
                                                    emote_state, npc_respawn_link, channel_spell, channel_target_sqlid, \
                                                    channel_target_sqlid_creature, standstate, death_state, mountdisplayid, sheath_state, \
                                                    slot1item, slot2item, slot3item, CanFly, phase, waypoint_group, event_entry \
                                                    FROM creature_spawns WHERE min_build <= %u AND max_build >= %u AND event_entry > 0";
        bool success = false;
        auto result = WorldDatabase.Query(&success, loadEventCreatureSpawnsQuery, VERSION_STRING, VERSION_STRING);
        if (!success)
        {
            sLogger.failure("Query failed: {}", loadEventCreatureSpawnsQuery);
            return;
        }

        uint32_t pCount = 0;
        if (result)
        {
            do
            {
                Field* field = result->Fetch();

                uint32_t event_id = field[28].asUint32();

                auto gameEvent = GetEventById(event_id);
                if (gameEvent == nullptr)
                {
                    sLogger.failure("Could not find event for creature_spawns entry {}", event_id);
                    continue;
                }

                EventCreatureSpawnsQueryResult dbResult;
                dbResult.event_entry = field[28].asUint32();
                dbResult.id = field[0].asUint32();
                dbResult.entry = field[1].asUint32();
                auto creature_properties = sMySQLStore.getCreatureProperties(dbResult.entry);
                if (creature_properties == nullptr)
                {
                    sLogger.failure("Could not create CreatureSpawn for invalid entry {} (missing in table creature_properties)", dbResult.entry);
                    continue;
                }
                dbResult.map_id = field[2].asUint16();
                dbResult.position_x = field[3].asFloat();
                dbResult.position_y = field[4].asFloat();
                dbResult.position_z = field[5].asFloat();
                dbResult.orientation = field[6].asFloat();
                dbResult.movetype = field[7].asUint8();
                dbResult.displayid = field[8].asUint32();
                dbResult.faction = field[9].asUint32();
                dbResult.flags = field[10].asUint32();
                dbResult.pvp_flagged = field[11].asUint8();
                dbResult.bytes0 = field[12].asUint32();
                dbResult.emote_state = field[13].asUint16();
                dbResult.npc_respawn_link = field[14].asUint32();
                dbResult.channel_spell = field[15].asUint32();
                dbResult.channel_target_sqlid = field[16].asUint32();
                dbResult.channel_target_sqlid_creature = field[17].asUint32();
                dbResult.standstate = field[18].asUint8();
                dbResult.death_state = field[19].asUint8();
                dbResult.mountdisplayid = field[20].asUint32();
                dbResult.sheath_state = field[21].asUint8();
                dbResult.slot1item = field[22].asUint32();
                dbResult.slot2item = field[23].asUint32();
                dbResult.slot3item = field[24].asUint32();
                dbResult.CanFly = field[25].asUint16();
                dbResult.phase = field[26].asUint32();
                dbResult.waypoint_group = field[27].asUint32();

                gameEvent->npc_data.push_back(dbResult);

                ++pCount;

                //mNPCGuidList.insert(NPCGuidList::value_type(event_id, id));

            } while (result->NextRow());
        }
        sLogger.info("GameEventMgr : {} creature spawns for {} events from table event_creature_spawns loaded.", pCount, static_cast<uint32_t>(mGameEvents.size()));
    }
    // Loading event_gameobject from WorldDB
    sLogger.info("GameEventMgr : Start loading game event gameobject spawns");
    {
        const char* loadEventGameobjectSpawnsQuery = "SELECT id, entry, map, phase, position_x, position_y, \
                                                      position_z, orientation, rotation0, rotation1, rotation2, \
                                                      rotation3, spawntimesecs, state, \
                                                      event_entry FROM gameobject_spawns WHERE min_build <= %u AND max_build >= %u AND event_entry > 0;";
        bool success = false;
        auto result = WorldDatabase.Query(&success, loadEventGameobjectSpawnsQuery, VERSION_STRING, VERSION_STRING);
        if (!success)
        {
            sLogger.failure("Query failed: {}", loadEventGameobjectSpawnsQuery);
            return;
        }

        uint32_t pCount = 0;
        if (result)
        {
            do
            {
                Field* field = result->Fetch();
                uint32_t event_id = field[14].asUint32();

                auto gameEvent = GetEventById(event_id);
                if (gameEvent == nullptr)
                {
                    sLogger.debugFlag(AscEmu::Logging::LF_DB_TABLES, "Could not find event for gameobject_spawns entry {}", event_id);
                    continue;
                }

                EventGameObjectSpawnsQueryResult dbResult;
                dbResult.event_entry = event_id;
                dbResult.id = field[0].asUint32();
                dbResult.entry = field[1].asUint32();
                auto gameobject_info = sMySQLStore.getGameObjectProperties(dbResult.entry);
                if (gameobject_info == nullptr)
                {
                    sLogger.debugFlag(AscEmu::Logging::LF_DB_TABLES, "Could not create GameobjectSpawn for invalid entry {} (missing in table gameobject_properties)", dbResult.entry);
                    continue;
                }
                dbResult.map_id = field[2].asUint32();
                dbResult.phase = field[3].asUint32();
                dbResult.position_x = field[4].asFloat();
                dbResult.position_y = field[5].asFloat();
                dbResult.position_z = field[6].asFloat();
                dbResult.facing = field[7].asFloat();
                dbResult.orientation1 = field[8].asFloat();
                dbResult.orientation2 = field[9].asFloat();
                dbResult.orientation3 = field[10].asFloat();
                dbResult.orientation4 = field[11].asFloat();
                dbResult.spawnTimesecs = field[12].asUint32();
                dbResult.state = field[13].asUint32();

                gameEvent->gameobject_data.push_back(dbResult);

                ++pCount;

                //mGOBGuidList.insert(GOBGuidList::value_type(event_id, id));

            } while (result->NextRow());
        }
        sLogger.info("GameEventMgr : {} gameobject spawns for {} events from table gameobject_spawns loaded.", pCount, mGameEvents.size());
    }

    StartArenaEvents();
}

GameEventMgr::GameEventMgrThread& GameEventMgr::GameEventMgrThread::getInstance()
{
    static GameEventMgr::GameEventMgrThread mInstance;
    return mInstance;
}

void GameEventMgr::GameEventMgrThread::initialize()
{
    m_reloadThread = std::make_unique<AscEmu::Threading::AEThread>("UpdateGameEvents", [this](AscEmu::Threading::AEThread& /*thread*/) { this->Update(); }, std::chrono::seconds(1));
}

void GameEventMgr::GameEventMgrThread::finalize()
{
    sLogger.info("GameEventMgrThread : Stop Manager...");
    m_reloadThread->killAndJoin();
}

void GameEventMgr::GameEventMgrThread::Update()
{
    //sLogger.info("GameEventMgr : Tick!");
    auto now = time(0);

    for (const auto& gameEventPair : sGameEventMgr.mGameEvents)
    {
        GameEvent* gameEvent = gameEventPair.second.get();

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
