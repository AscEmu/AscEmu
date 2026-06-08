/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "Network/AE/Core/SocketStateHelpers.hpp"

#include <cstdint>

namespace AscEmu::Network::AE::IocpCompletion
{
    inline void handleReadComplete(Socket* socket, uint32_t len)
    {
        if (!AscEmu::Network::AE::isSocketUsable(socket))
            return;

        socket->m_readEvent.Unmark();

        if (len != 0)
        {
            socket->readBuffer.IncrementWritten(len);
            socket->OnRead();
            socket->SetupReadEvent();
        }
        else
        {
            socket->Delete();
        }
    }

    inline void handleWriteComplete(Socket* socket, uint32_t len)
    {
        if (!AscEmu::Network::AE::isSocketUsable(socket))
            return;

        socket->m_writeEvent.Unmark();
        socket->BurstBegin();
        socket->writeBuffer.Remove(len);

        if (socket->writeBuffer.GetContiguiousBytes() > 0)
            socket->WriteCallback();
        else
            socket->DecSendLock();

        socket->BurstEnd();
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
