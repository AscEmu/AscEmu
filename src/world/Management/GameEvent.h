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

#ifndef GAMEEVENT_H
#define GAMEEVENT_H

#include "GameEventMgr.h"
#include <vector>
#include <string>

struct EventNamesQueryResult;
struct EventCreatureSpawnsQueryResult;
struct EventGameObjectSpawnsQueryResult;

class EventScript;
#define CALL_EVENTSCRIPT_EVENT(obj, func) if (TO< GameEvent* >(obj)->mEventScript != nullptr) TO< GameEvent* >(obj)->mEventScript->func

class GameEvent
{
    private:

        void CreateNPCs();
        void CreateObjects();

        std::vector<Creature*> active_npcs;
        std::vector<GameObject*> active_gameobjects;

    public:

        uint32 event_id; // Event ID
        time_t start; // Event start time
        time_t end; // Event end time
        uint32 occurence;
        uint32 length;
        HolidayIds holiday_id;
        std::string description;
        GameEventState state; // state of the game event, these are saved into the game_event table on change!
        uint8 announce;

        time_t nextstart; // The next time this event should be allowed to change from GAMEEVENT_INACTIVE

        std::vector<EventCreatureSpawnsQueryResult> npc_data;
        std::vector<EventGameObjectSpawnsQueryResult> gameobject_data;

        EventScript* mEventScript;

        // UNUSED
        GameEventConditionMap conditions;  // conditions to finish
        std::set<uint16 /*gameevent id*/> prerequisite_events;

        void SetState(GameEventState pState);
        GameEventState GetState() { return state; }

        GameEventState OnStateChange(GameEventState pOldState, GameEventState pNewState);
        bool StartEvent(bool forced = false);
        bool StopEvent(bool forced = false);

        GameEvent(EventNamesQueryResult result)
        {
            event_id = result.entry;
            start = result.start_time;
            end = result.end_time;
            occurence = result.occurence;
            length = result.length;
            holiday_id = result.holiday_id;
            description = std::string(result.description);
            //state = result.world_event;
            announce = result.announce;

            // Set later by event_save data, if any
            state = GAMEEVENT_INACTIVE;
            nextstart = 0;

            mEventScript = nullptr;
        }

        GameEvent(){}

        bool isValid() const { return length > 0 && end > time(0); }
        void SpawnAllEntities();
        void DestroyAllEntities();
};

#endif  // GAMEEVENT_H
