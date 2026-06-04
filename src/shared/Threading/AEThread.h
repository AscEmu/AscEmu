/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <functional>
#include <mutex>
#include <string>
#include <thread>

namespace AscEmu::Threading
{
    class AEThread
    {
    public:
        using ThreadFunc = std::function<void(AEThread&)>;

        AEThread(std::string name);
        AEThread(std::string name, ThreadFunc func, std::chrono::milliseconds intervalMs, bool autostart = true);
        ~AEThread();

        AEThread(const AEThread&) = delete;
        AEThread& operator=(const AEThread&) = delete;

        AEThread(AEThread&&) = delete;
        AEThread& operator=(AEThread&&) = delete;

        std::chrono::milliseconds getInterval() const;
        std::chrono::milliseconds setInterval(std::chrono::milliseconds val);
        std::chrono::milliseconds unsafeSetInterval(std::chrono::milliseconds val);

        std::string getName() const;
        unsigned int getId() const;

        bool isKilled() const;
        bool isDone() const;
        bool isWorking() const;
        bool hasWork() const;
        bool start();

        void requestKill();
        void join();
        void killAndJoin();
        void reboot();

        void addWork(ThreadFunc work, bool autostart = false);
        void addLoop(ThreadFunc work, std::chrono::milliseconds intervalMs, bool autostart = false);
        void addOneShot(ThreadFunc work, bool autostart = false);
        void clearWork();

        void setWork(ThreadFunc work);

    private:
        enum class ExecutionMode : uint8_t
        {
            None,
            OneShot,
            Dedicated,
            Periodic
        };

        void configure(ThreadFunc work, ExecutionMode mode, std::chrono::milliseconds intervalMs, bool autostart);
        void threadRunner();
        void waitUntil(std::chrono::steady_clock::time_point wakeUpTime);

    private:
        static std::atomic<unsigned int> s_thread_id_counter;

        std::string m_name;
        unsigned int m_id = 0;

        mutable std::mutex m_mutex;
        std::condition_variable m_condition;
        std::thread m_thread;
        ThreadFunc m_func;
        ExecutionMode m_mode = ExecutionMode::None;

        std::atomic_bool m_stopRequested{ true };
        std::atomic_bool m_running{ false };
        std::atomic_int64_t m_intervalMs{ 0 };
    };
}
