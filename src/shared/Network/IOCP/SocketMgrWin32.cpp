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

#include "Threading/ThreadPool.hpp"
#include "Debugging/CrashHandler.h"

SocketMgr& SocketMgr::getInstance()
{
    static SocketMgr mInstance;
    return mInstance;
}

void SocketMgr::initialize()
{
#ifdef ASCEMU_USE_AE_NETWORK
    m_backend = std::make_unique<AscEmu::Network::AE::IocpBackend>(*this);
    m_backend->initialize();
#else
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 0), &wsaData);
    m_completionPort = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, (ULONG_PTR)0, 0);
#endif
}

void SocketMgr::finalize()
{
#ifdef ASCEMU_USE_AE_NETWORK
    if (m_backend != nullptr)
    {
        m_backend->finalize();
        m_backend.reset();
    }
#endif
}

void SocketMgr::SpawnWorkerThreads()
{
#ifdef ASCEMU_USE_AE_NETWORK
    if (m_threadPool == nullptr)
    {
        sLogger.failure("SocketMgr::SpawnWorkerThreads called without AEThreadPool.");
        return;
    }

    if (m_backend != nullptr)
    {
        m_backend->spawnWorkers(*m_threadPool);
        return;
    }
#else
    SYSTEM_INFO si;
    GetSystemInfo(&si);

    threadcount = si.dwNumberOfProcessors;
    sLogger.info("IOCP: Spawning {} worker threads.", threadcount);

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
#endif
}

#ifndef ASCEMU_USE_AE_NETWORK
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
    if (!s->IsDeleted())
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
{}

void SocketMgr::CloseAll()
{
#ifdef ASCEMU_USE_AE_NETWORK
    if (m_backend != nullptr)
    {
        m_backend->closeAll();
        return;
    }
#else
    std::vector<Socket*> tokill;

    // Write toKill sockets is locked
    {
        std::lock_guard lock{ socketLock };

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
            std::lock_guard lock{ socketLock };
            size = _sockets.size();
        }
    } while (size);
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

    for (int i = 0; i < threadcount; ++i)
    {
        OverlappedStruct* ov = new OverlappedStruct(SOCKET_IO_THREAD_SHUTDOWN);
        PostQueuedCompletionStatus(m_completionPort, 0, (ULONG_PTR)0, &ov->m_overlap);
    }

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
#ifdef ASCEMU_USE_AE_NETWORK
    if (m_backend != nullptr)
    {
        m_backend->showStatus();
        return;
    }
#else
    sLogger.info("sockets count = {}", static_cast<uint32_t>(socket_count.load()));
#endif
}

#endif
