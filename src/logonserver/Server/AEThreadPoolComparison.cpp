/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/


#include "AEThreadPoolComparison.hpp"

#include "Threading/LegacyThreadPool.h"

#include <atomic>
#include <chrono>
#include <cstdio>
#include <functional>
#include <thread>

namespace
{
    class DeleteAfterRunTask final : public ThreadBase
    {
    public:
        explicit DeleteAfterRunTask(std::atomic_uint32_t& counter)
            : m_counter(counter)
        {
        }

        bool runThread() override
        {
            m_counter.fetch_add(1);
            return true;
        }

    private:
        std::atomic_uint32_t& m_counter;
    };

    class KeepAfterRunTask final : public ThreadBase
    {
    public:
        explicit KeepAfterRunTask(std::atomic_uint32_t& counter)
            : m_counter(counter)
        {
        }

        bool runThread() override
        {
            m_counter.fetch_add(1);
            return false;
        }

    private:
        std::atomic_uint32_t& m_counter;
    };

    class ShutdownAwareTask final : public ThreadBase
    {
    public:
        ShutdownAwareTask(std::atomic_uint32_t& runCounter, std::atomic_uint32_t& shutdownCounter)
            : m_runCounter(runCounter),
              m_shutdownCounter(shutdownCounter)
        {
        }

        bool runThread() override
        {
            m_runCounter.fetch_add(1);
            std::this_thread::sleep_for(std::chrono::milliseconds(200));
            return true;
        }

        void onShutdown() override
        {
            m_shutdownCounter.fetch_add(1);
        }

    private:
        std::atomic_uint32_t& m_runCounter;
        std::atomic_uint32_t& m_shutdownCounter;
    };

    bool waitUntil(std::chrono::milliseconds timeout, const std::function<bool()>& predicate)
    {
        const auto start = std::chrono::steady_clock::now();

        while (std::chrono::steady_clock::now() - start < timeout)
        {
            if (predicate())
                return true;

            std::this_thread::sleep_for(std::chrono::milliseconds(5));
        }

        return predicate();
    }

    void printResult(const char* name, bool ok)
    {
        std::printf("[LegacyThreadPoolComparison] %-32s %s\n", name, ok ? "OK" : "FAILED");
    }
}

void runAEThreadPoolComparison()
{
    std::printf("[LegacyThreadPoolComparison] Starting tests...\n");

#ifdef ASCEMU_USE_AE_THREADPOOL_LEGACY_ADAPTER
    std::printf("[LegacyThreadPoolComparison] Backend: AEThreadPool adapter\n");
#else
    std::printf("[LegacyThreadPoolComparison] Backend: original LegacyThreadPool\n");
#endif

    bool allOk = true;

    ThreadPool.Startup();

    std::atomic_uint32_t deleteAfterRunCounter = 0;
    std::atomic_uint32_t keepAfterRunCounter = 0;

    for (uint32_t i = 0; i < 100; ++i)
        ThreadPool.ExecuteTask(new DeleteAfterRunTask(deleteAfterRunCounter));

    auto* keepTask = new KeepAfterRunTask(keepAfterRunCounter);
    ThreadPool.ExecuteTask(keepTask);

    const bool deleteAfterRunOk = waitUntil(std::chrono::seconds(2), [&deleteAfterRunCounter]
    {
        return deleteAfterRunCounter.load() == 100;
    });

    const bool keepAfterRunOk = waitUntil(std::chrono::seconds(2), [&keepAfterRunCounter]
    {
        return keepAfterRunCounter.load() == 1;
    });

    printResult("Delete-after-run tasks", deleteAfterRunOk);
    printResult("Keep-after-run task", keepAfterRunOk);

    allOk &= deleteAfterRunOk;
    allOk &= keepAfterRunOk;

    delete keepTask;

    std::atomic_uint32_t shutdownRunCounter = 0;
    std::atomic_uint32_t shutdownCounter = 0;

    ThreadPool.ExecuteTask(new ShutdownAwareTask(shutdownRunCounter, shutdownCounter));

    waitUntil(std::chrono::milliseconds(100), [&shutdownRunCounter]
    {
        return shutdownRunCounter.load() == 1;
    });

    ThreadPool.Shutdown();

    const bool shutdownCallbackOk = shutdownCounter.load() >= 1;

    printResult("Shutdown callback", shutdownCallbackOk);
    allOk &= shutdownCallbackOk;

    ThreadPool.ShowStats();

    std::printf("[LegacyThreadPoolComparison] deleteAfterRunCounter: %u\n", deleteAfterRunCounter.load());
    std::printf("[LegacyThreadPoolComparison] keepAfterRunCounter:   %u\n", keepAfterRunCounter.load());
    std::printf("[LegacyThreadPoolComparison] shutdownRunCounter:    %u\n", shutdownRunCounter.load());
    std::printf("[LegacyThreadPoolComparison] shutdownCounter:       %u\n", shutdownCounter.load());
    std::printf("[LegacyThreadPoolComparison] Overall:               %s\n", allOk ? "OK" : "FAILED");
}
