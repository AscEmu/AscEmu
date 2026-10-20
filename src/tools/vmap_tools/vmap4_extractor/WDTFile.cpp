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

namespace
{
    constexpr uint32_t kWdtHasMaid = 0x200;         // MPHD flag: the MAID chunk names the tile files
    constexpr size_t kMaidEntryWords = 8;           // root, obj0, obj1, tex0, lod, map texture, normal, minimap
    constexpr size_t kMaidObjFileDataIdOffset = 1;
}

char* wdtGetPlainName(char* fileName)
{
    if (char* temp = strrchr(fileName, '\\'))
        fileName = temp + 1;
    return fileName;
}

WDTFile::WDTFile(std::string const& fileName, std::string const& mapName) :
    m_wmoInstanceNames(nullptr), m_wmoCount(0), m_wdt(OpenWorldFile(fileName)), m_filename(mapName)
{
}

WDTFile::WDTFile(uint32_t fileDataId, std::string const& mapName) :
    m_wmoInstanceNames(nullptr), m_wmoCount(0), m_wdt(OpenWorldFile(FileDataIdName(fileDataId))), m_filename(mapName)
{
}

bool WDTFile::init(std::string const& /*mapId*/, unsigned int mapID)
{
    if (m_wdt->isEof())
        return false;

    std::string dirName = std::string(szWorkDirWmo) + "/dir_bin";
    FILE* dirFile = fopen(dirName.c_str(), "ab");
    if (!dirFile)
    {
        printf("Can't open dirfile!'%s'\n", dirName.c_str());
        return false;
    }

    bool hasMaidFlag = false;
    mpqlib::ChunkReader chunks(m_wdt->reader());
    mpqlib::Chunk chunk;
    while (chunks.next(chunk))
    {
        if (chunk.tag == "MPHD")
        {
            if (chunk.payload.size() >= sizeof(uint32_t))
            {
                uint32_t flags = 0;
                std::memcpy(&flags, chunk.payload.data(), sizeof(flags));
                hasMaidFlag = (flags & kWdtHasMaid) != 0;
            }
        }
        else if (chunk.tag == "MAIN")
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
        else if (chunk.tag == "MAID")
        {
            // 8.1 and later: eight file data ids per tile, the _obj0 file is the second
            if (chunk.payload.size() >= 64 * 64 * kMaidEntryWords * sizeof(uint32_t))
            {
                auto const* entries = reinterpret_cast<uint32_t const*>(chunk.payload.data());
                for (int y = 0; y < 64; ++y)
                    for (int x = 0; x < 64; ++x)
                        m_tileObjFileDataIds[y][x] = entries[(y * 64 + x) * kMaidEntryWords + kMaidObjFileDataIdOffset];
                m_hasTileFileDataIds = true;
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
                std::string path(p);
                char* s = wdtGetPlainName(p);
                fixNameCase(s, strlen(s));
                p = p + strlen(p) + 1;
                m_wmoInstanceNames[q++] = s;
                ExtractSingleWmo(path);
            }
        }
        else if (chunk.tag == "MODF")
        {
            // global wmo instance data
            if (chunk.payload.empty())
                continue;

            auto const payloadOffset = static_cast<size_t>(chunk.payload.data() - m_wdt->reader().bytes().data());
            m_wdt->seek(static_cast<int>(payloadOffset));

            m_wmoCount = static_cast<int>(chunk.payload.size()) / static_cast<int>(sizeof(AdtMapObjDef));
            for (int i = 0; i < m_wmoCount; ++i)
            {
                AdtMapObjDef def{};
                m_wdt->read(&def, sizeof(def));

                std::string name;
                if (def.flags & kMapObjDefFileDataId)
                {
                    name = FileDataIdName(def.id);
                    std::string path = name;
                    ExtractSingleWmo(path);
                }
                else if (m_wmoInstanceNames != nullptr)
                    name = m_wmoInstanceNames[def.id];
                else
                    continue;

                WMOInstance inst(def, name.c_str(), mapID, 65, 65, dirFile);
            }
            delete[] m_wmoInstanceNames;
            m_wmoInstanceNames = nullptr;
        }
    }

    if (!hasMaidFlag)
        m_hasTileFileDataIds = false;

    m_wdt->close();
    fclose(dirFile);
    return true;
}

WDTFile::~WDTFile()
{
    m_wdt->close();
}

ADTFile* WDTFile::getMap(int x, int z)
{
    if (!(x >= 0 && z >= 0 && x < 64 && z < 64))
        return nullptr;

    if (!m_tileExists[z][x])
        return nullptr;

    if (m_hasTileFileDataIds)
    {
        uint32_t const fileDataId = m_tileObjFileDataIds[z][x];
        if (fileDataId == 0)
            return nullptr;
        return new ADTFile(FileDataIdName(fileDataId));
    }

    // Cata split each ADT tile's data across up to three files
    // (root/_obj0/_tex0); model/WMO names and placements - everything this
    // tool reads - live in _obj0. Legacy clients have just the one file.
    std::string const suffix = IsLegacyVmapArchiveLayout() ? ".adt" : "_obj0.adt";
    std::string const name = "World\\Maps\\" + m_filename + "\\" + m_filename + "_" +
        std::to_string(x) + "_" + std::to_string(z) + suffix;
    return new ADTFile(name);
}
