/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "CommonTypes.hpp"

#ifdef WIN32
    #define WIN32_LEAN_AND_MEAN
    #include <windows.h>
#else
    #include <pthread.h>
#endif

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
