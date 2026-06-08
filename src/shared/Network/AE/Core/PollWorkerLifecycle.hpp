/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "Threading/Thread.hpp"
#include "Threading/ThreadPool.hpp"

#include <cstdint>
#include <string>
#include <vector>

namespace AscEmu::Network::AE
{
    template <typename LoopFn>
    inline void spawnDedicatedPollWorkers(
        AscEmu::Threading::AEThreadPool& threadPool,
        std::vector<AscEmu::Threading::AEThread*>& workerThreads,
        uint32_t count,
        const char* namePrefix,
        const LoopFn& loopFn
    )
    {
        if (!workerThreads.empty())
            return;

        workerThreads.reserve(count);

        for (uint32_t i = 0; i < count; ++i)
        {
            auto& worker = threadPool.addDedicatedThread(
                std::string(namePrefix) + " " + std::to_string(i),
                [loopFn](AscEmu::Threading::AEThread& self)
                {
                    loopFn(self);
                }
            );

            workerThreads.push_back(&worker);
        }
    }

    inline void shutdownDedicatedPollWorkers(std::vector<AscEmu::Threading::AEThread*>& workerThreads)
    {
        for (auto* worker : workerThreads)
        {
            if (worker != nullptr)
                worker->requestKill();
        }

        for (auto* worker : workerThreads)
        {
            if (worker != nullptr)
                worker->join();
        }

        workerThreads.clear();
    }
}
