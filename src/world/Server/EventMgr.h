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

#ifndef EVENTMGR_H
#define EVENTMGR_H

#include <map>

#include "Utilities/CallBack.h"

enum EventTypes : uint16_t
{
    EVENT_UNK = 0,
    EVENT_WORLD_UPDATEAUCTIONS,
    EVENT_CREATURE_UPDATE,
    EVENT_PLAYER_UPDATE,
    EVENT_GAMEOBJECT_UPDATE,
    // EVENT_DYNAMICOBJECT_UPDATE,
    EVENT_CREATURE_REMOVE_CORPSE,
    EVENT_CREATURE_RESPAWN,
    EVENT_UNIT_EMOTE,
    EVENT_WEATHER_UPDATE,
    //EVENT_PLAYER_TAXI_DISMOUNT,
    EVENT_UNIT_CHAT_MSG,
    EVENT_TIMED_QUEST_EXPIRE,
    //EVENT_PLAYER_TAXI_INTERPOLATE,
    EVENT_PLAYER_CHECKFORCHEATS,
    EVENT_AURA_REMOVE,
    EVENT_AURA_PERIODIC_HEAL,
    // EVENT_AURA_PERIOCIC_MANA,
    EVENT_CANNIBALIZE,
    // EVENT_DELETE_TIMER,
    EVENT_GAMEOBJECT_ITEM_SPAWN,
    EVENT_PET_DELAYED_REMOVE,
    EVENT_PLAYER_CHARM_ATTACK,
    EVENT_PLAYER_KICK,
    EVENT_LOOT_ROLL_FINALIZE,
    EVENT_CORPSE_DESPAWN,
    EVENT_SCRIPT_UPDATE_EVENT,
    EVENT_PLAYER_DUEL_COUNTDOWN,
    EVENT_PLAYER_DUEL_BOUNDARY_CHECK,
    EVENT_PLAYER_TELEPORT,
    EVENT_CREATURE_CHANNEL_LINKUP,
    EVENT_AURA_PERIODIC_HEALINCOMB,
    // EVENT_AURA_PERIODIC_HEALPERC,
    EVENT_SUMMON_EXPIRE,                        /// Zack 2007 05 28: similar to pet expire but we can have multiple guardians
    EVENT_PLAYER_FORCED_RESURRECT,              /// Zack 2007 06 08: After player not pushing release spirit for 6 minutes while dead
    EVENT_PLAYER_SOFT_DISCONNECT,               /// Zack 2007 06 12: Kick AFK players to not eat resources
    EVENT_BATTLEGROUND_WSG_AUTO_RETURN_FLAG,
    EVENT_CORPSE_SPAWN_BONES,
    EVENT_DODGE_BLOCK_FLAG_EXPIRE,              /// yeah, there are more then 1 flags
    EVENT_REJUVENATION_FLAG_EXPIRE,
    EVENT_PARRY_FLAG_EXPIRE,
    EVENT_LASTKILLWITHHONOR_FLAG_EXPIRE,
    EVENT_BATTLEGROUND_QUEUE_UPDATE,
    EVENT_BATTLEGROUND_COUNTDOWN,
    EVENT_BATTLEGROUND_CLOSE,
    EVENT_BATTLEGROUND_RESOURCEUPDATE,
    EVENT_PLAYER_EJECT_FROM_INSTANCE,
    EVENT_AB_RESOURCES_UPDATE_TEAM_0,
    EVENT_AB_CAPTURE_CP_1,
    EVENT_AB_RESPAWN_BUFF,
    EVENT_AV_CAPTURE_CP_0,
    EVENT_COMBO_POINT_CLEAR_FOR_TARGET,
    EVENT_ITEM_UPDATE,
    EVENT_EOTS_GIVE_POINTS,
    EVENT_EOTS_CHECK_CAPTURE_POINT_STATUS,
    EVENT_EOTS_RESET_FLAG,
    EVENT_ARENA_SHADOW_SIGHT,
    EVENT_SEND_PACKET_TO_PLAYER_AFTER_LOGIN,
    EVENT_PLAYER_RUNE_REGEN,
    EVENT_SOTA_TIMER,
    EVENT_PLAYER_AVENGING_WRATH,
    EVENT_REMOVE_ITEM,
    EVENT_REMOVE_ITEM_FROM_REFUNDABLE_MAP,
    EVENT_GROUP_AREA_AURA_UPDATE,
    EVENT_RAID_AREA_AURA_UPDATE,
    EVENT_PET_AREA_AURA_UPDATE,
    EVENT_FRIEND_AREA_AURA_UPDATE,
    EVENT_ENEMY_AREA_AURA_UPDATE,
    EVENT_IOC_CAPTURE_CP_1,
    EVENT_IOC_RESOURCES_UPDATE,
    EVENT_IOC_BUILD_WORKSHOP_VEHICLE,
    EVENT_SOTA_START_ROUND,
    EVENT_EVENT_SCRIPTS,
    EVENT_STOP_CHANNELING,
    NUM_EVENT_TYPES
};

enum EventFlags : uint8_t
{
    EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT = 0x1,
    EVENT_FLAG_DELETES_OBJECT = 0x2,
};

struct SERVER_DECL TimedEvent
{
    TimedEvent(void* object, std::unique_ptr<CallbackBase> callback, uint32_t type, time_t time, uint32_t repeat, uint32_t flags) :
        obj(object), cb(std::move(callback)), eventType(type), eventFlag(static_cast<uint16_t>(flags)), msTime(time), currTime(time), repeats(static_cast<uint16_t>(repeat)), deleted(false), instanceId(0) {}

    void* obj;
    std::unique_ptr<CallbackBase> cb;
    uint32_t eventType;
    uint16_t eventFlag;
    time_t msTime;
    time_t currTime;
    uint16_t repeats;
    bool deleted;
    int instanceId;

    static std::shared_ptr<TimedEvent> Allocate(void* object, std::unique_ptr<CallbackBase> callback, uint32_t flags, time_t time, uint32_t repeat);
};

class EventMgr;
class EventableObjectHolder;
typedef std::map<int32_t, EventableObjectHolder*> HolderMap;

class SERVER_DECL EventMgr
{
    friend class MiniEventMgr;

    private:
        EventMgr() = default;
        ~EventMgr() = default;

    public:
        static EventMgr& getInstance();

        EventMgr(EventMgr&&) = delete;
        EventMgr(EventMgr const&) = delete;
        EventMgr& operator=(EventMgr&&) = delete;
        EventMgr& operator=(EventMgr const&) = delete;

        template <class Class>
        void AddEvent(Class* obj, void (Class::*method)(), uint32_t type, time_t time, uint32_t repeats, uint32_t flags)
        {
            // create a timed event
            auto event = std::make_shared<TimedEvent>(obj, std::make_unique<CallbackP0<Class>>(obj, method), type, time, repeats, flags);

            // add this to the object's list, updating will all be done later on...
            obj->event_AddEvent(std::move(event));
        }

        template <class Class, typename P1>
        void AddEvent(Class* obj, void (Class::*method)(P1), P1 p1, uint32_t type, time_t time, uint32_t repeats, uint32_t flags)
        {
            // create a timed event
            auto event = std::make_shared<TimedEvent>(obj, std::make_unique<CallbackP1<Class, P1>>(obj, method, p1), type, time, repeats, flags);

            // add this to the object's list, updating will all be done later on...
            obj->event_AddEvent(std::move(event));
        }

        template <class Class, typename P1, typename P2>
        void AddEvent(Class* obj, void (Class::*method)(P1, P2), P1 p1, P2 p2, uint32_t type, time_t time, uint32_t repeats, uint32_t flags)
        {
            // create a timed event
            auto event = std::make_shared<TimedEvent>(obj, std::make_unique<CallbackP2<Class, P1, P2>>(obj, method, p1, p2), type, time, repeats, flags);

            // add this to the object's list, updating will all be done later on...
            obj->event_AddEvent(std::move(event));
        }

        template <class Class, typename P1, typename P2, typename P3>
        void AddEvent(Class* obj, void (Class::*method)(P1, P2, P3), P1 p1, P2 p2, P3 p3, uint32_t type, time_t time, uint32_t repeats, uint32_t flags)
        {
            // create a timed event
            auto event = std::make_shared<TimedEvent>(obj, std::make_unique<CallbackP3<Class, P1, P2, P3>>(obj, method, p1, p2, p3), type, time, repeats, flags);

            // add this to the object's list, updating will all be done later on...
            obj->event_AddEvent(std::move(event));
        }

        template <class Class, typename P1, typename P2, typename P3, typename P4>
        void AddEvent(Class* obj, void (Class::*method)(P1, P2, P3, P4), P1 p1, P2 p2, P3 p3, P4 p4, uint32_t type, time_t time, uint32_t repeats, uint32_t flags)
        {
            // create a timed event
            auto event = std::make_shared<TimedEvent>(obj, std::make_unique<CallbackP4<Class, P1, P2, P3, P4>>(obj, method, p1, p2, p3, p4), type, time, repeats, flags);

            // add this to the object's list, updating will all be done later on...
            obj->event_AddEvent(std::move(event));
        }

        /// \note Please remember the Aura class will call remove events!
        /// Example: Aura::Virtual_Destructor calls: EventableObject::Virtual_Destructor & sEventMgr.RemoveEvents(this);

        template <class Class> void RemoveEvents(Class* obj) { obj->event_RemoveEvents(static_cast<uint32_t>(-1)); }
        template <class Class> void RemoveEvents(Class* obj, int32_t type)
        {
            obj->event_RemoveEvents(type);
        }

        template <class Class> void ModifyEventTimeLeft(Class* obj, uint32_t type, time_t time, bool unconditioned = true)
        {
            obj->event_ModifyTimeLeft(type, time, unconditioned);
        }

        template <class Class> void ModifyEventTimeAndTimeLeft(Class* obj, uint32_t type, time_t time)
        {
            obj->event_ModifyTimeAndTimeLeft(type, time);
        }

        template <class Class> void ModifyEventTime(Class* obj, uint32_t type, time_t time)
        {
            obj->event_ModifyTime(type, time);
        }

        template <class Class> bool HasEvent(Class* obj, uint32_t type)
        {
            return obj->event_HasEvent(type);
        }

        EventableObjectHolder* GetEventHolder(int32_t InstanceId)
        {
            std::lock_guard<std::mutex> guard(holderLock);

            HolderMap::iterator itr = mHolders.find(InstanceId);

            if (itr == mHolders.end())
            {
                return nullptr;
            }

            return itr->second;
        }

        void AddEventHolder(EventableObjectHolder* holder, int32_t InstanceId)
        {
            std::lock_guard<std::mutex> guard(holderLock);

            mHolders.insert(HolderMap::value_type(InstanceId, holder));
        }

        void RemoveEventHolder(int32_t InstanceId)
        {
            std::lock_guard<std::mutex> guard(holderLock);

            mHolders.erase(InstanceId);
        }

        void RemoveEventHolder(EventableObjectHolder* holder)
        {
            std::lock_guard<std::mutex> guard(holderLock);

            HolderMap::iterator itr = mHolders.begin();
            for (; itr != mHolders.end(); ++itr)
            {
                if (itr->second == holder)
                {
                    mHolders.erase(itr);
                    return;
                }
            }
        }

    protected:
        HolderMap mHolders;
        std::mutex holderLock;
};

#define sEventMgr EventMgr::getInstance()

#endif // EVENTMGR_H
