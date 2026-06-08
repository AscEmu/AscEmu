/*
 * Multiplatform Async Network Library
 * Copyright (c) 2007 Burlex
 *
 * SocketMgr - iocp-based SocketMgr for windows.
 *
 */

#ifndef SOCKETMGR_H_WIN32
#define SOCKETMGR_H_WIN32

#ifdef CONFIG_USE_IOCP

#include "../SocketDefines.h"
#include "Threading/Thread.hpp"
#include <vector>
#include <mutex>

#include <memory>

#ifdef ASCEMU_USE_AE_NETWORK
    #include "Network/AE/Backends/IOCP/IocpBackend.hpp"
#endif

namespace AscEmu::Threading
{
    class AEThreadPool;
}

class Socket;
class SERVER_DECL SocketMgr
{
    HANDLE m_completionPort;
    std::set<Socket*> _sockets;
    std::mutex socketLock;
    std::atomic<unsigned long> socket_count;

private:
    SocketMgr() = default;
    ~SocketMgr() = default;

    AscEmu::Threading::AEThreadPool* m_threadPool = nullptr;
    std::vector<AscEmu::Threading::AEThread*> m_workerThreads;

#ifndef ASCEMU_USE_AE_NETWORK
    void WorkerThreadLoop(AscEmu::Threading::AEThread& self);
#endif

#ifdef ASCEMU_USE_AE_NETWORK
    std::unique_ptr<AscEmu::Network::AE::IocpBackend> m_backend;
#endif

public:
    void SetThreadPool(AscEmu::Threading::AEThreadPool& threadPool)
    {
        m_threadPool = &threadPool;
    }

    static SocketMgr& getInstance();
    void initialize();
    // todo: empty on windows, should it be?
    void finalize();

    SocketMgr(SocketMgr&&) = delete;
    SocketMgr(SocketMgr const&) = delete;
    SocketMgr& operator=(SocketMgr&&) = delete;
    SocketMgr& operator=(SocketMgr const&) = delete;

    inline HANDLE GetCompletionPort()
    {
#ifdef ASCEMU_USE_AE_NETWORK
        return m_backend != nullptr ? m_backend->completionPort() : m_completionPort;
#else
        return m_completionPort;
#endif
    }
    void SpawnWorkerThreads();
    void CloseAll();
    void ShowStatus();
    uint32_t GetSocketCount()
    {
#ifdef ASCEMU_USE_AE_NETWORK
        return m_backend != nullptr ? m_backend->socketCount() : static_cast<uint32_t>(socket_count.load());
#else
        return socket_count;
#endif
    }

    void AddSocket(Socket* s)
    {
#ifdef ASCEMU_USE_AE_NETWORK
        if (m_backend != nullptr)
        {
            m_backend->addSocket(s);
            return;
        }
#else
        std::lock_guard lock{socketLock};

        _sockets.insert(s);
        ++socket_count;
#endif
    }

    void RemoveSocket(Socket* s)
    {
#ifdef ASCEMU_USE_AE_NETWORK
        if (m_backend != nullptr)
        {
            m_backend->removeSocket(s);
            return;
        }
#else
        std::lock_guard lock{socketLock};

        _sockets.erase(s);
        --socket_count;
#endif
    }

    void ShutdownThreads();
    long threadcount;
};

#define sSocketMgr SocketMgr::getInstance()

typedef void(*OperationHandler)(Socket* s, uint32_t len);

void SERVER_DECL HandleReadComplete(Socket* s, uint32_t len);
void SERVER_DECL HandleWriteComplete(Socket* s, uint32_t len);
void SERVER_DECL HandleShutdown(Socket* s, uint32_t len);

static OperationHandler ophandlers[NUM_SOCKET_IO_EVENTS] =
{
    &HandleReadComplete,
    &HandleWriteComplete,
    &HandleShutdown
};

#endif

#endif  //SOCKETMGR_H_WIN32
