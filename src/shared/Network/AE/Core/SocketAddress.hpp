/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "Network/NetworkIncludes.hpp"

#include <cstdint>
#include <cstring>

namespace AscEmu::Network::AE
{
    class SocketAddressIPv4
    {
    public:
        SocketAddressIPv4()
        {
            std::memset(&m_address, 0, sizeof(m_address));
            m_address.sin_family = AF_INET;
        }

        explicit SocketAddressIPv4(const sockaddr_in& address) : m_address(address)
        {
        }

        [[nodiscard]] sockaddr_in& native()
        {
            return m_address;
        }

        [[nodiscard]] const sockaddr_in& native() const
        {
            return m_address;
        }

        [[nodiscard]] sockaddr* asSockaddr()
        {
            return reinterpret_cast<sockaddr*>(&m_address);
        }

        [[nodiscard]] const sockaddr* asSockaddr() const
        {
            return reinterpret_cast<const sockaddr*>(&m_address);
        }

        [[nodiscard]] int size() const
        {
            return static_cast<int>(sizeof(sockaddr_in));
        }

        void setPort(uint16_t port)
        {
            m_address.sin_port = htons(port);
        }

        [[nodiscard]] uint16_t port() const
        {
            return ntohs(m_address.sin_port);
        }

        void setAnyAddress()
        {
            m_address.sin_addr.s_addr = htonl(INADDR_ANY);
        }

    private:
        sockaddr_in m_address{};
    };
}
