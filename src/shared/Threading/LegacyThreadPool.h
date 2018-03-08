/*
 * Thread Pool Class
 * Copyright (C) Burlex <burlex@gmail.com>
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
#pragma once
#include "../Common.hpp"

#ifndef __THREADPOOL_H
#define __THREADPOOL_H

 // This HAS to be called outside the threads __try / __except block!
void SetThreadName(const char* format, ...);

#ifdef WIN32

typedef struct tagTHREADNAME_INFO
{
    DWORD dwType; // must be 0x1000
    LPCSTR szName; // pointer to name (in user addr space)
    DWORD dwThreadID; // thread ID (-1=caller thread)
    DWORD dwFlags; // reserved for future use, must be zero
} THREADNAME_INFO;

#endif

namespace Arcemu
{
    /////////////////////////////////////////////////////////////////////////
    //void Sleep( unsigned long timems );
    //  Puts the calling thread to sleep for the specified miliseconds
    //
    //Parameter(s)
    //  unsigned long timemes  -  time interval to put the thread to sleep for
    //
    //Return Value
    //  None
    //
    //
    /////////////////////////////////////////////////////////////////////////
    void Sleep(unsigned long timems);
}

#ifdef WIN32

class SERVER_DECL ThreadController
{
    public:
        HANDLE hThread;
        uint32 thread_id;

        void Setup(HANDLE h)
        {
            hThread = h;
            // whoops! GetThreadId is for windows 2003 and up only! :<         - Burlex
            //thread_id = (uint32)GetThreadId(h);
        }

        void Suspend()
        {
            // We can't be suspended by someone else. That is a big-no-no and will lead to crashes.
            ASSERT(GetCurrentThreadId() == thread_id);
            SuspendThread(hThread);
        }

        void Resume()
        {
            // This SHOULD be called by someone else.
            ASSERT(GetCurrentThreadId() != thread_id);
            if(ResumeThread(hThread) == DWORD(-1))
            {
                DWORD le = GetLastError();
                printf("lasterror: %u\n", le);
            }
        }

        void Join()
        {
            WaitForSingleObject(hThread, INFINITE);
        }

        uint32 GetId() { return thread_id; }
};

#else
#ifndef __APPLE__
#include <semaphore.h>
int GenerateThreadId();

class ThreadController
{
        sem_t sem;
        pthread_t handle;
        int thread_id;
    public:
        void Setup(pthread_t h)
        {
            handle = h;
            sem_init(&sem, PTHREAD_PROCESS_PRIVATE, 0);
            thread_id = GenerateThreadId();
        }
        ~ThreadController()
        {
            sem_destroy(&sem);
        }

        void Suspend()
        {
            ASSERT(pthread_equal(pthread_self(), handle));
            sem_wait(&sem);
        }

        void Resume()
        {
            ASSERT(!pthread_equal(pthread_self(), handle));
            sem_post(&sem);
        }

        void Join()
        {
            // waits until the thread finishes then returns
            pthread_join(handle, NULL);
        }

        inline uint32 GetId() { return (uint32)thread_id; }
};

#else
int GenerateThreadId();
class ThreadController
{
        pthread_cond_t cond;
        pthread_mutex_t mutex;
        int thread_id;
        pthread_t handle;
    public:
        void Setup(pthread_t h)
        {
            handle = h;
            pthread_mutex_init(&mutex, NULL);
            pthread_cond_init(&cond, NULL);
            thread_id = GenerateThreadId();
        }
        ~ThreadController()
        {
            pthread_mutex_destroy(&mutex);
            pthread_cond_destroy(&cond);
        }
        void Suspend()
        {
            pthread_cond_wait(&cond, &mutex);
        }
        void Resume()
        {
            pthread_cond_signal(&cond);
        }
        void Join()
        {
            pthread_join(handle, NULL);
        }
        inline uint32 GetId() { return (uint32)thread_id; }
};

#endif

#endif

struct SERVER_DECL Thread
{
    ThreadBase* ExecutionTarget;
    ThreadController ControlInterface;
    Mutex SetupMutex;
    bool DeleteAfterExit;
};

class SERVER_DECL CThreadPool
{
    typedef std::set<Thread*> ThreadSet;
    int GetNumCpus();

    uint32 _threadsRequestedSinceLastCheck;
    uint32 _threadsFreedSinceLastCheck;
    uint32 _threadsExitedSinceLastCheck;
    uint32 _threadsToExit;
    int32 _threadsEaten;
    Mutex _mutex;

    ThreadSet m_activeThreads;
    ThreadSet m_freeThreads;

public:
    CThreadPool();

    // call every 2 minutes or so.
    void IntegrityCheck();

    // call at startup
    void Startup();

    // shutdown all threads
    void Shutdown();

    // return true - suspend ourselves, and wait for a future task.
    // return false - exit, we're shutting down or no longer needed.
    bool ThreadExit(Thread* t);

    // creates a thread, returns a handle to it.
    Thread* StartThread(ThreadBase* ExecutionTarget);

    // grabs/spawns a thread, and tells it to execute a task.
    void ExecuteTask(ThreadBase* ExecutionTarget);

    // prints some neat debug stats
    void ShowStats();

    // kills x free threads
    void KillFreeThreads(uint32 count);

    // resets the gobble counter
    inline void Gobble() { _threadsEaten = (int32)m_freeThreads.size(); }

    // gets active thread count
    inline uint32 GetActiveThreadCount() { return (uint32)m_activeThreads.size(); }

    // gets free thread count
    inline uint32 GetFreeThreadCount() { return (uint32)m_freeThreads.size(); }
};

volatile long Sync_Add(volatile long* value);

volatile long Sync_Sub(volatile long* value);

extern SERVER_DECL CThreadPool ThreadPool;

#endif
