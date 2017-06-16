/*
* AscEmu Framework based on ArcEmu MMORPG Server
* Copyright (c) 2014-2017 AscEmu Team <http://www.ascemu.org/>
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

#pragma once

#include "EventableObject.h"
#include "IUpdatable.h"
#include "Definitions.h"
#include "Storage/DBC/DBCStores.h"
#include "WorldSession.h"
#include "WorldConfig.h"

#include <set>
#include <string>
#include <vector>

#define IS_INSTANCE(a) ((a > 1) && (a != 530) && (a != 571))

class Object;
class WorldPacket;
class WorldSession;
class Unit;
class Creature;
class GameObject;
class DynamicObject;
class Player;
class EventableObjectHolder;
class MapMgr;
class Battleground;
struct DatabaseConnection;

enum EnviromentalDamage
{
    DAMAGE_EXHAUSTED = 0,
    DAMAGE_DROWNING = 1,
    DAMAGE_FALL = 2,
    DAMAGE_LAVA = 3,
    DAMAGE_SLIME = 4,
    DAMAGE_FIRE = 5
};

// ServerMessages.dbc
enum ServerMessageType
{
    SERVER_MSG_SHUTDOWN_TIME = 1,
    SERVER_MSG_RESTART_TIME = 2,
    SERVER_MSG_STRING = 3,
    SERVER_MSG_SHUTDOWN_CANCELLED = 4,
    SERVER_MSG_RESTART_CANCELLED = 5,
    SERVER_MSG_BATTLEGROUND_SHUTDOWN = 6,
    SERVER_MSG_BATTLEGROUND_RESTART = 7,
    SERVER_MSG_INSTANCE_SHUTDOWN = 8,
    SERVER_MSG_INSTANCE_RESTART = 9
};

enum WorldMapInfoFlag
{
    WMI_INSTANCE_ENABLED = 0x001,
    WMI_INSTANCE_WELCOME = 0x002,
    WMI_INSTANCE_ARENA = 0x004,
    WMI_INSTANCE_XPACK_01 = 0x008, //The Burning Crusade expansion
    WMI_INSTANCE_XPACK_02 = 0x010, //Wrath of the Lich King expansion
    WMI_INSTANCE_HAS_NORMAL_10MEN = 0x020,
    WMI_INSTANCE_HAS_NORMAL_25MEN = 0x040,
    WMI_INSTANCE_HAS_HEROIC_10MEN = 0x080,
    WMI_INSTANCE_HAS_HEROIC_25MEN = 0x100
};

enum AccountFlags
{
    ACCOUNT_FLAG_VIP = 0x1,
    ACCOUNT_FLAG_NO_AUTOJOIN = 0x2,
    //ACCOUNT_FLAG_XTEND_INFO   = 0x4,
    ACCOUNT_FLAG_XPACK_01 = 0x8,
    ACCOUNT_FLAG_XPACK_02 = 0x10,
    ACCOUNT_FLAG_XPACK_03 = 0x20
};

#pragma pack(push,1)
struct MapInfo
{
    uint32 mapid;
    uint32 screenid;
    uint32 type;
    uint32 playerlimit;
    uint32 minlevel;
    uint32 minlevel_heroic;
    float repopx;
    float repopy;
    float repopz;
    uint32 repopmapid;
    std::string name;
    uint32 flags;
    uint32 cooldown;
    uint32 lvl_mod_a;
    uint32 required_quest_A;
    uint32 required_quest_H;
    uint32 required_item;
    uint32 heroic_key_1;
    uint32 heroic_key_2;
    float update_distance;
    uint32 checkpoint_id;

    //////////////////////////////////////////////////////////////////////////////////////////
    /// Tells if the map has this particular flag
    /// \param  uint32 flag  -  flag to check
    /// \return true if the map has the flag, otherwise false if the map doesn't have the flag.
    //////////////////////////////////////////////////////////////////////////////////////////
    bool HasFlag(uint32 flag) const
    {
        if ((flags & flag) != 0)
            return true;
        else
            return false;
    }

    //////////////////////////////////////////////////////////////////////////////////////////
    /// Tells if the map has a particular raid difficulty.
    /// Valid difficulties are in the RAID_MODE enum.
    /// \param    uint32 difficulty  -  difficulty to check
    /// \return   true if the map has this difficulty, otherwise false.
    //////////////////////////////////////////////////////////////////////////////////////////
    bool HasDifficulty(uint32 difficulty) const
    {
        if (difficulty > uint32(TOTAL_RAID_MODES))
            return false;

        return HasFlag(uint32(WMI_INSTANCE_HAS_NORMAL_10MEN) << difficulty);
    }
};

#pragma pack(pop)


class BasicTaskExecutor : public ThreadBase
{
    CallbackBase* cb;
    uint32 priority;

public:

    BasicTaskExecutor(CallbackBase* Callback, uint32 Priority) : cb(Callback), priority(Priority)
    {}
    ~BasicTaskExecutor()
    {
        delete cb;
    }
    bool run();
};

class Task
{
    CallbackBase* _cb;

public:

    Task(CallbackBase* cb) : _cb(cb), completed(false), in_progress(false)
    {}
    ~Task()
    {
        delete _cb;
    }
    bool completed;
    bool in_progress;
    void execute();
};

struct CharacterLoaderThread : public ThreadBase
{
    Arcemu::Threading::ConditionVariable cond;

    bool running;

public:

    CharacterLoaderThread();
    ~CharacterLoaderThread();
    void onShutdown();
    bool runThread();
};

class TaskList
{
    std::set<Task*> tasks;
    Mutex queueLock;

public:

    TaskList() : thread_count(0), running(false)
    {};
    Task* GetTask();
    void AddTask(Task* task);
    void RemoveTask(Task* task)
    {
        queueLock.Acquire();
        tasks.erase(task);
        queueLock.Release();
    }

    void spawn();
    void kill();

    void wait();
    void waitForThreadsToExit();
    Arcemu::Threading::AtomicCounter thread_count;
    bool running;
};

enum BasicTaskExecutorPriorities
{
    BTE_PRIORITY_LOW = 0,
    BTE_PRIORITY_MED = 1,
    BTW_PRIORITY_HIGH = 2
};

class TaskExecutor : public ThreadBase
{
    TaskList* starter;

public:

    TaskExecutor(TaskList* l) : starter(l)
    {
        ++l->thread_count;
    }
    ~TaskExecutor()
    {
        --starter->thread_count;
    }

    bool runThread();
};

class WorldSocket;

typedef std::set<WorldSession*> SessionSet;
