/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Network/Network.hpp"

#ifdef CONFIG_USE_IOCP

void Socket::writeCallback()
{
    if (isDeleted() || !isConnected())
        return;

    std::lock_guard lock{ m_writeMutex };

    if (writeBuffer.GetContiguiousBytes())
    {
        DWORD writeLength = 0;
        DWORD flags = 0;

        WSABUF buffer;
        buffer.len = static_cast<u_long>(writeBuffer.GetContiguiousBytes());
        buffer.buf = reinterpret_cast<char*>(writeBuffer.GetBufferStart());

        m_writeEvent.Mark();
        m_writeEvent.Reset(SOCKET_IO_EVENT_WRITE_END);
        int result = WSASend(m_socket, &buffer, 1, &writeLength, flags, &m_writeEvent.m_overlap, 0);
        if (result == SOCKET_ERROR)
        {
            int wsaError = WSAGetLastError();

            if (wsaError != WSA_IO_PENDING)
            {
                sLogger.failure("WSAGetLastError() = {} on socket {}", wsaError, m_socket);

                m_writeEvent.Unmark();
                decrementSendLock();
                disconnect();
            }
        }
        m_bytesSent += writeLength;
    }
    else
    {
        decrementSendLock();
        completeDelayedDisconnectIfReady();
    }
}

void Socket::setupReadEvent()
{
    if (isDeleted() || !isConnected())
        return;

    std::unique_lock lock{ m_readMutex };

    DWORD readLength = 0;
    DWORD flags = 0;
    WSABUF buffer;
    buffer.len = static_cast<u_long>(readBuffer.GetSpace());
    buffer.buf = reinterpret_cast<char*>(readBuffer.GetBuffer());

    m_readEvent.Mark();
    m_readEvent.Reset(SOCKET_IO_EVENT_READ_COMPLETE);
    if (WSARecv(m_socket, &buffer, 1, &readLength, &flags, &m_readEvent.m_overlap, 0) == SOCKET_ERROR)
    {
        if (WSAGetLastError() != WSA_IO_PENDING)
        {
            m_readEvent.Unmark();

            lock.unlock();

            disconnect();
            return;
        }
    }
    m_bytesReceived += readLength;
}

void Socket::readCallback(uint32_t length)
{
    readBuffer.IncrementWritten(length);
    onRead();
    setupReadEvent();
}

void Socket::assignToCompletionPort()
{
    CreateIoCompletionPort((HANDLE)m_socket, m_completionPort, (ULONG_PTR)this, 0);
}

void Socket::burstPush()
{
    if (acquireSendLock())
        writeCallback();
}

#endif
