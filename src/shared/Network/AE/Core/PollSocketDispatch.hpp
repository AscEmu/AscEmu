/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "Network/AE/Core/SocketDescriptorDispatch.hpp"

class Socket;
class ListenSocketBase;

namespace AscEmu::Network::AE
{
    template <typename InvalidHandler, typename ListenerHandler, typename SocketHandler>
    inline void dispatchPollDescriptor(int fd, std::size_t holderSize, Socket* const* sockets, ListenSocketBase* const* listeners,
        InvalidHandler&& onInvalid, ListenerHandler&& onListener, SocketHandler&& onSocket)
    {
        const auto lookup = lookupDescriptor(fd, holderSize, sockets, listeners);

        if (lookup.outOfRange)
        {
            onInvalid(fd, true);
            return;
        }

        if (lookup.hasListener())
        {
            onListener(*lookup.listener, fd);
            return;
        }

        if (!lookup.hasSocket())
        {
            onInvalid(fd, false);
            return;
        }

        onSocket(*lookup.socket, fd);
    }
}
