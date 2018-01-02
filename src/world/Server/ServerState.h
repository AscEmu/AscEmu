/*
*Copyright (c) 2014-2018 AscEmu Team <http://www.ascemu.org>
*This file is released under the MIT license. See README-MIT for more information.
*/
#pragma once

#include <chrono>
#include <atomic>
#include <memory>

class ServerState
{
    friend class Master;

    // When this class is instantiated
    std::chrono::time_point<std::chrono::high_resolution_clock> m_initTime;
    // Set to ::now() when update() is called
    std::chrono::time_point<std::chrono::high_resolution_clock> m_currentTime;
    // long long representation of the delta between currentTime and initTime
    std::atomic<long long> m_delta;
public:
    long long getDelta() const;
    void update();

    ServerState();
private:
    static ServerState* singletonPtr;
public:
    static ServerState* instance(ServerState* existingPtr = nullptr);
};