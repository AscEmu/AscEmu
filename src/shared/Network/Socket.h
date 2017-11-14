/*
 * Multiplatform Async Network Library
 * Copyright (c) 2007 Burlex
 *
 * Socket implementable class.
 *
 */

#ifndef SOCKET_H
#define SOCKET_H

#include "SocketDefines.h"
#include "NetworkIncludes.hpp"
#include "CircularBuffer.h"
#include "Singleton.h"
#include "Log.hpp"
#include <string>
#include <mutex>
#include <atomic>
#include <map>
#include <set>

class SERVER_DECL Socket
{
    public:
        // Constructor. If fd = 0, it will be assigned
        Socket(SOCKET fd, uint32 sendbuffersize, uint32 recvbuffersize);

        // Destructor.
        virtual ~Socket();

        // Open a connection to another machine.
        bool Connect(const char* Address, uint32 Port);

        // Disconnect the socket.
        void Disconnect();

        // Accept from the already-set fd.
        void Accept(sockaddr_in* address);

        /* Implementable methods */

        // Called when data is received.
        virtual void OnRead() {}

        // Called when a connection is first successfully established.
        virtual void OnConnect() {}

        // Called when the socket is disconnected from the client (either forcibly or by the connection dropping)
        virtual void OnDisconnect() {}

        /* Sending Operations */

        // Locks sending mutex, adds bytes, unlocks mutex.
        bool Send(const uint8* Bytes, uint32 Size);

        // Burst system - Locks the sending mutex.
        inline void BurstBegin() { m_writeMutex.Acquire(); }

        // Burst system - Adds bytes to output buffer.
        bool BurstSend(const uint8* Bytes, uint32 Size);

        // Burst system - Pushes event to queue - do at the end of write events.
        void BurstPush();

        // Burst system - Unlocks the sending mutex.
        inline void BurstEnd() { m_writeMutex.Release(); }

        /* Client Operations */

        // Get the client's ip in numerical form.
        std::string GetRemoteIP();
        inline uint32 GetRemotePort() { return ntohs(m_client.sin_port); }
        inline SOCKET GetFd() { return m_fd; }

        /* Platform-specific methods */

        void SetupReadEvent();
        void ReadCallback(uint32 len);
        void WriteCallback();

        inline bool IsDeleted()
        {
            return m_deleted;
        }
        inline bool IsConnected()
        {
            return m_connected;
        }
        inline sockaddr_in & GetRemoteStruct() { return m_client; }

        void Delete();

        inline in_addr GetRemoteAddress() { return m_client.sin_addr; }


        CircularBuffer readBuffer;
        CircularBuffer writeBuffer;

    protected:

        // Called when connection is opened.
        void _OnConnect();

        SOCKET m_fd;

        Mutex m_writeMutex;
        Mutex m_readMutex;

        // we are connected? stop from posting events.
        std::atomic<bool> m_connected;

        // We are deleted? Stop us from posting events.
        std::atomic<bool> m_deleted;

        sockaddr_in m_client;

        unsigned long m_BytesSent;
        unsigned long m_BytesRecieved;

    public:

        // Atomic wrapper functions for increasing read/write locks
        inline void IncSendLock() { ++m_writeLock; }
        inline void DecSendLock() { --m_writeLock; }
        inline bool AcquireSendLock()
        {

            if (m_writeLock != 0)
                return false;

            m_writeLock = 1;
            return true;
        }

    private:

        // Write lock, stops multiple write events from being posted.
        std::atomic<unsigned long> m_writeLock;

        /* Win32 - IOCP Specific Calls */
#ifdef CONFIG_USE_IOCP

    public:

        // Set completion port that this socket will be assigned to.
        inline void SetCompletionPort(HANDLE cp) { m_completionPort = cp; }

        OverlappedStruct m_readEvent;
        OverlappedStruct m_writeEvent;

    private:
        // Completion port socket is assigned to
        HANDLE m_completionPort;

        // Assigns the socket to his completion port.
        void AssignToCompletionPort();

#endif

        /* Linux - EPOLL Specific Calls */
#ifdef CONFIG_USE_EPOLL
    public:
        // Posts a epoll event with the specifed arguments.
        void PostEvent(uint32 events);

        inline bool HasSendLock()
        {
            bool res;
            res = (m_writeLock.load() != 0);
            return res;
        }
#endif

        /* FreeBSD - kqueue specific calls */
#ifdef CONFIG_USE_KQUEUE
    public:
        // Posts a epoll event with the specifed arguments.
        void PostEvent(int events, bool oneshot);
        inline bool HasSendLock()
        {
            bool res;
            res = (m_writeLock.load() != 0);
            return res;
        }
#endif

    public:
        // Polls and resets the traffic data
        void PollTraffic(unsigned long* sent, unsigned long* recieved)
        {

            m_writeMutex.Acquire();
            *sent = m_BytesSent;
            *recieved = m_BytesRecieved;
            m_BytesSent = 0;
            m_BytesRecieved = 0;

            m_writeMutex.Release();
        }
};

/** Connect to a server.
* @param hostname Hostname or IP address to connect to
* @param port Port to connect to
* @return templated type if successful, otherwise null
*/
template<class T>
T* ConnectTCPSocket(const char* hostname, u_short port)
{
    sockaddr_in conn;
    hostent* host;

    /* resolve the peer */
    host = gethostbyname(hostname);

    if(!host)
        return NULL;

    /* copy into our address struct */
    memcpy(&conn.sin_addr, host->h_addr_list[0], sizeof(in_addr));
    conn.sin_family = AF_INET;
    conn.sin_port = ntohs(port);

    T* s = new T(0);
    if(!s->Connect(hostname, port))
    {
        s->Delete();
        return 0;
    }
    return s;
}

/* Socket Garbage Collector */
#define SOCKET_GC_TIMEOUT 15

class SocketGarbageCollector : public Singleton<SocketGarbageCollector>
{
        std::map<Socket*, time_t> deletionQueue;
        Mutex lock;
    public:

        ~SocketGarbageCollector()
        {
            std::map<Socket*, time_t>::iterator i;
            for(i = deletionQueue.begin(); i != deletionQueue.end(); ++i)
                delete i->first;
        }

        void Update()
        {
            std::map<Socket*, time_t>::iterator i, i2;
            time_t t = UNIXTIME;
            lock.Acquire();
            for(i = deletionQueue.begin(); i != deletionQueue.end();)
            {
                i2 = i++;
                if(i2->second <= t)
                {
                    delete i2->first;
                    deletionQueue.erase(i2);
                }
            }
            lock.Release();
        }

        void QueueSocket(Socket* s)
        {
            lock.Acquire();
            deletionQueue.insert(std::map<Socket*, time_t>::value_type(s, UNIXTIME + SOCKET_GC_TIMEOUT));
            lock.Release();
        }
};

#define sSocketGarbageCollector SocketGarbageCollector::getSingleton()

#endif  //SOCKET_H
