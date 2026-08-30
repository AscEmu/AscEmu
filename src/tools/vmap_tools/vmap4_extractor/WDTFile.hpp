/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <string>

#include "mpqlib/MPQFile.hpp"

class ADTFile;

class WDTFile
{
public:
    WDTFile(std::string const& fileName, std::string const& fileName1);
    ~WDTFile();

    bool init(std::string const& mapId, unsigned int mapID);
    ADTFile* getMap(int x, int z);

    std::string* m_wmoInstanceNames;
    int m_wmoCount;

private:
    MPQFile m_wdt;
    std::string m_filename;

    // MAIN chunk's per-tile existence bitmap, read once in init() (before
    // m_wdt gets closed) - getMap() consults this so it can correctly
    // report "this tile isn't in the WDT" instead of unconditionally
    // returning an ADTFile for all 4096 grid positions regardless of
    // whether the map actually has that many tiles.
    bool m_tileExists[64][64] = {};
};
