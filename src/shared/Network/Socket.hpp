/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "SocketDefines.hpp"
#include "NetworkIncludes.hpp"
#include "BipBuffer.hpp"
#include "Logging/Log.hpp"

#include <atomic>
#include <map>
#include <mutex>
#include <set>
#include <string>

class SERVER_DECL Socket
{
public:
    Socket(SOCKET socket, uint32_t sendBufferSize, uint32_t recvBufferSize);
    virtual ~Socket();

    bool connect(const char* address, uint32_t port);
    void disconnect();
    void delayedDisconnect();
    void completeDelayedDisconnectIfReady();
    void accept(sockaddr_in* address);

    virtual void onRead() {}
    virtual void onConnect() {}
    virtual void onDisconnect() {}

    bool send(const uint8_t* bytes, uint32_t size);

    inline void burstBegin() { m_writeMutex.lock(); }
    bool burstSend(const uint8_t* bytes, uint32_t size);
    void burstPush();
    inline void burstEnd() { m_writeMutex.unlock(); }

    std::string getRemoteIp();
    inline uint32_t getRemotePort() const { return ntohs(m_remoteAddress.sin_port); }
    inline SOCKET getFd() const { return m_socket; }

    void setupReadEvent();
    void readCallback(uint32_t length);
    void writeCallback();

    inline bool isDeleted() const { return m_isDeleted; }
    inline bool isConnected() const { return m_isConnected; }
    inline sockaddr_in& getRemoteStruct() { return m_remoteAddress; }

    void deleteSocket();

    inline in_addr getRemoteAddress() const { return m_remoteAddress.sin_addr; }

    AscEmu::BipBuffer readBuffer;
    AscEmu::BipBuffer writeBuffer;

protected:
    void onConnectInternal();

    SOCKET m_socket;

    std::recursive_mutex m_writeMutex;
    std::recursive_mutex m_readMutex;

    std::atomic<bool> m_isConnected;
    std::atomic<bool> m_isDeleted;
    std::atomic<bool> m_delayedDisconnectRequested{ false };

    sockaddr_in m_remoteAddress;

    unsigned long m_bytesSent;
    unsigned long m_bytesReceived;

public:
    inline void incrementSendLock() { ++m_sendLock; }
    inline void decrementSendLock() { --m_sendLock; }

    inline bool acquireSendLock()
    {
        if (m_sendLock != 0)
            return false;

        m_sendLock = 1;
        return true;
    }

private:
    std::atomic<unsigned long> m_sendLock;

public:
    void pollTraffic(unsigned long* sent, unsigned long* received)
    {
        std::lock_guard lock{ m_writeMutex };

        *sent = m_bytesSent;
        *received = m_bytesReceived;
        m_bytesSent = 0;
        m_bytesReceived = 0;
    }

#ifdef CONFIG_USE_IOCP
public:
    inline void setCompletionPort(HANDLE completionPort) { m_completionPort = completionPort; }

    OverlappedStruct m_readEvent;
    OverlappedStruct m_writeEvent;

private:
    HANDLE m_completionPort;

    void assignToCompletionPort();
#endif

#ifdef CONFIG_USE_EPOLL
public:
    void postEvent(uint32_t events);
    inline bool hasSendLock() const { return m_sendLock.load() != 0; }
#endif

#ifdef CONFIG_USE_KQUEUE
public:
    void postEvent(int events, bool oneShot);
    inline bool hasSendLock() const { return m_sendLock.load() != 0; }
#endif
};

template <class T>
T* ConnectTCPSocket(const char* hostname, u_short port)
{
    T* socket = new T(0);
    if (!socket->connect(hostname, port))
    {
        socket->deleteSocket();
        return nullptr;
    }

    return socket;
}

#define SOCKET_GC_TIMEOUT 15

class SocketGarbageCollector
{
    std::map<Socket*, time_t> deletionQueue;
    std::mutex lock;

private:
    SocketGarbageCollector() = default;
    ~SocketGarbageCollector() = default;

public:
    static SocketGarbageCollector& getInstance()
    {
        static SocketGarbageCollector instance;
        return instance;
    }

    void finalize()
    {
        for (auto entry : deletionQueue)
            delete entry.first;
    }

    SocketGarbageCollector(SocketGarbageCollector&&) = delete;
    SocketGarbageCollector(SocketGarbageCollector const&) = delete;
    SocketGarbageCollector& operator=(SocketGarbageCollector&&) = delete;
    SocketGarbageCollector& operator=(SocketGarbageCollector const&) = delete;

    void QueueSocket(Socket* socket)
    {
        std::lock_guard guard{ lock };
        deletionQueue.insert(std::make_pair(socket, UNIXTIME));
    }

    void Update()
    {
        std::lock_guard guard{ lock };
        time_t t = UNIXTIME;

        for (auto itr = deletionQueue.begin(); itr != deletionQueue.end();)
        {
            if ((itr->second + SOCKET_GC_TIMEOUT) < t)
            {
                delete itr->first;
                itr = deletionQueue.erase(itr);
            }
            else
            {
                ++itr;
            }
        }
    }
};

#define sSocketGarbageCollector SocketGarbageCollector::getInstance()
