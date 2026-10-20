/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <cstdint>
#include <memory>
#include <string>

#include "mpqlib/MPQFile.hpp"

class ADTFile;

// The WDT of a map: which tiles exist, the map wide WMOs and, from 8.1 on,
// the file data ids of the tile files.
class WDTFile
{
public:
    // The MPQ clients and the CASC clients before 8.1 address the WDT by path.
    WDTFile(std::string const& fileName, std::string const& mapName);
    // 8.1 and later name the WDT in the map table.
    WDTFile(uint32_t fileDataId, std::string const& mapName);
    ~WDTFile();

    bool init(std::string const& mapId, unsigned int mapID);
    ADTFile* getMap(int x, int z);

    std::string* m_wmoInstanceNames;
    int m_wmoCount;

private:
    std::unique_ptr<MPQFile> m_wdt;
    std::string m_filename;

    // MAIN chunk's per-tile existence bitmap, read once in init() (before
    // m_wdt gets closed) - getMap() consults this so it can correctly
    // report "this tile isn't in the WDT" instead of unconditionally
    // returning an ADTFile for all 4096 grid positions regardless of
    // whether the map actually has that many tiles.
    bool m_tileExists[64][64] = {};

    // MAID chunk (8.1 and later): the _obj0 file of every tile by file data id.
    bool m_hasTileFileDataIds = false;
    uint32_t m_tileObjFileDataIds[64][64] = {};
};
