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
#ifdef ASCEMU_USE_AE_NETWORK_THREADPOOL
    #include "Threading/AEThread.h"
    #include <vector>
#else
    #include "Threading/LegacyThreadBase.h"
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

#ifdef ASCEMU_USE_AE_NETWORK_THREADPOOL
namespace AscEmu::Threading
{
    class AEThreadPool;
}
#endif

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

#ifdef ASCEMU_USE_AE_NETWORK_THREADPOOL
    AscEmu::Threading::AEThreadPool* m_threadPool = nullptr;
    std::vector<AscEmu::Threading::AEThread*> m_workerThreads;

    void WorkerThreadLoop(AscEmu::Threading::AEThread& self);
#endif

public:
    /// friend class of the worker thread -> it has to access our private resources
    friend class SocketWorkerThread;

    static SocketMgr& getInstance()
    {
        static SocketMgr mInstance;
        return mInstance;
    }

#ifdef ASCEMU_USE_AE_NETWORK_THREADPOOL
    void SetThreadPool(AscEmu::Threading::AEThreadPool& threadPool)
    {
        m_threadPool = &threadPool;
    }

    void ShutdownThreads();
#endif

    /// constructor > create epoll device handle + initialize event set
    void initialize()
    {
        kq = kqueue();
        if(kq == -1)
        {
            sLogger.failure("Could not create a kqueue fd.");
            exit(-1);
        }

        // null out the pointer array
        memset(fds, 0, sizeof(Socket*) * SOCKET_HOLDER_SIZE);
        memset(listenfds, 0, sizeof(Socket*) * SOCKET_HOLDER_SIZE);
    }

    /// destructor > destroy epoll handle
    void finalize()
    {
        // close epoll handle
        close(kq);
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

#ifndef ASCEMU_USE_AE_NETWORK_THREADPOOL
class SocketWorkerThread : public ThreadBase
{
    /// epoll event struct
    struct kevent events[THREAD_EVENT_SIZE];
    bool running;

public:
    bool runThread();
    void onShutdown()
    {
        running = false;
    }
};
#endif

#define sSocketMgr SocketMgr::getInstance()

#endif

#endif  //SOCKETMGR_FREE_BSD_H
