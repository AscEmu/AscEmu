/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Network/AE/Core/SocketPlatformOps.hpp"

#ifdef CONFIG_USE_IOCP

namespace AscEmu::Network::AE::SocketPlatformOps
{
    SOCKET createTcpSocket()
    {
        return ::WSASocketW(AF_INET, SOCK_STREAM, 0, nullptr, 0, WSA_FLAG_OVERLAPPED);
    }

    bool setNonBlocking(SOCKET socket)
    {
        u_long arg = 1;
        return ::ioctlsocket(socket, FIONBIO, &arg) == 0;
    }

    bool setBlocking(SOCKET socket)
    {
        u_long arg = 0;
        return ::ioctlsocket(socket, FIONBIO, &arg) == 0;
    }

    bool disableBuffering(SOCKET socket)
    {
        uint32_t arg = 1;
        return ::setsockopt(socket, IPPROTO_TCP, TCP_NODELAY, reinterpret_cast<const char*>(&arg), sizeof(arg)) == 0;
    }

    bool enableBuffering(SOCKET socket)
    {
        uint32_t arg = 0;
        return ::setsockopt(socket, IPPROTO_TCP, TCP_NODELAY, reinterpret_cast<const char*>(&arg), sizeof(arg)) == 0;
    }

    bool setSendBufferSize(SOCKET socket, uint32_t size)
    {
        return ::setsockopt(socket, SOL_SOCKET, SO_SNDBUF, reinterpret_cast<const char*>(&size), sizeof(size)) == 0;
    }

    bool setRecvBufferSize(SOCKET socket, uint32_t size)
    {
        return ::setsockopt(socket, SOL_SOCKET, SO_RCVBUF, reinterpret_cast<const char*>(&size), sizeof(size)) == 0;
    }

    bool setTimeout(SOCKET socket, uint32_t timeoutSeconds)
    {
        struct timeval to;
        to.tv_sec = timeoutSeconds;
        to.tv_usec = 0;

        if (::setsockopt(socket, SOL_SOCKET, SO_SNDTIMEO, reinterpret_cast<const char*>(&to), (socklen_t)sizeof(to)) != 0)
            return false;

        return ::setsockopt(socket, SOL_SOCKET, SO_RCVTIMEO, reinterpret_cast<const char*>(&to), (socklen_t)sizeof(to)) == 0;
    }

    void closeSocket(SOCKET socket)
    {
        ::shutdown(socket, SD_BOTH);
        ::closesocket(socket);
    }

    void setReuseAddress(SOCKET socket)
    {
        uint32_t option = 1;
        ::setsockopt(socket, SOL_SOCKET, SO_REUSEADDR, reinterpret_cast<const char*>(&option), sizeof(option));
    }
}

#endif
