/*
 * AscEmu Framework based on ArcEmu MMORPG Server
 * Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
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
 */

#ifndef MAP_H
#define MAP_H

#include "TerrainMgr.h"
#include "CellHandlerDefines.hpp"
#include "Storage/DBC/DBCStructures.hpp"
#include "Storage/MySQLStructures.h"

class MapMgr;
class TerrainMgr;


typedef std::vector<MySQLStructure::CreatureSpawn*> CreatureSpawnList;
typedef std::vector<MySQLStructure::GameobjectSpawn*> GameobjectSpawnList;

struct CellSpawns
{
    CreatureSpawnList CreatureSpawns;
    GameobjectSpawnList GameobjectSpawns;
};


class SERVER_DECL Map
{
    public:

        Map(uint32 mapid, MySQLStructure::MapInfo const* inf);
        ~Map();

        std::string GetMapName();

        CellSpawns* GetSpawnsList(uint32 cellx, uint32 celly);

        CellSpawns* GetSpawnsListAndCreate(uint32 cellx, uint32 celly);

        void LoadSpawns(bool reload);           // set to true to make clean up
        uint32 CreatureSpawnCount;
        uint32 GameObjectSpawnCount;

    private:

        MySQLStructure::MapInfo const* _mapInfo;
        uint32 _mapId;
        std::string name;

        CellSpawns** spawns[_sizeX];

    public:

        CellSpawns staticSpawns;
};

#endif // MAP_H
