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
    mInputBufferLength(ConsoleDefines::localBuffer),
    mInputBufferPosition(0),
    mConsoleSocketState(ConsoleDefines::RemoteConsoleState::WaitForUsername),
    mFailedLoginCount(0),
    mRequestId(0),
    isWebClient(false)
{
    mInputBuffer = new char[ConsoleDefines::localBuffer];
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
                case ConsoleDefines::RemoteConsoleState::WaitForUsername:
                {
                    mConsoleAuthName = std::string(mInputBuffer);
                    mRemoteConsole->Write("password: ");
                    mConsoleSocketState = ConsoleDefines::RemoteConsoleState::WaitForPassword;

                } break;
                case ConsoleDefines::RemoteConsoleState::WaitForPassword:
                {
                    mConsoleAuthPassword = std::string(mInputBuffer);
                    mRemoteConsole->Write("\r\nAttempting to authenticate. Please wait.\r\n");

                    mRequestId = sConsoleAuthMgr.getGeneratedId();
                    sConsoleAuthMgr.addRequestIdSocket(mRequestId, this);

                    testConsoleLogin(mConsoleAuthName, mConsoleAuthPassword, mRequestId);

                } break;
                case ConsoleDefines::RemoteConsoleState::UserLoggedIn:
                {
                    if (!strnicmp(mInputBuffer, "quit", 4))
                    {
                        Disconnect();
                        break;
                    }

                    if (!strnicmp(mInputBuffer, "webclient", 9))
                    {
                        isWebClient = true;
                        break;
                    }
                }
                default:
                {
                    processConsoleInput(mRemoteConsole, mInputBuffer, isWebClient);

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

    if (mConsoleSocketState == ConsoleDefines::RemoteConsoleState::UserLoggedIn)
    {
        LogNotice("RemoteConsole : User `%s` disconnected.", mConsoleAuthName.c_str());
    }
}

void ConsoleSocket::getConsoleAuthResult(bool result)
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
            mConsoleSocketState = ConsoleDefines::RemoteConsoleState::WaitForUsername;
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
        //const char* argv[1];
        //handServerleInfoCommand(mRemoteConsole, 1, "");
        mRemoteConsole->Write("Type ? to see commands, quit to end session.\r\n");
        mConsoleSocketState = ConsoleDefines::RemoteConsoleState::UserLoggedIn;
    }
}

void ConsoleSocket::testConsoleLogin(std::string& username, std::string& password, uint32_t requestno)
{
    sLogonCommHandler.testConsoleLogon(username, password, requestno);
}
