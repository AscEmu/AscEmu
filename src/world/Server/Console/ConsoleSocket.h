/*
Copyright (c) 2014-2017 AscEmu Team <http://www.ascemu.org/>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <cstdint>

#include "Network/Network.h"
#include "BaseConsole.h"

#define LOCAL_BUFFER_SIZE 2048

enum STATES
{
    STATE_USER = 1,
    STATE_PASSWORD = 2,
    STATE_LOGGED = 3,
    STATE_WAITING = 4
};

class ConsoleSocket : public Socket
{
    public:

        ConsoleSocket(SOCKET iFd);
        ~ConsoleSocket();

    //////////////////////////////////////////////////////////////////////////////////////////
    // virtual functions (Socket)

        void OnConnect();
        void OnRead();
        void OnDisconnect();

    //////////////////////////////////////////////////////////////////////////////////////////
    // handle console input

    private:

        RemoteConsole* mRemoteConsole;

        char* mInputBuffer;

        uint32_t mInputBufferLength;
        uint32_t mInputBufferPosition;
        uint32_t mConsoleSocketState;

        std::string mConsoleAuthName;
        std::string mConsoleAuthPassword;

        uint32_t mRequestId;
        uint8_t mFailedLoginCount;

    public:

        void sendLoginMessage();
        void handleConsoleInput();
        void closeRemoteConnection();

        void AuthCallback(bool result);

        void TestConsoleLogin(std::string& username, std::string& password, uint32_t requestno);
};
