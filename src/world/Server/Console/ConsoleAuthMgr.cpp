/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "ConsoleAuthMgr.h"

ConsoleAuthMgr& ConsoleAuthMgr::getInstance()
{
    static ConsoleAuthMgr mInstance;
    return mInstance;
}

void ConsoleAuthMgr::initialize()
{
    std::lock_guard lock(consoleAuthMgrLock);

    authRequestId = 1;
}

uint32_t ConsoleAuthMgr::getGeneratedId()
{
    std::lock_guard lock(consoleAuthMgrLock);

    uint32_t requestId = authRequestId++;

    return requestId;
}

void ConsoleAuthMgr::addRequestIdSocket(uint32_t id, ConsoleSocket* sock)
{
    std::lock_guard lock(consoleAuthMgrLock);

    if (sock == nullptr)
        consoleRequestMap.erase(id);
    else
        consoleRequestMap.insert(std::make_pair(id, sock));
}

ConsoleSocket* ConsoleAuthMgr::getSocketByRequestId(uint32_t id)
{
    ConsoleSocket* consoleSocket = nullptr;

    std::lock_guard lock(consoleAuthMgrLock);

    std::map<uint32_t, ConsoleSocket*>::iterator itr = consoleRequestMap.find(id);
    if (itr != consoleRequestMap.end())
        consoleSocket = itr->second;

    return consoleSocket;
}
