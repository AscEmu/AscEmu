/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once
#include <atomic>
#include <mutex>
#include <thread>
#include <functional>

// Lightweight wrapper for std::thread to provide extensibility
namespace AscEmu { namespace Threading
{
    class AEThread
    {
        typedef std::function<void(AEThread&)> ThreadFunc;

        static std::atomic<unsigned int> s_thread_id_counter;
        const long long m_longSleepDelay = 64;

        // Meta
        std::string m_name;
        unsigned int m_id;
        ThreadFunc m_func;
        std::chrono::milliseconds m_interval;

        // State
        std::mutex m_mtx;
        std::thread m_thread;
        bool m_killed;
        bool m_done;
        bool m_longSleep;

        void threadRunner();
    public:
        AEThread(std::string name, ThreadFunc func, std::chrono::milliseconds intervalMs, bool autostart = true);
        ~AEThread();

        std::chrono::milliseconds getInterval() const;
        std::chrono::milliseconds setInterval(std::chrono::milliseconds val);
        std::chrono::milliseconds unsafeSetInterval(std::chrono::milliseconds val);

        std::string getName() const;

        unsigned int getId() const;

        bool isKilled() const;
        void requestKill();
        void join();
        void killAndJoin();

        void reboot();

        bool isDone() const;
        void setWork(ThreadFunc work);
        bool isWorking() const;
    };
}}
