/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "CommonTypes.hpp"

class SERVER_DECL ThreadBase
{
public:
    ThreadBase() {}
    virtual ~ThreadBase() {}
    virtual bool runThread() = 0;
    virtual void onShutdown() {}
#ifdef WIN32
    HANDLE THREAD_HANDLE;
#else
    pthread_t THREAD_HANDLE;
#endif
};
