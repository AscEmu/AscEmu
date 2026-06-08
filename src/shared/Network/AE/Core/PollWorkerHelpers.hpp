/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "Network/AE/Core/PollSocketDispatch.hpp"
#include "Network/AE/Core/SocketEventHandlers.hpp"

class Socket;
class ListenSocketBase;

namespace AscEmu::Network::AE
{
    template <typename InvalidHandler, typename ListenerHandler, typename EventHandler>
    inline void dispatchPollEvent(int fd, std::size_t holderSize, Socket* const* sockets, ListenSocketBase* const* listeners,
        InvalidHandler&& onInvalid, ListenerHandler&& onListener, EventHandler&& onSocketEvent)
    {
        dispatchPollDescriptor(fd, holderSize, sockets, listeners,
            std::forward<InvalidHandler>(onInvalid), std::forward<ListenerHandler>(onListener), std::forward<EventHandler>(onSocketEvent));
    }

    template <typename HasErrorFn, typename HasReadFn, typename HasWriteFn, typename RearmReadFn, typename RearmWriteFn, typename RearmWritePendingFn>
    inline void handlePollSocketEvent(Socket& socket, HasErrorFn&& hasError, HasReadFn&& hasRead, HasWriteFn&& hasWrite,
        RearmReadFn&& rearmRead, RearmWriteFn&& rearmWrite, RearmWritePendingFn&& rearmWritePending)
    {
        if (hasError())
        {
            handleSocketError(socket);
            return;
        }

        if (hasRead())
        {
            handleSocketReadable(socket, std::forward<RearmWriteFn>(rearmWrite));
            return;
        }

        if (hasWrite())
        {
            handleSocketWritable(socket, std::forward<RearmWritePendingFn>(rearmWritePending), std::forward<RearmReadFn>(rearmRead));
        }
    }
}
