/*
 * Multiplatform Async Network Library
 * Copyright (c) 2007 Burlex
 *
 * SocketMgr - epoll manager for Linux.
 *
 */


#ifndef SOCKETMGR_LINUX_H
#define SOCKETMGR_LINUX_H

#include "SocketDefines.h"
#include <atomic>

#ifdef CONFIG_USE_EPOLL

#define SOCKET_HOLDER_SIZE 30000    // You don't want this number to be too big, otherwise you're gonna be eating
// memory. 65536 = 256KB, so thats no big issue for now, and I really can't
// see anyone wanting to have more than 65536 concurrent connections.

#define THREAD_EVENT_SIZE 4096      // This is the number of socket events each thread can receieve at once.
// This default value should be more than enough.

class Socket;
class SocketWorkerThread;
class ListenSocketBase;

class SocketMgr
{
        /// /dev/epoll instance handle
        int epoll_fd;

        // fd -> pointer binding.
        Socket* fds[SOCKET_HOLDER_SIZE];
        ListenSocketBase* listenfds[SOCKET_HOLDER_SIZE];

        /// socket counter
        std::atomic<unsigned long> socket_count;

        int max_fd;

    private:
        SocketMgr() = default;
        ~SocketMgr() = default;

    public:

        /// friend class of the worker thread -> it has to access our private resources
        friend class SocketWorkerThread;

        static SocketMgr& getInstance()
        {
            static SocketMgr mInstance;
            return mInstance;
        }

        /// constructor > create epoll device handle + initialize event set
        void initialize()
        {
            epoll_fd = epoll_create(SOCKET_HOLDER_SIZE);
            if(epoll_fd == -1)
            {
                LogError("Could not create epoll fd (/dev/epoll).");
                exit(-1);
            }

            // null out the pointer array
            memset(fds, 0, sizeof(void*) * SOCKET_HOLDER_SIZE);
            memset(listenfds, 0, sizeof(void*) * SOCKET_HOLDER_SIZE);
            max_fd = 0;
        }

        /// destructor > destroy epoll handle
        void finalize()
        {
            // close epoll handle
            close(epoll_fd);
        }

        SocketMgr(SocketMgr&&) = delete;
        SocketMgr(SocketMgr const&) = delete;
        SocketMgr& operator=(SocketMgr&&) = delete;
        SocketMgr& operator=(SocketMgr const&) = delete;

        /// add a new socket to the epoll set and to the fd mapping
        void AddSocket(Socket* s);
        void AddListenSocket(ListenSocketBase* s);

        /// remove a socket from epoll set/fd mapping
        void RemoveSocket(Socket* s);

        /// returns epoll fd
        inline int GetEpollFd() { return epoll_fd; }

        /// closes all sockets
        void CloseAll();

        uint32 GetSocketCount() { return socket_count.load(); }

        /// spawns worker threads
        void SpawnWorkerThreads();

        /// show status
        void ShowStatus();
};

class SocketWorkerThread : public ThreadBase
{
        /// epoll event struct
        struct epoll_event events[THREAD_EVENT_SIZE];
        bool running;
    public:
        bool runThread();
        void onShutdown()
        {
            running = false;
        }
};

#define sSocketMgr SocketMgr::getInstance()

#endif

#endif      //SOCKETMGR_LINUX_H
