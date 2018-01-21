/*
Copyright (c) 2014-2018 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "AEThreadPool.h"
#include <memory>
#include <string>

using std::chrono::milliseconds;
using std::chrono::steady_clock;
using std::lock_guard;
using std::make_unique;
using std::mutex;
using std::string;
using std::unique_ptr;


namespace AscEmu { namespace Threading
{
    unique_ptr<AEThreadPool> AEThreadPool::s_global_thread_pool;

    unique_ptr<AEThreadPool>& AEThreadPool::globalThreadPool()
    {
        uint16_t minimum_thread_count = 10;
        uint16_t soft_maximum_thread_count = 64;
        uint16_t hard_maximum_thread_count = 256;
        if (s_global_thread_pool == nullptr)
            // Arbitrary thread count values - TODO load from config
            s_global_thread_pool = make_unique<AEThreadPool>("Global Pool", minimum_thread_count,
                                                             soft_maximum_thread_count, hard_maximum_thread_count);

        return s_global_thread_pool;
    }

    AEThread* AEThreadPool::spawnThread()
    {
        if (m_threadCount + 1 > m_hardMaximumThreadCount)
            return nullptr;

        ++m_threadCount;
        ++m_totalThreadsEverCreated;
        m_threads.push_back(make_unique<AEThread>(
            m_poolName + " (Thread " + std::to_string(m_totalThreadsEverCreated.load()) + ")", [](auto&)
            {
            }, milliseconds(128), false));
        return m_threads.back().get();
    }


    void AEThreadPool::queueTask(ThreadFunc task, milliseconds repeatDelay, string taskName, bool highPriority)
    {
        lock_guard<mutex> guard(m_mtx);
        auto task_obj = make_unique<Task>(task, taskName, repeatDelay);
        if (highPriority)
            m_tasks.push_back(move(task_obj));
        else
            m_highPriorityTasks.push_back(move(task_obj));
    }

    void AEThreadPool::pulse(AEThread& pulseThread)
    {
        if (m_shutdownRequested)
        {
            lock_guard<mutex> guard(m_mtx);

            m_tasks.clear();
            for (auto& thread : m_threads) { thread->killAndJoin(); }
            m_threads.clear();
            pulseThread.killAndJoin();
            return;
        }

        {
            // Prune threads if we're above the soft limit
            if (m_threadCount > m_softMaximumThreadCount)
            {
                lock_guard<mutex> guard(m_mtx);

                m_threads.remove_if([this](unique_ptr<AEThread>& thread)
                {
                    if (thread->isWorking())
                        return false;

                    --this->m_threadCount;
                    return true;
                });
            }

            // Adjust thread count if we need to
            if (m_threadCount < m_minimumThreadCount)
            {
                lock_guard<mutex> guard(m_mtx);
                while (m_threadCount < m_minimumThreadCount)
                    spawnThread();
            }
        }

        {
            lock_guard<mutex> guard(m_mtx);

            // Remove dead tasks
            const auto remove_dead_tasks = [](unique_ptr<Task>& task) { return task->m_done; };
            m_tasks.remove_if(remove_dead_tasks);
            m_highPriorityTasks.remove_if(remove_dead_tasks);

            const auto internal_task_runner = [](unique_ptr<Task>& task, AEThread& poolThread)
            {
                task->m_running = true;
                task->m_task(poolThread);
                task->m_running = false;
                poolThread.requestKill();
                if (task->isFireOnceTask())
                    task->m_done = true;
                else
                    task->m_lastPulse = steady_clock::now();
            };

            // Check every thread to see if it's doing work
            for (auto& thread : m_threads)
            {
                if (thread->isDone())
                {
                    const auto now = steady_clock::now();

                    for (auto& task : m_highPriorityTasks)
                    {
                        if (task->m_running)
                            continue;

                        thread->setWork([internal_task_runner, &task](AEThread& poolThread)
                        {
                            internal_task_runner(task, poolThread);
                        });

                        thread->reboot();
                        break;
                    }

                    // We might have work at this point, so check again
                    if (thread->isWorking())
                        continue;

                    for (auto& task : m_tasks)
                    {
                        if (task->canFire(now))
                        {
                            thread->setWork([internal_task_runner, &task](AEThread& poolThread)
                            {
                                internal_task_runner(task, poolThread);
                            });

                            thread->reboot();
                            break;
                        }
                    }
                }
            }

            // Spawn new threads for any high priority tasks that weren't dispatched already
            for (auto& task : m_highPriorityTasks)
            {
                if (task->m_running)
                    continue;

                // Try to spawn a thread, give up if we can't
                auto new_thread = spawnThread();
                if (new_thread == nullptr)
                    break;

                new_thread->setWork([internal_task_runner, &task](AEThread& poolThread)
                {
                    internal_task_runner(task, poolThread);
                });
                new_thread->reboot();
            }
        }
    }

    void AEThreadPool::queueRecurringTask(ThreadFunc task, milliseconds repeatDelay, string taskName)
    {
        queueTask(task, repeatDelay, taskName);
    }

    void AEThreadPool::queueFireOnceTask(ThreadFunc task, string taskName)
    {
        queueTask(task, milliseconds(-1), taskName);
    }

    void AEThreadPool::queueHighPriorityTask(ThreadFunc task, std::string taskName)
    {
        queueTask(task, milliseconds(-1), taskName, true);
    }

    AEThreadPool::AEThreadPool(string poolName, uint16_t minimumThreadCount, uint16_t softMaximumThreadCount,
                               uint16_t hardMaximumThreadCount) :
        AEThreadPool(poolName, minimumThreadCount, softMaximumThreadCount, hardMaximumThreadCount, milliseconds(64))
    {
    }

    AEThreadPool::AEThreadPool(string poolName, uint16_t minimumThreadCount, uint16_t softMaximumThreadCount,
                               uint16_t hardMaximumThreadCount, milliseconds pulseFrequency) :
        m_poolName(poolName),
        m_pulseFrequency(pulseFrequency),
        m_threadCount(0),
        m_totalThreadsEverCreated(0),
        m_minimumThreadCount(minimumThreadCount),
        m_softMaximumThreadCount(softMaximumThreadCount),
        m_hardMaximumThreadCount(hardMaximumThreadCount),
        m_shutdownRequested(false)
    {
        m_pulseThread = make_unique<AEThread>(poolName + " (Pulse Thread)",
                                              [this](AEThread& thread) { this->pulse(thread); }, m_pulseFrequency,
                                              true);
    }
}}
