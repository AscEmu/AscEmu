/*
 * Multiplatform Async Network Library
 * Copyright (c) 2007 Burlex
 *
 * SocketMgr - iocp-based SocketMgr for windows.
 *
 */

#include <vector>

#include "../Network.h"

#ifdef CONFIG_USE_IOCP

#include "Debugging/CrashHandler.h"

SocketMgr& SocketMgr::getInstance()
{
    static SocketMgr mInstance;
    return mInstance;
}

void SocketMgr::initialize()
{
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 0), &wsaData);
    m_completionPort = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, (ULONG_PTR)0, 0);
}

void SocketMgr::SpawnWorkerThreads()
{
    SYSTEM_INFO si;
    GetSystemInfo(&si);

    threadcount = si.dwNumberOfProcessors;

    sLogger.info("IOCP: Spawning {} worker threads.", threadcount);
    for(long x = 0; x < threadcount; ++x)
        ThreadPool.ExecuteTask(new SocketWorkerThread());
}

bool SocketWorkerThread::runThread()
{
    THREAD_TRY_EXECUTION

    HANDLE cp = sSocketMgr.GetCompletionPort();
    DWORD len;
    Socket* s;
    OverlappedStruct* ov;
    LPOVERLAPPED ol_ptr;

    while(true)
    {
#ifndef _WIN64
        if(!GetQueuedCompletionStatus(cp, &len, (LPDWORD)&s, &ol_ptr, 10000))
#else
        if(!GetQueuedCompletionStatus(cp, &len, (PULONG_PTR)&s, &ol_ptr, 10000))
#endif
            continue;

        ov = CONTAINING_RECORD(ol_ptr, OverlappedStruct, m_overlap);

        if(ov->m_event == SOCKET_IO_THREAD_SHUTDOWN)
        {
            delete ov;
            return true;
        }

        if(ov->m_event < NUM_SOCKET_IO_EVENTS)
            ophandlers[ov->m_event](s, len);
    }

    THREAD_HANDLE_CRASH

    return true;
}

void HandleReadComplete(Socket* s, uint32_t len)
{
    //s->m_readEvent= NULL;
    if(!s->IsDeleted())
    {
        s->m_readEvent.Unmark();
        if(len)
        {
            s->readBuffer.IncrementWritten(len);
            s->OnRead();
            s->SetupReadEvent();
        }
        else
            s->Delete(); // Queue deletion.
    }
}

void HandleWriteComplete(Socket* s, uint32_t len)
{
    if(!s->IsDeleted())
    {
        s->m_writeEvent.Unmark();
        s->BurstBegin(); // Lock
        s->writeBuffer.Remove(len);
        if(s->writeBuffer.GetContiguiousBytes() > 0)
            s->WriteCallback();
        else
            s->DecSendLock();
        s->BurstEnd(); // Unlock
    }
}

void HandleShutdown(Socket* /*s*/, uint32_t /*len*/)
{

}

void SocketMgr::CloseAll()
{
    std::vector<Socket*> tokill;

    socketLock.acquire();
    for(std::set<Socket*>::iterator itr = _sockets.begin(); itr != _sockets.end(); ++itr)
        tokill.push_back(*itr);
    socketLock.release();

    for(std::vector<Socket*>::iterator itr = tokill.begin(); itr != tokill.end(); ++itr)
        (*itr)->Disconnect();

    size_t size;
    do
    {
        socketLock.acquire();
        size = _sockets.size();
        socketLock.release();
    }
    while(size);
}

void SocketMgr::ShutdownThreads()
{
    for(int i = 0; i < threadcount; ++i)
    {
        OverlappedStruct* ov = new OverlappedStruct(SOCKET_IO_THREAD_SHUTDOWN);
        PostQueuedCompletionStatus(m_completionPort, 0, (ULONG_PTR)0, &ov->m_overlap);
    }
}

void SocketMgr::ShowStatus()
{
    sLogger.info("sockets count = {}", static_cast<uint32_t>(socket_count.load()));
}

#endif