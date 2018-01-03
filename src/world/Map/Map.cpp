/*
 * AscEmu Framework based on ArcEmu MMORPG Server
 * Copyright (c) 2014-2018 AscEmu Team <http://www.ascemu.org>
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
    LogNotice("Map : ~Map %u", this->_mapId);

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

#define CREATURESPAWNSFIELDCOUNT 27
#define GOSPAWNSFIELDCOUNT 18

void Map::LoadSpawns(bool reload)
{
    if (reload)   //perform cleanup
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
    for (std::set<std::string>::iterator tableiterator = CreatureSpawnsTables.begin(); tableiterator != CreatureSpawnsTables.end(); ++tableiterator)
    {
        QueryResult* creature_spawn_result = WorldDatabase.Query("SELECT * FROM %s WHERE Map = %u", (*tableiterator).c_str(), this->_mapId);
        if (creature_spawn_result)
        {
            uint32 creature_spawn_fields = creature_spawn_result->GetFieldCount();
            if (creature_spawn_fields != CREATURESPAWNSFIELDCOUNT)
            {
                LOG_ERROR("Table `%s` has %u columns, but needs %u columns! Skipped!", (*tableiterator).c_str(), creature_spawn_fields, CREATURESPAWNSFIELDCOUNT);
                continue;
            }
            else
            {
                do
                {
                    Field* fields = creature_spawn_result->Fetch();
                    MySQLStructure::CreatureSpawn* cspawn = new MySQLStructure::CreatureSpawn;
                    cspawn->id = fields[0].GetUInt32();
                    cspawn->form = sMySQLStore.getCreatureFormationBySpawnId(cspawn->id);

                    uint32 creature_entry = fields[1].GetUInt32();
                    auto creature_properties = sMySQLStore.getCreatureProperties(creature_entry);
                    if (creature_properties == nullptr)
                    {
                        LOG_ERROR("Creature spawn ID: %u has invalid entry: %u which is not in creature_properties table! Skipped loading.", cspawn->id, creature_entry);
                        delete cspawn;
                        continue;
                    }

                    cspawn->entry = creature_entry;
                    cspawn->x = fields[3].GetFloat();
                    cspawn->y = fields[4].GetFloat();
                    cspawn->z = fields[5].GetFloat();
                    cspawn->o = fields[6].GetFloat();

                    uint32 cellx = CellHandler<MapMgr>::GetPosX(cspawn->x);
                    uint32 celly = CellHandler<MapMgr>::GetPosY(cspawn->y);
                    if (spawns[cellx] == NULL)
                    {
                        spawns[cellx] = new CellSpawns*[_sizeY];
                        memset(spawns[cellx], 0, sizeof(CellSpawns*)*_sizeY);
                    }

                    if (!spawns[cellx][celly])
                        spawns[cellx][celly] = new CellSpawns;

                    cspawn->movetype = fields[7].GetUInt8();
                    cspawn->displayid = fields[8].GetUInt32();
                    if (cspawn->displayid != 0)
                    {
                        DBC::Structures::CreatureDisplayInfoEntry const* creature_display = sCreatureDisplayInfoStore.LookupEntry(cspawn->displayid);
                        if (creature_display == nullptr)
                        {
                            LogError("Table %s includes invalid displayid %u for npc entry: %u, spawn_id: %u. Set to a random modelid!", (*tableiterator).c_str(), cspawn->displayid, cspawn->entry, cspawn->id);
                            cspawn->displayid = creature_properties->GetRandomModelId();
                        }
                    }
                    else
                    {
                        cspawn->displayid = creature_properties->GetRandomModelId();
                    }

                    cspawn->factionid = fields[9].GetUInt32();
                    cspawn->flags = fields[10].GetUInt32();
                    cspawn->bytes0 = fields[11].GetUInt32();
                    cspawn->bytes1 = fields[12].GetUInt32();
                    cspawn->bytes2 = fields[13].GetUInt32();
                    cspawn->emote_state = fields[14].GetUInt32();
                    //cspawn->respawnNpcLink = fields[15].GetUInt32();
                    cspawn->channel_spell = fields[16].GetUInt16();
                    cspawn->channel_target_go = fields[17].GetUInt32();
                    cspawn->channel_target_creature = fields[18].GetUInt32();
                    cspawn->stand_state = fields[19].GetUInt16();
                    cspawn->death_state = fields[20].GetUInt32();
                    cspawn->MountedDisplayID = fields[21].GetUInt32();
                    cspawn->Item1SlotDisplay = fields[22].GetUInt32();
                    cspawn->Item2SlotDisplay = fields[23].GetUInt32();
                    cspawn->Item3SlotDisplay = fields[24].GetUInt32();
                    cspawn->CanFly = fields[25].GetUInt32();

                    cspawn->phase = fields[26].GetUInt32();
                    if (cspawn->phase == 0)
                        cspawn->phase = 0xFFFFFFFF;

                    if (!stricmp((*tableiterator).c_str(), "creature_staticspawns"))
                    {
                        staticSpawns.CreatureSpawns.push_back(cspawn);
                        ++CreatureSpawnCount;
                    }
                    else
                    {
                        spawns[cellx][celly]->CreatureSpawns.push_back(cspawn);
                        ++CreatureSpawnCount;
                    }

                } while (creature_spawn_result->NextRow());
            }

            delete creature_spawn_result;
        }
    }

    GameObjectSpawnCount = 0;
    for (std::set<std::string>::iterator tableiterator = GameObjectSpawnsTables.begin(); tableiterator != GameObjectSpawnsTables.end(); ++tableiterator)
    {
        QueryResult* gobject_spawn_result = WorldDatabase.Query("SELECT * FROM %s WHERE map = %u", (*tableiterator).c_str(), this->_mapId);
        if (gobject_spawn_result)
        {
            uint32 gobject_spawn_fields = gobject_spawn_result->GetFieldCount();
            if (gobject_spawn_fields != GOSPAWNSFIELDCOUNT)
            {
                LOG_ERROR("Table `%s` has %u columns, but needs %u columns! Skipped!", (*tableiterator).c_str(), gobject_spawn_fields, GOSPAWNSFIELDCOUNT);
                continue;
            }
            else
            {
                do
                {
                    Field* fields = gobject_spawn_result->Fetch();
                    MySQLStructure::GameobjectSpawn* go_spawn = new MySQLStructure::GameobjectSpawn;
                    go_spawn->id = fields[0].GetUInt32();

                    uint32 gameobject_entry = fields[1].GetUInt32();
                    auto gameobject_info = sMySQLStore.getGameObjectProperties(gameobject_entry);
                    if (gameobject_info == nullptr)
                    {
                        LOG_ERROR("Gameobject spawn ID: %u has invalid entry: %u which is not in gameobject_properties table! Skipped loading.", go_spawn->id, gameobject_entry);
                        delete go_spawn;
                        continue;
                    }

                    go_spawn->entry = gameobject_entry;
                    go_spawn->map = fields[2].GetUInt32();
                    go_spawn->position_x = fields[3].GetFloat();
                    go_spawn->position_y = fields[4].GetFloat();
                    go_spawn->position_z = fields[5].GetFloat();
                    go_spawn->orientation = fields[6].GetFloat();
                    go_spawn->rotation_0 = fields[7].GetFloat();
                    go_spawn->rotation_1 = fields[8].GetFloat();
                    go_spawn->rotation_2 = fields[9].GetFloat();
                    go_spawn->rotation_3 = fields[10].GetFloat();
                    go_spawn->state = fields[11].GetUInt32();
                    go_spawn->flags = fields[12].GetUInt32();
                    go_spawn->faction = fields[13].GetUInt32();
                    go_spawn->scale = fields[14].GetFloat();
                    //gspawn->stateNpcLink = fields[15].GetUInt32();
                    go_spawn->phase = fields[16].GetUInt32();

                    if (go_spawn->phase == 0)
                        go_spawn->phase = 0xFFFFFFFF;

                    go_spawn->overrides = fields[17].GetUInt32();

                    if (go_spawn->overrides & GAMEOBJECT_MAPWIDE || !stricmp((*tableiterator).c_str(), "gameobject_staticspawns"))
                    {
                        staticSpawns.GameobjectSpawns.push_back(go_spawn); //We already have a staticSpawns in the Map class, and it does just the right thing
                        ++GameObjectSpawnCount;
                    }
                    else
                    {
                        uint32 cellx = CellHandler<MapMgr>::GetPosX(go_spawn->position_x);
                        uint32 celly = CellHandler<MapMgr>::GetPosY(go_spawn->position_y);
                        if (spawns[cellx] == NULL)
                        {
                            spawns[cellx] = new CellSpawns*[_sizeY];
                            memset(spawns[cellx], 0, sizeof(CellSpawns*)*_sizeY);
                        }

                        if (!spawns[cellx][celly])
                            spawns[cellx][celly] = new CellSpawns;

                        spawns[cellx][celly]->GameobjectSpawns.push_back(go_spawn);
                        ++GameObjectSpawnCount;
                    }

                } while (gobject_spawn_result->NextRow());
            }

            delete gobject_spawn_result;
        }
    }

    LOG_DETAIL("%u creatures / %u gobjects on map %u cached.", CreatureSpawnCount, GameObjectSpawnCount, _mapId);
}
