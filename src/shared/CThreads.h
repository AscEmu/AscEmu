/*
 * AscEmu Framework based on ArcEmu MMORPG Server
 * Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
 * Copyright (C) 2008-2012 ArcEmu Team <http://www.ArcEmu.org/>
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

//\brief Class CThread - Base class for all threads in the server, and allows for easy management by ThreadMgr.

#pragma once

#include "Threading/LegacyThreadBase.h"
#include "CommonTypes.hpp"
#include <atomic>
#include <ctime>

class MapMgr;
class Player;
class WorldSession;
class Creature;
class GameObject;

enum CThreadState
{
    THREADSTATE_TERMINATE = 0,
    THREADSTATE_PAUSED    = 1,
    THREADSTATE_SLEEPING  = 2,
    THREADSTATE_BUSY      = 3,
    THREADSTATE_AWAITING  = 4,
};

struct NameTableEntry;

class SERVER_DECL CThread : public ThreadBase
{
public:
    CThread();
    ~CThread() override;

    void SetThreadState(CThreadState thread_state)
    {
        ThreadState.store(thread_state);
    }

    CThreadState GetThreadState() const
    {
        return ThreadState.load();
    }

    int GetThreadId() { return ThreadId; }
    time_t GetStartTime() { return start_time; }

    bool runThread() override
    {
        return run();
    }

    virtual bool run();
    void onShutdown() override;

protected:
    CThread& operator=(const CThread& other)
    {
        start_time = other.start_time;
        ThreadId = other.ThreadId;
        ThreadState.store(other.ThreadState.load());
        return *this;
    }

    std::atomic<CThreadState> ThreadState{ THREADSTATE_AWAITING };
    time_t start_time = 0;
    int ThreadId = 0;
};
