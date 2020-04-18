/*
* AscEmu Framework based on ArcEmu MMORPG Server
* Copyright (c) 2014-2020 AscEmu Team <http://www.ascemu.org>
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
*
*/

#include "StdAfx.h"

#include "WorldConf.h"
#include "Management/AddonMgr.h"
#include "Management/AuctionMgr.h"
#include "Management/CalendarMgr.h"
#include "Management/Item.h"
#include "Management/LFG/LFGMgr.h"
#include "Management/WordFilter.h"
#include "Management/WeatherMgr.h"
#include "Management/TaxiMgr.h"
#include "Management/ItemInterface.h"
#include "Management/Channel.h"
#include "Management/ChannelMgr.h"
#include "WorldSocket.h"
#include "Storage/MySQLDataStore.hpp"
#include "Storage/MySQLStructures.h"
#include <CrashHandler.h>
#include "Server/MainServerDefines.h"
#include "Config/Config.h"
#include "Map/MapCell.h"
#include "Spell/SpellMgr.h"
#include "Map/WorldCreator.h"
#include "Storage/DayWatcherThread.h"
#include "Server/BroadcastMgr.h"
#include "World.Legacy.h"


bool BasicTaskExecutor::run()
{
    /* Set thread priority, this is a bitch for multiplatform :P */
#ifdef WIN32
    switch (priority)
    {
        case BTE_PRIORITY_LOW:
            ::SetThreadPriority(::GetCurrentThread(), THREAD_PRIORITY_LOWEST);
            break;

        case BTW_PRIORITY_HIGH:
            ::SetThreadPriority(::GetCurrentThread(), THREAD_PRIORITY_ABOVE_NORMAL);
            break;

        default: // BTW_PRIORITY_MED
            ::SetThreadPriority(::GetCurrentThread(), THREAD_PRIORITY_NORMAL);
            break;
    }
#else
    struct sched_param param;
    switch (priority)
    {
        case BTE_PRIORITY_LOW:
            param.sched_priority = 0;
            break;

        case BTW_PRIORITY_HIGH:
            param.sched_priority = 10;
            break;

        default:        // BTW_PRIORITY_MED
            param.sched_priority = 5;
            break;
    }
    pthread_setschedparam(pthread_self(), SCHED_OTHER, &param);
#endif

    // Execute the task in our new context.
    cb->execute();
#ifdef WIN32
    ::SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_NORMAL);
#else
    param.sched_priority = 5;
    pthread_setschedparam(pthread_self(), SCHED_OTHER, &param);
#endif

    return true;
}

void TaskList::AddTask(Task* task)
{
    queueLock.Acquire();
    tasks.insert(task);
    queueLock.Release();
}

Task* TaskList::GetTask()
{
    queueLock.Acquire();

    Task* t = 0;

    for (std::set<Task*>::iterator itr = tasks.begin(); itr != tasks.end(); ++itr)
    {
        if (!(*itr)->in_progress)
        {
            t = (*itr);
            t->in_progress = true;
            break;
        }
    }

    queueLock.Release();

    return t;
}

void TaskList::spawn()
{
    running = true;
    thread_count = 0;

    uint32_t threadcount;
    if (worldConfig.startup.enableMultithreadedLoading)
    {
        // get processor count
#ifndef WIN32
#if UNIX_FLAVOUR == UNIX_FLAVOUR_LINUX
#if defined(__x86_64__) || defined(__x86_64) || defined(__amd64__)
        threadcount = 2;
#else
        long affmask;
        sched_getaffinity(0, 4, (cpu_set_t*)&affmask);
        threadcount = (BitCount8(affmask)) * 2;
        if (threadcount > 8) threadcount = 8;
        else if (threadcount <= 0) threadcount = 1;
#endif
#else
        threadcount = 2;
#endif
#else
        SYSTEM_INFO s;
        GetSystemInfo(&s);
        threadcount = s.dwNumberOfProcessors * 2;
        if (threadcount > 8)
            threadcount = 8;
#endif
    }
    else
        threadcount = 1;

    LogNotice("World : Beginning %s server startup with %u threads.", (threadcount == 1) ? "progressive" : "parallel", threadcount);

    for (uint32_t x = 0; x < threadcount; ++x)
        ThreadPool.ExecuteTask(new TaskExecutor(this));
}

void TaskList::wait()
{
    bool has_tasks = true;
    while (has_tasks)
    {
        queueLock.Acquire();
        has_tasks = false;
        for (std::set<Task*>::iterator itr = tasks.begin(); itr != tasks.end(); ++itr)
        {
            if (!(*itr)->completed)
            {
                has_tasks = true;
                break;
            }
        }
        queueLock.Release();
        Arcemu::Sleep(20);
    }
}

void TaskList::kill()
{
    running = false;
}

void Task::execute()
{
    _cb->execute();
}

bool TaskExecutor::runThread()
{
    Task* t;

    THREAD_TRY_EXECUTION
        while (starter->running)
        {
            t = starter->GetTask();
            if (t)
            {
                t->execute();
                t->completed = true;
                starter->RemoveTask(t);
                delete t;
            }
            else
                Arcemu::Sleep(20);
        }

    THREAD_HANDLE_CRASH

        return true;
}

void TaskList::waitForThreadsToExit()
{
    while (thread_count)
    {
        Arcemu::Sleep(20);
    }
}

struct insert_playeritem
{
    uint32_t ownerguid;
    uint32_t entry;
    uint32_t wrapped_item_id;
    uint32_t wrapped_creator;
    uint32_t creator;
    uint32_t count;
    uint32_t charges;
    uint32_t flags;
    uint32_t randomprop;
    uint32_t randomsuffix;
    uint32_t itemtext;
    uint32_t durability;
    int32_t containerslot;
    int32_t slot;
    std::string enchantments;
};

#define LOAD_THREAD_SLEEP 180

void CharacterLoaderThread::onShutdown()
{
    running = false;
    cond.Signal();
}

CharacterLoaderThread::CharacterLoaderThread()
{
    running = false;
}

CharacterLoaderThread::~CharacterLoaderThread()
{
}

bool CharacterLoaderThread::runThread()
{
    running = true;
    for (;;)
    {
        // Get a single connection to maintain for the whole process.
        DatabaseConnection* con = CharacterDatabase.GetFreeConnection();

        con->Busy.Release();

        cond.Wait(LOAD_THREAD_SLEEP * 1000);

        if (!running)
            break;
    }

    return true;
}
