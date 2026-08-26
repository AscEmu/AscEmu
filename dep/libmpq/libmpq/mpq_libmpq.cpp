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

#include "mpq_libmpq04.h"
#include "mpqlib/MpqPatchChain.hpp"

#include <cstdio>
#include <cstring>
#include <memory>

namespace
{
    // The old code kept a flat, priority-ordered ArchiveSet where every
    // archive was an independent peer. mpqlib::MpqPatchChain instead has an
    // explicit base + patches - the first archive opened becomes the base,
    // every one after it is added as a patch, which reproduces the exact same
    // most-recently-opened-wins priority order the ArchiveSet gave via
    // push_front()+forward iteration.
    std::unique_ptr<mpqlib::MpqPatchChain> gMpqChain;
}

MPQArchive::MPQArchive(const char* filename) :
    m_opened(false)
{
    printf("Opening %s\n", filename);

    if (!gMpqChain)
    {
        gMpqChain = std::make_unique<mpqlib::MpqPatchChain>(filename);
        m_opened = gMpqChain->isOpen();
        if (!m_opened)
        {
            printf("Error opening archive '%s': Does file really exist?\n", filename);
            gMpqChain.reset();
        }
    }
    else
    {
        m_opened = gMpqChain->addPatch(filename);
        if (!m_opened)
            printf("Error opening archive '%s': Does file really exist?\n", filename);
    }
}

void MPQArchive::GetFileListTo(std::vector<std::string>& filelist)
{
    filelist = GetMpqFileList();
}

std::vector<std::string> GetMpqFileList()
{
    if (gMpqChain)
        return gMpqChain->listFiles();
    return {};
}

bool HasOpenMpqArchive()
{
    return gMpqChain != nullptr;
}

void CloseMpqArchives()
{
    gMpqChain.reset();
}

MPQFile::MPQFile(const char* filename):
    eof(false),
    buffer(0),
    pointer(0),
    size(0)
{
    std::vector<uint8_t> data;
    if (!gMpqChain || !gMpqChain->readFile(filename, data) || data.size() <= 1)
    {
        // HACK: in patch.mpq some files don't want to open and give 1 for filesize
        eof = true;
        buffer = 0;
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
    if (rpos > size) {
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
