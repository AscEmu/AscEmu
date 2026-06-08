/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <Logging/Logger.hpp>
#include "SocketDefines.h"
#include "SocketOps.h"
#include "Socket.h"

#ifdef CONFIG_USE_IOCP
#include "IOCP/SocketMgrWin32.h"
    #ifdef ASCEMU_USE_AE_NETWORK
        #include "AE/Backends/IOCP/ListenSocketWin32.hpp"
    #else
        #include "IOCP/ListenSocketWin32.h"
    #endif
#endif

#ifdef CONFIG_USE_EPOLL
#include "EPOLL/SocketMgrLinux.h"
    #ifdef ASCEMU_USE_AE_NETWORK
        #include "AE/Core/PollListenSocket.hpp"
    #else
        #include "EPOLL/ListenSocketLinux.h"
    #endif
#endif

#ifdef CONFIG_USE_KQUEUE
#include "KQUEUE/SocketMgrFreeBSD.h"
    #ifdef ASCEMU_USE_AE_NETWORK
        #include "AE/Core/PollListenSocket.hpp"
    #else
        #include "KQUEUE/ListenSocketFreeBSD.h"
    #endif
#endif
