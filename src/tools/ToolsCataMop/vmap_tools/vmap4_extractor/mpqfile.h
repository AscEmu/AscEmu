/*
 * AscEmu Framework based on ArcEmu MMORPG Server
 * Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
 * Copyright (C) 2005-2011 MaNGOS <http://getmangos.com/>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#define _CRT_SECURE_NO_DEPRECATE
#ifndef _CRT_SECURE_NO_WARNINGS // fuck the police^Wwarnings
#define _CRT_SECURE_NO_WARNINGS
#endif

#ifndef MPQ_H
#define MPQ_H

#include <cstdint>
#include <string.h>
#include <ctype.h>
#include <vector>
#include <iostream>
#include <deque>
#include "StormLib.h"

using namespace std;

class MPQFile
{
    //MPQHANDLE handle;
    bool eof;
    char *buffer;
    size_t pointer, size;

    // disable copying
    MPQFile(const MPQFile &f);
    void operator=(const MPQFile &f);

public:
    MPQFile(HANDLE mpq, const char* filename, bool warnNoExist = true);    // filenames are not case sensitive
    ~MPQFile() {
        close();
    }
    size_t read(void* dest, size_t bytes);
    size_t getSize() {
        return size;
    }
    size_t getPos() {
        return pointer;
    }
    char* getBuffer() {
        return buffer;
    }
    char* getPointer() {
        return buffer + pointer;
    }
    bool isEof() {
        return eof;
    }
    void seek(int offset);
    void seekRelative(int offset);
    void close();
};

inline void flipcc(char *fcc)
{
    char t;
    t = fcc[0];
    fcc[0] = fcc[3];
    fcc[3] = t;
    t = fcc[1];
    fcc[1] = fcc[2];
    fcc[2] = t;
}

#endif  //MPQ_H
