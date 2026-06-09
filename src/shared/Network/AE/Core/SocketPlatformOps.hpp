/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "Network/NetworkIncludes.hpp"
#include "Network/SocketDefines.h"

#include <cstdint>

namespace AscEmu::Network::AE::SocketPlatformOps
{
    SOCKET createTcpSocket();
    bool setNonBlocking(SOCKET socket);
    bool setBlocking(SOCKET socket);
    bool disableBuffering(SOCKET socket);
    bool enableBuffering(SOCKET socket);
    bool setSendBufferSize(SOCKET socket, uint32_t size);
    bool setRecvBufferSize(SOCKET socket, uint32_t size);
    bool setTimeout(SOCKET socket, uint32_t seconds);
    void closeSocket(SOCKET socket);
    void setReuseAddress(SOCKET socket);
}
