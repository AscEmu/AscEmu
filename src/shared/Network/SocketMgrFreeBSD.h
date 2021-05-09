/*
 * Multiplatform Async Network Library
 * Copyright (c) 2007 Burlex
 *
 * SocketMgr - kqueue manager for FreeBSD.
 *
 */


#ifndef SOCKETMGR_FREE_BSD_H
#define SOCKETMGR_FREE_BSD_H

#include "SocketDefines.h"

#ifdef CONFIG_USE_KQUEUE

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
        // kqueue handle
        int kq;

        // fd -> pointer binding.
        Socket* fds[SOCKET_HOLDER_SIZE];
        ListenSocketBase* listenfds[SOCKET_HOLDER_SIZE];        // shouldnt be more than 1024

        /// socket counter
        int socket_count;

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

        /// returns number of sockets in array
        inline int Count() { return socket_count; }

        /// closes all sockets
        void CloseAll();

        uint32 GetSocketCount() { return socket_count.load(); }

        /// spawns worker threads
        void SpawnWorkerThreads();
};

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

#define sSocketMgr SocketMgr::getInstance()

#endif

#endif  //SOCKETMGR_FREE_BSD_H
