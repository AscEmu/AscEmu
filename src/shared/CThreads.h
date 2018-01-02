/*
 * AscEmu Framework based on ArcEmu MMORPG Server
 * Copyright (c) 2014-2018 AscEmu Team <http://www.ascemu.org>
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

#ifndef _C_THREADS_H
#define _C_THREADS_H

#include "Threading/LegacyThreadBase.h"
#include <atomic>

class MapMgr;
class Player;
class WorldSession;
class Creature;
class GameObject;

#define MAPMGR_UPDATEOBJECT_LOOP_DELAY 100
#define MAPMGR_SESSION_UPDATE_DELAY    50

#define MAPMGR_UPDATE_DELAY            100

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
        ~CThread();

        inline void SetThreadState(CThreadState thread_state) { ThreadState = thread_state; }
        inline CThreadState GetThreadState()
        {
            unsigned long val = ThreadState;
            return static_cast<CThreadState>(val);
        }
        int GetThreadId() { return ThreadId; }
        time_t GetStartTime() { return start_time; }
        virtual bool run();
        virtual void onShutdown();

    protected:

        CThread & operator=(CThread & other)
        {
            this->start_time = other.start_time;
            this->ThreadId = other.ThreadId;
            this->ThreadState = other.ThreadState.load();
            return *this;
        }

        std::atomic<unsigned long> ThreadState;
        time_t start_time;
        int ThreadId;
};

#endif  //_C_THREADS_H
