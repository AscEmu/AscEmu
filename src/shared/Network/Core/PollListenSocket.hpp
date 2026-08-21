/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "Network/Core/ListenCommon.hpp"
#include "Network/Core/SocketPlatformOps.hpp"
#include "Network/SocketDefines.hpp"
#include "Network/Core/NetworkBackend.hpp"

#include <cerrno>

template <class T>
class ListenSocket : public ListenSocketBase
{
public:
    ListenSocket(const char* listenAddress, uint32_t port)
    {
        m_socket = socket(AF_INET, SOCK_STREAM, 0);
        if (m_socket < 0)
            return;

        AscEmu::Network::SocketPlatformOps::setReuseAddress(m_socket);
        AscEmu::Network::SocketPlatformOps::setNonBlocking(m_socket);
        AscEmu::Network::SocketPlatformOps::setTimeout(m_socket, 60);

        if (!AscEmu::Network::resolveListenAddress(listenAddress, static_cast<uint16_t>(port), m_address))
            return;

        if (!AscEmu::Network::bindAndListenSocket(m_socket, m_address, port))
            return;

        m_tempLength = static_cast<socklen_t>(sizeof(sockaddr_in));
        m_opened = true;
        sSocketMgr.AddListenSocket(this);
    }

    ~ListenSocket() override
    {
        Close();
    }

    void Close()
    {
        const bool wasOpened = m_opened;
        m_opened = false;

        if (wasOpened)
            AscEmu::Network::SocketPlatformOps::closeSocket(m_socket);
    }

    void onAccept() override
    {
        for (;;)
        {
            m_tempLength = static_cast<socklen_t>(sizeof(sockaddr_in));

            SOCKET acceptedSocket = accept(m_socket, reinterpret_cast<sockaddr*>(&m_tempAddress), &m_tempLength);

            if (acceptedSocket == -1)
            {
#ifdef _WIN32
                break;
#else
                if (errno == EAGAIN || errno == EWOULDBLOCK)
                    break;

                break;
#endif
            }

            auto* accepted = new T(acceptedSocket);
            accepted->accept(&m_tempAddress);
        }
    }

    bool IsOpen() const
    {
        return m_opened;
    }

    int getFd() override
    {
        return m_socket;
    }

private:
    SOCKET m_socket = INVALID_SOCKET;
    AscEmu::Network::SocketAddressIPv4 m_address{};
    sockaddr_in m_tempAddress{};
    bool m_opened = false;
    socklen_t m_tempLength = 0;
};
