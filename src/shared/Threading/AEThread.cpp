#include "AEThread.h"
#include <iostream>
#include <algorithm>

using std::atomic;
using std::lock_guard;
using std::mutex;
using std::string;
using std::thread;
using std::chrono::duration_cast;
using std::chrono::milliseconds;

namespace AscEmu { namespace Threading
{
    atomic<unsigned int> AEThread::ThreadIdCounter(0);

    void AEThread::threadRunner()
    {
        std::cout << "[" << m_name.c_str() << "] Hello" << std::endl;

        m_done = false;

        while (!m_killed)
        {
            auto begin = std::chrono::steady_clock::now();

            m_func(*this);

            auto target = begin + m_interval;

            // We use this instead of sleep_until so we can interrupt
            while (!m_killed)
            {
                auto now = std::chrono::steady_clock::now();
                if (now > target)
                    break;

                auto distance = duration_cast<milliseconds>(target - now);

                if (m_longSleep)
                    std::this_thread::sleep_for(milliseconds(std::min(static_cast<long long>(distance.count()), m_longSleepDelay)));
                else
                    std::this_thread::sleep_for(milliseconds(1));
            }
        }

        std::cout << "[" << m_name.c_str() << "] Goodbye" << std::endl;

        m_done = true;
    }

    AEThread::AEThread(string name, ThreadFunc func, milliseconds interval)
    {
        m_id = ThreadIdCounter++;
        m_name = name;
        m_func = func;
        m_killed = false;
        m_done = true;

        unsafeSetInterval(interval);
        reboot();
    }

    AEThread::~AEThread() { killThread(); }

    string AEThread::getName() const { return m_name; }

    unsigned AEThread::getId() const { return m_id; }

    milliseconds AEThread::getInterval() const { return m_interval; }

    milliseconds AEThread::unsafeSetInterval(milliseconds interval)
    {
        m_longSleep = interval > milliseconds(m_longSleepDelay);
        auto old_value = m_interval;
        m_interval = interval;
        return old_value;
    }

    milliseconds AEThread::setInterval(milliseconds interval)
    {
        lock_guard<mutex> guard(m_mtx);
        return unsafeSetInterval(interval);
    }

    void AEThread::lock() { m_mtx.lock(); }
    void AEThread::unlock() { m_mtx.unlock(); }

    bool AEThread::isKilled() const { return m_killed; }

    void AEThread::kill()
    {
        if (m_killed)
            return;

        m_killed = true;
    }

    void AEThread::join()
    {
        kill();
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

        join();
        reboot();
    }

    void AEThread::killThread() { join(); }
}}
