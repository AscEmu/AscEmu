/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Logging/Logger.hpp"
#include "Network/Socket.h"
#include "Network/Network.h"

#ifdef ASCEMU_USE_AE_NETWORK

#if defined(CONFIG_USE_EPOLL)

void Socket::PostEvent(uint32_t events)
{
    int epoll_fd = sSocketMgr.GetEpollFd();

    struct epoll_event ev;
    memset(&ev, 0, sizeof(epoll_event));
    ev.data.fd = m_fd;
    ev.events = events | EPOLLET;

    if (epoll_ctl(epoll_fd, EPOLL_CTL_MOD, ev.data.fd, &ev))
        sLogger.warning("epoll : Could not post event on fd {}", m_fd);
}

void Socket::ReadCallback(uint32_t)
{
    if (IsDeleted() || !IsConnected())
        return;

    std::unique_lock lock{m_readMutex};

    size_t space = readBuffer.GetSpace();
    int bytes = recv(m_fd, readBuffer.GetBuffer(), space, 0);
    if (bytes <= 0)
    {
        lock.unlock();
        Disconnect();
        return;
    }
    else if (bytes > 0)
    {
        readBuffer.IncrementWritten(bytes);
        OnRead();
    }
    m_BytesRecieved += bytes;
}

void Socket::WriteCallback()
{
    if (IsDeleted() || !IsConnected())
        return;

    int bytes_written = send(m_fd, writeBuffer.GetBufferStart(), writeBuffer.GetContiguiousBytes(), 0);
    if (bytes_written < 0)
    {
        Disconnect();
        return;
    }
    m_BytesSent += bytes_written;
    writeBuffer.Remove(bytes_written);
}

void Socket::SetupReadEvent()
{
}

void Socket::BurstPush()
{
    if (AcquireSendLock())
        PostEvent(EPOLLOUT);
}

#elif defined(CONFIG_USE_KQUEUE)

void Socket::PostEvent(int events, bool oneshot)
{
    int kq = sSocketMgr.GetKq();

    struct kevent ev;
    if (oneshot)
        EV_SET(&ev, m_fd, events, EV_ADD | EV_ONESHOT, 0, 0, NULL);
    else
        EV_SET(&ev, m_fd, events, EV_ADD, 0, 0, NULL);

    if (kevent(kq, &ev, 1, 0, 0, NULL) < 0)
        sLogger.warning("kqueue : Could not modify event for fd {}", GetFd());
}

void Socket::ReadCallback(uint32_t)
{
    if (IsDeleted() || !IsConnected())
        return;

    std::unique_lock lock{m_readMutex};

    size_t space = readBuffer.GetSpace();
    int bytes = recv(m_fd, readBuffer.GetBuffer(), space, 0);
    if (bytes <= 0)
    {
        lock.unlock();
        Disconnect();
        return;
    }
    else if (bytes > 0)
    {
        readBuffer.IncrementWritten(bytes);
        OnRead();
    }
    m_BytesRecieved += bytes;
}

void Socket::WriteCallback()
{
    if (IsDeleted() || !IsConnected())
        return;

    int bytes_written = send(m_fd, writeBuffer.GetBufferStart(), writeBuffer.GetContiguiousBytes(), 0);
    if (bytes_written < 0)
    {
        Disconnect();
        return;
    }

    m_BytesSent += bytes_written;
    writeBuffer.Remove(bytes_written);
}

void Socket::SetupReadEvent()
{
}

void Socket::BurstPush()
{
    if (AcquireSendLock())
        PostEvent(EVFILT_WRITE, true);
}

#endif
#endif
