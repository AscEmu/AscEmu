/*
Copyright (c) 2014-2026 AscEmu Team
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

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
    class AEThreadPool
    {
    public:
        using ThreadFunc = std::function<void()>;
        using Clock = std::chrono::steady_clock;
        using TimePoint = Clock::time_point;

        AEThreadPool(
            std::string poolName,
            uint16_t minimumThreadCount,
            uint16_t softMaximumThreadCount,
            uint16_t hardMaximumThreadCount
        );

        AEThreadPool(
            std::string poolName,
            uint16_t minimumThreadCount,
            uint16_t softMaximumThreadCount,
            uint16_t hardMaximumThreadCount,
            std::chrono::milliseconds pulseFrequency
        );

        ~AEThreadPool();

        AEThreadPool(const AEThreadPool&) = delete;
        AEThreadPool& operator=(const AEThreadPool&) = delete;

        AEThreadPool(AEThreadPool&&) = delete;
        AEThreadPool& operator=(AEThreadPool&&) = delete;

        void start();
        void shutdown();
        void join();

        void queueRecurringTask(ThreadFunc task, std::chrono::milliseconds repeatDelay, std::string taskName);
        void queueFireOnceTask(ThreadFunc task, std::string taskName);
        void queueHighPriorityTask(ThreadFunc task, const std::string& taskName);

        [[nodiscard]] bool isStarted() const noexcept;
        [[nodiscard]] bool isShutdownRequested() const noexcept;
        [[nodiscard]] size_t queuedTaskCount() const;
        [[nodiscard]] size_t workerCount() const noexcept;
        [[nodiscard]] uint64_t completedTaskCount() const noexcept;

        bool hasPendingTasks() const;

        static std::unique_ptr<AEThreadPool>& globalThreadPool();

    private:
        struct QueuedTask
        {
            ThreadFunc task;
            std::string name;
            std::chrono::milliseconds repeatDelay{ -1 };
            TimePoint nextRun{};
            bool recurring = false;
        };

        void queueTask(ThreadFunc task, std::chrono::milliseconds repeatDelay, std::string taskName, bool highPriority);
        void workerLoop(size_t workerIndex);

        [[nodiscard]] TimePoint nextTaskTimeLocked() const;
        [[nodiscard]] bool tryPopRunnableTaskLocked(TimePoint now, QueuedTask& outTask);

        void requeueRecurringTask(QueuedTask task);

    private:
        mutable std::mutex m_mutex;
        std::condition_variable m_condition;

        std::deque<QueuedTask> m_tasks;
        std::vector<std::thread> m_workers;

        std::string m_poolName;
        std::chrono::milliseconds m_pulseFrequency;

        std::atomic_bool m_started{ false };
        std::atomic_bool m_shutdownRequested{ false };
        std::atomic_uint64_t m_completedTaskCount{ 0 };

        uint16_t m_minimumThreadCount = 1;
        uint16_t m_softMaximumThreadCount = 1;
        uint16_t m_hardMaximumThreadCount = 1;

    private:
        static std::unique_ptr<AEThreadPool> s_global_thread_pool;
    };
}
