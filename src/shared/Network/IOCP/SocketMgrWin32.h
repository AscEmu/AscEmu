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

    void WorkerThreadLoop(AscEmu::Threading::AEThread& self);

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
        return m_completionPort;
    }
    void SpawnWorkerThreads();
    void CloseAll();
    void ShowStatus();
    uint32_t GetSocketCount()
    {
        return socket_count;
    }

    void AddSocket(Socket* s)
    {
        std::lock_guard lock{socketLock};

        _sockets.insert(s);
        ++socket_count;
    }

    void RemoveSocket(Socket* s)
    {
        std::lock_guard lock{socketLock};

        _sockets.erase(s);
        --socket_count;
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
