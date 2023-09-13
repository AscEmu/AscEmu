/*
Copyright (c) 2014-2023 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Map/Cells/CellHandlerDefines.hpp"
#include "Storage/MySQLDataStore.hpp"
#include "Storage/MySQLStructures.h"
#include "MMapManager.h"
#include "Logging/Logger.hpp"
#include "Map/Cells/CellHandler.hpp"
#include "Map/Management/MapMgr.hpp"
#include "Objects/GameObjectProperties.hpp"

BaseMap::BaseMap(uint32_t mapId, MySQLStructure::MapInfo const* mapInfo, WDB::Structures::MapEntry const* mapEntry) :
        _mapEntry(mapEntry), _mapInfo(mapInfo), _mapId(mapId)
{
    memset(spawns, 0, sizeof(CellSpawns*) * Map::Cell::_sizeX);

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

    for (CreatureSpawnList::iterator i = mapWideSpawns.CreatureSpawns.begin(); i != mapWideSpawns.CreatureSpawns.end(); ++i)
        delete* i;
    for (GameobjectSpawnList::iterator i = mapWideSpawns.GameobjectSpawns.begin(); i != mapWideSpawns.GameobjectSpawns.end(); ++i)
        delete* i;
    for (CreatureSpawnList::iterator i = areaWideSpawns.CreatureSpawns.begin(); i != areaWideSpawns.CreatureSpawns.end(); ++i)
        delete* i;
    for (GameobjectSpawnList::iterator i = areaWideSpawns.GameobjectSpawns.begin(); i != areaWideSpawns.GameobjectSpawns.end(); ++i)
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

bool BaseMap::getEntrancePos(int32_t& mapid, float& x, float& y) const
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
        GameObjectOverrides m_overrides = GAMEOBJECT_NORMAL_DISTANCE;
        if (GameObjectProperties const* gameobject_properties = sMySQLStore.getGameObjectProperties(go_spawn->entry))
        {
            
            // Check if GameObject is Large
            if (gameobject_properties->isLargeGameObject())
                m_overrides = GAMEOBJECT_AREAWIDE;  // not implemented yet

            // Check if GameObject is Infinite
            if (gameobject_properties->isInfiniteGameObject())
                m_overrides = GAMEOBJECT_MAPWIDE;
        }

        switch (m_overrides)
        {
            case GAMEOBJECT_NORMAL_DISTANCE:
            {
                uint32_t cellx = CellHandler<MapMgr>::getPosX(go_spawn->spawnPoint.x);
                uint32_t celly = CellHandler<MapMgr>::getPosY(go_spawn->spawnPoint.y);
                if (spawns[cellx] == NULL)
                {
                    spawns[cellx] = new CellSpawns * [Map::Cell::_sizeY];
                    memset(spawns[cellx], 0, sizeof(CellSpawns*) * Map::Cell::_sizeY);
                }

                if (!spawns[cellx][celly])
                    spawns[cellx][celly] = new CellSpawns;

                spawns[cellx][celly]->GameobjectSpawns.push_back(go_spawn);

                ++GameObjectSpawnCount;
            } break;
            case GAMEOBJECT_MAPWIDE:
            {
                mapWideSpawns.GameobjectSpawns.push_back(go_spawn);
                ++GameObjectSpawnCount;
            } break;
            case GAMEOBJECT_AREAWIDE:
            {
                areaWideSpawns.GameobjectSpawns.push_back(go_spawn);
                ++GameObjectSpawnCount;
            } break;
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
