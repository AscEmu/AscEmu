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

#ifdef ASCEMU_USE_AE_NETWORK_THREADPOOL
    #include "Threading/AEThreadPool.h"
#else
    #include "Threading/LegacyThreadPool.h"
#endif

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

#ifdef ASCEMU_USE_AE_NETWORK_THREADPOOL
    if (m_threadPool == nullptr)
    {
        sLogger.failure("SocketMgr::SpawnWorkerThreads called without AEThreadPool.");
        return;
    }

    if (!m_workerThreads.empty())
        return;

    m_workerThreads.reserve(static_cast<size_t>(threadcount));

    for (long x = 0; x < threadcount; ++x)
    {
        auto& worker = m_threadPool->addDedicatedThread(
            "IOCP Worker " + std::to_string(x),
            [this](AscEmu::Threading::AEThread& self)
            {
                WorkerThreadLoop(self);
            }
        );

        m_workerThreads.push_back(&worker);
    }
#else
    for (long x = 0; x < threadcount; ++x)
        ThreadPool.ExecuteTask(new SocketWorkerThread());
#endif
}

#ifdef ASCEMU_USE_AE_NETWORK_THREADPOOL
void SocketMgr::WorkerThreadLoop(AscEmu::Threading::AEThread& self)
{
    try
    {
        HANDLE completionPort = GetCompletionPort();
        DWORD bytesTransferred = 0;
        ULONG_PTR completionKey = 0;
        LPOVERLAPPED overlappedPtr = nullptr;

        while (!self.isKilled())
        {
            if (!GetQueuedCompletionStatus(completionPort, &bytesTransferred, &completionKey, &overlappedPtr, 1000))
                continue;

            if (overlappedPtr == nullptr)
                continue;

            Socket* socket = reinterpret_cast<Socket*>(completionKey);
            OverlappedStruct* overlapped = CONTAINING_RECORD(overlappedPtr, OverlappedStruct, m_overlap);

            if (overlapped->m_event == SOCKET_IO_THREAD_SHUTDOWN)
            {
                delete overlapped;
                return;
            }

            if (overlapped->m_event < NUM_SOCKET_IO_EVENTS)
                ophandlers[overlapped->m_event](socket, bytesTransferred);
        }
    }
    catch (const std::exception& e)
    {
        sLogger.failure("IOCP worker stopped due to C++ exception: {}", e.what());
    }
    catch (...)
    {
        sLogger.failure("IOCP worker stopped due to an unknown C++ exception.");
    }
}
#else
bool SocketWorkerThread::runThread()
{
    try
    {
        HANDLE completionPort = sSocketMgr.GetCompletionPort();
        DWORD bytesTransferred = 0;
        ULONG_PTR completionKey = 0;
        LPOVERLAPPED overlappedPtr = nullptr;

        while (true)
        {
            // ULONG_PTR handles both 32-bit and 64-bit architectures automatically
            if (!GetQueuedCompletionStatus(completionPort, &bytesTransferred, &completionKey, &overlappedPtr, 10000))
                continue;

            Socket* socket = reinterpret_cast<Socket*>(completionKey);
            OverlappedStruct* overlapped = CONTAINING_RECORD(overlappedPtr, OverlappedStruct, m_overlap);

            if (overlapped->m_event == SOCKET_IO_THREAD_SHUTDOWN)
            {
                delete overlapped; // Clean up the shutdown signal object
                return true;
            }

            if (overlapped->m_event < NUM_SOCKET_IO_EVENTS)
                ophandlers[overlapped->m_event](socket, bytesTransferred);
        }
    }
    catch (const std::exception& e)
    {
        sLogger.failure("SocketWorkerThread stopped due to C++ exception: {}", e.what());
    }
    catch (...)
    {
        sLogger.failure("SocketWorkerThread stopped due to an unknown C++ exception.");
    }

    return true;
}
#endif

void HandleReadComplete(Socket* s, uint32_t len)
{
    //s->m_readEvent= NULL;
    if (!s->IsDeleted())
    {
        s->m_readEvent.Unmark();
        if (len)
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
        if (s->writeBuffer.GetContiguiousBytes() > 0)
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

    // Write toKill sockets is locked
    {
        std::lock_guard lock{socketLock};

        for (std::set<Socket*>::iterator itr = _sockets.begin(); itr != _sockets.end(); ++itr)
            tokill.push_back(*itr);
    }


    for (std::vector<Socket*>::iterator itr = tokill.begin(); itr != tokill.end(); ++itr)
        (*itr)->Disconnect();

    size_t size = 0;
    do
    {
        // Read size of sockets is locked
        {
            std::lock_guard lock{socketLock};
            size = _sockets.size();
        }
    }
    while(size);
}

void SocketMgr::ShutdownThreads()
{
#ifdef ASCEMU_USE_AE_NETWORK_THREADPOOL
    for (auto* worker : m_workerThreads)
    {
        if (worker != nullptr)
            worker->requestKill();
    }
#endif

    for (int i = 0; i < threadcount; ++i)
    {
        OverlappedStruct* ov = new OverlappedStruct(SOCKET_IO_THREAD_SHUTDOWN);
        PostQueuedCompletionStatus(m_completionPort, 0, (ULONG_PTR)0, &ov->m_overlap);
    }

#ifdef ASCEMU_USE_AE_NETWORK_THREADPOOL
    for (auto* worker : m_workerThreads)
    {
        if (worker != nullptr)
            worker->join();
    }

    m_workerThreads.clear();
#endif
}

void SocketMgr::ShowStatus()
{
    sLogger.info("sockets count = {}", static_cast<uint32_t>(socket_count.load()));
}

#endif
