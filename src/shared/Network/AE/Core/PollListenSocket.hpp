/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ListenCommon.hpp"

#include <cerrno>

class ListenSocketBase
{
public:
    virtual ~ListenSocketBase() = default;
    virtual void OnAccept() = 0;
    virtual int GetFd() = 0;
};

template <class T>
class ListenSocket : public ListenSocketBase
{
public:
    ListenSocket(const char* listenAddress, uint32_t port)
    {
        m_socket = socket(AF_INET, SOCK_STREAM, 0);
        if (m_socket < 0)
            return;

        SocketOps::ReuseAddr(m_socket);
        SocketOps::Nonblocking(m_socket);
        SocketOps::SetTimeout(m_socket, 60);

        if (!AscEmu::Network::AE::resolveListenAddress(listenAddress, static_cast<uint16_t>(port), m_address))
            return;

        if (!AscEmu::Network::AE::bindAndListenSocket(m_socket, m_address, port))
            return;

        m_tempLength = sizeof(sockaddr_in);
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
            SocketOps::CloseSocket(m_socket);
    }

    void OnAccept() override
    {
        for (;;)
        {
            SOCKET acceptedSocket = accept(
                m_socket,
                reinterpret_cast<sockaddr*>(&m_tempAddress),
                reinterpret_cast<socklen_t*>(&m_tempLength)
            );

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
            accepted->Accept(&m_tempAddress);
        }
    }

    bool IsOpen() const
    {
        return m_opened;
    }

    int GetFd() override
    {
        return m_socket;
    }

private:
    SOCKET m_socket = INVALID_SOCKET;
    sockaddr_in m_address{};
    sockaddr_in m_tempAddress{};
    bool m_opened = false;
    uint32_t m_tempLength = 0;
};

