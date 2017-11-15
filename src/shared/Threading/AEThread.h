#pragma once
#include "ThreadState.h"

#include <atomic>
#include <mutex>
#include <thread>

// Lightweight wrapper for std::thread to provide extensibility
namespace AscEmu { namespace Threading
{
    class AEThread
    {
        typedef std::function<void(AEThread&)> ThreadFunc;

        static std::atomic<unsigned int> ThreadIdCounter;

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

        void threadRunner();
        void killThread();
    public:
        AEThread(std::string name, ThreadFunc func, std::chrono::milliseconds intervalMs);
        ~AEThread();

        std::chrono::milliseconds getInterval() const;
        std::chrono::milliseconds setInterval(std::chrono::milliseconds val);
        std::chrono::milliseconds unsafeSetInterval(std::chrono::milliseconds val);

        std::string getName() const;

        unsigned int getId() const;

        bool isKilled() const;
        void kill();
        void join();

        void reboot();

        void lock();
        void unlock();
    };
}}
