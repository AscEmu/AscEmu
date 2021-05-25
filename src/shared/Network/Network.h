/*
 * Multiplatform Async Network Library
 * Copyright (c) 2007 Burlex
 *
 * Network.h - Include this file in your .cpp files to gain access
 *             to features offered by this library.
 *
 */

#ifndef _NETWORK_H
#define _NETWORK_H

#include "../Logging/Logger.hpp"
#include "CircularBuffer.h"
#include "SocketDefines.h"
#include "SocketOps.h"
#include "Socket.h"

#ifdef CONFIG_USE_IOCP
#include "SocketMgrWin32.h"
#include "ListenSocketWin32.h"
#endif

#ifdef CONFIG_USE_EPOLL
#include "SocketMgrLinux.h"
#include "ListenSocketLinux.h"
#endif

#ifdef CONFIG_USE_KQUEUE
#include "SocketMgrFreeBSD.h"
#include "ListenSocketFreeBSD.h"
#endif

#endif      // _NETWORK_H
