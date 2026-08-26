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

// Historically a thin wrapper around libmpq's C API plus a global list of open
// archives (ArchiveSet/gOpenArchives). Internally reimplemented on top of
// mpqlib::MpqPatchChain (the same MPQ layer AscEmu's Cata/Mop tools use) - the
// public API below is unchanged so every existing call site (System.cpp,
// vmapexport.cpp, dbcfile.cpp, loadlib.cpp, CreatureDataExtractor.cpp) keeps
// working without modification to their MPQArchive/MPQFile usage.

#ifndef MPQ_H
#define MPQ_H

#include "loadlib.h"
#include <string>
#include <vector>

class MPQArchive
{
public:
    explicit MPQArchive(const char* filename);
    ~MPQArchive() = default;

    bool isOpen() const { return m_opened; }
    void GetFileListTo(std::vector<std::string>& filelist);
    void close() {}

private:
    bool m_opened;
};

class MPQFile
{
    //MPQHANDLE handle;
    bool eof;
    char *buffer;
    size_t pointer, size;

    // disable copying
    MPQFile(const MPQFile& /*f*/) = delete;
    void operator=(const MPQFile& /*f*/) = delete;

public:
    MPQFile(const char* filename);    // filenames are not case sensitive
    ~MPQFile() { close(); }
    size_t read(void* dest, size_t bytes);
    size_t getSize() { return size; }
    size_t getPos() { return pointer; }
    char* getBuffer() { return buffer; }
    char* getPointer() { return buffer + pointer; }
    bool isEof() { return eof; }
    void seek(int offset);
    void seekRelative(int offset);
    void close();
};

// Union listfile across every archive opened so far (base + patches),
// de-duplicated - replaces manually iterating the old ArchiveSet/gOpenArchives.
std::vector<std::string> GetMpqFileList();

// True once at least one archive has been successfully opened.
bool HasOpenMpqArchive();

// Discards the current archive chain so a fresh one can be opened from
// scratch - replaces the old per-archive CloseMPQFiles()/gOpenArchives.clear().
void CloseMpqArchives();

inline void flipcc(char *fcc)
{
    char t;
    t=fcc[0];
    fcc[0]=fcc[3];
    fcc[3]=t;
    t=fcc[1];
    fcc[1]=fcc[2];
    fcc[2]=t;
}

#endif
