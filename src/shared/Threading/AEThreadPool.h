/*
Copyright (c) 2014-2018 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once
#include <functional>
#include <list>
#include <map>
#include <memory>
#include <vector>

#include "AEThread.h"

namespace AscEmu { namespace Threading
{
    class AEThreadPool
    {
        typedef std::function<void(AEThread&)> ThreadFunc;
        typedef std::chrono::time_point<std::chrono::steady_clock> TimePoint;

        struct Task
        {
            ThreadFunc m_task;
            std::string m_name;
            std::chrono::milliseconds m_repeat;
            TimePoint m_lastPulse;
            bool m_done;
            bool m_running;

            Task(ThreadFunc task, std::string name, std::chrono::milliseconds repeat) :
                m_task(task),
                m_name(name),
                m_repeat(repeat),
                m_lastPulse(std::chrono::milliseconds(0)),
                m_done(false),
                m_running(false) {
            }

            bool isFireOnceTask() const { return m_repeat < std::chrono::milliseconds(0); }
            bool canFire(TimePoint point) const { return !m_done && m_lastPulse + m_repeat < point; }
        };

        std::mutex m_mtx;
        std::list<std::unique_ptr<AEThread>> m_threads;
        std::list<std::unique_ptr<Task>> m_tasks;
        std::list<std::unique_ptr<Task>> m_highPriorityTasks;
        std::string m_poolName;
        std::chrono::milliseconds m_pulseFrequency;
        std::unique_ptr<AEThread> m_pulseThread;

        std::atomic<uint16_t> m_threadCount;
        std::atomic<uint32_t> m_totalThreadsEverCreated;
        uint16_t m_minimumThreadCount;
        uint16_t m_softMaximumThreadCount;
        uint16_t m_hardMaximumThreadCount;

        AEThread* spawnThread();

        bool m_shutdownRequested;

        void queueTask(ThreadFunc task, std::chrono::milliseconds repeatDelay, std::string taskName, bool highPriority = false);
        void pulse(AEThread& thread);
    public:
        void queueRecurringTask(ThreadFunc task, std::chrono::milliseconds repeatDelay, std::string taskName);
        void queueFireOnceTask(ThreadFunc task, std::string taskName);
        void queueHighPriorityTask(ThreadFunc task, std::string taskName);

        AEThreadPool(std::string poolName, uint16_t minimumThreadCount, uint16_t softMaximumThreadCount, uint16_t hardMaximumThreadCount);
        AEThreadPool(std::string poolName, uint16_t minimumThreadCount, uint16_t softMaximumThreadCount, uint16_t hardMaximumThreadCount, std::chrono::milliseconds pulseFrequency);

#pragma region Singleton
    private:
        static std::unique_ptr<AEThreadPool> s_global_thread_pool;
    public:
        static std::unique_ptr<AEThreadPool>& globalThreadPool();
    private:
#pragma endregion
    };
}}
