/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "fmt/format.h"
#include <cstring>

#if defined(_WIN32) || defined(WIN32)
    #define CONFIG_USE_IOCP
    #include <windows.h>
#else // unix
    #define SOCKET int
    #define SD_BOTH SHUT_RDWR
    #include <pthread.h>
    #if __linux__
        #include <sys/epoll.h>
        #define CONFIG_USE_EPOLL
    #elif __FreeBSD__
        #include <sys/stat.h>
        #include <sys/types.h>
        #include <sys/event.h>
        #include <sys/time.h>
        #include <sys/errno.h>
        #define CONFIG_USE_KQUEUE
    #elif __APPLE__
        #include <sys/event.h>
        #define CONFIG_USE_KQUEUE
    #endif

    #ifndef INVALID_SOCKET
        #define INVALID_SOCKET (-1)
    #endif

    #ifndef SOCKET_ERROR
        #define SOCKET_ERROR (-1)
    #endif
#endif


#ifdef CONFIG_USE_IOCP

enum SocketIOEvent
{
    SOCKET_IO_EVENT_READ_COMPLETE   = 0,
    SOCKET_IO_EVENT_WRITE_END        = 1,
    SOCKET_IO_THREAD_SHUTDOWN        = 2,
    NUM_SOCKET_IO_EVENTS            = 3,
};

class OverlappedStruct
{
    public:
        OVERLAPPED m_overlap;
        SocketIOEvent m_event;
        volatile long m_inUse;
        OverlappedStruct(SocketIOEvent ev) : m_event(ev)
        {
            std::memset(&m_overlap, 0, sizeof(OVERLAPPED));
            m_inUse = 0;
        };

        OverlappedStruct()
        {
            std::memset(&m_overlap, 0, sizeof(OVERLAPPED));
            m_inUse = 0;
        }

        __forceinline void Reset(SocketIOEvent ev)
        {
            std::memset(&m_overlap, 0, sizeof(OVERLAPPED));
            m_event = ev;
        }

        void Mark()
        {
            long val = InterlockedCompareExchange(&m_inUse, 1, 0);
            if(val != 0)
                fmt::println("!!!! Network: Detected double use of read/write event! Previous event was {}.", static_cast<int>(m_event));
        }

        void Unmark()
        {
            InterlockedExchange(&m_inUse, 0);
        }
};

#endif
