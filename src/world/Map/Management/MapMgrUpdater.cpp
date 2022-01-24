/*
Copyright (c) 2014-2022 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "MapMgrUpdater.hpp"
#include "Map/Maps/BaseMap.hpp"
#include "Map/Maps/WorldMap.hpp"
#include <mutex>

class ThreadingRequest
{
private:
    WorldMap& worldMap;
    uint32_t diffTime;

public:
    ThreadingRequest(WorldMap& map, uint32_t diff)
        : worldMap(map), diffTime(diff)
    {
    }

    void execute()
    {
         worldMap.update(diffTime);
    }
};

void MapMgrUpdate::activate(size_t num_threads)
{
    for (auto i = 0u; i < num_threads; ++i)
    {
        pool.push_back(std::thread(&MapMgrUpdate::threadRun, this));
    }
}

void MapMgrUpdate::shutdown()
{
    {
        std::unique_lock<std::mutex> lock(threadpool_mutex);
        terminate_pool = true;
    }

    // wake up all threads
    condition.notify_all(); 

    // Join all threads.
    for (auto& thread : pool)
    {
        thread.join();
    }

    pool.clear();
}

bool MapMgrUpdate::isActivated()
{
    return pool.size() > 0;
}

size_t MapMgrUpdate::getQueueSize()
{
    std::unique_lock<std::mutex> lock{ threadpool_mutex };
    return updateQueue.size();
}

void MapMgrUpdate::addJob(WorldMap& map, uint32_t diff)
{
    {
        std::unique_lock<std::mutex> lock{ queue_mutex };
        updateQueue.emplace(std::move(new ThreadingRequest(map, diff)));
    }

    // notify a sleeper to awake and do its job :P
    condition.notify_one();
}

void MapMgrUpdate::threadRun()
{
    while (true)
    {
        ThreadingRequest* request;

        {
            std::unique_lock<std::mutex> lock(queue_mutex);

            condition.wait(lock, [=] { return !updateQueue.empty() || terminate_pool;  });

            if (terminate_pool && updateQueue.empty())
                break;

            request = std::move(updateQueue.front());
            updateQueue.pop();
        }

        request->execute();
        delete request;
    }
}