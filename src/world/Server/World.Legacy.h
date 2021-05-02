/*
* AscEmu Framework based on ArcEmu MMORPG Server
* Copyright (c) 2014-2021 AscEmu Team <http://www.ascemu.org>
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

//MIT start
// Values based on ServerMessages.dbc
enum ServerMessageType
{
    // Vanilla
    SERVER_MSG_SHUTDOWN_TIME = 1,                               // [SERVER] Shutdown in %s
    SERVER_MSG_RESTART_TIME = 2,                                // [SERVER] Restart in %s
    SERVER_MSG_STRING = 3,                                      // %s
    SERVER_MSG_SHUTDOWN_CANCELLED = 4,                          // [SERVER] Shutdown cancelled
    SERVER_MSG_RESTART_CANCELLED = 5,                           // [SERVER] Restart cancelled
    // TBC
    SERVER_MSG_BATTLEGROUND_SHUTDOWN = 6,                       // [SERVER] Battleground shutdown in %s
    SERVER_MSG_BATTLEGROUND_RESTART = 7,                        // [SERVER] Battleground restart in %s
    SERVER_MSG_INSTANCE_SHUTDOWN = 8,                           // [SERVER] Instance shutdown in %s
    SERVER_MSG_INSTANCE_RESTART = 9,                            // [SERVER] Instance restart in %s
    // Cataclysm
    SERVER_MSG_CATACLYSM_CONTENT_AVAILABLE = 10,                // Cataclysm content is now available. Please completely quit and restart World of Warcraft, then enjoy the game.
    SERVER_MSG_TICKET_WILL_BE_SERVICED_SOON = 11,               // Your ticket will be serviced soon.
    SERVER_MSG_WAIT_TIME_CURRENTLY_UNAVAILABLE = 12,            // Wait time currently unavailable.
    SERVER_MSG_AVERAGE_TICKET_WAIT_TIME = 13,                   // Average ticket wait time:\n %s
    // MOP
    SERVER_MSG_MOP_HAS_LAUNCHED = 14,                           // Mists of Pandaria has launched!
    SERVER_MSG_MOP_HAS_LAUNCHED_VISIT_ORGRIMMAR = 15,           // Mists of Pandaria has launched! Visit Orgrimmar to begin your adventure!
    SERVER_MSG_MOP_HAS_LAUNCHED_VISIT_STORMWIND = 16,           // Mists of Pandaria has launched! Visit Stormwind to begin your adventure!
    SERVER_MSG_CROSS_REALM_SHUTDOWN = 17,                       // [SERVER] Cross realm shutdown in %s
    SERVER_MSG_CROSS_REALM_RESTART = 18,                        // [SERVER] Cross realm restart in %s
};

// Values based on ...
enum AccountFlags
{
    ACCOUNT_FLAG_TOURNAMENT     = 0x01,
    ACCOUNT_FLAG_NO_AUTOJOIN    = 0x02,
    //ACCOUNT_FLAG_XTEND_INFO   = 0x04,
    ACCOUNT_FLAG_XPACK_01       = 0x08,        // The Burning Crusade
    ACCOUNT_FLAG_XPACK_02       = 0x10,       // Wrath of the Lich King
    ACCOUNT_FLAG_XPACK_03       = 0x20,       // Cataclysm
    ACCOUNT_FLAG_XPACK_04       = 0x40,       // Mists of Pandaria

    AF_FULL_WOTLK               = ACCOUNT_FLAG_XPACK_01 | ACCOUNT_FLAG_XPACK_02,
    AF_FULL_CATA                = AF_FULL_WOTLK | ACCOUNT_FLAG_XPACK_03,
    AF_FULL_MOP                 = AF_FULL_CATA | ACCOUNT_FLAG_XPACK_04
};
//MIT end 

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
    std::atomic<unsigned long> thread_count;
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
