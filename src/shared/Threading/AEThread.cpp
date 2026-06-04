/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "AEThread.h"

#include <algorithm>
#include <utility>

using std::chrono::milliseconds;
using std::chrono::steady_clock;

namespace AscEmu::Threading
{
    std::atomic<unsigned int> AEThread::s_thread_id_counter{ 0 };

    AEThread::AEThread(std::string name)
        : m_name(std::move(name)), m_id(s_thread_id_counter.fetch_add(1, std::memory_order_relaxed))
    {}

    AEThread::AEThread(std::string name, ThreadFunc func, milliseconds intervalMs, bool autostart)
        : AEThread(std::move(name))
    {
        if (intervalMs > milliseconds::zero())
            addLoop(std::move(func), intervalMs, autostart);
        else
            addWork(std::move(func), autostart);
    }

    AEThread::~AEThread()
    {
        killAndJoin();
    }

    milliseconds AEThread::getInterval() const
    {
        return milliseconds(m_intervalMs.load(std::memory_order_relaxed));
    }

    milliseconds AEThread::unsafeSetInterval(milliseconds intervalMs)
    {
        const auto sanitizedInterval = std::max<int64_t>(0, intervalMs.count());
        const milliseconds previousValue(m_intervalMs.exchange(sanitizedInterval, std::memory_order_relaxed));
        return previousValue;
    }

    milliseconds AEThread::setInterval(milliseconds intervalMs)
    {
        std::lock_guard lock(m_mutex);
        return unsafeSetInterval(intervalMs);
    }

    std::string AEThread::getName() const { return m_name; }

    unsigned int AEThread::getId() const { return m_id; }

    bool AEThread::isKilled() const
    {
        return m_stopRequested.load(std::memory_order_acquire);
    }

    bool AEThread::isDone() const
    {
        return !m_running.load(std::memory_order_acquire);
    }

    bool AEThread::isWorking() const
    {
        return m_running.load(std::memory_order_acquire);
    }

    bool AEThread::hasWork() const
    {
        std::lock_guard lock(m_mutex);
        return static_cast<bool>(m_func);
    }

    bool AEThread::start()
    {
        std::unique_lock lock(m_mutex);

        if (m_running.load(std::memory_order_acquire))
            return false;

        if (!m_func || m_mode == ExecutionMode::None)
            return false;

        if (m_thread.joinable())
        {
            const bool isSelfJoin = m_thread.get_id() == std::this_thread::get_id();
            lock.unlock();

            if (!isSelfJoin)
                m_thread.join();

            lock.lock();
        }

        m_stopRequested.store(false, std::memory_order_release);
        m_running.store(true, std::memory_order_release);
        m_thread = std::thread(&AEThread::threadRunner, this);
        return true;
    }

    void AEThread::requestKill()
    {
        m_stopRequested.store(true, std::memory_order_release);
        m_condition.notify_all();
    }

    void AEThread::join()
    {
        if (!m_thread.joinable())
            return;

        if (m_thread.get_id() == std::this_thread::get_id())
            return;

        m_thread.join();
    }

    void AEThread::killAndJoin()
    {
        requestKill();
        join();
    }

    void AEThread::reboot()
    {
        killAndJoin();
        start();
    }

    void AEThread::addWork(ThreadFunc work, bool autostart)
    {
        configure(std::move(work), ExecutionMode::Dedicated, milliseconds::zero(), autostart);
    }

    void AEThread::addLoop(ThreadFunc work, milliseconds intervalMs, bool autostart)
    {
        if (intervalMs <= milliseconds::zero())
        {
            addWork(std::move(work), autostart);
            return;
        }

        configure(std::move(work), ExecutionMode::Periodic, intervalMs, autostart);
    }

    void AEThread::addOneShot(ThreadFunc work, bool autostart)
    {
        configure(std::move(work), ExecutionMode::OneShot, milliseconds::zero(), autostart);
    }

    void AEThread::clearWork()
    {
        killAndJoin();

        std::lock_guard lock(m_mutex);
        m_func = nullptr;
        m_mode = ExecutionMode::None;
        unsafeSetInterval(milliseconds::zero());
    }

    void AEThread::setWork(ThreadFunc work)
    {
        addWork(std::move(work), false);
    }

    void AEThread::configure(ThreadFunc work, ExecutionMode mode, milliseconds intervalMs, bool autostart)
    {
        if (isWorking())
            killAndJoin();

        {
            std::lock_guard lock(m_mutex);
            m_func = std::move(work);
            m_mode = mode;
            unsafeSetInterval(intervalMs);
        }

        if (autostart)
            start();
    }

    void AEThread::waitUntil(steady_clock::time_point wakeUpTime)
    {
        std::unique_lock lock(m_mutex);
        m_condition.wait_until(lock, wakeUpTime, [this]
            {
                return m_stopRequested.load(std::memory_order_acquire);
            });
    }

    void AEThread::threadRunner()
    {
        ThreadFunc work;
        ExecutionMode mode = ExecutionMode::None;

        {
            std::lock_guard lock(m_mutex);
            work = m_func;
            mode = m_mode;
        }

        if (work)
        {
            switch (mode)
            {
                case ExecutionMode::OneShot:
                {
                    work(*this);
                    break;
                }
                case ExecutionMode::Dedicated:
                {
                    work(*this);
                    break;
                }
                case ExecutionMode::Periodic:
                {
                    while (!isKilled())
                    {
                        const auto begin = steady_clock::now();
                        work(*this);

                        if (isKilled())
                            break;

                        const auto interval = getInterval();
                        if (interval <= milliseconds::zero())
                            continue;

                        waitUntil(begin + interval);
                    }

                    break;
                }
                case ExecutionMode::None:
                default:
                    break;
            }
        }

        m_running.store(false, std::memory_order_release);
        m_condition.notify_all();
    }
}
