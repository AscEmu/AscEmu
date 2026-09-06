/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "Map/Management/TerrainMgr.hpp"
#include <array>
#include <memory>
#include <string>
#include <vector>
#include <cstdint>

namespace WDB::Structures
{
    struct MapEntry;
}

namespace MySQLStructure
{
    struct MapInfo;
}

enum LineOfSightChecks : uint8_t
{
    LINEOFSIGHT_CHECK_VMAP              = 0x1, // check static floor layout data
    LINEOFSIGHT_CHECK_GOBJECT           = 0x2, // check dynamic game object data

    LINEOFSIGHT_ALL_CHECKS = (LINEOFSIGHT_CHECK_VMAP | LINEOFSIGHT_CHECK_GOBJECT)
};

class SERVER_DECL BaseMap
{
public:
    BaseMap(uint32_t mapid, MySQLStructure::MapInfo const* inf, WDB::Structures::MapEntry const*);
    ~BaseMap();

    std::string getMapName();
    MySQLStructure::MapInfo const* getMapInfo() const { return _mapInfo; }

    // MapEntry
    WDB::Structures::MapEntry const* getMapEntry() const { return _mapEntry; }
    uint32_t getMapId() const;
    bool isDungeon() const;
    bool isRaid() const;
    bool isBattleground() const;
    bool isArena() const;
    bool isBattlegroundOrArena() const;
    bool isWorldMap() const;
    bool isInstanceMap() const;
    bool isInstanceableMap() const;
    bool getEntrancePos(int32_t& mapid, float& x, float& y) const;

private:
    WDB::Structures::MapEntry const* _mapEntry = nullptr;
    MySQLStructure::MapInfo const* _mapInfo = nullptr;
    uint32_t _mapId;
   
    std::string name;
};
