/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include <iostream>

#include "Server/Console/BaseConsole.h"
#include "Server/Console/ConsoleThread.h"
#include "Logging/Logger.hpp"
#include "Threading/Thread.hpp"

#include <chrono>
#include <thread>

#ifndef _WIN32
#include <poll.h>
#endif

void ConsoleThread::run(AscEmu::Threading::AEThread& thread)
{
    LocalConsole localConsole;
    mStopConsoleThread = false;
    mIsConsoleThreadRunning = true;

    while (!mStopConsoleThread && !thread.isKilled())
    {
        std::string cmd;
        std::getline(std::cin, cmd);

        if (mStopConsoleThread || thread.isKilled())
            break;

        if (cmd.empty())
            continue;

        processConsoleInput(&localConsole, cmd);
    }

    mIsConsoleThreadRunning = false;
}

void ConsoleThread::stopThread()
{
    mStopConsoleThread = true;

#ifdef _WIN32
    // write the return keydown/keyup event
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

    sLogger.info("Waiting for console thread to terminate...");

    while (mIsConsoleThreadRunning)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    sLogger.info("Console shut down.");
}
