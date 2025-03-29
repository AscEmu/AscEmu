/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "Map/Cells/TerrainMgr.hpp"
#include <array>
#include <memory>
#include <string>
#include <vector>

namespace WDB::Structures
{
    struct MapEntry;
}

namespace MySQLStructure
{
    struct CreatureSpawn;
    struct GameobjectSpawn;
    struct MapInfo;
}

typedef std::vector<MySQLStructure::CreatureSpawn*> CreatureSpawnList;
typedef std::vector<MySQLStructure::GameobjectSpawn*> GameobjectSpawnList;

struct CellSpawns
{
    CreatureSpawnList CreatureSpawns;
    GameobjectSpawnList GameobjectSpawns;
};

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
    bool instanceable() const;
    bool isDungeon() const;
    bool isNonRaidDungeon() const;
    bool isRaid() const;
    bool isBattleground() const;
    bool isBattleArena() const;
    bool isBattlegroundOrArena() const;
    bool getEntrancePos(int32_t& mapid, float& x, float& y) const;

    // Cell
    void loadSpawns(bool reload);    // set to true to make clean up
    CellSpawns* getSpawnsList(uint32_t cellx, uint32_t celly);
    CellSpawns* getSpawnsListAndCreate(uint32_t cellx, uint32_t celly);

    uint32_t CreatureSpawnCount;
    uint32_t GameObjectSpawnCount;

    CellSpawns mapWideSpawns;
    CellSpawns areaWideSpawns;

private:
    WDB::Structures::MapEntry const* _mapEntry = nullptr;
    MySQLStructure::MapInfo const* _mapInfo = nullptr;
    uint32_t _mapId;
   
    std::string name;

    std::array<std::unique_ptr<std::array<std::unique_ptr<CellSpawns>, Map::Cell::_sizeY>>, Map::Cell::_sizeX> spawns = { nullptr };
};
