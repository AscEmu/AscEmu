#include "AEThread.h"
#include <iostream>

using std::atomic;
using std::lock_guard;
using std::mutex;
using std::string;
using std::thread;
using std::chrono::milliseconds;

namespace AscEmu { namespace Threading
{
    atomic<unsigned int> AEThread::ThreadIdCounter = 0;

    void AEThread::threadRunner()
    {
        std::cout << "[" << m_name.c_str() << "] Hello" << std::endl;

        m_done = false;

        while (!m_killed)
        {
            auto begin = std::chrono::steady_clock::now();

            m_func(*this);

            auto target = begin + m_interval;

            do
            {
                // We use this instead of sleep_until so we can interrupt
                std::this_thread::sleep_for(milliseconds(64));
            }
            while (!m_killed && std::chrono::steady_clock::now() < target);
        }

        std::cout << "[" << m_name.c_str() << "] Goodbye" << std::endl;

        m_done = true;
    }

    AEThread::AEThread(string name, ThreadFunc func, milliseconds interval)
    {
        m_id = ThreadIdCounter++;
        m_name = name;
        m_func = func;
        m_interval = interval;
        m_killed = false;
        m_done = true;
        reboot();
    }

    AEThread::~AEThread() { killThread(); }

    string AEThread::getName() const { return m_name; }

    unsigned AEThread::getId() const { return m_id; }

    milliseconds AEThread::getInterval() const { return m_interval; }

    milliseconds AEThread::unsafeSetInterval(milliseconds interval)
    {
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
