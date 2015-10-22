/*
 * AscEmu Framework based on ArcEmu MMORPG Server
 * Copyright (C) 2014-2015 AscEmu Team <http://www.ascemu.org/>
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
 *
 */

#include "LogonStdAfx.h"
#include "LogonConsole.h"

initialiseSingleton(LogonConsole);
bool Rehash();

void LogonConsole::TranslateRehash(char* str)
{
    sLog.outString("rehashing config file...");
    Rehash();
}

void LogonConsole::Kill()
{
    if (_thread != nullptr)
        _thread->kill.SetVal(true);
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
    LOG_BASIC("Waiting for console thread to terminate....");
    while (_thread != nullptr)
    {
        Arcemu::Sleep(100);
    }
    LOG_BASIC("Console shut down.");
}

bool LogonConsoleThread::run()
{
    new LogonConsole;

    SetThreadName("Console Interpreter");
    sLogonConsole._thread = this;
    size_t i = 0, len = 0;
    char cmd[96];

#ifndef WIN32
    fd_set fds;
    struct timeval tv;
#endif

    while (!kill.GetVal())
    {
#ifndef WIN32
        tv.tv_sec = 1;
        tv.tv_usec = 0;
        FD_ZERO(&fds);
        FD_SET(STDIN_FILENO, &fds);
        if(select(1, &fds, NULL, NULL, &tv) <= 0)
        {
            if(!kill.GetVal()) // timeout
                continue;
            else
                break;
        }
#endif
        // Make sure our buffer is clean to avoid Array bounds overflow
        memset(cmd, 0, sizeof(cmd));
        // Read in single line from "stdin"
        fgets(cmd, 80, stdin);

        if (kill.GetVal())
            break;

        len = strlen(cmd);
        for (i = 0; i < len; ++i)
        {
            if (cmd[i] == '\n' || cmd[i] == '\r')
                cmd[i] = '\0';
        }
        sLogonConsole.ProcessCmd(cmd);
    }

    sLogonConsole._thread = NULL;
    return true;
}

//------------------------------------------------------------------------------
// Protected methods:
//------------------------------------------------------------------------------
// Process one command
void LogonConsole::ProcessCmd(char* cmd)
{
    typedef void (LogonConsole::*PTranslater)(char * str);
    struct SCmd
    {
        const char* name;
        PTranslater tr;
    };

    SCmd cmds[] =
    {
        { "?", &LogonConsole::TranslateHelp },
        { "help", &LogonConsole::TranslateHelp },
        { "account create", &LogonConsole::AccountCreate },
        { "account delete", &LogonConsole::AccountDelete },
        { "account set gm", &LogonConsole::AccountSetGm },
        { "account set password", &LogonConsole::AccountSetPassword },
        { "account change password", &LogonConsole::AccountChangePassword },
        { "reload", &LogonConsole::ReloadAccts },
        { "rehash", &LogonConsole::TranslateRehash },
        { "netstatus", &LogonConsole::NetworkStatus },
        { "shutdown", &LogonConsole::TranslateQuit },
        { "exit", &LogonConsole::TranslateQuit },
        { "info", &LogonConsole::Info },
    };

    char cmd2[80];
    strcpy(cmd2, cmd);
    for (size_t i = 0; i < strlen(cmd); ++i)
        cmd2[i] = static_cast<char>(tolower(cmd[i]));

    for (size_t i = 0; i < sizeof(cmds) / sizeof(SCmd); i++)
        if (strncmp(cmd2, cmds[i].name, strlen(cmds[i].name)) == 0)
        {
            (this->*(cmds[i].tr))(cmd + strlen(cmds[i].name));
            return;
        }

    printf("Console: Unknown console command (use \"help\" for help).\n");
}

void LogonConsole::ReloadAccts(char* str)
{
    AccountMgr::getSingleton().ReloadAccounts(false);
    IPBanner::getSingleton().Reload();
}

void LogonConsole::NetworkStatus(char* str)
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
void LogonConsole::ProcessQuit(int delay)
{
    mrunning.SetVal(false);
}
//------------------------------------------------------------------------------
// help | ?
void LogonConsole::TranslateHelp(char* str)
{
    ProcessHelp(NULL);
}
void LogonConsole::ProcessHelp(char* command)
{
    if (command == NULL)
    {
        printf("Console:--------help--------\n");
        printf("    Help, ?: Prints this help text.\n");
        printf("    account create: Creates a new account\n");
        printf("    account delete: Deletes an account\n");
        printf("    account set gm: Sets gm access to account\n");
        printf("    account set password: Sets a new password for an account\n");
        printf("    account change password: Change the current password for an account\n");
        printf("    Reload: Reloads accounts.\n");
        printf("    Netstatus: Shows network status.\n");
        printf("    info:  shows some information about the server.\n");
        printf("    Shutdown, exit: Closes the logonserver.\n");
    }
}

void LogonConsole::Info(char* str)
{
    std::cout << "LogonServer information" << std::endl;
    std::cout << "-----------------------" << std::endl;
    std::cout << "CPU Usage: " << LogonServer::getSingleton().perfcounter.GetCurrentCPUUsage() << "%" << std::endl;
    std::cout << "RAM Usage: " << LogonServer::getSingleton().perfcounter.GetCurrentRAMUsage() << "MB" << std::endl;
}

void LogonConsole::AccountCreate(char* str)
{
    char name[512];
    char password[512];
    char email[512];

    int count = sscanf(str, "%s %s %s", name, password, email);
    if (count != 3)
    {
        std::cout << "usage: account create <name> <password> <email>" << std::endl;
        std::cout << "example: account create ghostcrawler Ih4t3p4l4dins greg.street@blizzard.com" << std::endl;
        return;
    }

    checkAccountName(name, ACC_NAME_NOT_EXIST);

    std::string pass;
    pass.assign(name);
    pass.push_back(':');
    pass.append(password);

    std::stringstream query;
    query << "INSERT INTO `accounts`( `login`,`encrypted_password`,`gm`,`banned`,`email`,`flags`,`banreason`) VALUES ( '";
    query << name << "',";
    query << "SHA( UPPER( '" << pass << "' ) ),'0','0','";
    query << email << "','";
    query << 24 << "','' );";

    if (!sLogonSQL->WaitExecuteNA(query.str().c_str()))
    {
        std::cout << "Couldn't save new account to database. Aborting." << std::endl;
        return;
    }

    AccountMgr::getSingleton().ReloadAccounts(true);

    std::cout << "Account created." << std::endl;
}

void LogonConsole::AccountDelete(char* str)
{
    char name[512];

    int count = sscanf(str, "%s", name);
    if (count != 1)
    {
        std::cout << "usage: account delete <name>" << std::endl;
        std::cout << "example: account delete ghostcrawler" << std::endl;
        return;
    }

    checkAccountName(name, ACC_NAME_DO_EXIST);

    std::stringstream query;
    query << "DELETE FROM `accounts` WHERE `login` = '";
    query << name << "';";

    if (!sLogonSQL->WaitExecuteNA(query.str().c_str()))
    {
        std::cout << "Couldn't delete account. Aborting." << std::endl;
        return;
    }

    AccountMgr::getSingleton().ReloadAccounts(true);

    std::cout << "Account deleted." << std::endl;
}

void LogonConsole::AccountSetGm(char* str)
{
    char name[512];
    char gmlevel[512];

    int count = sscanf(str, "%s %s", name, gmlevel);
    if (count != 2)
    {
        std::cout << "usage: account set gm <name> <gmlevel>" << std::endl;
        std::cout << "example: account set gm ghostcrawler az" << std::endl;
        return;
    }

    checkAccountName(name, ACC_NAME_DO_EXIST);

    std::string pass;
    pass.assign(name);
    pass.push_back(':');

    std::stringstream query;
    query << "UPDATE `accounts` SET `gm` = '";
    query << gmlevel << "' WHERE `login` = '";
    query << name << "'";

    if (!sLogonSQL->WaitExecuteNA(query.str().c_str()))
    {
        std::cout << "Couldn't update gmlevel to database. Aborting." << std::endl;
        return;
    }

    AccountMgr::getSingleton().ReloadAccounts(true);

    std::cout << "Account gmlevel set." << std::endl;
}

void LogonConsole::AccountSetPassword(char* str)
{
    char name[512];
    char password[512];

    int count = sscanf(str, "%s %s", name, password);
    if (count != 2)
    {
        std::cout << "usage: account set password <name> <password>" << std::endl;
        std::cout << "example: account set password ghostcrawler NewPassWoRd" << std::endl;
        return;
    }

    checkAccountName(name, ACC_NAME_DO_EXIST);

    std::string pass;
    pass.assign(name);
    pass.push_back(':');
    pass.append(password);

    std::stringstream query;
    query << "UPDATE `accounts` SET `encrypted_password` = ";
    query << "SHA( UPPER( '" << pass << "' ) ) WHERE `login` = '";
    query << name << "'";

    if (!sLogonSQL->WaitExecuteNA(query.str().c_str()))
    {
        std::cout << "Couldn't update password in database. Aborting." << std::endl;
        return;
    }

    AccountMgr::getSingleton().ReloadAccounts(true);

    std::cout << "Account password updated." << std::endl;
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
        std::cout << "usage: account change password <account> <old_password> <new_password> <new_password>" << std::endl;
        std::cout << "example: account change password ghostcrawler OldPasSworD FreshNewPassword FreshNewPassword" << std::endl;
        return;
    }

    checkAccountName(account_name, ACC_NAME_DO_EXIST);

    if (std::string(new_password_1) != std::string(new_password_2))
    {
        std::cout << "The new passwords doesn't match!" << std::endl;
        return;
    }

    std::string pass;
    pass.assign(account_name);
    pass.push_back(':');
    pass.append(old_password);

    auto check_oldpass_query = sLogonSQL->Query("SELECT login, encrypted_password FROM accounts WHERE encrypted_password=SHA(UPPER('%s')) AND login='%s'", pass.c_str(), std::string(account_name).c_str());

    if (!check_oldpass_query)
    {
        std::cout << "Your current password doesn't match with your input" << std::endl;
        return;
    }
    else
    {
        std::string new_pass;
        new_pass.assign(account_name);
        new_pass.push_back(':');
        new_pass.append(new_password_1);

        auto new_pass_query = sLogonSQL->Query("UPDATE accounts SET encrypted_password=SHA(UPPER('%s')) WHERE login='%s'", new_pass.c_str(), std::string(account_name).c_str());

        if (!new_pass_query)
        {
            //The query is already done, don't know why we are here. \todo check sLogonSQL query handling.
            //std::cout << "Can't update the password. Abort." << std::endl;
            return;
        }

    }

    AccountMgr::getSingleton().ReloadAccounts(true);

    std::cout << "Account password changed." << std::endl;
}

void LogonConsole::checkAccountName(std::string name, uint8 type)
{
    std::string aname(name);

    for (std::string::iterator itr = aname.begin(); itr != aname.end(); ++itr)
        *itr = toupper(*itr);

    switch (type)
    {
        case ACC_NAME_DO_EXIST:
        {
            if (AccountMgr::getSingleton().GetAccount(aname) == NULL)
            {
                std::cout << "There's no account with name " << name << std::endl;
                return;
            }

        } break;
        case ACC_NAME_NOT_EXIST:
        {
            if (AccountMgr::getSingleton().GetAccount(aname) != NULL)
            {
                std::cout << "There's already an account with name " << name << std::endl;
                return;
            }

        } break;
    }
}

//------------------------------------------------------------------------------

LogonConsoleThread::LogonConsoleThread()
{
    kill.SetVal(false);
}

LogonConsoleThread::~LogonConsoleThread()
{
}
