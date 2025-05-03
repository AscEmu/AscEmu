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

#ifndef LOAD_LIB_H
#define LOAD_LIB_H

#include "StormLib.h"
#include <cstdint>
#include <map>
#include <string>

#define FILE_FORMAT_VERSION    18

#pragma pack(push, 1)

union u_map_fcc
{
    char   fcc_txt[4];
    uint32_t fcc;
};

//
// File version chunk
//
struct file_MVER
{
    union {
        uint32_t fcc;
        char   fcc_txt[4];
    };
    uint32_t size;
    uint32_t ver;
};

struct file_MWMO
{
    u_map_fcc fcc;
    uint32_t size;
    char FileList[1];
};

class FileChunk
{
public:
    FileChunk(uint8_t* data_, uint32_t size_) : data(data_), size(size_) { }
    ~FileChunk();

    uint8_t* data;
    uint32_t size;

    template<class T>
    T* As() { return (T*)data; }
    void parseSubChunks();
    std::multimap<std::string, FileChunk*> subchunks;
    FileChunk* GetSubChunk(std::string const& name);
};

class ChunkedFile
{
public:
    uint8_t* data;
    uint32_t  data_size;

    uint8_t* GetData() { return data; }
    uint32_t GetDataSize() { return data_size; }

    ChunkedFile();
    virtual ~ChunkedFile();
    bool prepareLoadedData();
    bool loadFile(HANDLE mpq, std::string const& fileName, bool log = true);
    void free();

    void parseChunks();
    std::multimap<std::string, FileChunk*> chunks;
    FileChunk* GetChunk(std::string const& name);
};

#pragma pack(pop)

#endif