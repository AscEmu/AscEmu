/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Network.hpp"
#include "Network/Core/Resolver.hpp"
#include "Network/Core/SocketPlatformOps.hpp"

#ifdef _WIN32
#include <Ws2tcpip.h>
#endif

Socket::Socket(SOCKET socket, uint32_t sendBufferSize, uint32_t recvBufferSize)
    : m_socket(socket), m_isConnected(false), m_isDeleted(false), m_sendLock(0)
{
    readBuffer.Allocate(recvBufferSize);
    writeBuffer.Allocate(sendBufferSize);

    m_bytesSent = 0;
    m_bytesReceived = 0;

#ifdef CONFIG_USE_IOCP
    m_completionPort = nullptr;
#endif

    if (m_socket == 0)
        m_socket = AscEmu::Network::SocketPlatformOps::createTcpSocket();

    sLogger.debug("Created Socket {}", m_socket);
}

Socket::~Socket() = default;

bool Socket::connect(const char* address, uint32_t port)
{
    AscEmu::Network::SocketAddressIPv4 resolvedAddress;
    if (!AscEmu::Network::Resolver::resolveRemoteIPv4(address, static_cast<uint16_t>(port), resolvedAddress))
        return false;

    m_remoteAddress = resolvedAddress.native();

    AscEmu::Network::SocketPlatformOps::setBlocking(m_socket);

    if (::connect(m_socket, reinterpret_cast<const sockaddr*>(&m_remoteAddress), sizeof(m_remoteAddress)) == -1)
        return false;

#ifdef CONFIG_USE_IOCP
    m_completionPort = sSocketMgr.GetCompletionPort();
#endif

    onConnectInternal();
    return true;
}

void Socket::accept(sockaddr_in* address)
{
    std::memcpy(&m_remoteAddress, address, sizeof(*address));
    onConnectInternal();
}

void Socket::onConnectInternal()
{
    AscEmu::Network::SocketPlatformOps::setNonBlocking(m_socket);
    AscEmu::Network::SocketPlatformOps::disableBuffering(m_socket);

    m_isConnected = true;

#ifdef CONFIG_USE_IOCP
    assignToCompletionPort();
    setupReadEvent();
#endif

    sSocketMgr.AddSocket(this);
    sLogger.info("Socket::onConnectInternal called");
    onConnect();
}

bool Socket::send(const uint8_t* bytes, uint32_t size)
{
    burstBegin();
    const bool result = burstSend(bytes, size);
    if (result)
        burstPush();
    burstEnd();

    return result;
}

bool Socket::burstSend(const uint8_t* bytes, uint32_t size)
{
    return writeBuffer.Write(bytes, size);
}

std::string Socket::getRemoteIp()
{
    char buffer[INET_ADDRSTRLEN] = {};

#ifdef _WIN32
    if (InetNtopA(AF_INET, const_cast<IN_ADDR*>(&m_remoteAddress.sin_addr), buffer, INET_ADDRSTRLEN) != nullptr)
        return std::string(buffer);
#else
    if (::inet_ntop(AF_INET, &m_remoteAddress.sin_addr, buffer, INET_ADDRSTRLEN) != nullptr)
        return std::string(buffer);
#endif

    return std::string("noip");
}

void Socket::disconnect()
{
    if (!m_isConnected)
        return;

    m_isConnected = false;

    sLogger.info("Socket::disconnect on socket {}", m_socket);

    sSocketMgr.RemoveSocket(this);
    onDisconnect();

    if (!isDeleted())
        deleteSocket();
}

void Socket::delayedDisconnect()
{
    if (!m_isConnected)
        return;

    m_delayedDisconnectRequested = true;
    completeDelayedDisconnectIfReady();
}

void Socket::completeDelayedDisconnectIfReady()
{
    if (!m_delayedDisconnectRequested.load() || !m_isConnected.load())
        return;

    bool writeQueueEmpty = false;
    {
        std::lock_guard lock{ m_writeMutex };
        writeQueueEmpty = writeBuffer.GetSize() == 0;
    }

    if (!writeQueueEmpty)
        return;

    m_delayedDisconnectRequested = false;
    disconnect();
}

void Socket::deleteSocket()
{
    if (m_isDeleted)
        return;

    m_isDeleted = true;

    sLogger.debug("Socket::deleteSocket() on socket {}", m_socket);

    if (isConnected())
        disconnect();

    AscEmu::Network::SocketPlatformOps::closeSocket(m_socket);
    sSocketGarbageCollector.QueueSocket(this);
}
