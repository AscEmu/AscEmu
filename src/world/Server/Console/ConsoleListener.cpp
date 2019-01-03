/*
 * AscEmu Framework based on ArcEmu MMORPG Server
 * Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
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

#include <cstdint>
#include <Network/Network.h>
#include <Config/Config.h>
#include <git_version.h>

#include "BaseConsole.h"
#include "ConsoleCommands.h"
#include "Server/World.h"
#include "Server/World.Legacy.h"

#include "ConsoleSocket.h"
#include "ConsoleAuthMgr.h"

ListenSocket<ConsoleSocket>* g_pListenSocket = nullptr;

void ConsoleAuthCallback(uint32_t request, uint32_t result)
{
    ConsoleSocket* consoleSocket = sConsoleAuthMgr.getSocketByRequestId(request);
    if (consoleSocket == nullptr || consoleSocket->IsConnected() == false)
    {
        return;
    }

    if (result)
    {
        consoleSocket->getConsoleAuthResult(true);
    }
    else
    {
        consoleSocket->getConsoleAuthResult(false);
    }
}

void CloseConsoleListener()
{
    if (g_pListenSocket != nullptr)
    {
        g_pListenSocket->Close();
    }
}

bool StartConsoleListener()
{
    if (worldConfig.remoteConsole.isEnabled == false)
    {
        return false;
    }

    std::string consoleListenHost = worldConfig.remoteConsole.host;
    uint32_t consoleListenPort = worldConfig.remoteConsole.port;

    g_pListenSocket = new ListenSocket<ConsoleSocket>(consoleListenHost.c_str(), consoleListenPort);
    if (g_pListenSocket == nullptr)
    {
        return false;
    }

    if (g_pListenSocket->IsOpen() == false)
    {
        g_pListenSocket->Close();

        delete g_pListenSocket;
        g_pListenSocket = nullptr;

        return false;
    }

    new ConsoleAuthMgr;
    return true;
}

ThreadBase* GetConsoleListener()
{
    return (ThreadBase*)g_pListenSocket;
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
    bool(*CommandPointer)(BaseConsole*, int, std::string, bool);
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
    { &handleCreateAccountCommand,      "createaccount",    2,  "<accountname> <password>",             "Creates an account X with password y" },
    { &handleCancelShutdownCommand,     "cancel",           0,  "None",                                 "Cancels a pending shutdown." },
    { &handleServerInfoCommand,         "info",             0,  "None",                                 "Return current Server information." },
    { &handleOnlineGmsCommand,          "gms",              0,  "None",                                 "Shows online GMs." },
    { &handleKickPlayerCommand,         "kick",             2,  "<player name> [reason]",               "Kicks player <player name> for optional reason [reason]." },
    { &handleMotdCommand,               "getmotd",          0,  "None",                                 "View the current MOTD" },
    { &handleMotdCommand,               "setmotd",          1,  "<motd>",                               "Sets a new MOTD" },
    { &handleAccountPermission,         "setaccpermission", 2,  "<account> <permission>",               "Aets permission y for account name x" },
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
    { &handleGetAccountsCommand,        "getaccountdata",   0,  "None",                                 "Prints out all account data" },
    { nullptr,                          "",                 0,  "",                                     "" },
};

void processConsoleInput(BaseConsole* baseConsole, std::string consoleInput, bool isWebClient)
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
        if (isWebClient == false)
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
    }
    else
    {
        if (commandFound)
        {
            if (isWebClient == false)
            {
                baseConsole->Write("Received command: %s\r\n", commandName.c_str());
            }

            if (commandList.argumentCount > 0 && commandVars.empty() == false)
            {
                if (isWebClient == false)
                {
                    baseConsole->Write("Received vars: %s\r\n", commandVars.c_str());
                }
            }

            if (!commandList.CommandPointer(baseConsole, commandList.argumentCount, commandVars, isWebClient))
            {
                if (isWebClient == false)
                {
                    baseConsole->Write("[!]Error, '%s' used an incorrect syntax, the correct syntax is: '%s'.\r\n\r\n",
                        commandList.consoleCommand.c_str(), commandList.argumentFormat.c_str());
                }
            }
        }
        else
        {
            if (isWebClient == false)
            {
                baseConsole->Write("[!]Error, Command '%s' doesn't exist. Type '?' or 'help' to get a command overview.\r\n\r\n",
                    commandName.c_str());
            }
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
