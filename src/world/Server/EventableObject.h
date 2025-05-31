/*
 * AscEmu Framework based on ArcEmu MMORPG Server
 * Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
 * Copyright (C) 2008-2012 ArcEmu Team <http://www.ArcEmu.org/>
 * Copyright (C) 2005-2007 Ascent Team
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef EVENTABLEOBJECT_H
#define EVENTABLEOBJECT_H

#include "CommonTypes.hpp"
#include <list>
#include <map>
#include <mutex>
#include <set>

struct TimedEvent;
class EventableObjectHolder;

typedef std::list<std::shared_ptr<TimedEvent>> EventList;
typedef std::multimap<uint32_t, std::shared_ptr<TimedEvent>> EventMap;

#define EVENT_REMOVAL_FLAG_ALL 0xFFFFFFFF
#define WORLD_INSTANCE -1

//////////////////////////////////////////////////////////////////////////////////////////
/// \note EventableObject means that the class inheriting this is able to take
/// events. This 'base' class will store and update these events upon
/// receiving the call from the instance thread / WorldRunnable thread.
//////////////////////////////////////////////////////////////////////////////////////////
class SERVER_DECL EventableObject
{
    friend class EventMgr;
    friend class EventableObjectHolder;

    protected:
        void event_RemoveEvents();
        void event_RemoveEvents(uint32_t EventType);
        void event_ModifyTimeLeft(uint32_t EventType, time_t TimeLeft, bool unconditioned = false);
        void event_ModifyTime(uint32_t EventType, time_t Time);
        void event_ModifyTimeAndTimeLeft(uint32_t EventType, time_t Time);
        bool event_HasEvent(uint32_t EventType);
        void event_RemoveByPointer(TimedEvent* ev);
        int32_t event_GetCurrentInstanceId() { return m_event_Instanceid; }
        bool event_GetTimeLeft(uint32_t EventType, time_t* Time);

    public:
        uint32_t event_GetEventPeriod(uint32_t EventType);
        // Public methods
        EventableObject();
        virtual ~EventableObject();

        bool event_HasEvents() { return m_events.size() > 0 ? true : false; }
        void event_AddEvent(std::shared_ptr<TimedEvent> ptr);
        void event_Relocate();

        /// this func needs to be implemented by all eventable classes. use it to retrieve the instance id that it needs to attach itself to.
        virtual int32_t event_GetInstanceID() { return WORLD_INSTANCE; }

    protected:
        int32_t m_event_Instanceid;
        std::mutex m_lock;
        EventMap m_events;
        EventableObjectHolder* m_holder;
};


typedef std::set<EventableObject*> EventableObjectSet;

//////////////////////////////////////////////////////////////////////////////////////////
///class EventableObjectHolder
/// \note EventableObjectHolder will store eventable objects, and remove/add them when they change
/// from one holder to another (changing maps / instances).
/// EventableObjectHolder also updates all the timed events in all of its objects when its
/// update function is called.
//////////////////////////////////////////////////////////////////////////////////////////
class EventableObjectHolder
{
    public:
        EventableObjectHolder(int32_t instance_id);
        ~EventableObjectHolder();

        void Update(time_t time_difference);

        void AddEvent(std::shared_ptr<TimedEvent> ev);
        void AddObject(EventableObject* obj);

        uint32_t GetInstanceID() { return mInstanceId; }

    protected:
        int32_t mInstanceId;
        std::mutex m_lock;
        EventList m_events;

        std::mutex m_insertPoolLock;
        typedef std::list<std::shared_ptr<TimedEvent>> InsertableQueue;
        InsertableQueue m_insertPool;
};

#endif // EVENTABLEOBJECT_H
