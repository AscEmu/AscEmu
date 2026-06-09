/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Network/AE/Core/SocketPlatformOps.hpp"

#if defined(CONFIG_USE_EPOLL) || defined(CONFIG_USE_KQUEUE)

namespace AscEmu::Network::AE::SocketPlatformOps
{
    SOCKET createTcpSocket()
    {
        return socket(AF_INET, SOCK_STREAM, 0);
    }

    bool setNonBlocking(SOCKET socket)
    {
        uint32_t arg = 1;
        return (::ioctl(socket, FIONBIO, &arg) == 0);
    }

    bool setBlocking(SOCKET socket)
    {
        uint32_t arg = 0;
        return (ioctl(socket, FIONBIO, &arg) == 0);
    }

    bool disableBuffering(SOCKET socket)
    {
        uint32_t arg = 1;
        return (setsockopt(socket, 0x6, 0x1, (const char*)&arg, sizeof(arg)) == 0);
    }

    bool enableBuffering(SOCKET socket)
    {
        uint32_t arg = 0;
        return (setsockopt(socket, 0x6, 0x1, (const char*)&arg, sizeof(arg)) == 0);
    }

    bool setSendBufferSize(SOCKET socket, uint32_t size)
    {
        return (setsockopt(socket, SOL_SOCKET, SO_SNDBUF, (const char*)&size, sizeof(size)) == 0);
    }

    bool setRecvBufferSize(SOCKET socket, uint32_t size)
    {
        return (setsockopt(socket, SOL_SOCKET, SO_RCVBUF, (const char*)&size, sizeof(size)) == 0);
    }

    bool setTimeout(SOCKET socket, uint32_t timeout)
    {
        struct timeval to;
        to.tv_sec = timeout;
        to.tv_usec = 0;
        if (setsockopt(socket, SOL_SOCKET, SO_SNDTIMEO, (const char*)&to, (socklen_t)sizeof(to)) != 0)
            return false;
        return (setsockopt(socket, SOL_SOCKET, SO_RCVTIMEO, (const char*)&to, (socklen_t)sizeof(to)) == 0);
    }

    void closeSocket(SOCKET socket)
    {
        shutdown(socket, SHUT_RDWR);
        close(socket);
    }

    void setReuseAddress(SOCKET socket)
    {
        uint32_t option = 1;
        setsockopt(socket, SOL_SOCKET, SO_REUSEADDR, reinterpret_cast<const char*>(&option), sizeof(option));
    }
}

#endif
