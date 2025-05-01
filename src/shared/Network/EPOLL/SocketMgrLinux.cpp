/*
 * Multiplatform Async Network Library
 * Copyright (c) 2007 Burlex
 *
 * SocketMgr - epoll manager for Linux.
 *
 */

#include "../Network.h"
#include "Threading/LegacyThreading.h"
#include <cassert>

#ifdef CONFIG_USE_EPOLL

//#define ENABLE_ANTI_DOS

void SocketMgr::AddSocket(Socket* s)
{
#ifdef ENABLE_ANTI_DOS
    uint32_t saddr;
    int i, count;

    // Check how many connections we already have from that ip
    saddr = s->GetRemoteAddress().s_addr;
    for(i = 0, count = 0; i <= max_fd; i++)
    {
        if(fds[i])
        {
            if(fds[i]->GetRemoteAddress().s_addr == saddr) count++;
        }
    }

    // More than 16 connections from the same ip? enough! xD
    if(count > 16)
    {
        s->Disconnect(false);
        return;
    }
#endif

    if(fds[s->GetFd()] != NULL)
    {
        //fds[s->GetFd()]->Delete();
        //fds[s->GetFd()] = NULL;
        s->Delete();
        return;
    }

    if(max_fd < s->GetFd()) max_fd = s->GetFd();
    fds[s->GetFd()] = s;
    ++socket_count;

    // Add epoll event based on socket activity.
    struct epoll_event ev;
    memset(&ev, 0, sizeof(epoll_event));
    ev.events = (s->writeBuffer.GetSize()) ? EPOLLOUT : EPOLLIN;
    ev.events |= EPOLLET; // use edge-triggered instead of level-triggered because we're using nonblocking sockets.
    ev.data.fd = s->GetFd();

    if(epoll_ctl(epoll_fd, EPOLL_CTL_ADD, ev.data.fd, &ev))
        sLogger.failure("Could not add event to epoll set on fd {}", s->GetFd());
}

void SocketMgr::AddListenSocket(ListenSocketBase* s)
{
    assert(listenfds[s->GetFd()] == 0);
    listenfds[s->GetFd()] = s;

    // Add epoll event based on socket activity.
    struct epoll_event ev;
    memset(&ev, 0, sizeof(epoll_event));
    ev.events = EPOLLIN;
    ev.events |= EPOLLET; // use edge-triggered instead of level-triggered because we're using nonblocking sockets.
    ev.data.fd = s->GetFd();

    if(epoll_ctl(epoll_fd, EPOLL_CTL_ADD, ev.data.fd, &ev))
        sLogger.failure("Could not add event to epoll set on fd {}", s->GetFd());
}

void SocketMgr::RemoveSocket(Socket* s)
{
    if(fds[s->GetFd()] != s)
    {
        sLogger.failure("Could not remove fd {} from the set due to it not existing?", s->GetFd());
        return;
    }

    fds[s->GetFd()] = NULL;
    --socket_count;

    // Remove from epoll list.
    struct epoll_event ev;
    memset(&ev, 0, sizeof(epoll_event));
    ev.data.fd = s->GetFd();
    ev.events = EPOLLIN | EPOLLOUT | EPOLLERR | EPOLLHUP | EPOLLONESHOT;

    if(epoll_ctl(epoll_fd, EPOLL_CTL_DEL, ev.data.fd, &ev))
        sLogger.failure("Could not remove fd {} from epoll set, errno {}", s->GetFd(), errno);
}

void SocketMgr::CloseAll()
{
    for(uint32_t i = 0; i < SOCKET_HOLDER_SIZE; ++i)
        if(fds[i] != NULL)
            fds[i]->Delete();
}

void SocketMgr::SpawnWorkerThreads()
{
    uint32_t count = 1;
    for(uint32_t i = 0; i < count; ++i)
        ThreadPool.ExecuteTask(new SocketWorkerThread());
}

void SocketMgr::ShowStatus()
{
    sLogger.info("sockets count = {}", static_cast<uint32_t>(socket_count.load()));
}

bool SocketWorkerThread::runThread()
{
    int fd_count;
    Socket* ptr;
    int i;
    running = true;
    // events[i].data.fd = ptr->GetFd();

    while(running)
    {
        fd_count = epoll_wait(sSocketMgr.epoll_fd, events, THREAD_EVENT_SIZE, 5000);
        for(i = 0; i < fd_count; ++i)
        {
            if(events[i].data.fd >= SOCKET_HOLDER_SIZE)
            {
                sLogger.failure("Requested FD that is too high ({})", ptr->GetFd());
                continue;
            }

            ptr = sSocketMgr.fds[events[i].data.fd];

            if(ptr == NULL)
            {
                if((ptr = ((Socket*)sSocketMgr.listenfds[events[i].data.fd])) != NULL)
                    ((ListenSocketBase*)ptr)->OnAccept();
                else
                    sLogger.failure("Returned invalid fd (no pointer) of FD {}", ptr->GetFd());
                continue;
            }

            if(events[i].events & EPOLLHUP || events[i].events & EPOLLERR)
            {
                ptr->Disconnect();
                continue;
            }
            else if(events[i].events & EPOLLIN)
            {
                ptr->ReadCallback(0); // Len is unknown at this point.

                /* changing to written state? */
                if(ptr->writeBuffer.GetSize() && !ptr->HasSendLock() && ptr->IsConnected())
                    ptr->PostEvent(EPOLLOUT);
            }
            else if(events[i].events & EPOLLOUT)
            {
                ptr->BurstBegin();      // Lock receive mutex
                ptr->WriteCallback();   // Perform actual send()
                if(ptr->writeBuffer.GetSize() > 0)
                {
                    /* we don't have to do anything here. no more oneshots :) */
                }
                else
                {
                    /* change back to a read event */
                    ptr->DecSendLock();
                    ptr->PostEvent(EPOLLIN);
                }
                ptr->BurstEnd(); // Unlock
            }
        }
    }
    return true;
}

#endif
