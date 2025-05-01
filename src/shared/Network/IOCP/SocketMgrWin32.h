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
class SERVER_DECL SocketMgr
{
    HANDLE m_completionPort;
    std::set<Socket*> _sockets;
    Mutex socketLock;
    std::atomic<unsigned long> socket_count;

private:
    SocketMgr() = default;
    ~SocketMgr() = default;

public:
    static SocketMgr& getInstance();
    void initialize();
    // todo: empty on windows, should it be?
    void finalize() { };

    SocketMgr(SocketMgr&&) = delete;
    SocketMgr(SocketMgr const&) = delete;
    SocketMgr& operator=(SocketMgr&&) = delete;
    SocketMgr& operator=(SocketMgr const&) = delete;

    inline HANDLE GetCompletionPort() { return m_completionPort; }
    void SpawnWorkerThreads();
    void CloseAll();
    void ShowStatus();
    uint32_t GetSocketCount() { return socket_count; }

    void AddSocket(Socket* s)
    {
        socketLock.acquire();
        _sockets.insert(s);
        ++socket_count;
        socketLock.release();
    }

    void RemoveSocket(Socket* s)
    {
        socketLock.acquire();
        _sockets.erase(s);
        --socket_count;
        socketLock.release();
    }

    void ShutdownThreads();
    long threadcount;
};

#define sSocketMgr SocketMgr::getInstance()

typedef void(*OperationHandler)(Socket* s, uint32_t len);

class SocketWorkerThread : public ThreadBase
{
public:
    bool runThread();
};

void SERVER_DECL HandleReadComplete(Socket* s, uint32_t len);
void SERVER_DECL HandleWriteComplete(Socket* s, uint32_t len);
void SERVER_DECL HandleShutdown(Socket* s, uint32_t len);

static OperationHandler ophandlers[NUM_SOCKET_IO_EVENTS] =
{
    &HandleReadComplete,
    &HandleWriteComplete,
    &HandleShutdown
};

#endif

#endif  //SOCKETMGR_H_WIN32
