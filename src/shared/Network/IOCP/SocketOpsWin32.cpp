/*
 * Multiplatform Async Network Library
 * Copyright (c) 2007 Burlex
 *
 * SocketOpsWin32.cpp - Windows implementation of SocketOps.
 *
 */

#include "../Network.h"

#ifdef CONFIG_USE_IOCP

namespace SocketOps
{
    // Create file descriptor for socket i/o operations.
    SOCKET CreateTCPFileDescriptor()
    {
        // create a socket for use with overlapped i/o.
        return ::WSASocketW(AF_INET, SOCK_STREAM, 0, 0, 0, WSA_FLAG_OVERLAPPED);
    }

    // Disable blocking send/recv calls.
    bool Nonblocking(SOCKET fd)
    {
        u_long arg = 1;
        return (::ioctlsocket(fd, FIONBIO, &arg) == 0);
    }

    // Disable blocking send/recv calls.
    bool Blocking(SOCKET fd)
    {
        u_long arg = 0;
        return (ioctlsocket(fd, FIONBIO, &arg) == 0);
    }

    // Disable nagle buffering algorithm
    bool DisableBuffering(SOCKET fd)
    {
        uint32_t arg = 1;
        return (setsockopt(fd, 0x6, TCP_NODELAY, (const char*)&arg, sizeof(arg)) == 0);
    }

    // Enable nagle buffering algorithm
    bool EnableBuffering(SOCKET fd)
    {
        uint32_t arg = 0;
        return (setsockopt(fd, 0x6, TCP_NODELAY, (const char*)&arg, sizeof(arg)) == 0);
    }

    // Set internal buffer size to socket.
    bool SetSendBufferSize(SOCKET fd, uint32_t size)
    {
        return (setsockopt(fd, SOL_SOCKET, SO_SNDBUF, (const char*)&size, sizeof(size)) == 0);
    }

    // Set internal buffer size to socket.
    bool SetRecvBufferSize(SOCKET fd, uint32_t size)
    {
        return (setsockopt(fd, SOL_SOCKET, SO_RCVBUF, (const char*)&size, sizeof(size)) == 0);
    }

    // Set internal timeout.
    bool SetTimeout(SOCKET fd, uint32_t timeout)
    {
        struct timeval to;
        to.tv_sec = timeout;
        to.tv_usec = 0;
        if(setsockopt(fd, SOL_SOCKET, SO_SNDTIMEO, (const char*)&to, (socklen_t)sizeof(to)) != 0) return false;
        return (setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&to, (socklen_t)sizeof(to)) == 0);
    }

    // Closes a socket fully.
    void CloseSocket(SOCKET fd)
    {
        shutdown(fd, SD_BOTH);
        closesocket(fd);
    }

    // Sets reuseaddr
    void ReuseAddr(SOCKET fd)
    {
        uint32_t option = 1;
        if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (const char*)&option, 4) < 0)
            printf("SO_REUSEADDR setsockopt error\n");
    }
}

#endif
