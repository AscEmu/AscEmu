/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#ifndef ASCEMU_USE_AE_NETWORK_THREADPOOL
#include "CommonTypes.hpp"

#ifdef WIN32
#include <windows.h>
#else
#include <pthread.h>
#endif

class SERVER_DECL ThreadBase
{
public:
    ThreadBase() = default;
    virtual ~ThreadBase() = default;

    // Return true if the ThreadPool should delete this object after runThread().
    // Return false if ownership is kept by the caller / task implementation.
    virtual bool runThread() = 0;

    // Called during legacy pool shutdown for active tasks.
    virtual void onShutdown() {}

#ifdef WIN32
    HANDLE THREAD_HANDLE;
#else
    pthread_t THREAD_HANDLE;
#endif
};

#endif
