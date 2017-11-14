/*
 * Multiplatform Async Network Library
 * Copyright (c) 2007 Burlex
 *
 * SocketMgr - iocp-based SocketMgr for windows.
 *
 */


#ifndef SOCKETMGR_H_WIN32
#define SOCKETMGR_H_WIN32

#ifdef CONFIG_USE_IOCP

class Socket;
class SERVER_DECL SocketMgr : public Singleton<SocketMgr>
{
    HANDLE m_completionPort;
    std::set<Socket*> _sockets;
    Mutex socketLock;
    std::atomic<unsigned long> socket_count;

    public:

        SocketMgr();
        ~SocketMgr();

        inline HANDLE GetCompletionPort() { return m_completionPort; }
        void SpawnWorkerThreads();
        void CloseAll();
        void ShowStatus();
        uint32 GetSocketCount() { return socket_count; }
        void AddSocket(Socket* s)
        {
            socketLock.Acquire();
            _sockets.insert(s);
            ++socket_count;
            socketLock.Release();
        }

        void RemoveSocket(Socket* s)
        {
            socketLock.Acquire();
            _sockets.erase(s);
            --socket_count;
            socketLock.Release();
        }

        void ShutdownThreads();
        long threadcount;

};

#define sSocketMgr SocketMgr::getSingleton()

typedef void(*OperationHandler)(Socket* s, uint32 len);

class SocketWorkerThread : public ThreadBase
{
    public:

        bool runThread();
};

void SERVER_DECL HandleReadComplete(Socket* s, uint32 len);
void SERVER_DECL HandleWriteComplete(Socket* s, uint32 len);
void SERVER_DECL HandleShutdown(Socket* s, uint32 len);

static OperationHandler ophandlers[NUM_SOCKET_IO_EVENTS] =
{
    &HandleReadComplete,
    &HandleWriteComplete,
    &HandleShutdown
};

#endif

#endif  //SOCKETMGR_H_WIN32
