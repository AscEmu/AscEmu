/*
Copyright (c) 2014-2022 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "DynamicTree.h"
#include "Map/Cells/TerrainMgr.hpp"

#include "Storage/MySQLStructures.h"

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
    BaseMap(uint32_t mapid, MySQLStructure::MapInfo const* inf, DBC::Structures::MapEntry const*);
    ~BaseMap();

    std::string getMapName();
    MySQLStructure::MapInfo const* getMapInfo() const { return _mapInfo; }

    // MapEntry
    DBC::Structures::MapEntry const* getMapEntry() const { return _mapEntry; }
    uint32_t getMapId() const;
    bool instanceable() const;
    bool isDungeon() const;
    bool isNonRaidDungeon() const;
    bool isRaid() const;
    bool isBattleground() const;
    bool isBattleArena() const;
    bool isBattlegroundOrArena() const;
    bool getEntrancePos(int32& mapid, float& x, float& y) const;

    // Cell
    void loadSpawns(bool reload);    // set to true to make clean up
    CellSpawns* getSpawnsList(uint32_t cellx, uint32_t celly);
    CellSpawns* getSpawnsListAndCreate(uint32_t cellx, uint32_t celly);

    uint32_t CreatureSpawnCount;
    uint32_t GameObjectSpawnCount;

    CellSpawns staticSpawns;

private:
    DBC::Structures::MapEntry const* _mapEntry;
    MySQLStructure::MapInfo const* _mapInfo;
    uint32_t _mapId;
   
    std::string name;

    CellSpawns** spawns[Map::Cell::_sizeX];
};