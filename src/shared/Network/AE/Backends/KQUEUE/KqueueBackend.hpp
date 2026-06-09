/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "Network/AE/Core/NetworkBackend.hpp"
#include "Network/AE/Core/PollBackendLoop.hpp"
#include "Network/AE/Core/PollWorkerHelpers.hpp"
#include "Network/AE/Core/SocketEventHandlers.hpp"
#include "Network/SocketDefines.h"

#include <atomic>
#include <cstring>

class SocketMgr;

namespace AscEmu::Network::AE
{
    class KqueueBackend final : public NetworkBackend
    {
    public:
        explicit KqueueBackend(SocketMgr& owner)
            : m_owner(owner)
        {
            std::memset(m_fds, 0, sizeof(m_fds));
            std::memset(m_listenFds, 0, sizeof(m_listenFds));
        }

        void initialize() override
        {
            m_kq = kqueue();
            if (m_kq == -1)
            {
                sLogger.failure("Could not create a kqueue fd.");
                std::exit(-1);
            }
        }

        void finalize() override
        {
            shutdownWorkers();

            if (m_kq != -1)
            {
                close(m_kq);
                m_kq = -1;
            }
        }

        void addSocket(Socket* socket) override
        {
            if (socket == nullptr)
                return;

            m_fds[socket->GetFd()] = socket;
            ++m_socketCount;

            struct kevent ev;
            if (socket->writeBuffer.GetSize() > 0)
                EV_SET(&ev, socket->GetFd(), EVFILT_WRITE, EV_ADD | EV_ONESHOT, 0, 0, nullptr);
            else
                EV_SET(&ev, socket->GetFd(), EVFILT_READ, EV_ADD, 0, 0, nullptr);

            kevent(m_kq, &ev, 1, nullptr, 0, nullptr);
        }

        void removeSocket(Socket* socket) override
        {
            if (socket == nullptr)
                return;

            if (m_fds[socket->GetFd()] != socket)
                return;

            m_fds[socket->GetFd()] = nullptr;
            --m_socketCount;

            struct kevent evWrite;
            struct kevent evRead;
            EV_SET(&evWrite, socket->GetFd(), EVFILT_WRITE, EV_DELETE, 0, 0, nullptr);
            EV_SET(&evRead, socket->GetFd(), EVFILT_READ, EV_DELETE, 0, 0, nullptr);
            kevent(m_kq, &evWrite, 1, nullptr, 0, nullptr);
            kevent(m_kq, &evRead, 1, nullptr, 0, nullptr);
        }

        void addListenSocket(ListenSocketBase* socket) override
        {
            if (socket == nullptr)
                return;

            m_listenFds[socket->GetFd()] = socket;

            struct kevent ev;
            EV_SET(&ev, socket->GetFd(), EVFILT_READ, EV_ADD, 0, 0, nullptr);
            kevent(m_kq, &ev, 1, nullptr, 0, nullptr);
        }

        void spawnWorkers(AscEmu::Threading::AEThreadPool& threadPool) override
        {
            std::lock_guard lock(m_workerLock);

            m_workerSet.spawn(
                threadPool,
                m_workers,
                1,
                "KQUEUE Worker",
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
            sLogger.info("Sockets: {}", static_cast<uint32_t>(m_socketCount.load()));
        }

        uint32_t socketCount() const override
        {
            return m_socketCount.load();
        }

        int kqueueFd() const
        {
            return m_kq;
        }

    private:
        void workerLoop(AscEmu::Threading::AEThread& self)
        {
            struct kevent events[THREAD_EVENT_SIZE];

            AscEmu::Network::AE::runPollBackendLoop(
                self,
                events,
                THREAD_EVENT_SIZE,
                [this](kevent* eventStorage, int capacity)
                {
                    struct timespec ts;
                    ts.tv_nsec = 0;
                    ts.tv_sec = 1;
                    return kevent(m_kq, nullptr, 0, eventStorage, capacity, &ts);
                },
                [](const kevent& event)
                {
                    return static_cast<int>(event.ident);
                },
                SOCKET_HOLDER_SIZE,
                m_fds,
                m_listenFds,
                [this](int invalidFd, bool outOfRange)
                {
                    if (outOfRange)
                    {
                        sLogger.warning("kqueue : Requested FD that is too high ({})", invalidFd);
                        return;
                    }

                    sLogger.warning("kqueue : Returned invalid fd (no pointer) of FD {}", invalidFd);

                    struct kevent evWrite;
                    struct kevent evRead;
                    EV_SET(&evWrite, invalidFd, EVFILT_WRITE, EV_DELETE, 0, 0, nullptr);
                    EV_SET(&evRead, invalidFd, EVFILT_READ, EV_DELETE, 0, 0, nullptr);
                    kevent(m_kq, &evWrite, 1, nullptr, 0, nullptr);
                    kevent(m_kq, &evRead, 1, nullptr, 0, nullptr);
                },
                [](ListenSocketBase& listener, int, const kevent&)
                {
                    listener.OnAccept();
                },
                [](Socket& socket, int, const kevent& event)
                {
                    AscEmu::Network::AE::handlePollSocketEvent(
                        socket,
                        [&]()
                        {
                            return (event.flags & EV_EOF) != 0 || (event.flags & EV_ERROR) != 0;
                        },
                        [&]()
                        {
                            return event.filter == EVFILT_READ;
                        },
                        [&]()
                        {
                            return event.filter == EVFILT_WRITE;
                        },
                        [](Socket& writableSocket)
                        {
                            writableSocket.PostEvent(EVFILT_READ, false);
                        },
                        [](Socket& readableSocket)
                        {
                            readableSocket.PostEvent(EVFILT_WRITE, true);
                            readableSocket.IncSendLock();
                        },
                        [](Socket& writableSocket)
                        {
                            writableSocket.PostEvent(EVFILT_WRITE, true);
                        }
                    );
                }
            );
        }

    private:
        SocketMgr& m_owner;
        int m_kq = -1;
        Socket* m_fds[SOCKET_HOLDER_SIZE]{};
        ListenSocketBase* m_listenFds[SOCKET_HOLDER_SIZE]{};
        std::atomic_uint32_t m_socketCount{ 0 };

        std::mutex m_workerLock;
        std::vector<AscEmu::Threading::AEThread*> m_workers;
        BackendWorkerSet m_workerSet;
    };
}
