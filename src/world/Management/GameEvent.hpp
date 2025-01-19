/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <ctime>
#include <map>
#include <set>
#include <vector>
#include <string>

class GameObject;
class Creature;
class EventScript;

struct GameEventFinishCondition;
struct EventNamesQueryResult;
struct EventCreatureSpawnsQueryResult;
struct EventGameObjectSpawnsQueryResult;

enum HolidayIds : uint32_t;
enum GameEventState : uint8_t;

class GameEvent
{
public:
    GameEvent(EventNamesQueryResult result);
    GameEvent() = default;

    void SetState(GameEventState pState);
    GameEventState GetState();

    GameEventState OnStateChange(GameEventState pOldState, GameEventState pNewState);
    bool StartEvent(bool forced = false);
    bool StopEvent(bool forced = false);

    bool isValid() const;
    void SpawnAllEntities();
    void DestroyAllEntities();

    uint32_t event_id; // Event ID
    time_t start; // Event start time
    time_t end; // Event end time
    uint32_t occurence;
    uint32_t length;
    HolidayIds holiday_id;
    std::string description;
    GameEventState state; // state of the game event, these are saved into the game_event table on change!
    uint8_t announce;

    time_t nextstart; // The next time this event should be allowed to change from GAMEEVENT_INACTIVE

    std::vector<EventCreatureSpawnsQueryResult> npc_data;
    std::vector<EventGameObjectSpawnsQueryResult> gameobject_data;

    EventScript* mEventScript;

    // UNUSED
    std::map<uint32_t, GameEventFinishCondition> conditions;  // conditions to finish
    std::set<uint16_t> prerequisite_events;

private:
    void CreateNPCs();
    void CreateObjects();

    std::vector<Creature*> active_npcs;
    std::vector<GameObject*> active_gameobjects;
};
