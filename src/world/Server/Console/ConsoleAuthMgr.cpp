/*
Copyright (c) 2014-2018 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "StdAfx.h"

#include "ConsoleAuthMgr.h"

initialiseSingleton(ConsoleAuthMgr);

ConsoleAuthMgr::ConsoleAuthMgr()
{
    authRequestId = 1;
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
