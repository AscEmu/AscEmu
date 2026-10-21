/*
 * AscEmu Framework based on ArcEmu MMORPG Server
 * Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
 * Copyright (C) 2008-2012 ArcEmu Team <http://www.ArcEmu.org/>
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
 */

#include "LogonConsole.h"
#include "Server/AccountService.hpp"
#include "Server/Logon.h"
#include "Server/LogonServerDefines.hpp"
#include <Logging/Logger.hpp>
#include <Server/Master.hpp>
#include <Server/AccountMgr.h>
#include <Server/IpBanMgr.h>
#include <Network/Network.hpp>
#include <LogonConf.hpp>
#include <sstream>
#include <Utilities/Strings.hpp>
#include "Database/Database.hpp"
#include <algorithm>
#include "Threading/ThreadPool.hpp"

LogonConsole& LogonConsole::getInstance()
{
    static LogonConsole mInstance;
    return mInstance;
}

void LogonConsole::TranslateRehash(char* /*str*/)
{
    sLogger.info("rehashing config file...");
    if (sMasterLogon.LoadLogonConfiguration())
        sLogger.info("Rehashing config file finished succesfull!");
}

void LogonConsole::demoTicker(AscEmu::Threading::AEThread& /*thread*/)
{
    fmt::println("Thread ticker: {}", m_demoCounter);
    ++m_demoCounter;
}

void LogonConsole::threadDemoCmd(char* /*str*/)
{
    fmt::println("Thread Demo init");

    if (m_demoCounter != 0)
    {
        fmt::println("Existing thread found, rebooting");
        m_demoThread->reboot();
        return;
    }

    std::function<void(AscEmu::Threading::AEThread&)> f = [this](AscEmu::Threading::AEThread& thread) { this->demoTicker(thread); };
    m_demoThread = std::make_unique<AscEmu::Threading::AEThread>(std::string("DemoThread"), f, std::chrono::milliseconds(100));
}

void LogonConsole::Kill()
{
    if (_thread != nullptr)
        _thread->kill = true;
#ifdef WIN32
    /* write the return keydown/keyup event */
    DWORD dwTmp;
    INPUT_RECORD ir[2];
    ir[0].EventType = KEY_EVENT;
    ir[0].Event.KeyEvent.bKeyDown = TRUE;
    ir[0].Event.KeyEvent.dwControlKeyState = 288;
    ir[0].Event.KeyEvent.uChar.AsciiChar = 13;
    ir[0].Event.KeyEvent.wRepeatCount = 1;
    ir[0].Event.KeyEvent.wVirtualKeyCode = 13;
    ir[0].Event.KeyEvent.wVirtualScanCode = 28;
    ir[1].EventType = KEY_EVENT;
    ir[1].Event.KeyEvent.bKeyDown = FALSE;
    ir[1].Event.KeyEvent.dwControlKeyState = 288;
    ir[1].Event.KeyEvent.uChar.AsciiChar = 13;
    ir[1].Event.KeyEvent.wRepeatCount = 1;
    ir[1].Event.KeyEvent.wVirtualKeyCode = 13;
    ir[1].Event.KeyEvent.wVirtualScanCode = 28;
    WriteConsoleInput(GetStdHandle(STD_INPUT_HANDLE), ir, 2, &dwTmp);
#endif
    sLogger.info("Waiting for console thread to terminate....");
    while (_thread != nullptr)
    {
        AscEmu::Threading::sleep(100);
    }
    sLogger.info("Console shut down.");
}

void LogonConsoleThread::run(AscEmu::Threading::AEThread& thread)
{
    sLogonConsole._thread = this;
    kill = false;

    size_t i = 0;
    size_t len = 0;
    char cmd[96];

#ifndef WIN32
    fd_set fds;
    struct timeval tv;
#endif

    while (!kill.load() && !thread.isKilled())
    {
#ifndef WIN32
        tv.tv_sec = 1;
        tv.tv_usec = 0;
        FD_ZERO(&fds);
        FD_SET(STDIN_FILENO, &fds);

        if (select(1, &fds, NULL, NULL, &tv) <= 0)
        {
            if (!kill.load() && !thread.isKilled())
                continue;
            else
                break;
        }
#endif

        memset(cmd, 0, sizeof(cmd));
        fgets(cmd, 80, stdin);

        if (kill.load() || thread.isKilled())
            break;

        len = strlen(cmd);
        for (i = 0; i < len; ++i)
        {
            if (cmd[i] == '\n' || cmd[i] == '\r')
                cmd[i] = '\0';
        }

        sLogonConsole.ProcessCmd(cmd);
    }

    sLogonConsole._thread = nullptr;
}

void LogonConsole::ProcessCmd(char* cmd)
{
    using PTranslater = void (LogonConsole::*)(char*);

    struct SCmd
    {
        std::string_view name;
        PTranslater tr;
    };

    static constexpr SCmd cmds[] =
    {
        { "?", &LogonConsole::TranslateHelp },
        { "z", &LogonConsole::threadDemoCmd },
        { "help", &LogonConsole::TranslateHelp },
        { "account create", &LogonConsole::AccountCreate },
        { "account delete", &LogonConsole::AccountDelete },
        { "account set password", &LogonConsole::AccountSetPassword },
        { "account change password", &LogonConsole::AccountChangePassword },
        { "reload", &LogonConsole::ReloadAccts },
        { "rehash", &LogonConsole::TranslateRehash },
        { "netstatus", &LogonConsole::NetworkStatus },
        { "shutdown", &LogonConsole::TranslateQuit },
        { "exit", &LogonConsole::TranslateQuit },
        { "info", &LogonConsole::Info },
    };

    if (!cmd)
        return;

    std::string input(cmd);
    std::transform(input.begin(), input.end(), input.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });

    for (const auto& c : cmds)
    {
        if (input.starts_with(c.name))
        {
            char* args = cmd + c.name.size();
            (this->*(c.tr))(args);
            return;
        }
    }

    fmt::println("[!]Error, Command '{}' doesn't exist. Type '?' or 'help' to get a command overview.", cmd);
}

void LogonConsole::ReloadAccts(char* /*str*/)
{
    sAccountMgr.reloadAccounts(false);
    sIpBanMgr.reload();
}

void LogonConsole::NetworkStatus(char* /*str*/)
{
    sSocketMgr.ShowStatus();
}

// quit | exit
void LogonConsole::TranslateQuit(char* str)
{
    int delay = str != NULL ? atoi(str) : 5000;
    if (!delay)
        delay = 5000;
    else
        delay *= 1000;

    ProcessQuit(delay);
}
void LogonConsole::ProcessQuit(int /*delay*/)
{
    mrunning = false;
}

///////////////////////////////////////////////////////////////////////////////
// Console commands - help | ?
void LogonConsole::TranslateHelp(char* /*str*/)
{
    ProcessHelp(NULL);
}

void LogonConsole::ProcessHelp(char* /*command*/)
{
    fmt::println("Console::Help");
    fmt::println("=============");
    fmt::println("Help, ?                   : Prints this help text.");
    fmt::println("Account create            : Creates an account for every client version.");
    fmt::println("Account delete            : Deletes an account.");
    fmt::println("Account set password      : Sets a new password for an account.");
    fmt::println("Account change password   : Change the current password for an account.");
    fmt::println("Info                      : Shows some information about the server.");
    fmt::println("Netstatus                 : Shows network status.");
    fmt::println("Rehash                    : Rehashing config file.");
    fmt::println("Reload                    : Reloads accounts.");
    fmt::println("Shutdown, Exit            : Closes the logonserver.");
}

void LogonConsole::Info(char* /*str*/)
{
    fmt::println("LogonServer information");
    fmt::println("=======================");
    fmt::println("CPU Usage : {}%", sLogon.getCPUUsage());
    fmt::println("RAM Usage : {}MB", sLogon.getRAMUsage());
}

void LogonConsole::AccountCreate(char* str)
{
    char name[512];
    char password[512];
    char email[512] = {};

    int count = sscanf(str, "%s %s %s", name, password, email);
    if (count < 2)
    {
        fmt::println("usage: account create <name> <password> [email]");
        fmt::println("example: account create ghostcrawler Ih4t3p4l4dins greg.street@blizzard.com");
        fmt::println("The account serves every client version; the 6.x and later clients log in with the name or the email.");
        return;
    }

    const auto result = AscEmu::Accounts::createAccount(name, password, email);
    if (result != AscEmu::Accounts::Result::Ok)
    {
        fmt::println("Couldn't create the account: {}.", AscEmu::Accounts::describe(result));
        return;
    }

    fmt::println("Account created.");
}

void LogonConsole::AccountDelete(char* str)
{
    char name[512];

    int count = sscanf(str, "%s", name);
    if (count != 1)
    {
        fmt::println("usage: account delete <name>");
        fmt::println("example: account delete ghostcrawler");
        return;
    }

    const auto result = AscEmu::Accounts::deleteAccount(name);
    if (result != AscEmu::Accounts::Result::Ok)
    {
        fmt::println("Couldn't delete the account: {}.", AscEmu::Accounts::describe(result));
        return;
    }

    fmt::println("Account deleted.");
}

void LogonConsole::AccountSetPassword(char* str)
{
    char name[512];
    char password[512];

    int count = sscanf(str, "%s %s", name, password);
    if (count != 2)
    {
        fmt::println("usage: account set password <name> <password>");
        fmt::println("example: account set password ghostcrawler NewPassWoRd");
        return;
    }

    const auto result = AscEmu::Accounts::setPassword(name, password);
    if (result != AscEmu::Accounts::Result::Ok)
    {
        fmt::println("Couldn't update the password: {}.", AscEmu::Accounts::describe(result));
        return;
    }

    fmt::println("Account password updated.");
}

void LogonConsole::AccountChangePassword(char* str)
{
    char account_name[512];
    char old_password[512];
    char new_password_1[512];
    char new_password_2[512];

    int count = sscanf(str, "%s %s %s %s", account_name, old_password, new_password_1, new_password_2);
    if (count != 4)
    {
        fmt::println("usage: account change password <account> <old_password> <new_password> <new_password>");
        fmt::println("example: account change password ghostcrawler OldPasSworD FreshNewPassword FreshNewPassword");
        return;
    }

    if (std::string(new_password_1) != std::string(new_password_2))
    {
        fmt::println("The new passwords don't match!");
        return;
    }

    const auto result = AscEmu::Accounts::changePassword(account_name, old_password, new_password_1);
    if (result != AscEmu::Accounts::Result::Ok)
    {
        fmt::println("Couldn't change the password: {}.", AscEmu::Accounts::describe(result));
        return;
    }

    fmt::println("Account password changed.");
}

void LogonConsole::checkAccountName(std::string name, uint8_t type)
{
    std::string aname(name);

    AscEmu::Util::Strings::toUpperCase(aname);

    switch (type)
    {
        case ACC_NAME_DO_EXIST:
        {
            if (sAccountMgr.getAccountByName(aname) == nullptr)
            {
                fmt::println("There's no account with name {}", name);
            }

        } break;
        case ACC_NAME_NOT_EXIST:
        {
            if (sAccountMgr.getAccountByName(aname) != nullptr)
            {
                fmt::println("There's already an account with name {}", name);
            }
        } break;
    }
}

LogonConsoleThread::LogonConsoleThread()
{
    kill = false;
}

LogonConsoleThread::~LogonConsoleThread()
{
}
