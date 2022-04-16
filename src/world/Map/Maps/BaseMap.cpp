/*
Copyright (c) 2014-2022 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Storage/MySQLDataStore.hpp"
#include "Map/Cells/TerrainMgr.hpp"

#include "VMapFactory.h"
#include "VMapManager2.h"
#include "MMapManager.h"
#include "MMapFactory.h"

#include "Map/Cells/CellHandlerDefines.hpp"

BaseMap::BaseMap(uint32_t mapId, MySQLStructure::MapInfo const* mapInfo, DBC::Structures::MapEntry const* mapEntry)
{
    memset(spawns, 0, sizeof(CellSpawns*) * Map::Cell::_sizeX);

    _mapEntry = mapEntry;
    _mapInfo = mapInfo;
    _mapId = mapId;

    //new stuff Load Spawns
    loadSpawns(false);

    // get our name
    if (_mapInfo)
        name = _mapInfo->name;
    else
        name = "Unknown";
}

BaseMap::~BaseMap()
{
    sLogger.info("BaseMap : Close Tempelate from Map %u", this->_mapId);

    for (uint32_t x = 0; x < Map::Cell::_sizeX; x++)
    {
        if (spawns[x])
        {
            for (uint32_t y = 0; y < Map::Cell::_sizeY; y++)
            {
                if (spawns[x][y])
                {
                    CellSpawns* sp = spawns[x][y];
                    for (CreatureSpawnList::iterator i = sp->CreatureSpawns.begin(); i != sp->CreatureSpawns.end(); ++i)
                        delete(*i);
                    for (GameobjectSpawnList::iterator it = sp->GameobjectSpawns.begin(); it != sp->GameobjectSpawns.end(); ++it)
                        delete(*it);

                    delete sp;
                    spawns[x][y] = NULL;
                }
            }
            delete[] spawns[x];
        }
    }

    for (CreatureSpawnList::iterator i = staticSpawns.CreatureSpawns.begin(); i != staticSpawns.CreatureSpawns.end(); ++i)
        delete* i;
    for (GameobjectSpawnList::iterator i = staticSpawns.GameobjectSpawns.begin(); i != staticSpawns.GameobjectSpawns.end(); ++i)
        delete* i;
}

std::string BaseMap::getMapName()
{
    return name;
}

uint32_t BaseMap::getMapId() const
{
    return _mapEntry->id;
}

bool BaseMap::instanceable() const
{
    return _mapEntry && _mapEntry->instanceable();
}

bool BaseMap::isDungeon() const
{
    return _mapEntry && _mapEntry->isDungeon();
}

bool BaseMap::isNonRaidDungeon() const
{
    return _mapEntry && _mapEntry->isNonRaidDungeon();
}

bool BaseMap::isRaid() const
{
    return _mapEntry && _mapEntry->isRaid();
}

bool BaseMap::isBattleground() const
{
    return _mapEntry && _mapEntry->isBattleground();
}

bool BaseMap::isBattleArena() const
{
#if VERSION_STRING > Classic
    return _mapEntry && _mapEntry->isBattleArena();
#else
    return false;
#endif
}

bool BaseMap::isBattlegroundOrArena() const
{
    return _mapEntry && _mapEntry->isBattlegroundOrArena();
}

bool BaseMap::getEntrancePos(int32& mapid, float& x, float& y) const
{
#if VERSION_STRING > Classic
    if (!_mapEntry)
        return false;
    return _mapEntry->getEntrancePos(mapid, x, y);
#else
    return false;
#endif
}

void BaseMap::loadSpawns(bool reload)
{
    if (reload) // perform cleanup
    {
        for (uint32_t x = 0; x < Map::Cell::_sizeX; x++)
        {
            for (uint32_t y = 0; y < Map::Cell::_sizeY; y++)
            {
                if (spawns[x][y])
                {
                    CellSpawns* sp = spawns[x][y];
                    for (CreatureSpawnList::iterator i = sp->CreatureSpawns.begin(); i != sp->CreatureSpawns.end(); ++i)
                        delete(*i);
                    for (GameobjectSpawnList::iterator it = sp->GameobjectSpawns.begin(); it != sp->GameobjectSpawns.end(); ++it)
                        delete(*it);

                    delete sp;
                    spawns[x][y] = NULL;
                }
            }
        }
    }

    CreatureSpawnCount = 0;
    for (auto cspawn : sMySQLStore._creatureSpawnsStore[this->_mapId])
    {
        if (!sMySQLStore.isTransportMap(this->_mapId))
        {
            uint32_t cellx = CellHandler<MapMgr>::getPosX(cspawn->x);
            uint32_t celly = CellHandler<MapMgr>::getPosY(cspawn->y);
            if (!spawns[cellx])
            {
                spawns[cellx] = new CellSpawns * [Map::Cell::_sizeY];
                memset(spawns[cellx], 0, sizeof(CellSpawns*) * Map::Cell::_sizeY);
            }

            if (!spawns[cellx][celly])
                spawns[cellx][celly] = new CellSpawns;

            spawns[cellx][celly]->CreatureSpawns.push_back(cspawn);
            ++CreatureSpawnCount;
        }
    }

    GameObjectSpawnCount = 0;
    for (auto go_spawn : sMySQLStore._gameobjectSpawnsStore[this->_mapId])
    {
        if (go_spawn->overrides & GAMEOBJECT_MAPWIDE)
        {
            staticSpawns.GameobjectSpawns.push_back(go_spawn); //We already have a staticSpawns in the Map class, and it does just the right thing
            ++GameObjectSpawnCount;
        }
        else
        {
            // Zyres: transporter stuff
            if (sMySQLStore.getGameObjectProperties(go_spawn->entry)->type == 11 || sMySQLStore.getGameObjectProperties(go_spawn->entry)->type == 15)
            {
                staticSpawns.GameobjectSpawns.push_back(go_spawn);
            }
            else
            {
                uint32_t cellx = CellHandler<MapMgr>::getPosX(go_spawn->position_x);
                uint32_t celly = CellHandler<MapMgr>::getPosY(go_spawn->position_y);
                if (spawns[cellx] == NULL)
                {
                    spawns[cellx] = new CellSpawns * [Map::Cell::_sizeY];
                    memset(spawns[cellx], 0, sizeof(CellSpawns*) * Map::Cell::_sizeY);
                }

                if (!spawns[cellx][celly])
                    spawns[cellx][celly] = new CellSpawns;

                spawns[cellx][celly]->GameobjectSpawns.push_back(go_spawn);
            }

            ++GameObjectSpawnCount;
        }
    }
    sLogger.info("MapMgr : %u creatures / %u gobjects on map %u cached.", CreatureSpawnCount, GameObjectSpawnCount, _mapId);
}

CellSpawns* BaseMap::getSpawnsList(uint32_t cellx, uint32_t celly)
{
    if (cellx < Map::Cell::_sizeX && celly < Map::Cell::_sizeY)
    {
        if (spawns[cellx] == nullptr)
            return nullptr;

        return spawns[cellx][celly];
    }

    sLogger.failure("BaseMap::getSpawnsList invalid cell count! x: %u (max: %u) y:%u (max: %u)", cellx, Map::Cell::_sizeX, celly, Map::Cell::_sizeY);
    return nullptr;
}

CellSpawns* BaseMap::getSpawnsListAndCreate(uint32_t cellx, uint32_t celly)
{
    if (cellx < Map::Cell::_sizeX && celly < Map::Cell::_sizeY)
    {
        if (spawns[cellx] == nullptr)
        {
            spawns[cellx] = new CellSpawns * [Map::Cell::_sizeY];
            memset(spawns[cellx], 0, sizeof(CellSpawns*) * Map::Cell::_sizeY);
        }

        if (spawns[cellx][celly] == nullptr)
            spawns[cellx][celly] = new CellSpawns;
        return spawns[cellx][celly];
    }

    sLogger.failure("BaseMap::getSpawnsListAndCreate invalid cell count! x: %u (max: %u) y:%u (max: %u)", cellx, Map::Cell::_sizeX, celly, Map::Cell::_sizeY);
    return nullptr;
}
