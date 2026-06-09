/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "Network/SocketDefines.h"
#include "Network/AE/Core/NetworkBackend.hpp"
#include "Threading/Thread.hpp"

#include <memory>

class Socket;
class ListenSocketBase;

namespace AscEmu::Threading
{
    class AEThreadPool;
}

class SocketMgr
{
public:
    static SocketMgr& getInstance();

    void SetThreadPool(AscEmu::Threading::AEThreadPool& threadPool)
    {
        m_threadPool = &threadPool;
    }

    void initialize();
    void finalize();

    SocketMgr(SocketMgr&&) = delete;
    SocketMgr(SocketMgr const&) = delete;
    SocketMgr& operator=(SocketMgr&&) = delete;
    SocketMgr& operator=(SocketMgr const&) = delete;

    void AddSocket(Socket* socket);
    void AddListenSocket(ListenSocketBase* socket);
    void RemoveSocket(Socket* socket);

    void SpawnWorkerThreads();
    void ShutdownThreads();
    void CloseAll();
    void ShowStatus();

    uint32_t GetSocketCount() const;

#ifdef CONFIG_USE_IOCP
    HANDLE GetCompletionPort() const;
#endif

#ifdef CONFIG_USE_EPOLL
    int GetEpollFd() const;
#endif

#ifdef CONFIG_USE_KQUEUE
    int GetKq() const;
#endif

private:
    SocketMgr() = default;
    ~SocketMgr() = default;

    static std::unique_ptr<AscEmu::Network::AE::NetworkBackend> createBackend(SocketMgr& owner);

private:
    AscEmu::Threading::AEThreadPool* m_threadPool = nullptr;
    std::unique_ptr<AscEmu::Network::AE::NetworkBackend> m_backend;
};

#define sSocketMgr SocketMgr::getInstance()
