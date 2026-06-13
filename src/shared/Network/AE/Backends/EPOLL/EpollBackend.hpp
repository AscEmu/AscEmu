/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "Logging/Logger.hpp"
#include "Network/AE/Core/NetworkBackend.hpp"
#include "Network/AE/Core/PollBackendLoop.hpp"
#include "Network/AE/Core/PollWorkerHelpers.hpp"
#include "Network/AE/Core/SocketEventHandlers.hpp"
#include "Network/AE/Core/NetworkBackendCommon.hpp"
#include "Network/Socket.h"

#include <atomic>
#include <cstring>

#ifndef SOCKET_HOLDER_SIZE
    #define SOCKET_HOLDER_SIZE 30000
#endif

#ifndef THREAD_EVENT_SIZE
    #define THREAD_EVENT_SIZE 4096
#endif

class SocketMgr;

namespace AscEmu::Network::AE
{
    class EpollBackend final : public NetworkBackend
    {
    public:
        explicit EpollBackend(SocketMgr& owner)
            : m_owner(owner)
        {
            std::memset(m_fds, 0, sizeof(m_fds));
            std::memset(m_listenFds, 0, sizeof(m_listenFds));
        }

        void initialize() override
        {
            m_epollFd = epoll_create(SOCKET_HOLDER_SIZE);
            if (m_epollFd == -1)
            {
                sLogger.failure("Could not create epoll fd.");
                std::exit(-1);
            }
        }

        void finalize() override
        {
            shutdownWorkers();

            if (m_epollFd != -1)
            {
                close(m_epollFd);
                m_epollFd = -1;
            }
        }

        void addSocket(Socket* socket) override
        {
            if (socket == nullptr)
                return;

            if (m_fds[socket->GetFd()] != nullptr)
            {
                socket->Delete();
                return;
            }

            m_fds[socket->GetFd()] = socket;
            ++m_socketCount;

            epoll_event ev{};
            ev.events = (socket->writeBuffer.GetSize() > 0) ? EPOLLOUT : EPOLLIN;
            ev.events |= EPOLLET;
            ev.data.fd = socket->GetFd();

            if (epoll_ctl(m_epollFd, EPOLL_CTL_ADD, ev.data.fd, &ev) != 0)
                sLogger.failure("Could not add event to epoll set on fd {}", socket->GetFd());
        }

        void removeSocket(Socket* socket) override
        {
            if (socket == nullptr)
                return;

            if (m_fds[socket->GetFd()] != socket)
                return;

            m_fds[socket->GetFd()] = nullptr;
            --m_socketCount;

            epoll_event ev{};
            ev.data.fd = socket->GetFd();
            ev.events = EPOLLIN | EPOLLOUT | EPOLLERR | EPOLLHUP | EPOLLONESHOT;
            epoll_ctl(m_epollFd, EPOLL_CTL_DEL, ev.data.fd, &ev);
        }

        void addListenSocket(ListenSocketBase* socket) override
        {
            if (socket == nullptr)
                return;

            m_listenFds[socket->GetFd()] = socket;

            epoll_event ev{};
            ev.events = EPOLLIN | EPOLLET;
            ev.data.fd = socket->GetFd();

            if (epoll_ctl(m_epollFd, EPOLL_CTL_ADD, ev.data.fd, &ev) != 0)
                sLogger.failure("Could not add event to epoll set on fd {}", socket->GetFd());
        }

        void spawnWorkers(AscEmu::Threading::AEThreadPool& threadPool) override
        {
            std::lock_guard lock(m_workerLock);

            m_workerSet.spawn(
                threadPool,
                m_workers,
                1,
                "EPOLL Worker",
                [this](AscEmu::Threading::AEThread& self)
                {
                    this->workerLoop(self);
                }
            );
        }

        void shutdownWorkers() override
        {
            std::vector<AscEmu::Threading::AEThread*> workers;

            {
                std::lock_guard lock(m_workerLock);
                workers.swap(m_workers);
            }

            m_workerSet.shutdown(workers);
        }

        void closeAll() override
        {
            for (uint32_t i = 0; i < SOCKET_HOLDER_SIZE; ++i)
            {
                if (m_fds[i] != nullptr)
                    m_fds[i]->Delete();
            }
        }

        void showStatus() override
        {
            sLogger.info("sockets count = {}", static_cast<uint32_t>(m_socketCount.load()));
        }

        uint32_t socketCount() const override
        {
            return m_socketCount.load();
        }

        int epollFd() const
        {
            return m_epollFd;
        }

    private:
        void workerLoop(AscEmu::Threading::AEThread& self)
        {
            epoll_event events[THREAD_EVENT_SIZE];

            AscEmu::Network::AE::runPollBackendLoop(
                self,
                events,
                THREAD_EVENT_SIZE,
                [this](epoll_event* eventStorage, int capacity)
                {
                    return epoll_wait(m_epollFd, eventStorage, capacity, 1000);
                },
                [](const epoll_event& event)
                {
                    return event.data.fd;
                },
                SOCKET_HOLDER_SIZE,
                m_fds,
                m_listenFds,
                [](int invalidFd, bool outOfRange)
                {
                    if (outOfRange)
                        sLogger.failure("Requested FD that is too high ({})", invalidFd);
                    else
                        sLogger.failure("Returned invalid fd (no pointer) of FD {}", invalidFd);
                },
                [](ListenSocketBase& listener, int, const epoll_event&)
                {
                    listener.OnAccept();
                },
                [](Socket& socket, int, const epoll_event& event)
                {
                    AscEmu::Network::AE::handlePollSocketEvent(
                        socket,
                        [&]()
                        {
                            return (event.events & EPOLLHUP) != 0 || (event.events & EPOLLERR) != 0;
                        },
                        [&]()
                        {
                            return (event.events & EPOLLIN) != 0;
                        },
                        [&]()
                        {
                            return (event.events & EPOLLOUT) != 0;
                        },
                        [](Socket& writableSocket)
                        {
                            writableSocket.PostEvent(EPOLLIN);
                        },
                        [](Socket& readableSocket)
                        {
                            readableSocket.PostEvent(EPOLLOUT);
                        },
                        [](Socket&)
                        {
                        }
                    );
                }
            );
        }

    private:
        SocketMgr& m_owner;
        int m_epollFd = -1;
        Socket* m_fds[SOCKET_HOLDER_SIZE]{};
        ListenSocketBase* m_listenFds[SOCKET_HOLDER_SIZE]{};
        std::atomic_uint32_t m_socketCount{ 0 };

        std::mutex m_workerLock;
        std::vector<AscEmu::Threading::AEThread*> m_workers;
        BackendWorkerSet m_workerSet;
    };
}
