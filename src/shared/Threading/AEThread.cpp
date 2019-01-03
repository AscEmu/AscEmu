/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "AEThread.h"
#include <algorithm>

using std::atomic;
using std::lock_guard;
using std::mutex;
using std::string;
using std::thread;
using std::chrono::duration_cast;
using std::chrono::steady_clock;
using std::chrono::milliseconds;
using std::this_thread::sleep_for;

namespace AscEmu { namespace Threading
{
    atomic<unsigned int> AEThread::s_thread_id_counter(0);

    void AEThread::threadRunner()
    {
        m_done = false;

        while (!m_killed)
        {
            const auto begin = steady_clock::now();

            m_func(*this);

            const auto target = begin + m_interval;

            // We use this instead of sleep_until so we can interrupt
            while (!m_killed)
            {
                const auto now = steady_clock::now();
                if (now > target)
                    break;

                auto distance = duration_cast<milliseconds>(target - now);

                if (m_longSleep)
                    sleep_for(milliseconds(std::min(static_cast<long long>(distance.count()), m_longSleepDelay)));
                else
                    sleep_for(milliseconds(1));
            }
        }

        m_done = true;
    }

    AEThread::AEThread(string name, ThreadFunc func, milliseconds interval, bool autostart) :
        m_name(name),
        m_id(s_thread_id_counter++),
        m_func(func),
        m_interval(interval),
        m_killed(false),
        m_done(true),
        m_longSleep(false)
    {
        m_id = s_thread_id_counter++;
        m_name = name;
        m_func = func;
        m_killed = true;
        m_done = true;

        unsafeSetInterval(interval);
        if (autostart)
            reboot();
    }

    AEThread::~AEThread() { killAndJoin(); }

    string AEThread::getName() const { return m_name; }

    unsigned AEThread::getId() const { return m_id; }

    milliseconds AEThread::getInterval() const { return m_interval; }

    milliseconds AEThread::unsafeSetInterval(milliseconds interval)
    {
        m_longSleep = interval > milliseconds(m_longSleepDelay);
        const auto old_value = m_interval;
        m_interval = interval;
        return old_value;
    }

    milliseconds AEThread::setInterval(milliseconds interval)
    {
        lock_guard<mutex> guard(m_mtx);
        return unsafeSetInterval(interval);
    }

    bool AEThread::isDone() const { return m_done; }

    void AEThread::setWork(ThreadFunc work) { this->m_func = work; }

    bool AEThread::isWorking() const { return !m_done; }

    bool AEThread::isKilled() const { return m_killed; }

    void AEThread::requestKill()
    {
        if (m_killed)
            return;

        m_killed = true;
    }

    void AEThread::join()
    {
        if (m_thread.joinable())
            m_thread.join();
    }

    void AEThread::reboot()
    {
        if (m_done)
        {
            m_killed = false;
            m_done = false;

            m_thread = thread(&AEThread::threadRunner, this);
            return;
        }

        // Thread is currently running, so wait for it to die and try to reboot again
        killAndJoin();
        reboot();
    }

    void AEThread::killAndJoin()
    {
        requestKill();
        join();
    }
}}
