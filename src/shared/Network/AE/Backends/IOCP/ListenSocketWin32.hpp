/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#ifdef CONFIG_USE_IOCP

#include "Network/IOCP/SocketMgrWin32.h"
#include "Network/SocketDefines.h"
#include "Network/SocketOps.h"
#include "Network/NetworkIncludes.hpp"
#include "Network/AE/Core/ListenCommon.hpp"

template <class T>
class SERVER_DECL ListenSocket
{
public:
    ListenSocket(const char* listenAddress, uint32_t port)
    {
        m_socket = WSASocketW(AF_INET, SOCK_STREAM, 0, nullptr, 0, WSA_FLAG_OVERLAPPED);
        if (m_socket == INVALID_SOCKET)
            return;

        SocketOps::ReuseAddr(m_socket);
        SocketOps::Blocking(m_socket);
        SocketOps::SetTimeout(m_socket, 60);

        if (!AscEmu::Network::AE::resolveListenAddress(listenAddress, static_cast<uint16_t>(port), m_address))
            return;

        if (!AscEmu::Network::AE::bindAndListenSocket(m_socket, m_address, port))
            return;

        m_opened = true;
        m_tempLength = sizeof(sockaddr_in);
        m_completionPort = sSocketMgr.GetCompletionPort();
    }

    ~ListenSocket()
    {
        Close();
    }

    bool runThread()
    {
        while (m_opened)
        {
            m_tempLength = sizeof(sockaddr_in);

            SOCKET acceptedSocket = WSAAccept(m_socket, reinterpret_cast<sockaddr*>(&m_tempAddress),
                &m_tempLength, nullptr,0);

            if (acceptedSocket == INVALID_SOCKET)
                continue;

            auto* accepted = new T(acceptedSocket);
            accepted->SetCompletionPort(m_completionPort);
            accepted->Accept(&m_tempAddress);
        }

        return false;
    }

    void Close()
    {
        const bool wasOpened = m_opened;
        m_opened = false;

        if (wasOpened)
            SocketOps::CloseSocket(m_socket);
    }

    bool IsOpen() const
    {
        return m_opened;
    }

private:
    SOCKET m_socket = INVALID_SOCKET;
    sockaddr_in m_address{};
    sockaddr_in m_tempAddress{};
    bool m_opened = false;
    int m_tempLength = 0;
    HANDLE m_completionPort = nullptr;
};

#endif
