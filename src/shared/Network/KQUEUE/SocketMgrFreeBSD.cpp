/*
 * Multiplatform Async Network Library
 * Copyright (c) 2007 Burlex
 *
 * SocketMgr - kqueue manager for BSD.
 *
 */

#include "../Network.h"
#include "Debugging/Errors.hpp"
#include "Threading/ThreadPool.hpp"
#include <cassert>

#ifdef CONFIG_USE_KQUEUE

void SocketMgr::AddSocket(Socket* s)
{
#ifdef ASCEMU_USE_AE_NETWORK
    if (m_backend != nullptr)
    {
        m_backend->addSocket(s);
        return;
    }
#else
    assert(fds[s->GetFd()] == 0);
    fds[s->GetFd()] = s;

    struct kevent ev;
    if(s->writeBuffer.GetSize())
        EV_SET(&ev, s->GetFd(), EVFILT_WRITE, EV_ADD | EV_ONESHOT, 0, 0, NULL);
    else
        EV_SET(&ev, s->GetFd(), EVFILT_READ, EV_ADD, 0, 0, NULL);

    if(kevent(kq, &ev, 1, 0, 0, NULL) < 0)
    {
        sLogger.failure("Could not add initial kevent for fd {}!", s->GetFd());
        return;
    }
#endif
}
void SocketMgr::ShowStatus()
{
#ifdef ASCEMU_USE_AE_NETWORK
    if (m_backend != nullptr)
    {
        m_backend->showStatus();
        return;
    }
#else
    sLogger.info("Sockets: {}", 0);
#endif
}

void SocketMgr::AddListenSocket(ListenSocketBase* s)
{
#ifdef ASCEMU_USE_AE_NETWORK
    if (m_backend != nullptr)
    {
        m_backend->addListenSocket(s);
        return;
    }
#else
    assert(listenfds[s->GetFd()] == 0);
    listenfds[s->GetFd()] = s;

    struct kevent ev;
    EV_SET(&ev, s->GetFd(), EVFILT_READ, EV_ADD, 0, 0, NULL);

    if(kevent(kq, &ev, 1, 0, 0, NULL) < 0)
    {
        sLogger.failure("Could not add initial kevent for fd {}!", s->GetFd());
        return;
    }
#endif
}

void SocketMgr::RemoveSocket(Socket* s)
{
#ifdef ASCEMU_USE_AE_NETWORK
    if (m_backend != nullptr)
    {
        m_backend->removeSocket(s);
        return;
    }
#else
    if(fds[s->GetFd()] != s)
    {
        // already removed.
        sLogger.warning("kqueue : Duplicate removal of fd {}!", s->GetFd());
        return;
    }
    fds[s->GetFd()] = 0;

    // remove kevent.
    struct kevent ev, ev2;
    EV_SET(&ev, s->GetFd(), EVFILT_WRITE, EV_DELETE, 0, 0, NULL);
    EV_SET(&ev2, s->GetFd(), EVFILT_READ, EV_DELETE, 0, 0, NULL);
    if(kevent(kq, &ev, 1, 0, 0, NULL) && kevent(kq, &ev2, 1, 0, 0, NULL))
        sLogger.warning("kqueue : Could not remove from kqueue: fd {}", s->GetFd());
#endif
}

void SocketMgr::CloseAll()
{
#ifdef ASCEMU_USE_AE_NETWORK
    if (m_backend != nullptr)
    {
        m_backend->closeAll();
        return;
    }
#else
    for(uint32_t i = 0; i < SOCKET_HOLDER_SIZE; ++i)
        if(fds[i] != NULL)
            fds[i]->Delete();
#endif
}

void SocketMgr::SpawnWorkerThreads()
{
    uint32_t count = 1;
    if (m_threadPool == nullptr)
    {
        sLogger.failure("SocketMgr::SpawnWorkerThreads called without AEThreadPool.");
        return;
    }

#ifdef ASCEMU_USE_AE_NETWORK
    if (m_backend != nullptr)
    {
        m_backend->spawnWorkers(*m_threadPool);
        return;
    }
#else
    if (!m_workerThreads.empty())
        return;

    m_workerThreads.reserve(count);

    for (uint32_t i = 0; i < count; ++i)
    {
        auto& worker = m_threadPool->addDedicatedThread(
            "KQUEUE Worker " + std::to_string(i),
            [this](AscEmu::Threading::AEThread& self)
            {
                WorkerThreadLoop(self);
            }
        );

        m_workerThreads.push_back(&worker);
    }
#endif
}

uint32_t SocketMgr::GetSocketCount()
{
#ifdef ASCEMU_USE_AE_NETWORK
    return m_backend != nullptr ? m_backend->socketCount() : 0;
#else
    return socket_count.load();
#endif
}

void SocketMgr::ShutdownThreads()
{
#ifdef ASCEMU_USE_AE_NETWORK
    if (m_backend != nullptr)
    {
        m_backend->shutdownWorkers();
        return;
    }
#else
    for (auto* worker : m_workerThreads)
    {
        if (worker != nullptr)
            worker->requestKill();
    }

    for (auto* worker : m_workerThreads)
    {
        if (worker != nullptr)
            worker->join();
    }

    m_workerThreads.clear();
#endif
}

#ifndef ASCEMU_USE_AE_NETWORK
void SocketMgr::WorkerThreadLoop(AscEmu::Threading::AEThread& self)
{
    struct kevent events[THREAD_EVENT_SIZE];
    struct timespec ts;
    ts.tv_nsec = 0;
    ts.tv_sec = 1;

    const int local_kq = GetKq();

    while (!self.isKilled())
    {
        const int fd_count = kevent(local_kq, nullptr, 0, &events[0], THREAD_EVENT_SIZE, &ts);

        for (int i = 0; i < fd_count; ++i)
        {
            const int fd = events[i].ident;

            if (events[i].ident >= SOCKET_HOLDER_SIZE)
            {
                sLogger.warning("kqueue : Requested FD that is too high ({})", fd);
                continue;
            }

            Socket* ptr = fds[fd];

            if (ptr == nullptr)
            {
                if ((ptr = ((Socket*)listenfds[events[i].ident])) != nullptr)
                {
                    ((ListenSocketBase*)ptr)->OnAccept();
                }
                else
                {
                    struct kevent ev;
                    struct kevent ev2;
                    EV_SET(&ev, events[i].ident, EVFILT_WRITE, EV_DELETE, 0, 0, nullptr);
                    EV_SET(&ev2, events[i].ident, EVFILT_READ, EV_DELETE, 0, 0, nullptr);
                    kevent(local_kq, &ev, 1, 0, 0, nullptr);
                    kevent(local_kq, &ev2, 1, 0, 0, nullptr);
                }

                continue;
            }

            if (events[i].flags & EV_EOF || events[i].flags & EV_ERROR)
            {
                ptr->Disconnect();
                continue;
            }

            if (events[i].filter == EVFILT_WRITE)
            {
                ptr->BurstBegin();
                ptr->WriteCallback();

                if (ptr->writeBuffer.GetSize() > 0)
                    ptr->PostEvent(EVFILT_WRITE, true);
                else
                {
                    ptr->DecSendLock();
                    ptr->PostEvent(EVFILT_READ, false);
                }

                ptr->BurstEnd();
                continue;
            }

            if (events[i].filter == EVFILT_READ)
            {
                ptr->ReadCallback(0);

                if (ptr->writeBuffer.GetSize() > 0 && ptr->IsConnected() && !ptr->HasSendLock())
                {
                    ptr->PostEvent(EVFILT_WRITE, true);
                    ptr->IncSendLock();
                }
            }
        }
    }
}
#endif

#endif
