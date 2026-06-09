/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "Logging/Logger.hpp"
#include "SocketDefines.h"
#include "SocketOps.h"
#include "Socket.h"

#ifdef ASCEMU_USE_AE_NETWORK

    #include "AE/SocketMgr.hpp"

    #ifdef CONFIG_USE_IOCP
        #include "AE/Backends/IOCP/ListenSocketWin32.hpp"
    #endif

    #if defined(CONFIG_USE_EPOLL) || defined(CONFIG_USE_KQUEUE)
        #include "AE/Core/PollListenSocket.hpp"
    #endif

#else

    #ifdef CONFIG_USE_IOCP
        #include "IOCP/SocketMgrWin32.h"
        #include "IOCP/ListenSocketWin32.h"
    #endif

    #ifdef CONFIG_USE_EPOLL
        #include "EPOLL/SocketMgrLinux.h"
        #include "EPOLL/ListenSocketLinux.h"
    #endif

    #ifdef CONFIG_USE_KQUEUE
        #include "KQUEUE/SocketMgrFreeBSD.h"
        #include "KQUEUE/ListenSocketFreeBSD.h"
    #endif

#endif
