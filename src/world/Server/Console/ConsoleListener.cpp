/*
 * AscEmu Framework based on ArcEmu MMORPG Server
 * Copyright (c) 2014-2017 AscEmu Team <http://www.ascemu.org/>
 * Copyright (C) 2008-2012 ArcEmu Team <http://www.ArcEmu.org/>
 * Copyright (C) 2005-2007 Ascent Team
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "StdAfx.h"

#include "Common.hpp"
#include <Network/Network.h>
#include <Config/Config.h>
#include <git_version.h>

#include "BaseConsole.h"
#include "ConsoleCommands.h"
#include "Server/World.h"
#include "Server/World.Legacy.h"

#define LOCAL_BUFFER_SIZE 2048
#define ENABLE_REMOTE_CONSOLE 1

enum STATES
{
    STATE_USER      = 1,
    STATE_PASSWORD  = 2,
    STATE_LOGGED    = 3,
    STATE_WAITING   = 4
};

class ConsoleSocket : public Socket
{
    RemoteConsole* m_pConsole;

    char* m_pBuffer;

    uint32 m_pBufferLen;
    uint32 m_pBufferPos;
    uint32 m_state;

    std::string m_username;
    std::string m_password;

    uint32 m_requestNo;
    uint8 m_failedLogins;

    public:

        ConsoleSocket(SOCKET iFd);
        ~ConsoleSocket();

        void OnRead();
        void OnDisconnect();
        void OnConnect();
        void TryAuthenticate();
        void AuthCallback(bool result);
};

class ConsoleAuthMgr : public Singleton < ConsoleAuthMgr >
{
    Mutex authmgrlock;
    uint32 highrequestid;
    std::map<uint32, ConsoleSocket*> requestmap;

    public:

        ConsoleAuthMgr()
        {
            highrequestid = 1;
        }

        uint32 GenerateRequestId()
        {
            uint32 n;
            authmgrlock.Acquire();
            n = highrequestid++;
            authmgrlock.Release();
            return n;
        }

        void SetRequest(uint32 id, ConsoleSocket* sock)
        {
            authmgrlock.Acquire();
            if (sock == NULL)
                requestmap.erase(id);
            else
                requestmap.insert(std::make_pair(id, sock));
            authmgrlock.Release();
        }

        ConsoleSocket* GetRequest(uint32 id)
        {
            ConsoleSocket* rtn;
            authmgrlock.Acquire();
            std::map<uint32, ConsoleSocket*>::iterator itr = requestmap.find(id);
            if (itr == requestmap.end())
                rtn = NULL;
            else
                rtn = itr->second;
            authmgrlock.Release();
            return rtn;
        }
};

ListenSocket<ConsoleSocket> * g_pListenSocket = NULL;
initialiseSingleton(ConsoleAuthMgr);

void ConsoleAuthCallback(uint32 request, uint32 result)
{
    ConsoleSocket* pSocket = ConsoleAuthMgr::getSingleton().GetRequest(request);
    if (pSocket == NULL || !pSocket->IsConnected())
        return;

    if (result)
        pSocket->AuthCallback(true);
    else
        pSocket->AuthCallback(false);
}

void CloseConsoleListener()
{
    if (g_pListenSocket != NULL)
        g_pListenSocket->Close();
}

bool StartConsoleListener()
{
#ifndef ENABLE_REMOTE_CONSOLE
    return false;
#else
    std::string lhost = worldConfig.remoteConsole.host;
    uint32 lport = worldConfig.remoteConsole.port;
    bool enabled = worldConfig.remoteConsole.isEnabled;

    if (!enabled)
        return false;

    g_pListenSocket = new ListenSocket<ConsoleSocket>(lhost.c_str(), lport);
    if (g_pListenSocket == NULL)
        return false;

    if (!g_pListenSocket->IsOpen())
    {
        g_pListenSocket->Close();
        delete g_pListenSocket;
        g_pListenSocket = NULL;
        return false;
    }

    new ConsoleAuthMgr;
    return true;
#endif
}

ThreadBase* GetConsoleListener()
{
    return (ThreadBase*)g_pListenSocket;
}

ConsoleSocket::ConsoleSocket(SOCKET iFd) : Socket(iFd, 10000, 1000)
{
    m_pBufferLen = LOCAL_BUFFER_SIZE;
    m_pBufferPos = 0;
    m_pBuffer = new char[LOCAL_BUFFER_SIZE];
    m_pConsole = new RemoteConsole(this);
    m_state = STATE_USER;
    m_failedLogins = 0;
    m_requestNo = 0;
}

ConsoleSocket::~ConsoleSocket()
{
    if (m_pBuffer != NULL)
        delete[] m_pBuffer;

    if (m_pConsole != NULL)
        delete m_pConsole;

    if (m_requestNo)
    {
        ConsoleAuthMgr::getSingleton().SetRequest(m_requestNo, NULL);
        m_requestNo = 0;
    }
}

void TestConsoleLogin(std::string & username, std::string & password, uint32 requestno)
{
    sLogonCommHandler.TestConsoleLogon(username, password, requestno);
}

void ConsoleSocket::OnRead()
{
    uint32 readlen = (uint32)readBuffer.GetSize();
    uint32 len;
    char* p;

    if ((readlen + m_pBufferPos) >= m_pBufferLen)
    {
        Disconnect();
        return;
    }

    readBuffer.Read((uint8*)&m_pBuffer[m_pBufferPos], readlen);
    m_pBufferPos += readlen;

    // let's look for any newline bytes.
    p = strchr(m_pBuffer, '\n');
    while (p != NULL)
    {
        // windows is stupid. :P
        len = (uint32)((p + 1) - m_pBuffer);
        if (*(p - 1) == '\r')
            *(p - 1) = '\0';

        *p = '\0';

        // handle the command
        if (*m_pBuffer != '\0')
        {
            switch (m_state)
            {
                case STATE_USER:
                    m_username = std::string(m_pBuffer);
                    m_pConsole->Write("password: ");
                    m_state = STATE_PASSWORD;
                    break;

                case STATE_PASSWORD:
                    m_password = std::string(m_pBuffer);
                    m_pConsole->Write("\r\nAttempting to authenticate. Please wait.\r\n");
                    m_state = STATE_WAITING;

                    m_requestNo = ConsoleAuthMgr::getSingleton().GenerateRequestId();
                    ConsoleAuthMgr::getSingleton().SetRequest(m_requestNo, this);

                    TestConsoleLogin(m_username, m_password, m_requestNo);
                    break;

                case STATE_LOGGED:
                    if (!strnicmp(m_pBuffer, "quit", 4))
                    {
                        Disconnect();
                        break;
                    }

                    processConsoleInput(m_pConsole, m_pBuffer);
                    break;
            }
        }

        // move the bytes back
        if (len == m_pBufferPos)
        {
            m_pBuffer[0] = '\0';
            m_pBufferPos = 0;
        }
        else
        {
            memcpy(m_pBuffer, &m_pBuffer[len], m_pBufferPos - len);
            m_pBufferPos -= len;
        }

        p = strchr(m_pBuffer, '\n');
    }
}

void ConsoleSocket::OnConnect()
{
    m_pConsole->Write("Welcome to AscEmu's Remote Administration Console.\r\n");
    m_pConsole->Write("Please authenticate to continue. \r\n\r\n");
    m_pConsole->Write("login: ");
}

void ConsoleSocket::OnDisconnect()
{
    if (m_requestNo)
    {
        ConsoleAuthMgr::getSingleton().SetRequest(m_requestNo, NULL);
        m_requestNo = 0;
    }
    if (m_state == STATE_LOGGED)
    {
        LogNotice("RemoteConsole : User `%s` disconnected.", m_username.c_str());
    }
}

void ConsoleSocket::AuthCallback(bool result)
{
    ConsoleAuthMgr::getSingleton().SetRequest(m_requestNo, NULL);
    m_requestNo = 0;

    if (!result)
    {
        m_pConsole->Write("Authentication failed.\r\n\r\n");
        m_failedLogins++;
        if (m_failedLogins < 3)
        {
            m_pConsole->Write("login: ");
            m_state = STATE_USER;
        }
        else
        {
            Disconnect();
        }
    }
    else
    {
        m_pConsole->Write("User `%s` authenticated.\r\n\r\n", m_username.c_str());
        LogNotice("RemoteConsole : User `%s` authenticated.", m_username.c_str());
        const char* argv[1];
        handServerleInfoCommand(m_pConsole, 1, "");
        m_pConsole->Write("Type ? to see commands, quit to end session.\r\n");
        m_state = STATE_LOGGED;
    }
}

RemoteConsole::RemoteConsole(ConsoleSocket* pSocket)
{
    m_pSocket = pSocket;
}

void RemoteConsole::Write(const char* Format, ...)
{
    char obuf[65536];
    va_list ap;

    va_start(ap, Format);
    vsnprintf(obuf, 65536, Format, ap);
    va_end(ap);

    if (*obuf == '\0')
        return;

    m_pSocket->Send((const uint8*)obuf, (uint32)strlen(obuf));
}

struct ConsoleCommand
{
    bool(*CommandPointer)(BaseConsole*, int, std::string);
    std::string consoleCommand;
    int argumentCount;
    std::string argumentFormat;
    std::string commandDescription;
};

static ConsoleCommand Commands[] =
{
    { &handleSendChatAnnounceCommand,   "a",                1,  "<announce>",                           "Send chat announce." },
    { &handleSendChatAnnounceCommand,   "announce",         1,  "<announce>",                           "Send chat announce." },
    { &handleBanAccountCommand,         "ban",              3,  "<account> <time e.g. 3d> [reason]",    "Bans account <account> for time <time> with optional reason [reason]." },
    { &handleBanAccountCommand,         "banaccount",       3,  "<account> <time e.g. 3d> [reason]",    "Bans account <account> for time <time> with optional reason [reason]." },
    { &handleCancelShutdownCommand,     "cancel",           0,  "None",                                 "Cancels a pending shutdown." },
    { &handServerleInfoCommand,         "info",             0,  "None",                                 "Return current Server information." },
    { &handleOnlineGmsCommand,          "gms",              0,  "None",                                 "Shows online GMs." },
    { &handleKickPlayerCommand,         "kick",             2,  "<player name> [reason]",               "Kicks player <player name> for optional reason [reason]." },
    { &handleMotdCommand,               "getmotd",          0,  "None",                                 "View the current MOTD" },
    { &handleMotdCommand,               "setmotd",          1,  "<motd>",                               "Sets a new MOTD" },
    { &handleListOnlinePlayersCommand,  "online",           0,  "None",                                 "Shows online players." },
    { &handlePlayerInfoCommand,         "playerinfo",       1,  "<player name>",                        "Shows information about a player." },
    { &handleShutDownServerCommand,     "exit",             0,  "[delay]",                              "Server shutdown with optional delay in seconds." },
    { &handleShutDownServerCommand,     "shutdown",         0,  "[delay]",                              "Server shutdown with optional delay in seconds." },
    { &handleRehashConfigCommand,       "rehash",           0,  "None",                                 "Reload world-config." },
    { &handleUnbanAccountCommand,       "unban",            1,  "<account>",                            "Unbans account x." },
    { &handleUnbanAccountCommand,       "unbanaccount",     1,  "<account>",                            "Unbans account x." },
    { &handleSendWAnnounceCommand,      "w",                1,  "<wannounce>",                          "Send announce on screen for all." },
    { &handleSendWAnnounceCommand,      "wannounce",        1,  "<wannounce>",                          "Send announce on screen for all." },
    { &handleWhisperCommand,            "whisper",          2,  "<player> <message>",                   "Whispers message to someone." },
    { &handleCreateNameHashCommand,     "getnamehash",      1,  "<text>",                               "Returns the crc32 hash of <text>" },
    { &handleRevivePlayerCommand,       "reviveplr",        1,  "<player name>",                        "Revives a Player <player name>" },
    { &handleClearConsoleCommand,       "clear",            0,  "None",                                 "Clears the console." },
    { &handleReloadScriptEngineCommand, "reloadscripts",    0,  "None",                                 "Reloads all scripting engines currently loaded." },
    { &handlePrintTimeDateCommand,      "datetime",         0,  "None",                                 "Shows time and date according to localtime()" },
    { nullptr,                          "",                 0,  "",                                     "" },
};

void processConsoleInput(BaseConsole* baseConsole, std::string consoleInput)
{
    ConsoleCommand commandList;

    std::stringstream consoleInputStream(consoleInput);

    std::string commandName;
    std::string commandVars;

    consoleInputStream >> commandName;
    std::getline(consoleInputStream, commandVars);

    bool commandFound = false;
    bool isHelpCommand = false;
    for (int i = 0; Commands[i].consoleCommand.empty() == false; ++i)
    {
        if (commandName.empty())
            break;

        if (commandName.compare("help") == 0 || commandName.compare("?") == 0)
        {
            isHelpCommand = true;
            break;
        }

        if (Commands[i].consoleCommand.compare(commandName) == 0)
        {
            commandFound = true;
            commandList = Commands[i];
            break;
        }
    }

    if (isHelpCommand)
    {
        baseConsole->Write("Show Command list with ----- :%s\r\n", commandName.c_str());

        baseConsole->Write("===============================================================================\r\n");
        baseConsole->Write("| %15s | %57s |\r\n", "Name", "Arguments");
        baseConsole->Write("===============================================================================\r\n");

        for (int j = 0; Commands[j].consoleCommand.empty() == false; ++j)
        {
            baseConsole->Write("| %15s | %57s |\r\n", Commands[j].consoleCommand.c_str(), Commands[j].argumentFormat.c_str());
        }

        baseConsole->Write("===============================================================================\r\n");
        baseConsole->Write("| type 'quit' to terminate a Remote Console Session                           |\r\n");
        baseConsole->Write("===============================================================================\r\n");
    }
    else
    {
        if (commandFound)
        {
            baseConsole->Write("Received command: %s\r\n", commandName.c_str());
            if (commandList.argumentCount > 0 && commandVars.empty() == false)
                baseConsole->Write("Received vars: %s\r\n", commandVars.c_str());

            if (!commandList.CommandPointer(baseConsole, commandList.argumentCount, commandVars))
                baseConsole->Write("[!]Error, '%s' used an incorrect syntax, the correct syntax is: '%s'.\r\n\r\n",
                    commandList.consoleCommand.c_str(), commandList.argumentFormat.c_str());
        }
        else
        {
            baseConsole->Write("[!]Error, Command '%s' doesn't exist. Type '?' or 'help' to get a command overview.\r\n\r\n",
                commandName.c_str());
        }
    }
}

void LocalConsole::Write(const char* Format, ...)
{
    va_list ap;
    va_start(ap, Format);
    vprintf(Format, ap);
    va_end(ap);
}
