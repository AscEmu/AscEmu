/*
Copyright (c) 2014-2017 AscEmu Team <http://www.ascemu.org/>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "StdAfx.h"

#include "ConsoleSocket.h"
#include "ConsoleAuthMgr.h"
#include "ConsoleCommands.h"
#include "BaseConsole.h"

#include "Log.hpp"
#include "Server/LogonCommClient/LogonCommHandler.h"
#include "Network/Network.h"


ConsoleSocket::ConsoleSocket(SOCKET iFd) :
    Socket(iFd, 10000, 1000),
    mInputBufferLength(LOCAL_BUFFER_SIZE),
    mInputBufferPosition(0),
    mConsoleSocketState(STATE_USER),
    mFailedLoginCount(0),
    mRequestId(0)
{
    mInputBuffer = new char[LOCAL_BUFFER_SIZE];
    mRemoteConsole = new RemoteConsole(this);
    
}

ConsoleSocket::~ConsoleSocket()
{
    if (mInputBuffer != NULL)
    {
        delete[] mInputBuffer;
    }

    if (mRemoteConsole != nullptr)
    {
        delete mRemoteConsole;
    }

    if (mRequestId)
    {
        sConsoleAuthMgr.addRequestIdSocket(mRequestId, nullptr);
        mRequestId = 0;
    }
}


//////////////////////////////////////////////////////////////////////////////////////////
// virtual functions (Socket)

void ConsoleSocket::OnConnect()
{
    sendLoginMessage();
}

void ConsoleSocket::OnRead()
{
    handleConsoleInput();
}

void ConsoleSocket::OnDisconnect()
{
    closeRemoteConnection();
}


//////////////////////////////////////////////////////////////////////////////////////////
// handle console input

void ConsoleSocket::sendLoginMessage()
{
    mRemoteConsole->Write("Welcome to AscEmu's Remote Administration Console.\r\n");
    mRemoteConsole->Write("Please authenticate to continue. \r\n\r\n");
    mRemoteConsole->Write("login: ");
}

void ConsoleSocket::handleConsoleInput()
{
    uint32_t readLength = (uint32_t)readBuffer.GetSize();
    if ((readLength + mInputBufferPosition) >= mInputBufferLength)
    {
        Disconnect();
        return;
    }

    readBuffer.Read((uint8_t*)&mInputBuffer[mInputBufferPosition], readLength);
    mInputBufferPosition += readLength;

    char* inputChar = strchr(mInputBuffer, '\n');
    while (inputChar != NULL)
    {
        uint32_t inputLength = (uint32_t)((inputChar + 1) - mInputBuffer);
        if (*(inputChar - 1) == '\r')
        {
            *(inputChar - 1) = '\0';
        }

        *inputChar = '\0';

        if (*mInputBuffer != '\0')
        {
            switch (mConsoleSocketState)
            {
                case STATE_USER:
                {
                    mConsoleAuthName = std::string(mInputBuffer);
                    mRemoteConsole->Write("password: ");
                    mConsoleSocketState = STATE_PASSWORD;

                } break;
                case STATE_PASSWORD:
                {
                    mConsoleAuthPassword = std::string(mInputBuffer);
                    mRemoteConsole->Write("\r\nAttempting to authenticate. Please wait.\r\n");
                    mConsoleSocketState = STATE_WAITING;

                    mRequestId = sConsoleAuthMgr.getGeneratedId();
                    sConsoleAuthMgr.addRequestIdSocket(mRequestId, this);

                    TestConsoleLogin(mConsoleAuthName, mConsoleAuthPassword, mRequestId);

                } break;
                case STATE_LOGGED:
                {
                    if (!strnicmp(mInputBuffer, "quit", 4))
                    {
                        Disconnect();
                        break;
                    }
                }
                default:
                {
                    processConsoleInput(mRemoteConsole, mInputBuffer);

                } break;
            }
        }

        if (inputLength == mInputBufferPosition)
        {
            mInputBuffer[0] = '\0';
            mInputBufferPosition = 0;
        }
        else
        {
            memcpy(mInputBuffer, &mInputBuffer[inputLength], mInputBufferPosition - inputLength);
            mInputBufferPosition -= inputLength;
        }

        inputChar = strchr(mInputBuffer, '\n');
    }
}

void ConsoleSocket::closeRemoteConnection()
{
    if (mRequestId != 0)
    {
        sConsoleAuthMgr.addRequestIdSocket(mRequestId, nullptr);
        mRequestId = 0;
    }

    if (mConsoleSocketState == STATE_LOGGED)
    {
        LogNotice("RemoteConsole : User `%s` disconnected.", mConsoleAuthName.c_str());
    }
}

void ConsoleSocket::AuthCallback(bool result)
{
    sConsoleAuthMgr.addRequestIdSocket(mRequestId, nullptr);
    mRequestId = 0;

    if (result == false)
    {
        mRemoteConsole->Write("Authentication failed.\r\n\r\n");
        mFailedLoginCount++;
        if (mFailedLoginCount < 3)
        {
            mRemoteConsole->Write("login: ");
            mConsoleSocketState = STATE_USER;
        }
        else
        {
            Disconnect();
        }
    }
    else
    {
        mRemoteConsole->Write("User `%s` authenticated.\r\n\r\n", mConsoleAuthName.c_str());
        LogNotice("RemoteConsole : User `%s` authenticated.", mConsoleAuthName.c_str());
        const char* argv[1];
        handServerleInfoCommand(mRemoteConsole, 1, "");
        mRemoteConsole->Write("Type ? to see commands, quit to end session.\r\n");
        mConsoleSocketState = STATE_LOGGED;
    }
}

void ConsoleSocket::TestConsoleLogin(std::string& username, std::string& password, uint32_t requestno)
{
    sLogonCommHandler.TestConsoleLogon(username, password, requestno);
}
