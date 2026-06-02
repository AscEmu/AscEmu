/*
Copyright (c) 2014-2026 AscEmu Team
This file is released under the MIT license. See README-MIT for more information.
*/

#include "AEThreadPool.h"

#include <algorithm>
#include <exception>
#include <utility>

namespace AscEmu::Threading
{
    std::unique_ptr<AEThreadPool> AEThreadPool::s_global_thread_pool;

    std::unique_ptr<AEThreadPool>& AEThreadPool::globalThreadPool()
    {
        constexpr uint16_t minimumThreadCount = 10;
        constexpr uint16_t softMaximumThreadCount = 32;
        constexpr uint16_t hardMaximumThreadCount = 64;

        if (s_global_thread_pool == nullptr)
        {
            s_global_thread_pool = std::make_unique<AEThreadPool>("Global Pool",
                minimumThreadCount, softMaximumThreadCount, hardMaximumThreadCount);

            s_global_thread_pool->start();
        }

        return s_global_thread_pool;
    }

    AEThreadPool::AEThreadPool(std::string poolName,
        uint16_t minimumThreadCount, uint16_t softMaximumThreadCount, uint16_t hardMaximumThreadCount
    )
        : AEThreadPool( std::move(poolName), minimumThreadCount, softMaximumThreadCount, hardMaximumThreadCount,
            std::chrono::milliseconds(64)
        )
    {
    }

    AEThreadPool::AEThreadPool( std::string poolName,
        uint16_t minimumThreadCount, uint16_t softMaximumThreadCount, uint16_t hardMaximumThreadCount,
        std::chrono::milliseconds pulseFrequency
    )
        : m_poolName(std::move(poolName)),
          m_pulseFrequency(pulseFrequency),
          m_minimumThreadCount(std::max<uint16_t>(1, minimumThreadCount)),
          m_softMaximumThreadCount(std::max<uint16_t>(minimumThreadCount, softMaximumThreadCount)),
          m_hardMaximumThreadCount(std::max<uint16_t>(softMaximumThreadCount, hardMaximumThreadCount))
    {
    }

    AEThreadPool::~AEThreadPool()
    {
        shutdown();
        join();
    }

    void AEThreadPool::start()
    {
        bool expected = false;

        if (!m_started.compare_exchange_strong(expected, true))
            return;

        m_shutdownRequested.store(false);

        const auto threadCount = static_cast<size_t>(std::max<uint16_t>(1, m_softMaximumThreadCount));

        m_workers.reserve(threadCount);

        for (size_t i = 0; i < threadCount; ++i)
        {
            m_workers.emplace_back([this, i]
            {
                workerLoop(i);
            });
        }
    }

    void AEThreadPool::shutdown()
    {
        {
            std::lock_guard lock(m_mutex);
            m_shutdownRequested.store(true);
        }

        m_condition.notify_all();
    }

    void AEThreadPool::join()
    {
        for (auto& worker : m_workers)
        {
            if (worker.joinable())
                worker.join();
        }

        m_workers.clear();
        m_started.store(false);
    }

    void AEThreadPool::queueFireOnceTask(ThreadFunc task, std::string taskName)
    {
        queueTask(std::move(task), std::chrono::milliseconds(-1), std::move(taskName), false);
    }

    void AEThreadPool::queueHighPriorityTask(ThreadFunc task, const std::string& taskName)
    {
        queueTask(std::move(task), std::chrono::milliseconds(-1), taskName, true);
    }

    void AEThreadPool::queueRecurringTask(ThreadFunc task, std::chrono::milliseconds repeatDelay, std::string taskName)
    {
        if (repeatDelay <= std::chrono::milliseconds(0))
            repeatDelay = m_pulseFrequency;

        queueTask(std::move(task), repeatDelay, std::move(taskName), false);
    }

    void AEThreadPool::queueTask(ThreadFunc task, std::chrono::milliseconds repeatDelay,
        std::string taskName, bool highPriority)
    {
        if (!task)
            return;

        if (!m_started.load())
            start();

        QueuedTask queuedTask;
        queuedTask.task = std::move(task);
        queuedTask.name = std::move(taskName);
        queuedTask.repeatDelay = repeatDelay;
        queuedTask.recurring = repeatDelay >= std::chrono::milliseconds(0);
        queuedTask.nextRun = Clock::now();

        {
            std::lock_guard lock(m_mutex);

            if (m_shutdownRequested.load())
                return;

            if (highPriority)
                m_tasks.emplace_front(std::move(queuedTask));
            else
                m_tasks.emplace_back(std::move(queuedTask));
        }

        m_condition.notify_one();
    }

    bool AEThreadPool::isStarted() const noexcept
    {
        return m_started.load();
    }

    bool AEThreadPool::isShutdownRequested() const noexcept
    {
        return m_shutdownRequested.load();
    }

    size_t AEThreadPool::queuedTaskCount() const
    {
        std::lock_guard lock(m_mutex);
        return m_tasks.size();
    }

    size_t AEThreadPool::workerCount() const noexcept
    {
        return m_workers.size();
    }

    uint64_t AEThreadPool::completedTaskCount() const noexcept
    {
        return m_completedTaskCount.load();
    }

    bool AEThreadPool::hasPendingTasks() const
    {
        std::lock_guard lock(m_mutex);
        return !m_tasks.empty();
    }

    AEThreadPool::TimePoint AEThreadPool::nextTaskTimeLocked() const
    {
        TimePoint nextTime = TimePoint::max();

        for (const auto& task : m_tasks)
        {
            if (task.nextRun < nextTime)
                nextTime = task.nextRun;
        }

        return nextTime;
    }

    bool AEThreadPool::tryPopRunnableTaskLocked(TimePoint now, QueuedTask& outTask)
    {
        const auto itr = std::find_if(
            m_tasks.begin(),
            m_tasks.end(),
            [now](const QueuedTask& task)
            {
                return task.nextRun <= now;
            }
        );

        if (itr == m_tasks.end())
            return false;

        outTask = std::move(*itr);
        m_tasks.erase(itr);
        return true;
    }

    void AEThreadPool::requeueRecurringTask(QueuedTask task)
    {
        task.nextRun = Clock::now() + task.repeatDelay;

        {
            std::lock_guard lock(m_mutex);

            if (m_shutdownRequested.load())
                return;

            m_tasks.emplace_back(std::move(task));
        }

        m_condition.notify_one();
    }

    void AEThreadPool::workerLoop(size_t workerIndex)
    {
        (void)workerIndex;

        for (;;)
        {
            QueuedTask task;

            {
                std::unique_lock lock(m_mutex);

                for (;;)
                {
                    if (m_shutdownRequested.load() && m_tasks.empty())
                        return;

                    const auto now = Clock::now();

                    if (tryPopRunnableTaskLocked(now, task))
                        break;

                    if (m_shutdownRequested.load())
                        return;

                    const auto nextRun = nextTaskTimeLocked();

                    if (nextRun == TimePoint::max())
                    {
                        m_condition.wait(lock, [this]
                        {
                            return m_shutdownRequested.load() || !m_tasks.empty();
                        });
                    }
                    else
                    {
                        m_condition.wait_until(lock, nextRun, [this]
                        {
                            return m_shutdownRequested.load();
                        });
                    }
                }
            }

            try
            {
                task.task();
                m_completedTaskCount.fetch_add(1);
            }
            catch (...)
            {
                // Keep the worker alive even if a task throws.
                // Logging can be wired in later if desired.
            }

            if (task.recurring && !m_shutdownRequested.load())
                requeueRecurringTask(std::move(task));
        }
    }
}
