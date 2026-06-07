/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "Thread.hpp"

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <cstddef>
#include <cstdint>
#include <deque>
#include <functional>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

namespace AscEmu::Threading
{
    static void sleep(unsigned long timems)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(timems));
    }

    class AEThreadPool
    {
    public:
        using TaskFunc = std::function<void()>;
        using DedicatedThreadFunc = AEThread::ThreadFunc;
        using Clock = std::chrono::steady_clock;
        using TimePoint = Clock::time_point;

        AEThreadPool(std::string poolName, uint16_t minimumThreadCount, uint16_t softMaximumThreadCount, uint16_t hardMaximumThreadCount);

        AEThreadPool(std::string poolName, uint16_t minimumThreadCount, uint16_t softMaximumThreadCount, uint16_t hardMaximumThreadCount,
            std::chrono::milliseconds pulseFrequency);

        ~AEThreadPool();

        AEThreadPool(const AEThreadPool&) = delete;
        AEThreadPool& operator=(const AEThreadPool&) = delete;

        AEThreadPool(AEThreadPool&&) = delete;
        AEThreadPool& operator=(AEThreadPool&&) = delete;

        void start();
        void shutdown();
        void join();

        void addTask(TaskFunc task, std::string taskName = {});
        void addRecurringTask(TaskFunc task, std::chrono::milliseconds repeatDelay, std::string taskName = {});
        void addHighPriorityTask(TaskFunc task, std::string taskName = {});
        AEThread& addDedicatedThread(std::string threadName, DedicatedThreadFunc threadFunc, std::chrono::milliseconds intervalMs = std::chrono::milliseconds(0),
            bool autostart = true);

        void queueRecurringTask(TaskFunc task, std::chrono::milliseconds repeatDelay, std::string taskName);
        void queueFireOnceTask(TaskFunc task, std::string taskName);
        void queueHighPriorityTask(TaskFunc task, const std::string& taskName);

        [[nodiscard]] bool isStarted() const noexcept;
        [[nodiscard]] bool isShutdownRequested() const noexcept;
        [[nodiscard]] size_t activeWorkerCount() const noexcept;
        [[nodiscard]] size_t idleWorkerCount() const;
        [[nodiscard]] size_t queuedTaskCount() const;
        [[nodiscard]] size_t workerCount() const;
        [[nodiscard]] size_t dedicatedThreadCount() const;
        [[nodiscard]] uint64_t completedTaskCount() const noexcept;

        bool hasPendingTasks() const;

        static std::unique_ptr<AEThreadPool>& globalThreadPool();

    private:
        struct QueuedTask
        {
            TaskFunc task;
            std::string name;
            std::chrono::milliseconds repeatDelay{ -1 };
            TimePoint nextRun{};
            bool recurring = false;
            bool highPriority = false;
        };

        void queueTask(TaskFunc task, std::chrono::milliseconds repeatDelay, std::string taskName, bool highPriority);
        void workerLoop(size_t workerIndex);

        [[nodiscard]] bool queuesEmptyLocked() const;
        [[nodiscard]] TimePoint nextTaskTimeLocked() const;
        [[nodiscard]] bool tryPopRunnableTaskFromQueueLocked(std::deque<QueuedTask>& queue, TimePoint now, QueuedTask& outTask);
        [[nodiscard]] bool tryPopRunnableTaskLocked(TimePoint now, QueuedTask& outTask);

        void requeueRecurringTask(QueuedTask task);
        void spawnWorkersLocked(size_t desiredWorkerCount);
        void ensureWorkerCapacityLocked(bool highPriority);

    private:
        mutable std::mutex m_mutex;
        std::condition_variable m_condition;

        std::deque<QueuedTask> m_highPriorityTasks;
        std::deque<QueuedTask> m_tasks;
        std::vector<std::thread> m_workers;
        std::vector<std::unique_ptr<AEThread>> m_dedicatedThreads;

        std::string m_poolName;
        std::chrono::milliseconds m_pulseFrequency;

        std::atomic_bool m_started{ false };
        std::atomic_bool m_shutdownRequested{ false };
        std::atomic_uint64_t m_completedTaskCount{ 0 };
        std::atomic_size_t m_activeWorkerCount{ 0 };

        uint16_t m_minimumThreadCount = 1;
        uint16_t m_softMaximumThreadCount = 1;
        uint16_t m_hardMaximumThreadCount = 1;
        size_t m_workerIdCounter = 0;

    private:
        static std::unique_ptr<AEThreadPool> s_global_thread_pool;
    };
}
