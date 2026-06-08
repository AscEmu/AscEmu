/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <cstddef>

class Socket;
class ListenSocketBase;

namespace AscEmu::Network::AE
{
    struct DescriptorLookupResult
    {
        bool outOfRange = false;
        Socket* socket = nullptr;
        ListenSocketBase* listener = nullptr;

        [[nodiscard]] bool hasSocket() const
        {
            return socket != nullptr;
        }

        [[nodiscard]] bool hasListener() const
        {
            return listener != nullptr;
        }

        [[nodiscard]] bool isUnhandled() const
        {
            return !outOfRange && socket == nullptr && listener == nullptr;
        }
    };

    inline DescriptorLookupResult lookupDescriptor(int fd, std::size_t holderSize, Socket* const* sockets, ListenSocketBase* const* listeners)
    {
        DescriptorLookupResult result{};

        if (fd < 0 || static_cast<std::size_t>(fd) >= holderSize)
        {
            result.outOfRange = true;
            return result;
        }

        result.socket = sockets[fd];
        if (result.socket != nullptr)
            return result;

        result.listener = listeners[fd];
        return result;
    }
}
