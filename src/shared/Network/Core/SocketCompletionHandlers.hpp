/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "Network/Core/SocketStateHelpers.hpp"

#include <cstdint>

namespace AscEmu::Network::IocpCompletion
{
    inline void handleReadComplete(Socket* socket, uint32_t len)
    {
        if (!AscEmu::Network::isSocketUsable(socket))
            return;

        socket->m_readEvent.Unmark();

        if (len != 0)
        {
            socket->readBuffer.IncrementWritten(len);
            socket->onRead();
            socket->setupReadEvent();
        }
        else
        {
            socket->deleteSocket();
        }
    }

    inline void handleWriteComplete(Socket* socket, uint32_t len)
    {
        if (!AscEmu::Network::isSocketUsable(socket))
            return;

        socket->m_writeEvent.Unmark();
        socket->burstBegin();
        socket->writeBuffer.Remove(len);

        if (socket->writeBuffer.GetContiguiousBytes() > 0)
            socket->writeCallback();
        else
            socket->decrementSendLock();

        socket->burstEnd();
        socket->completeDelayedDisconnectIfReady();
    }

    inline void handleShutdown(Socket* /*socket*/, uint32_t /*len*/)
    {
    }

    using Handler = void(*)(Socket*, uint32_t);

    inline constexpr Handler handlers[NUM_SOCKET_IO_EVENTS] =
    {
        &handleReadComplete,
        &handleWriteComplete,
        &handleShutdown
    };
}
