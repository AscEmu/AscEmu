/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Network/Network.h"

#ifdef ASCEMU_USE_AE_NETWORK
#ifdef CONFIG_USE_IOCP

void Socket::WriteCallback()
{
    if (IsDeleted() || !IsConnected())
        return;

    std::lock_guard lock{m_writeMutex};

    if (writeBuffer.GetContiguiousBytes())
    {
        DWORD w_length = 0;
        DWORD flags = 0;

        WSABUF buf;
        buf.len = (u_long)writeBuffer.GetContiguiousBytes();
        buf.buf = (char*)writeBuffer.GetBufferStart();

        m_writeEvent.Mark();
        m_writeEvent.Reset(SOCKET_IO_EVENT_WRITE_END);
        int r = WSASend(m_fd, &buf, 1, &w_length, flags, &m_writeEvent.m_overlap, 0);
        if (r == SOCKET_ERROR)
        {
            int wsaerror = WSAGetLastError();

            if (wsaerror != WSA_IO_PENDING)
            {
                sLogger.failure("WSAGetLastError() = {} on socket {}", wsaerror, m_fd);

                m_writeEvent.Unmark();
                DecSendLock();
                Disconnect();
            }
        }
        m_BytesSent += w_length;
    }
    else
    {
        DecSendLock();
    }
}

void Socket::SetupReadEvent()
{
    if (IsDeleted() || !IsConnected())
        return;

    std::unique_lock lock{m_readMutex};

    DWORD r_length = 0;
    DWORD flags = 0;
    WSABUF buf;
    buf.len = (u_long)readBuffer.GetSpace();
    buf.buf = (char*)readBuffer.GetBuffer();

    m_readEvent.Mark();
    m_readEvent.Reset(SOCKET_IO_EVENT_READ_COMPLETE);
    if (WSARecv(m_fd, &buf, 1, &r_length, &flags, &m_readEvent.m_overlap, 0) == SOCKET_ERROR)
    {
        if (WSAGetLastError() != WSA_IO_PENDING)
        {
            m_readEvent.Unmark();

            lock.unlock();

            Disconnect();
            return;
        }
    }
    m_BytesRecieved += r_length;
}

void Socket::ReadCallback(uint32_t len)
{
    readBuffer.IncrementWritten(len);
    OnRead();
    SetupReadEvent();
}

void Socket::AssignToCompletionPort()
{
    CreateIoCompletionPort((HANDLE)m_fd, m_completionPort, (ULONG_PTR)this, 0);
}

void Socket::BurstPush()
{
    if (AcquireSendLock())
        WriteCallback();
}

#endif
#endif
