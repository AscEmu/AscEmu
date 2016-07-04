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

