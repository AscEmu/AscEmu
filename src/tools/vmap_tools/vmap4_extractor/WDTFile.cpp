/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "vmapexport.h"
#include "WDTFile.hpp"
#include "ADTFile.hpp"

#include "mpqlib/ChunkReader.hpp"

#include <cstdio>
#include <cstring>
#include <memory>
#include <vector>

extern std::unique_ptr<mpqlib::MpqPatchChain> WorldMpq;

char* wdtGetPlainName(char* fileName)
{
    if (char* temp = strrchr(fileName, '\\'))
        fileName = temp + 1;
    return fileName;
}

WDTFile::WDTFile(std::string const& fileName, std::string const& fileName1) :
    m_wmoInstanceNames(nullptr), m_wmoCount(0), m_wdt(*WorldMpq, fileName.c_str()), m_filename(fileName1)
{
}

bool WDTFile::init(std::string const& /*mapId*/, unsigned int mapID)
{
    if (m_wdt.isEof())
        return false;

    std::string dirName = std::string(szWorkDirWmo) + "/dir_bin";
    FILE* dirFile = fopen(dirName.c_str(), "ab");
    if (!dirFile)
    {
        printf("Can't open dirfile!'%s'\n", dirName.c_str());
        return false;
    }

    mpqlib::ChunkReader chunks(m_wdt.reader());
    mpqlib::Chunk chunk;
    while (chunks.next(chunk))
    {
        if (chunk.tag == "MAIN")
        {
            // Per-tile existence bitmap: 64x64 entries of {uint32_t flag;
            // uint32_t data1;}, row-major [y][x], bit 0 of flag set means
            // this (x,y) ADT tile actually exists - same format
            // map_extractor's own WDT MAIN-chunk read already relies on.
            // Without this, getMap() had no way to tell a real tile from
            // one of the other, usually-thousands-of, grid positions a map
            // simply doesn't use, and unconditionally returned an ADTFile
            // for all 4096 of them regardless.
            size_t const expectedSize = 64 * 64 * 8;
            if (chunk.payload.size() >= expectedSize)
            {
                auto const* entries = reinterpret_cast<uint32_t const*>(chunk.payload.data());
                for (int y = 0; y < 64; ++y)
                    for (int x = 0; x < 64; ++x)
                        m_tileExists[y][x] = (entries[(y * 64 + x) * 2] & 0x1) != 0;
            }
        }
        else if (chunk.tag == "MWMO")
        {
            // global map objects
            if (chunk.payload.empty())
                continue;

            std::vector<char> buf(chunk.payload.size());
            std::memcpy(buf.data(), chunk.payload.data(), buf.size());

            char* p = buf.data();
            int q = 0;
            m_wmoInstanceNames = new std::string[buf.size()];
            while (p < buf.data() + buf.size())
            {
                char* s = wdtGetPlainName(p);
                fixNameCase(s, strlen(s));
                p = p + strlen(p) + 1;
                m_wmoInstanceNames[q++] = s;
            }
        }
        else if (chunk.tag == "MODF")
        {
            // global wmo instance data
            if (chunk.payload.empty())
                continue;

            auto const payloadOffset = static_cast<size_t>(chunk.payload.data() - m_wdt.reader().bytes().data());
            m_wdt.seek(static_cast<int>(payloadOffset));

            m_wmoCount = static_cast<int>(chunk.payload.size()) / 64;
            for (int i = 0; i < m_wmoCount; ++i)
            {
                int id;
                m_wdt.read(&id, 4);
                WMOInstance inst(m_wdt, m_wmoInstanceNames[id].c_str(), mapID, 65, 65, dirFile);
            }
            delete[] m_wmoInstanceNames;
            m_wmoInstanceNames = nullptr;
        }
    }

    m_wdt.close();
    fclose(dirFile);
    return true;
}

WDTFile::~WDTFile()
{
    m_wdt.close();
}

ADTFile* WDTFile::getMap(int x, int z)
{
    if (!(x >= 0 && z >= 0 && x < 64 && z < 64))
        return nullptr;

    if (!m_tileExists[z][x])
        return nullptr;

    // Cata split each ADT tile's data across up to three files
    // (root/_obj0/_tex0); model/WMO names and placements - everything this
    // tool reads - live in _obj0. Legacy clients have just the one file.
    std::string const suffix = IsLegacyVmapArchiveLayout() ? ".adt" : "_obj0.adt";
    std::string const name = "World\\Maps\\" + m_filename + "\\" + m_filename + "_" +
        std::to_string(x) + "_" + std::to_string(z) + suffix;
    return new ADTFile(name);
}
