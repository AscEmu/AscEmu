/*
 * Multiplatform Async Network Library
 * Copyright (c) 2007 Burlex
 *
 * SocketMgr - kqueue manager for FreeBSD.
 *
 */


#ifndef SOCKETMGR_FREE_BSD_H
#define SOCKETMGR_FREE_BSD_H

#include "../SocketDefines.h"
#include "Threading/Thread.hpp"
#include <vector>

#include <atomic>

#ifdef ASCEMU_USE_AE_NETWORK
    #include <memory>
    #include "Network/AE/Backends/KQUEUE/KqueueBackend.hpp"
#endif

#ifdef CONFIG_USE_KQUEUE

#define SOCKET_HOLDER_SIZE 30000    // You don't want this number to be too big, otherwise you're gonna be eating
// memory. 65536 = 256KB, so thats no big issue for now, and I really can't
// see anyone wanting to have more than 65536 concurrent connections.

#define THREAD_EVENT_SIZE 4096      // This is the number of socket events each thread can receieve at once.
// This default value should be more than enough.

class Socket;
class SocketWorkerThread;
class ListenSocketBase;

namespace AscEmu::Threading
{
    class AEThreadPool;
}

class SocketMgr
{
    // kqueue handle
    int kq;

    // fd -> pointer binding.
    Socket* fds[SOCKET_HOLDER_SIZE];
    ListenSocketBase* listenfds[SOCKET_HOLDER_SIZE]; // shouldnt be more than 1024

    /// socket counter
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
    std::unique_ptr<AscEmu::Network::AE::KqueueBackend> m_backend;
#endif

public:
    /// friend class of the worker thread -> it has to access our private resources
    friend class SocketWorkerThread;

    static SocketMgr& getInstance()
    {
        static SocketMgr mInstance;
        return mInstance;
    }

    void SetThreadPool(AscEmu::Threading::AEThreadPool& threadPool)
    {
        m_threadPool = &threadPool;
    }

    void ShutdownThreads();

    /// constructor > create epoll device handle + initialize event set
    void initialize()
    {
#ifdef ASCEMU_USE_AE_NETWORK
        m_backend = std::make_unique<AscEmu::Network::AE::KqueueBackend>(*this);
        m_backend->initialize();
#else
        kq = kqueue();
        if(kq == -1)
        {
            sLogger.failure("Could not create a kqueue fd.");
            exit(-1);
        }

        // null out the pointer array
        memset(fds, 0, sizeof(Socket*) * SOCKET_HOLDER_SIZE);
        memset(listenfds, 0, sizeof(Socket*) * SOCKET_HOLDER_SIZE);
#endif
    }

    /// destructor > destroy epoll handle
    void finalize()
    {
#ifdef ASCEMU_USE_AE_NETWORK
        if (m_backend != nullptr)
        {
            m_backend->finalize();
            m_backend.reset();
        }
#else
        // close epoll handle
        close(kq);
#endif
    }

    SocketMgr(SocketMgr&&) = delete;
    SocketMgr(SocketMgr const&) = delete;
    SocketMgr& operator=(SocketMgr&&) = delete;
    SocketMgr& operator=(SocketMgr const&) = delete;

    /// add a new socket to the set and to the fd mapping
    void AddSocket(Socket* s);
    void AddListenSocket(ListenSocketBase* s);

    void ShowStatus();    ///\todo Script it

    /// remove a socket from set/fd mapping
    void RemoveSocket(Socket* s);

    /// returns kqueue fd
    inline int GetKq() { return kq; }

    uint32_t GetSocketCount(); // used in linux socket and pass then to server commands

    /// closes all sockets
    void CloseAll();

    /// spawns worker threads
    void SpawnWorkerThreads();
};

#define sSocketMgr SocketMgr::getInstance()

#endif

#endif  //SOCKETMGR_FREE_BSD_H
