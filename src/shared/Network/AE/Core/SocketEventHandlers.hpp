/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "Network/AE/Core/SocketStateHelpers.hpp"

namespace AscEmu::Network::AE
{
    inline void handleSocketError(Socket& socket)
    {
        disconnectSocket(socket);
    }

    template <typename RearmWriteFn>
    inline void handleSocketReadable(Socket& socket, RearmWriteFn&& rearmWrite)
    {
        if (!isSocketUsable(&socket))
            return;

        socket.ReadCallback(0);

        if (shouldArmWriteAfterRead(socket))
            rearmWrite(socket);
    }

    template <typename OnWritePendingFn, typename OnWriteEmptyFn>
    inline void handleSocketWritable(Socket& socket, OnWritePendingFn&& onWritePending, OnWriteEmptyFn&& onWriteEmpty)
    {
        if (!isSocketUsable(&socket))
            return;

        socket.BurstBegin();
        socket.WriteCallback();

        if (hasPendingWrite(socket))
        {
            onWritePending(socket);
        }
        else
        {
            socket.DecSendLock();
            onWriteEmpty(socket);
        }

        socket.BurstEnd();
    }
}
