/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "Network/AE/Core/Resolver.hpp"
#include "Network/SocketDefines.h"
#include "Logging/Logger.hpp"

namespace AscEmu::Network::AE
{
    inline bool resolveListenAddress(const char* listenAddress, uint16_t port, SocketAddressIPv4& outAddress)
    {
        return Resolver::resolvePassiveIPv4(listenAddress, port, outAddress);
    }

    inline bool bindAndListenSocket(SOCKET socket, const SocketAddressIPv4& address, uint32_t port, int backlog = 5)
    {
        if (::bind(socket, address.asSockaddr(), address.size()) != 0)
        {
            sLogger.failure("Bind unsuccessful on port {}.", port);
            return false;
        }

        if (::listen(socket, backlog) != 0)
        {
            sLogger.failure("Unable to listen on port {}.", port);
            return false;
        }

        return true;
    }
}
