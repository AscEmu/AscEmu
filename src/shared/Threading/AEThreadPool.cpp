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
        uint16_t minimumThreadCount, uint16_t softMaximumThreadCount, uint16_t hardMaximumThreadCount)
        : AEThreadPool(std::move(poolName), minimumThreadCount, softMaximumThreadCount, hardMaximumThreadCount,
            std::chrono::milliseconds(64)
        )
    {}

    AEThreadPool::AEThreadPool(std::string poolName,
        uint16_t minimumThreadCount, uint16_t softMaximumThreadCount, uint16_t hardMaximumThreadCount,
        std::chrono::milliseconds pulseFrequency
    )
        : m_poolName(std::move(poolName)),
        m_pulseFrequency(std::max(pulseFrequency, std::chrono::milliseconds(1))),
        m_minimumThreadCount(std::max<uint16_t>(1, minimumThreadCount)),
        m_softMaximumThreadCount(std::max<uint16_t>(std::max<uint16_t>(1, minimumThreadCount), softMaximumThreadCount)),
        m_hardMaximumThreadCount(std::max<uint16_t>(std::max<uint16_t>(std::max<uint16_t>(1, minimumThreadCount), softMaximumThreadCount), hardMaximumThreadCount))
    {}

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

        m_shutdownRequested.store(false, std::memory_order_release);

        std::lock_guard lock(m_mutex);
        spawnWorkersLocked(static_cast<size_t>(m_minimumThreadCount));
    }

    void AEThreadPool::shutdown()
    {
        m_shutdownRequested.store(true, std::memory_order_release);

        {
            std::lock_guard lock(m_mutex);
            for (const auto& dedicatedThread : m_dedicatedThreads)
            {
                if (dedicatedThread != nullptr)
                    dedicatedThread->requestKill();
            }
        }

        m_condition.notify_all();
    }

    void AEThreadPool::join()
    {
        std::vector<std::thread> workersToJoin;
        std::vector<AEThread*> dedicatedThreadsToJoin;

        {
            std::lock_guard lock(m_mutex);
            workersToJoin.swap(m_workers);

            dedicatedThreadsToJoin.reserve(m_dedicatedThreads.size());
            for (const auto& dedicatedThread : m_dedicatedThreads)
            {
                if (dedicatedThread != nullptr)
                    dedicatedThreadsToJoin.push_back(dedicatedThread.get());
            }
        }

        for (AEThread* dedicatedThread : dedicatedThreadsToJoin)
            dedicatedThread->join();

        for (auto& worker : workersToJoin)
        {
            if (worker.joinable())
                worker.join();
        }

        m_started.store(false, std::memory_order_release);
    }

    void AEThreadPool::addTask(TaskFunc task, std::string taskName)
    {
        queueTask(std::move(task), std::chrono::milliseconds(-1), std::move(taskName), false);
    }

    void AEThreadPool::addRecurringTask(TaskFunc task, std::chrono::milliseconds repeatDelay, std::string taskName)
    {
        if (repeatDelay <= std::chrono::milliseconds::zero())
            repeatDelay = m_pulseFrequency;

        queueTask(std::move(task), repeatDelay, std::move(taskName), false);
    }

    void AEThreadPool::addHighPriorityTask(TaskFunc task, std::string taskName)
    {
        queueTask(std::move(task), std::chrono::milliseconds(-1), std::move(taskName), true);
    }

    AEThread& AEThreadPool::addDedicatedThread(std::string threadName, DedicatedThreadFunc threadFunc,
        std::chrono::milliseconds intervalMs, bool autostart)
    {
        auto dedicatedThread = std::make_unique<AEThread>(std::move(threadName));
        AEThread* threadPtr = dedicatedThread.get();

        {
            std::lock_guard lock(m_mutex);
            m_dedicatedThreads.emplace_back(std::move(dedicatedThread));
        }

        const bool shouldAutostart = autostart && !m_shutdownRequested.load(std::memory_order_acquire);

        if (intervalMs > std::chrono::milliseconds::zero())
            threadPtr->addLoop(std::move(threadFunc), intervalMs, shouldAutostart);
        else
            threadPtr->addWork(std::move(threadFunc), shouldAutostart);

        return *threadPtr;
    }

    void AEThreadPool::queueRecurringTask(TaskFunc task, std::chrono::milliseconds repeatDelay, std::string taskName)
    {
        addRecurringTask(std::move(task), repeatDelay, std::move(taskName));
    }

    void AEThreadPool::queueFireOnceTask(TaskFunc task, std::string taskName)
    {
        addTask(std::move(task), std::move(taskName));
    }

    void AEThreadPool::queueHighPriorityTask(TaskFunc task, const std::string& taskName)
    {
        addHighPriorityTask(std::move(task), taskName);
    }

    bool AEThreadPool::isStarted() const noexcept
    {
        return m_started.load(std::memory_order_acquire);
    }

    bool AEThreadPool::isShutdownRequested() const noexcept
    {
        return m_shutdownRequested.load(std::memory_order_acquire);
    }

    size_t AEThreadPool::queuedTaskCount() const
    {
        std::lock_guard lock(m_mutex);
        return m_highPriorityTasks.size() + m_tasks.size();
    }

    size_t AEThreadPool::workerCount() const
    {
        std::lock_guard lock(m_mutex);
        return m_workers.size();
    }

    size_t AEThreadPool::dedicatedThreadCount() const
    {
        std::lock_guard lock(m_mutex);
        return m_dedicatedThreads.size();
    }

    uint64_t AEThreadPool::completedTaskCount() const noexcept
    {
        return m_completedTaskCount.load(std::memory_order_relaxed);
    }

    bool AEThreadPool::hasPendingTasks() const
    {
        std::lock_guard lock(m_mutex);
        return !queuesEmptyLocked();
    }

    bool AEThreadPool::queuesEmptyLocked() const
    {
        return m_highPriorityTasks.empty() && m_tasks.empty();
    }

    AEThreadPool::TimePoint AEThreadPool::nextTaskTimeLocked() const
    {
        TimePoint nextTime = TimePoint::max();

        const auto updateNextTime = [&nextTime](const std::deque<QueuedTask>& queue)
            {
                for (const auto& task : queue)
                {
                    if (task.nextRun < nextTime)
                        nextTime = task.nextRun;
                }
            };

        updateNextTime(m_highPriorityTasks);
        updateNextTime(m_tasks);

        return nextTime;
    }

    bool AEThreadPool::tryPopRunnableTaskFromQueueLocked(std::deque<QueuedTask>& queue, TimePoint now, QueuedTask& outTask)
    {
        const auto taskItr = std::find_if(
            queue.begin(),
            queue.end(),
            [now](const QueuedTask& task)
            {
                return task.nextRun <= now;
            }
        );

        if (taskItr == queue.end())
            return false;

        outTask = std::move(*taskItr);
        queue.erase(taskItr);
        return true;
    }

    bool AEThreadPool::tryPopRunnableTaskLocked(TimePoint now, QueuedTask& outTask)
    {
        if (tryPopRunnableTaskFromQueueLocked(m_highPriorityTasks, now, outTask))
            return true;

        return tryPopRunnableTaskFromQueueLocked(m_tasks, now, outTask);
    }

    void AEThreadPool::requeueRecurringTask(QueuedTask task)
    {
        task.nextRun = Clock::now() + task.repeatDelay;

        {
            std::lock_guard lock(m_mutex);

            if (m_shutdownRequested.load(std::memory_order_acquire))
                return;

            if (task.highPriority)
                m_highPriorityTasks.emplace_back(std::move(task));
            else
                m_tasks.emplace_back(std::move(task));

            ensureWorkerCapacityLocked(task.highPriority);
        }

        m_condition.notify_one();
    }

    void AEThreadPool::spawnWorkersLocked(size_t desiredWorkerCount)
    {
        while (m_workers.size() < desiredWorkerCount)
        {
            const size_t workerIndex = m_workerIdCounter++;
            m_workers.emplace_back([this, workerIndex]
                {
                    workerLoop(workerIndex);
                });
        }
    }

    void AEThreadPool::ensureWorkerCapacityLocked(bool highPriority)
    {
        const size_t queuedTasks = m_highPriorityTasks.size() + m_tasks.size();
        const size_t activeWorkers = m_activeWorkerCount.load(std::memory_order_relaxed);
        const size_t currentWorkers = m_workers.size();

        const size_t upperBound = static_cast<size_t>(highPriority ? m_hardMaximumThreadCount : m_softMaximumThreadCount);
        const size_t desiredWorkers = std::clamp(activeWorkers + queuedTasks, static_cast<size_t>(m_minimumThreadCount), upperBound);

        if (currentWorkers < desiredWorkers)
            spawnWorkersLocked(desiredWorkers);
    }

    void AEThreadPool::queueTask(TaskFunc task, std::chrono::milliseconds repeatDelay, std::string taskName, bool highPriority)
    {
        if (!task)
            return;

        if (!m_started.load(std::memory_order_acquire))
            start();

        QueuedTask queuedTask;
        queuedTask.task = std::move(task);
        queuedTask.name = std::move(taskName);
        queuedTask.repeatDelay = repeatDelay;
        queuedTask.recurring = repeatDelay >= std::chrono::milliseconds::zero();
        queuedTask.nextRun = Clock::now();
        queuedTask.highPriority = highPriority;

        {
            std::lock_guard lock(m_mutex);

            if (m_shutdownRequested.load(std::memory_order_acquire))
                return;

            if (highPriority)
                m_highPriorityTasks.emplace_back(std::move(queuedTask));
            else
                m_tasks.emplace_back(std::move(queuedTask));

            ensureWorkerCapacityLocked(highPriority);
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
                    if (m_shutdownRequested.load(std::memory_order_acquire) && queuesEmptyLocked())
                        return;

                    const auto now = Clock::now();
                    if (tryPopRunnableTaskLocked(now, task))
                        break;

                    if (m_shutdownRequested.load(std::memory_order_acquire))
                        return;

                    const auto nextRun = nextTaskTimeLocked();

                    if (nextRun == TimePoint::max())
                    {
                        m_condition.wait(lock, [this]
                        {
                            return m_shutdownRequested.load(std::memory_order_acquire) || !queuesEmptyLocked();
                        });
                    }
                    else
                    {
                        m_condition.wait_until(lock, nextRun);
                    }
                }
            }

            m_activeWorkerCount.fetch_add(1, std::memory_order_relaxed);

            try
            {
                task.task();
                m_completedTaskCount.fetch_add(1, std::memory_order_relaxed);
            }
            catch (...)
            {
            }

            m_activeWorkerCount.fetch_sub(1, std::memory_order_relaxed);

            if (task.recurring && !m_shutdownRequested.load(std::memory_order_acquire))
                requeueRecurringTask(std::move(task));
        }
    }
}
