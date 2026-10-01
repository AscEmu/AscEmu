/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Logging/Logger.hpp"
#include "Network/Socket.hpp"
#include "Network/Network.hpp"

#if defined(CONFIG_USE_EPOLL)

void Socket::postEvent(uint32_t events)
{
    int epollFd = sSocketMgr.GetEpollFd();

    struct epoll_event event;
    std::memset(&event, 0, sizeof(epoll_event));
    event.data.fd = m_socket;
    event.events = events | EPOLLET;

    if (epoll_ctl(epollFd, EPOLL_CTL_MOD, event.data.fd, &event))
        sLogger.warning("epoll : Could not post event on fd {}", m_socket);
}

void Socket::readCallback(uint32_t)
{
    if (isDeleted() || !isConnected())
        return;

    std::unique_lock lock{ m_readMutex };

    size_t space = readBuffer.GetSpace();
    int bytes = recv(m_socket, readBuffer.GetBuffer(), space, 0);
    if (bytes <= 0)
    {
        lock.unlock();
        disconnect();
        return;
    }

    readBuffer.IncrementWritten(bytes);
    onRead();
    m_bytesReceived += bytes;
}

void Socket::writeCallback()
{
    if (isDeleted() || !isConnected())
        return;

    int bytesWritten = ::send(m_socket, writeBuffer.GetBufferStart(), writeBuffer.GetContiguiousBytes(), 0);
    if (bytesWritten < 0)
    {
        disconnect();
        return;
    }

    m_bytesSent += bytesWritten;
    writeBuffer.Remove(bytesWritten);
    completeDelayedDisconnectIfReady();
}

void Socket::setupReadEvent()
{}

void Socket::burstPush()
{
    if (acquireSendLock())
        postEvent(EPOLLOUT);
}

#elif defined(CONFIG_USE_KQUEUE)

void Socket::postEvent(int events, bool oneShot)
{
    int kq = sSocketMgr.GetKq();

    struct kevent event;
    if (oneShot)
        EV_SET(&event, m_socket, events, EV_ADD | EV_ONESHOT, 0, 0, NULL);
    else
        EV_SET(&event, m_socket, events, EV_ADD, 0, 0, NULL);

    if (kevent(kq, &event, 1, 0, 0, NULL) < 0)
        sLogger.warning("kqueue : Could not modify event for fd {}", getFd());
}

void Socket::readCallback(uint32_t)
{
    if (isDeleted() || !isConnected())
        return;

    std::unique_lock lock{ m_readMutex };

    size_t space = readBuffer.GetSpace();
    int bytes = recv(m_socket, readBuffer.GetBuffer(), space, 0);
    if (bytes <= 0)
    {
        lock.unlock();
        disconnect();
        return;
    }

    readBuffer.IncrementWritten(bytes);
    onRead();
    m_bytesReceived += bytes;
}

void Socket::writeCallback()
{
    if (isDeleted() || !isConnected())
        return;

    int bytesWritten = ::send(m_socket, writeBuffer.GetBufferStart(), writeBuffer.GetContiguiousBytes(), 0);
    if (bytesWritten < 0)
    {
        disconnect();
        return;
    }

    m_bytesSent += bytesWritten;
    writeBuffer.Remove(bytesWritten);
    completeDelayedDisconnectIfReady();
}

void Socket::setupReadEvent()
{}

void Socket::burstPush()
{
    if (acquireSendLock())
        postEvent(EVFILT_WRITE, true);
}

#endif
