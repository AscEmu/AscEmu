/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "Threading/Thread.hpp"
#include "Network/AE/Core/SocketDescriptorDispatch.hpp"

#include <cstddef>

class Socket;
class ListenSocketBase;

namespace AscEmu::Network::AE
{
    template <typename EventT, typename WaitFn, typename FdExtractor, typename InvalidHandler, typename ListenerHandler, typename SocketHandler>
    inline void runPollBackendLoop(
        AscEmu::Threading::AEThread& self,
        EventT* events,
        std::size_t eventCapacity,
        WaitFn&& waitForEvents,
        FdExtractor&& extractFd,
        std::size_t holderSize,
        Socket* const* sockets,
        ListenSocketBase* const* listeners,
        InvalidHandler&& onInvalid,
        ListenerHandler&& onListener,
        SocketHandler&& onSocket
    )
    {
        while (!self.isKilled())
        {
            const int eventCount = waitForEvents(events, static_cast<int>(eventCapacity));
            if (eventCount <= 0)
                continue;

            for (int i = 0; i < eventCount; ++i)
            {
                const int fd = extractFd(events[i]);
                const auto lookup = lookupDescriptor(fd, holderSize, sockets, listeners);

                if (lookup.outOfRange)
                {
                    onInvalid(fd, true);
                    continue;
                }

                if (lookup.hasListener())
                {
                    onListener(*lookup.listener, fd, events[i]);
                    continue;
                }

                if (!lookup.hasSocket())
                {
                    onInvalid(fd, false);
                    continue;
                }

                onSocket(*lookup.socket, fd, events[i]);
            }
        }
    }
}
