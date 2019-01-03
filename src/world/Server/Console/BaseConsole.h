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
 */


#pragma once

#include <string>

class ConsoleSocket;

class BaseConsole
{
    public:
        virtual ~BaseConsole() {}
        virtual void Write(const char* Format, ...) = 0;
};

class RemoteConsole : public BaseConsole
{
        ConsoleSocket* m_pSocket;
    public:
        RemoteConsole(ConsoleSocket* pSocket);
        void Write(const char* Format, ...);
};

class LocalConsole : public BaseConsole
{
    public:
        void Write(const char* Format, ...);
};

//MIT

void processConsoleInput(BaseConsole* baseConsole, std::string consoleInput, bool isWebClient = false);

//MIT end

