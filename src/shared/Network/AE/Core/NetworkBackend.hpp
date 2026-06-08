/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "Threading/ThreadPool.hpp"

#include <cstdint>

class Socket;
class ListenSocketBase;

namespace AscEmu::Network::AE
{
    class NetworkBackend
    {
    public:
        virtual ~NetworkBackend() = default;

        virtual void initialize() = 0;
        virtual void finalize() = 0;

        virtual void addSocket(Socket* socket) = 0;
        virtual void removeSocket(Socket* socket) = 0;
        virtual void addListenSocket(ListenSocketBase* socket) = 0;

        virtual void spawnWorkers(AscEmu::Threading::AEThreadPool& threadPool) = 0;
        virtual void shutdownWorkers() = 0;

        virtual void closeAll() = 0;
        virtual void showStatus() = 0;
        virtual uint32_t socketCount() const = 0;
    };
}
