/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <cstdint>
#include <optional>
#include <string>

#include "mpqlib/BinaryReader.hpp"
#include "mpqlib/MpqPatchChain.hpp"

#define FILE_FORMAT_VERSION 18

#pragma pack(push, 1)

union u_map_fcc
{
    char fcc_txt[4];
    uint32_t fcc;
};

// File version chunk
struct file_MVER
{
    union
    {
        uint32_t fcc;
        char fcc_txt[4];
    };
    uint32_t size;
    uint32_t ver;
};

class FileLoader
{
public:
    FileLoader();
    virtual ~FileLoader();

    virtual bool prepareLoadedData();
    virtual void free();

    bool loadFile(mpqlib::MpqPatchChain& mpq, std::string const& fileName, bool log = true);

    uint8_t* getData() const;
    uint32_t getDataSize() const;

    file_MVER* m_version;

private:
    std::optional<mpqlib::BinaryReader> m_reader;
};

#pragma pack(pop)
