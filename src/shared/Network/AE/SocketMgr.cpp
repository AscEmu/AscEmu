/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "SocketMgr.hpp"

#include "Logging/Logger.hpp"

#ifdef CONFIG_USE_EPOLL
    #include "Network/AE/Backends/EPOLL/EpollBackend.hpp"
#endif

#ifdef CONFIG_USE_IOCP
    #include "Network/AE/Backends/IOCP/IocpBackend.hpp"
#endif

#ifdef CONFIG_USE_KQUEUE
    #include "Network/AE/Backends/KQUEUE/KqueueBackend.hpp"
#endif

SocketMgr& SocketMgr::getInstance()
{
    static SocketMgr instance;
    return instance;
}

std::unique_ptr<AscEmu::Network::AE::NetworkBackend> SocketMgr::createBackend(SocketMgr& owner)
{
#ifdef CONFIG_USE_IOCP
    return std::make_unique<AscEmu::Network::AE::IocpBackend>(owner);
#elif defined(CONFIG_USE_EPOLL)
    return std::make_unique<AscEmu::Network::AE::EpollBackend>(owner);
#elif defined(CONFIG_USE_KQUEUE)
    return std::make_unique<AscEmu::Network::AE::KqueueBackend>(owner);
#else
sLogger.failure("No supported AE network backend configured.");
    return nullptr;
#endif
}

void SocketMgr::initialize()
{
    if (m_backend == nullptr)
        m_backend = createBackend(*this);

    m_backend->initialize();
}

void SocketMgr::finalize()
{
    if (m_backend == nullptr)
        return;

    m_backend->finalize();
    m_backend.reset();
}

void SocketMgr::AddSocket(Socket* socket)
{
    if (m_backend == nullptr)
        return;

    m_backend->addSocket(socket);
}

void SocketMgr::AddListenSocket(ListenSocketBase* socket)
{
    if (m_backend == nullptr)
        return;

    m_backend->addListenSocket(socket);
}

void SocketMgr::RemoveSocket(Socket* socket)
{
    if (m_backend == nullptr)
        return;

    m_backend->removeSocket(socket);
}

void SocketMgr::SpawnWorkerThreads()
{
    if (m_backend == nullptr || m_threadPool == nullptr)
        return;

    m_backend->spawnWorkers(*m_threadPool);
}

void SocketMgr::ShutdownThreads()
{
    if (m_backend == nullptr)
        return;

    m_backend->shutdownWorkers();
}

void SocketMgr::CloseAll()
{
    if (m_backend == nullptr)
        return;

    m_backend->closeAll();
}

void SocketMgr::ShowStatus()
{
    if (m_backend == nullptr)
        return;

    m_backend->showStatus();
}

uint32_t SocketMgr::GetSocketCount() const
{
    if (m_backend == nullptr)
        return 0;

    return m_backend->socketCount();
}

#ifdef CONFIG_USE_IOCP
HANDLE SocketMgr::GetCompletionPort() const
{
    if (m_backend == nullptr)
        return INVALID_HANDLE_VALUE;

    const auto* backend = dynamic_cast<const AscEmu::Network::AE::IocpBackend*>(m_backend.get());
    return backend != nullptr ? backend->completionPort() : INVALID_HANDLE_VALUE;
}
#endif

#ifdef CONFIG_USE_EPOLL
int SocketMgr::GetEpollFd() const
{
    if (m_backend == nullptr)
        return -1;

    const auto* backend = dynamic_cast<const AscEmu::Network::AE::EpollBackend*>(m_backend.get());
    return backend != nullptr ? backend->epollFd() : -1;
}
#endif

#ifdef CONFIG_USE_KQUEUE
int SocketMgr::GetKq() const
{
    if (m_backend == nullptr)
        return -1;

    const auto* backend = dynamic_cast<const AscEmu::Network::AE::KqueueBackend*>(m_backend.get());
    return backend != nullptr ? backend->kqueueFd() : -1;
}
#endif
