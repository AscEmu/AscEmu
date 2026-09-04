/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "AreaStorage.hpp"

#include <G3D/g3dmath.h>

#include "VMapFactory.h"
#include "VMapManager2.h"
#include "MMapFactory.h"
#include "Macros/MapsMacros.hpp"
#include "AreaManagementGlobals.hpp"
#include "Utilities/LocationVector.hpp"
#include "Logging/Logger.hpp"
#include "Map/Maps/WorldMap.hpp"
#include "Movement/MovementDefines.h"
#include "Storage/WDB/WDBGlobals.hpp"
#include "Storage/WDB/WDBStores.hpp"
#include "Storage/WDB/WDBStructures.hpp"

#include <cmath>
#include <cstdint>

namespace MapManagement::AreaManagement
{
    MapEntryPair* AreaStorage::getMapCollection()
    {
        return &AreaStorage::m_map_storage;
    }

    void AreaStorage::initialise(WDB::WDBStore<WDB::Structures::AreaTableEntry> const* container)
    {
        m_areaContainer = container;

        // Preload this stuff to make lookups easier elsewhere in code
        for (const auto& area : *m_areaContainer | std::views::values)
        {
            if (area.zone == 0 && area.map_id != 0 && area.map_id != 1 && area.map_id != 530 && area.map_id != 571)
            {
                m_areaIdByMapIdCollection.emplace(area.map_id, area.id);
            }
        }
    }

    WDB::WDBStore<WDB::Structures::AreaTableEntry> const* AreaStorage::getStorage()
    {
        return m_areaContainer;
    }

    WDB::Structures::AreaTableEntry const* AreaStorage::getAreaByMapId(uint32_t mapId)
    {
        auto const iter = m_areaIdByMapIdCollection.find(mapId);
        if (iter == m_areaIdByMapIdCollection.end())
            return nullptr;

        return getAreaById(iter->second);
    }

    WDB::Structures::AreaTableEntry const* AreaStorage::getExactArea(WorldMap* worldMap, LocationVector const& pos, uint32_t phaseMask)
    {
        if (worldMap == nullptr)
            return nullptr;

        uint32_t mogpFlags = 0;
        int32_t adtId = 0;
        int32_t rootId = 0;
        int32_t groupId = 0;
        uint32_t adtAreaId = 0;
        float_t tileMapHeight = INVALID_HEIGHT;

        const auto hasAreaInfo = worldMap->getAreaInfo(phaseMask, pos, mogpFlags, adtId, rootId, groupId);

        if (auto const* terrain = worldMap->getTerrain())
        {
            if (const auto* tile = worldMap->getTerrain()->getTile(pos.x, pos.y))
            {
                adtAreaId = tile->m_map.getArea(pos.x, pos.y);
                tileMapHeight = tile->m_map.getHeight(pos.x, pos.y);
            }
        }

        auto const* areaEntry = getAreaByPosition(adtAreaId, tileMapHeight, hasAreaInfo, mogpFlags, adtId, rootId, groupId, worldMap->getBaseMap()->getMapId(), pos.x, pos.y, pos.z);

        if (areaEntry == nullptr)
        {
            if (const auto linkedZoneId = worldMap->getBaseMap()->getMapEntry()->linkedZone)
                areaEntry = getAreaById(linkedZoneId);
        }

        return areaEntry;
    }

    WDB::Structures::AreaTableEntry const* AreaStorage::getAreaById(uint32_t areaId)
    {
        return m_areaContainer ? m_areaContainer->lookupEntry(areaId) : nullptr;
    }

    bool AreaStorage::IsOutdoor(uint32_t mapId, float x, float y, float z)
    {
        VMAP::VMapManager2* mgr = VMAP::VMapFactory::createOrGetVMapManager();

        uint32_t mogpFlags;
        int32_t adtId, rootId, groupId;

        if (!mgr->getAreaInfo(mapId, x, y, z, mogpFlags, adtId, rootId, groupId))
            return true;

        WDB::Structures::AreaTableEntry const* atEntry = nullptr;
        WDB::Structures::WMOAreaTableEntry const* wmoEntry = GetWMOAreaTableEntryByTriple(rootId, adtId, groupId);

        if (wmoEntry)
        {
            sLogger.debug("Got WMOAreaTableEntry! flag {}, areaid {}", wmoEntry->flags, wmoEntry->areaId);
            atEntry = getAreaById(wmoEntry->areaId);
        }

        return IsOutdoorWMO(mogpFlags, adtId, rootId, groupId, wmoEntry, atEntry);
    }

    bool AreaStorage::IsOutdoorWMO(uint32_t mogpFlags, int32_t /*adtId*/, int32_t /*rootId*/, int32_t /*groupId*/, WDB::Structures::WMOAreaTableEntry const* wmoEntry, WDB::Structures::AreaTableEntry const* atEntry)
    {
        bool outdoor = true;

        if (wmoEntry && atEntry)
        {
            if (atEntry->flags & AREA_FLAG_OUTSIDE)
                return true;
            if (atEntry->flags & AREA_FLAG_INSIDE)
                return false;
        }

        outdoor = (mogpFlags & 0x8) != 0;

        if (wmoEntry)
        {
            if (wmoEntry->flags & 4)
                return true;
            if (wmoEntry->flags & 2)
                outdoor = false;
        }
        return outdoor;
    }

    uint32_t AreaStorage::getIdByMapId(uint32_t mapId)
    {
        if (auto area = getAreaByMapId(mapId))
            return area->id;

        return 0;
    }

    WDB::Structures::AreaTableEntry const* AreaStorage::getAreaByPosition(uint32_t adtAreaId, float tileMapHeight, bool haveAreaInfo, uint32_t /*mogp_flags*/, int32_t adtId, int32_t rootId, int32_t groupId, uint32_t mapId, float /*x*/, float /*y*/, float z)
    {
        // floor is the height we are closer to (but only if above)
        //if (haveAreaInfo && G3D::fuzzyGe(z, z - GROUND_HEIGHT_TOLERANCE) && (G3D::fuzzyLt(z, tileMapHeight - GROUND_HEIGHT_TOLERANCE) || z > tileMapHeight))
        if (haveAreaInfo && (z + GROUND_HEIGHT_TOLERANCE >= tileMapHeight))
        {
            if (auto wmoTriple = GetWMOAreaTableEntryByTriple(rootId, adtId, groupId))
            {
                if (auto const* wmoArea = getAreaById(wmoTriple->areaId))
                    return wmoArea;
            }
        }

        if (adtAreaId != 0)
        {
            if (auto const* adtArea = getAreaById(adtAreaId))
                return adtArea;
        }

        return getAreaByMapId(mapId);
    }
} // MapManagement::AreaManagement
