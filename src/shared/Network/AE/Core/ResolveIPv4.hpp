/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "Network/NetworkIncludes.hpp"
#include "Logging/Logger.hpp"

#include <cstdint>
#include <cstring>
#include <string>

namespace AscEmu::Network::AE
{
    inline bool resolveRemoteAddress(const char* hostname, uint16_t port, sockaddr_in& outAddress)
    {
        if (hostname == nullptr || *hostname == '\0')
            return false;

        std::memset(&outAddress, 0, sizeof(outAddress));
        outAddress.sin_family = AF_INET;
        outAddress.sin_port = htons(port);

        addrinfo hints{};
        hints.ai_family = AF_INET;
        hints.ai_socktype = SOCK_STREAM;
        hints.ai_protocol = IPPROTO_TCP;

        addrinfo* results = nullptr;
        const std::string portString = std::to_string(port);

        const int rc = getaddrinfo(hostname, portString.c_str(), &hints, &results);
        if (rc != 0 || results == nullptr)
        {
#ifdef _WIN32
            sLogger.failure("Could not resolve remote address '{}' on port {} (getaddrinfo rc={}).", hostname, port, rc);
#else
            sLogger.failure("Could not resolve remote address '{}' on port {} ({}).", hostname, port, gai_strerror(rc));
#endif
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
}
