/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "Network/Socket.h"
#include "Threading/ThreadPool.hpp"

#include <mutex>
#include <vector>

namespace AscEmu::Network::AE
{
    class BackendWorkerSet
    {
    public:
        void spawn(
            AscEmu::Threading::AEThreadPool& threadPool,
            std::vector<AscEmu::Threading::AEThread*>& workers,
            uint32_t count,
            const char* prefix,
            const auto& loopFn
        )
        {
            if (!workers.empty())
                return;

            workers.reserve(count);

            for (uint32_t i = 0; i < count; ++i)
            {
                auto& worker = threadPool.addDedicatedThread(
                    std::string(prefix) + " " + std::to_string(i),
                    [loopFn](AscEmu::Threading::AEThread& self)
                    {
                        loopFn(self);
                    }
                );

                workers.push_back(&worker);
            }
        }

        void shutdown(std::vector<AscEmu::Threading::AEThread*>& workers)
        {
            for (auto* worker : workers)
            {
                if (worker != nullptr)
                    worker->requestKill();
            }

            for (auto* worker : workers)
            {
                if (worker != nullptr)
                    worker->join();
            }

            workers.clear();
        }
    };
}
