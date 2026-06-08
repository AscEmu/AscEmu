/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "Network/SocketDefines.h"
#include "Network/NetworkIncludes.hpp"
#include "Network/SocketOps.h"
#include "Logging/Logger.hpp"

#include <cstdint>
#include <cstring>
#include <string>

namespace AscEmu::Network::AE
{
    inline bool resolveListenAddress(const char* listenAddress, uint16_t port, sockaddr_in& outAddress)
    {
        std::memset(&outAddress, 0, sizeof(outAddress));
        outAddress.sin_family = AF_INET;
        outAddress.sin_port = htons(port);

        if (listenAddress == nullptr || std::strcmp(listenAddress, "0.0.0.0") == 0 || std::strcmp(listenAddress, "*") == 0)
        {
            outAddress.sin_addr.s_addr = htonl(INADDR_ANY);
            return true;
        }

        addrinfo hints{};
        hints.ai_family = AF_INET;
        hints.ai_socktype = SOCK_STREAM;
        hints.ai_protocol = IPPROTO_TCP;

        addrinfo* results = nullptr;
        const std::string portString = std::to_string(port);
        const int rc = getaddrinfo(listenAddress, portString.c_str(), &hints, &results);
        if (rc != 0 || results == nullptr)
        {
            sLogger.failure("Unable to resolve listen address '{}' on port {}.", listenAddress, port);
            return false;
        }

        bool resolved = false;
        for (addrinfo* it = results; it != nullptr; it = it->ai_next)
        {
            if (it->ai_family != AF_INET || it->ai_addrlen < static_cast<int>(sizeof(sockaddr_in)))
                continue;

            outAddress = *reinterpret_cast<sockaddr_in*>(it->ai_addr);
            outAddress.sin_port = htons(port);
            resolved = true;
            break;
        }

        freeaddrinfo(results);
        return resolved;
    }

    inline bool bindAndListenSocket(SOCKET socket, const sockaddr_in& address, uint32_t port, int backlog = 5)
    {
        if (::bind(socket, reinterpret_cast<const sockaddr*>(&address), sizeof(address)) != 0)
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
