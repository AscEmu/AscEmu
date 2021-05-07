/*
 * AscEmu Framework based on ArcEmu MMORPG Server
 * Copyright (c) 2014-2021 AscEmu Team <http://www.ascemu.org>
 * Copyright (C) 2008-2012 ArcEmu Team <http://www.ArcEmu.org/>
 * Copyright (C) 2005-2007 Ascent Team
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "StdAfx.h"
#include "Storage/MySQLDataStore.hpp"
#include "Server/MainServerDefines.h"
#include "CellHandler.h"
#include "WorldCreator.h"

// Class Map
// Holder for all instances of each mapmgr, handles transferring players between, and template holding.
Map::Map(uint32 mapid, MySQLStructure::MapInfo const* inf)
{
    memset(spawns, 0, sizeof(CellSpawns*) * _sizeX);

    _mapInfo = inf;
    _mapId = mapid;

    //new stuff Load Spawns
    LoadSpawns(false);

    // get our name
    if (_mapInfo)
        name = _mapInfo->name;
    else
        name = "Unknown";
}

Map::~Map()
{
    sLogger.info("Map : ~Map %u", this->_mapId);

    for (uint32 x = 0; x < _sizeX; x++)
    {
        if (spawns[x])
        {
            for (uint32 y = 0; y < _sizeY; y++)
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
        delete *i;
    for (GameobjectSpawnList::iterator i = staticSpawns.GameobjectSpawns.begin(); i != staticSpawns.GameobjectSpawns.end(); ++i)
        delete *i;
}

std::string Map::GetMapName()
{
    return name;
}

CellSpawns* Map::GetSpawnsList(uint32 cellx, uint32 celly)
{
    ARCEMU_ASSERT(cellx < _sizeX);
    ARCEMU_ASSERT(celly < _sizeY);
    if (spawns[cellx] == NULL)
        return NULL;
    return spawns[cellx][celly];
}

CellSpawns* Map::GetSpawnsListAndCreate(uint32 cellx, uint32 celly)
{
    ARCEMU_ASSERT(cellx < _sizeX);
    ARCEMU_ASSERT(celly < _sizeY);
    if (spawns[cellx] == NULL)
    {
        spawns[cellx] = new CellSpawns*[_sizeY];
        memset(spawns[cellx], 0, sizeof(CellSpawns*)*_sizeY);
    }

    if (spawns[cellx][celly] == 0)
        spawns[cellx][celly] = new CellSpawns;
    return spawns[cellx][celly];
}

void Map::LoadSpawns(bool reload)
{
    if (reload) // perform cleanup
    { 
        for (uint32 x = 0; x < _sizeX; x++)
        {
            for (uint32 y = 0; y < _sizeY; y++)
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
        uint32 cellx = CellHandler<MapMgr>::GetPosX(cspawn->x);
        uint32 celly = CellHandler<MapMgr>::GetPosY(cspawn->y);
        if (!spawns[cellx])
        {
            spawns[cellx] = new CellSpawns * [_sizeY];
            memset(spawns[cellx], 0, sizeof(CellSpawns*) * _sizeY);
        }

        if (!spawns[cellx][celly])
            spawns[cellx][celly] = new CellSpawns;

        spawns[cellx][celly]->CreatureSpawns.push_back(cspawn);
        ++CreatureSpawnCount;

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
                uint32 cellx = CellHandler<MapMgr>::GetPosX(go_spawn->position_x);
                uint32 celly = CellHandler<MapMgr>::GetPosY(go_spawn->position_y);
                if (spawns[cellx] == NULL)
                {
                    spawns[cellx] = new CellSpawns * [_sizeY];
                    memset(spawns[cellx], 0, sizeof(CellSpawns*) * _sizeY);
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
