/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "StdAfx.h"

#include "ConsoleAuthMgr.h"

ConsoleAuthMgr& ConsoleAuthMgr::getInstance()
{
    static ConsoleAuthMgr mInstance;
    return mInstance;
}

void ConsoleAuthMgr::initialize()
{
    consoleAuthMgrLock.Acquire();

    authRequestId = 1;

    consoleAuthMgrLock.Release();
}

uint32_t ConsoleAuthMgr::getGeneratedId()
{
    consoleAuthMgrLock.Acquire();

    uint32_t requestId = authRequestId++;

    consoleAuthMgrLock.Release();

    return requestId;
}

void ConsoleAuthMgr::addRequestIdSocket(uint32_t id, ConsoleSocket* sock)
{
    consoleAuthMgrLock.Acquire();

    if (sock == nullptr)
    {
        consoleRequestMap.erase(id);
    }
    else
    {
        consoleRequestMap.insert(std::make_pair(id, sock));
    }

    consoleAuthMgrLock.Release();
}

ConsoleSocket* ConsoleAuthMgr::getSocketByRequestId(uint32_t id)
{
    ConsoleSocket* consoleSocket = nullptr;

    consoleAuthMgrLock.Acquire();

    std::map<uint32_t, ConsoleSocket*>::iterator itr = consoleRequestMap.find(id);
    if (itr != consoleRequestMap.end())
    {
        consoleSocket = itr->second;
    }

    consoleAuthMgrLock.Release();

    return consoleSocket;
}
