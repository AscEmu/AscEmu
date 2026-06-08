/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "Network/Socket.h"

namespace AscEmu::Network::AE
{
    inline bool isSocketUsable(Socket* socket)
    {
        return socket != nullptr && !socket->IsDeleted();
    }

    inline bool hasPendingWrite(Socket& socket)
    {
        return socket.writeBuffer.GetSize() > 0;
    }

#if defined(CONFIG_USE_EPOLL) || defined(CONFIG_USE_KQUEUE)
    inline bool shouldArmWriteAfterRead(Socket& socket)
    {
        return socket.writeBuffer.GetSize() > 0 && socket.IsConnected() && !socket.HasSendLock();
    }
#endif

    inline void disconnectSocket(Socket& socket)
    {
        if (!socket.IsDeleted())
            socket.Disconnect();
    }
}
