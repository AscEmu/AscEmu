/*
Copyright (c) 2014-2018 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "StdAfx.h"

#include "Server/Console/BaseConsole.h"
#include "Server/Console/ConsoleThread.h"
#include "Log.hpp"

#include <iostream>

#ifndef WIN32
    #include <poll.h>
#endif

bool ConsoleThread::runThread()
{
    SetThreadName("Console Interpreter");

    LocalConsole g_localConsole;

#ifndef WIN32
    struct pollfd pollInput;

    pollInput.fd = 0;
    pollInput.events = POLLIN | POLLPRI;
    pollInput.revents = 0;
#endif

    mStopConsoleThread = false;
    mIsConsoleThreadRunning = true;

    while (mStopConsoleThread != true)
    {
        std::string cmdInputText;
        std::getline(std::cin, cmdInputText);

        if (cmdInputText.empty())
            continue;

        if (mStopConsoleThread)
            break;

        processConsoleInput(&g_localConsole, cmdInputText);
    }

    mIsConsoleThreadRunning = false;

    return false;
}

void ConsoleThread::stopThread()
{
    mStopConsoleThread = true;

#ifdef WIN32
    /* write the return keydown/keyup event */
    DWORD tempDWORD;
    INPUT_RECORD inputRecord[2];

    for (int i = 0; i < 2; ++i)
    {
        inputRecord[i].EventType = KEY_EVENT;
        inputRecord[i].Event.KeyEvent.bKeyDown = TRUE;
        inputRecord[i].Event.KeyEvent.dwControlKeyState = 288;
        inputRecord[i].Event.KeyEvent.uChar.AsciiChar = 13;
        inputRecord[i].Event.KeyEvent.wRepeatCount = 1;
        inputRecord[i].Event.KeyEvent.wVirtualKeyCode = 13;
        inputRecord[i].Event.KeyEvent.wVirtualScanCode = 28;
    }

    WriteConsoleInput(GetStdHandle(STD_INPUT_HANDLE), inputRecord, 2, &tempDWORD);
#endif

    LOG_BASIC("Waiting for console thread to terminate...");

    while (mIsConsoleThreadRunning)
    {
        Arcemu::Sleep(100);
    }

    LOG_BASIC("Console shut down.");
}
