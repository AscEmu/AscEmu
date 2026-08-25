/*
 * AscEmu Framework based on ArcEmu MMORPG Server
 * Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
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

#include "mpqfile.h"
#include <deque>
#include <cstdio>

MPQFile::MPQFile(mpqlib::MpqPatchChain& mpq, const char* filename, bool warnNoExist /*= true*/) :
    eof(false),
    buffer(0),
    pointer(0),
    size(0)
{
    std::vector<uint8_t> data;
    if (!mpq.readFile(filename, data))
    {
        if (warnNoExist)
            fprintf(stderr, "Can't open %s!\n", filename);
        eof = true;
        return;
    }

    if (data.size() <= 1)
    {
        fprintf(stderr, "Can't open %s, size = %u!\n", filename, uint32_t(data.size()));
        eof = true;
        return;
    }

    size = data.size();
    buffer = new char[size];
    memcpy(buffer, data.data(), size);
}

size_t MPQFile::read(void* dest, size_t bytes)
{
    if (eof) return 0;

    size_t rpos = pointer + bytes;
    if (rpos > size)
    {
        bytes = size - pointer;
        eof = true;
    }

    memcpy(dest, &(buffer[pointer]), bytes);

    pointer = rpos;

    return bytes;
}

void MPQFile::seek(int offset)
{
    pointer = offset;
    eof = (pointer >= size);
}

void MPQFile::seekRelative(int offset)
{
    pointer += offset;
    eof = (pointer >= size);
}

void MPQFile::close()
{
    if (buffer) delete[] buffer;
    buffer = 0;
    eof = true;
}
