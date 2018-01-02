/*
Copyright (c) 2014-2018 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <cstdint>

#include "Network/Network.h"
#include "BaseConsole.h"


namespace ConsoleDefines
{
    const uint32_t localBuffer = 2048;

    enum RemoteConsoleState
    {
        WaitForUsername = 1,
        WaitForPassword = 2,
        UserLoggedIn = 3
    };
}


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
        bool isWebClient;

        uint32_t mInputBufferLength;
        uint32_t mInputBufferPosition;
        ConsoleDefines::RemoteConsoleState mConsoleSocketState;

        std::string mConsoleAuthName;
        std::string mConsoleAuthPassword;

        uint32_t mRequestId;
        uint8_t mFailedLoginCount;

    public:

        void sendLoginMessage();
        void handleConsoleInput();
        void closeRemoteConnection();

        void getConsoleAuthResult(bool result);

        void testConsoleLogin(std::string& username, std::string& password, uint32_t requestno);
};
