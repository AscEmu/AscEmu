/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

bool StartConsoleListener();
void CloseConsoleListener();

#ifdef _WIN32
    // Returns the console listener thread
    ThreadBase* GetConsoleListener();
#endif