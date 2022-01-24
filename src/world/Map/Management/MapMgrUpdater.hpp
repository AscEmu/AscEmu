/*
Copyright (c) 2014-2022 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <mutex>
#include <thread>
#include <condition_variable>
#include <unordered_map>
#include <vector>
#include <queue>
#include <iostream>
#include <memory>
#include <functional>

class ThreadingRequest;
class WorldMap;

class MapMgrUpdate
{
public:
    MapMgrUpdate() {}
    ~MapMgrUpdate() 
    {
        shutdown();
    };

    friend class ThreadingRequest;

    void activate(size_t amount);
    void shutdown();
    void addJob(WorldMap& map, uint32_t diff);
    size_t getQueueSize();

    bool isActivated();

private:
    std::vector<std::thread> pool;

    std::condition_variable condition;

    std::mutex threadpool_mutex;
    bool terminate_pool = false;

    std::mutex queue_mutex;
    std::queue<ThreadingRequest*> updateQueue;

    void threadRun();
};