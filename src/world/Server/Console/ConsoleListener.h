/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

template <class T>
class ListenSocket;

#include "ConsoleSocket.h"

bool StartConsoleListener();
void CloseConsoleListener();

#ifdef _WIN32
ListenSocket<ConsoleSocket>* GetConsoleListener();
#endif
