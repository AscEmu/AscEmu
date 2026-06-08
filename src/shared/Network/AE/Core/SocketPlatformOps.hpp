/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "Network/SocketOps.h"

#include <cstdint>

namespace AscEmu::Network::AE
{
    namespace SocketPlatformOps
    {
        inline bool setNonBlocking(SOCKET socket)
        {
            return SocketOps::Nonblocking(socket);
        }

        inline bool setBlocking(SOCKET socket)
        {
            return SocketOps::Blocking(socket);
        }

        inline void setReuseAddress(SOCKET socket)
        {
            SocketOps::ReuseAddr(socket);
        }

        inline bool setTimeout(SOCKET socket, uint32_t seconds)
        {
            return SocketOps::SetTimeout(socket, seconds);
        }

        inline void closeSocket(SOCKET socket)
        {
            SocketOps::CloseSocket(socket);
        }
    }
}
