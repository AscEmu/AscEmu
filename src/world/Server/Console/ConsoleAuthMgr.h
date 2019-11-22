/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <cstdint>
#include <map>

#include "Threading/Mutex.h"

class ConsoleSocket;

class ConsoleAuthMgr
{
    Mutex consoleAuthMgrLock;
    uint32_t authRequestId;
    std::map<uint32_t, ConsoleSocket*> consoleRequestMap;

    private:
        
        ConsoleAuthMgr() = default;
        ~ConsoleAuthMgr() = default;

    public:

        static ConsoleAuthMgr& getInstance();
        void initialize();

        ConsoleAuthMgr(ConsoleAuthMgr&&) = delete;
        ConsoleAuthMgr(ConsoleAuthMgr const&) = delete;
        ConsoleAuthMgr& operator=(ConsoleAuthMgr&&) = delete;
        ConsoleAuthMgr& operator=(ConsoleAuthMgr const&) = delete;

        uint32_t getGeneratedId();

        void addRequestIdSocket(uint32_t id, ConsoleSocket* sock);

        ConsoleSocket* getSocketByRequestId(uint32_t id);
};

#define sConsoleAuthMgr ConsoleAuthMgr::getInstance()
