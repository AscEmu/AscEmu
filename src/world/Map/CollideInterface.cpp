/*
 * AscEmu Framework based on ArcEmu MMORPG Server
 * Copyright (C) 2014-2016 AscEmu Team <http://www.ascemu.org/>
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

#define MAX_MAP 800

CCollideInterface CollideInterface;
Mutex m_loadLock;
uint32 m_tilesLoaded[MAX_MAP][64][64];

void CCollideInterface::Init()
{
    Log.Notice("CollideInterface", "Init");
    //CollisionMgr = ((IVMapManager*)collision_init());
}

void CCollideInterface::ActivateTile(uint32 mapId, uint32 tileX, uint32 tileY)
{
    VMAP::IVMapManager* mgr = VMAP::VMapFactory::createOrGetVMapManager();
    m_loadLock.Acquire();
    if (m_tilesLoaded[mapId][tileX][tileY] == 0)
    {
        mgr->loadMap(sWorld.vMapPath.c_str(), mapId, tileX, tileY);
        LoadNavMeshTile(mapId, tileX, tileY);
    }
    ++m_tilesLoaded[mapId][tileX][tileY];
    m_loadLock.Release();
}

void CCollideInterface::DeactivateTile(uint32 mapId, uint32 tileX, uint32 tileY)
{
    VMAP::IVMapManager* mgr = VMAP::VMapFactory::createOrGetVMapManager();
    m_loadLock.Acquire();
    if (!(--m_tilesLoaded[mapId][tileX][tileY]))
    {
        mgr->unloadMap(mapId, tileX, tileY);

        NavMeshData* nav = GetNavMesh(mapId);

        if (nav != NULL)
        {
            uint32 key = tileX | (tileY << 16);
            nav->tilelock.Acquire();
            std::map<uint32, dtTileRef>::iterator itr = nav->tilerefs.find(key);

            if (itr != nav->tilerefs.end())
            {
                nav->mesh->removeTile(itr->second, NULL, NULL);
                nav->tilerefs.erase(itr);
            }

            nav->tilelock.Release();
        }
    }

    m_loadLock.Release();
}

void CCollideInterface::DeInit()
{
    Log.Notice("CollideInterface", "DeInit");
    //collision_shutdown();
}

void CCollideInterface::ActivateMap(uint32 mapid)
{
    m_navmaplock.Acquire();
    std::map<uint32, NavMeshData*>::iterator itr = m_navdata.find(mapid);

    if (itr != m_navdata.end())
        ++itr->second->refs;
    else
    {
        //load params
        char filename[1024];
        sprintf(filename, "%s/%03i.mmap", sWorld.mMapPath.c_str(), mapid);
        FILE* f = fopen(filename, "rb");

        if (f == nullptr)
        {
            Log.Debug("CCollideInterface::ActivateMap", "File: %s/%u.mmap was not found!", sWorld.mMapPath.c_str(), mapid);
            m_navmaplock.Release();
            return;
        }

        dtNavMeshParams params;
        if (fread(&params, sizeof(params), 1, f) != 1)
        {
            m_navmaplock.Release();
            fclose(f);
            return;
        }

        fclose(f);

        NavMeshData* d = new NavMeshData;
        d->mesh = dtAllocNavMesh();
        d->query = dtAllocNavMeshQuery();
        d->mesh->init(&params);
        d->query->init(d->mesh, 1024);
        d->AddRef();
        m_navdata.insert(std::make_pair(mapid, d));
    }
    m_navmaplock.Release();
}

void CCollideInterface::DeactiveMap(uint32 mapid)
{
    m_navmaplock.Acquire();

    std::map<uint32, NavMeshData*>::iterator itr = m_navdata.find(mapid);

    if (itr != m_navdata.end())
    {
        if (itr->second->DecRef())
            m_navdata.erase(itr);
    }

    m_navmaplock.Release();
}

NavMeshData* CCollideInterface::GetNavMesh(uint32 mapId)
{
    if (sWorld.Pathfinding)
    {
        //Log.Debug("CCollideInterface::GetNavMesh", "Loading NavMeshData for map %u", mapId);
        NavMeshData* retval = NULL;
        m_navmaplock.Acquire();
        std::map<uint32, NavMeshData*>::iterator itr = m_navdata.find(mapId);

        if (itr != m_navdata.end())
            retval = itr->second;

        m_navmaplock.Release();
        return retval;
    }
    else
    {
        return NULL;
    }
}

void CCollideInterface::LoadNavMeshTile(uint32 mapId, uint32 tileX, uint32 tileY)
{
    NavMeshData* nav = GetNavMesh(mapId);

    if (nav == NULL)
        return;

    char filename[1024];
    sprintf(filename, "%s/%03i%02i%02i.mmtile", sWorld.mMapPath.c_str(), mapId, tileX, tileY);
    FILE* f = fopen(filename, "rb");

    if (f == nullptr)
    {
        sLog.Debug("CCollideInterface::LoadNavMeshTile", "File: %s was not found!", filename);
        return;
    }

    MmapTileHeader header;

    if (fread(&header, sizeof(MmapTileHeader), 1, f) != 1)
    {
        sLog.Debug("CCollideInterface::LoadNavMeshTile", "Reading Error!");
        fclose(f);
        return;
    }

    if (header.mmapMagic != MMAP_MAGIC || header.mmapVersion != MMAP_VERSION)
    {
        sLog.Debug("CCollideInterface::LoadNavMeshTile", "Load failed (%u %u %u): tile headers incorrect", mapId, tileX, tileY);
        fclose(f);
        return;
    }

    unsigned char* data = (unsigned char*)dtAlloc(header.size, DT_ALLOC_PERM);
    ASSERT(data);

    size_t result = fread(data, header.size, 1, f);
    if (!result)
    {
        sLog.Debug("CCollideInterface::LoadNavMeshTile", "Bad header or data in mmap %03u%02i%02i.mmtile", mapId, tileX, tileY);
        fclose(f);
        return;
    }

    fclose(f);

    dtTileRef dtref;
    nav->mesh->addTile(data, header.size, DT_TILE_FREE_DATA, 0, &dtref);

    nav->tilelock.Acquire();
    nav->tilerefs.insert(std::make_pair(tileX | (tileY << 16), dtref));
    nav->tilelock.Release();
}
