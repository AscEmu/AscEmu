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
#include "Server/Console/BaseConsole.h"
#include "Server/Console/CConsole.h"
#include "Log.hpp"

LocalConsole g_localConsole;

#ifndef WIN32
#include <poll.h>
#endif

void ConsoleThread::terminate()
{
    m_killSwitch = true;
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
    while (m_isRunning)
    {
        Arcemu::Sleep(100);
    }
    LOG_BASIC("Console shut down.");
}

bool ConsoleThread::run()
{
    SetThreadName("Console Interpreter");

#ifndef WIN32
    struct pollfd input;

    input.fd      = 0;
    input.events  = POLLIN | POLLPRI;
    input.revents = 0;
#endif

    m_killSwitch = false;
    m_isRunning = true;

    while (m_killSwitch != true)
    {
        std::string cmdInputText;
        std::getline(std::cin, cmdInputText);

        if (cmdInputText.empty())
            continue;

        if (m_killSwitch)
            break;

        processConsoleInput(&g_localConsole, cmdInputText);
    }

    m_isRunning = false;
    return false;
}
