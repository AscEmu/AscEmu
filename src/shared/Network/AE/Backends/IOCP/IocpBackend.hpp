/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "Network/AE/Core/NetworkBackend.hpp"
#include "Network/AE/Core/SocketCompletionHandlers.hpp"
#include "Network/AE/Core/NetworkBackendCommon.hpp"
#include "Network/SocketDefines.h"

#include <atomic>
#include <mutex>
#include <set>
#include <vector>
#include <windows.h>

class SocketMgr;

namespace AscEmu::Network::AE
{
    class IocpBackend final : public NetworkBackend
    {
    public:
        explicit IocpBackend(SocketMgr& owner)
            : m_owner(owner)
        {
        }

        void initialize() override
        {
            WSADATA wsaData;
            WSAStartup(MAKEWORD(2, 0), &wsaData);
            m_completionPort = CreateIoCompletionPort(INVALID_HANDLE_VALUE, nullptr, static_cast<ULONG_PTR>(0), 0);
        }

        void finalize() override
        {
            shutdownWorkers();

            if (m_completionPort != INVALID_HANDLE_VALUE && m_completionPort != nullptr)
            {
                CloseHandle(m_completionPort);
                m_completionPort = INVALID_HANDLE_VALUE;
            }
        }

        void addSocket(Socket* socket) override
        {
            std::lock_guard lock(m_socketLock);
            m_sockets.insert(socket);
            ++m_socketCount;
        }

        void removeSocket(Socket* socket) override
        {
            std::lock_guard lock(m_socketLock);
            m_sockets.erase(socket);
            --m_socketCount;
        }

        void addListenSocket(ListenSocketBase* /*socket*/) override
        {
        }

        void spawnWorkers(AscEmu::Threading::AEThreadPool& threadPool) override
        {
            SYSTEM_INFO si;
            GetSystemInfo(&si);
            const uint32_t count = static_cast<uint32_t>(si.dwNumberOfProcessors);

            std::lock_guard lock(m_workerLock);

            m_workerSet.spawn(
                threadPool,
                m_workers,
                count,
                "IOCP Worker",
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

            for (std::size_t i = 0; i < workers.size(); ++i)
            {
                OverlappedStruct* ov = new OverlappedStruct(SOCKET_IO_THREAD_SHUTDOWN);
                PostQueuedCompletionStatus(m_completionPort, 0, static_cast<ULONG_PTR>(0), &ov->m_overlap);
            }

            m_workerSet.shutdown(workers);
        }

        void closeAll() override
        {
            std::vector<Socket*> tokill;

            {
                std::lock_guard lock(m_socketLock);
                for (auto* socket : m_sockets)
                    tokill.push_back(socket);
            }

            for (auto* socket : tokill)
                socket->Disconnect();
        }

        void showStatus() override
        {
            sLogger.info("sockets count = {}", static_cast<uint32_t>(m_socketCount.load()));
        }

        uint32_t socketCount() const override
        {
            return m_socketCount.load();
        }

        HANDLE completionPort() const
        {
            return m_completionPort;
        }

    private:
        void workerLoop(AscEmu::Threading::AEThread& self)
        {
            while (!self.isKilled())
            {
                DWORD bytesTransferred = 0;
                ULONG_PTR completionKey = 0;
                LPOVERLAPPED overlappedPtr = nullptr;

                if (!GetQueuedCompletionStatus(
                        m_completionPort,
                        &bytesTransferred,
                        &completionKey,
                        &overlappedPtr,
                        1000))
                {
                    continue;
                }

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
                    AscEmu::Network::AE::IocpCompletion::handlers[overlapped->m_event](socket, bytesTransferred);
            }
        }

    private:
        SocketMgr& m_owner;
        HANDLE m_completionPort = INVALID_HANDLE_VALUE;

        std::set<Socket*> m_sockets;
        std::mutex m_socketLock;
        std::atomic_uint32_t m_socketCount{ 0 };

        std::mutex m_workerLock;
        std::vector<AscEmu::Threading::AEThread*> m_workers;
        BackendWorkerSet m_workerSet;
    };
}
