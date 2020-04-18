/*
Copyright (c) 2014-2020 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/


#include "StdAfx.h"
#include "AreaStorage.hpp"
#include "VMapFactory.h"
#include "Management/ArenaTeam.h"
#include "AreaManagementGlobals.hpp"

namespace MapManagement::AreaManagement
{
    DBC::DBCStorage<DBC::Structures::AreaTableEntry>* AreaStorage::m_storage;
    MapEntryPair AreaStorage::m_map_storage;
    AreaFlagByAreaID AreaStorage::m_area_flag_by_id_collection;
    AreaFlagByMapID AreaStorage::m_area_flag_by_map_id_collection;

    MapEntryPair* AreaStorage::GetMapCollection()
    {
        return &AreaStorage::m_map_storage;
    }

    void AreaStorage::Initialise(DBC::DBCStorage<DBC::Structures::AreaTableEntry>* dbc_storage)
    {
        m_storage = dbc_storage;

        // Preload this stuff to make lookups easier elsewhere in code
        for (uint32_t i = 0; i < m_storage->GetNumRows(); ++i)
        {
            if (auto area = m_storage->LookupEntry(i))
            {
                m_area_flag_by_id_collection.insert(std::map<uint16_t, uint32_t>::value_type(uint16_t(area->id), area->explore_flag));
                if (area->zone == 0 && area->map_id != 0 && area->map_id != 1 && area->map_id != 530 && area->map_id != 571)
                {
                    m_area_flag_by_map_id_collection.insert(std::map<uint32_t, uint32_t>::value_type(area->map_id, area->explore_flag));
                }
            }
        }

    }

    DBC::DBCStorage<DBC::Structures::AreaTableEntry>* AreaStorage::GetStorage()
    {
        return m_storage;
    }

    int32_t AreaStorage::GetFlagById(uint32_t area_id)
    {
        auto iter = m_area_flag_by_id_collection.find(area_id);
        if (iter == m_area_flag_by_id_collection.end())
            return -1;

        return iter->second;
    }

    uint32_t AreaStorage::GetFlagByMapId(uint32_t map_id)
    {
        auto iter = m_area_flag_by_map_id_collection.find(map_id);
        if (iter == m_area_flag_by_map_id_collection.end())
            return 0;
        else
            return iter->second;
    }

    DBC::Structures::AreaTableEntry const* AreaStorage::GetAreaByFlag(uint32_t area_flag)
    {
        return m_storage->LookupEntry(area_flag);
    }

    DBC::Structures::AreaTableEntry const* AreaStorage::GetAreaByMapId(uint32_t map_id)
    {
        for (auto map_object : m_map_storage)
        {
            if (map_object.first == map_id)
            {
                return AreaStorage::GetAreaById(map_object.second);
            }
        }

        return nullptr;
    }

    DBC::Structures::AreaTableEntry const* AreaStorage::GetAreaById(uint32_t area_id)
    {
        int32_t area_flag = AreaStorage::GetFlagById(area_id);
        if (area_flag < 0)
            return NULL;

        return m_storage->LookupEntry(area_flag);
    }

    void AreaStorage::GetZoneAndIdByFlag(uint32_t& zone_id, uint32_t& area_id, uint16_t area_flag, uint32_t map_id)
    {
        auto area = AreaStorage::GetAreaByFlag(area_flag);
        if (!area)
        {
            area = AreaStorage::GetAreaByMapId(map_id);
        }

        area_id = area ? area->id : 0;
        zone_id = area ? ((area->zone != 0) ? area->zone : area->id) : 0;
    }

    bool AreaStorage::IsOutdoor(uint32_t mapId, float x, float y, float z)
    {
        VMAP::IVMapManager* mgr = VMAP::VMapFactory::createOrGetVMapManager();

        uint32_t mogpFlags;
        int32_t adtId, rootId, groupId;

        if (!mgr->getAreaInfo(mapId, x, y, z, mogpFlags, adtId, rootId, groupId))
            return true;

        DBC::Structures::AreaTableEntry const* atEntry = nullptr;
        DBC::Structures::WMOAreaTableEntry const* wmoEntry = GetWMOAreaTableEntryByTriple(rootId, adtId, groupId);

        if (wmoEntry)
        {
            LogDebugFlag(LF_MAP, "Got WMOAreaTableEntry! flag %u, areaid %u", wmoEntry->flags, wmoEntry->areaId);
            atEntry = sAreaStore.LookupEntry(wmoEntry->areaId);
        }

        return IsOutdoorWMO(mogpFlags, adtId, rootId, groupId, wmoEntry, atEntry);
    }

    bool AreaStorage::IsOutdoorWMO(uint32_t mogpFlags, int32_t /*adtId*/, int32_t /*rootId*/, int32_t /*groupId*/, DBC::Structures::WMOAreaTableEntry const* wmoEntry, DBC::Structures::AreaTableEntry const* atEntry)
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

    uint32_t AreaStorage::GetIdByFlag(uint32_t area_flag)
    {
        auto area = AreaStorage::GetAreaByFlag(area_flag);
        if (area)
            return area->id;
        else
            return 0;
    }

    uint32_t AreaStorage::GetIdByMapId(uint32_t map_id)
    {
        auto area = AreaStorage::GetAreaByMapId(map_id);
        if (area)
            return area->id;
        else
            return 0;
    }

    const uint32_t AreaStorage::GetFlagByPosition(uint32_t area_flag_without_adt_id, bool have_area_info, uint32_t /*mogp_flags*/, int32_t adt_id, int32_t root_id, int32_t group_id, uint32_t map_id, float /*x*/, float /*y*/, float /*z*/, bool* /*_out_is_outdoors*/)
    {
        ::DBC::Structures::AreaTableEntry const* at_entry = nullptr;
        if (have_area_info)
        {
            auto wmo_triple = GetWMOAreaTableEntryByTriple(root_id, adt_id, group_id);
            if (wmo_triple)
            {
                at_entry = AreaStorage::GetAreaById(wmo_triple->areaId);
            }
        }

        //uint16_t area_flag;

        if (at_entry)
        {
            return at_entry->explore_flag;
        }
        else
        {
            if (area_flag_without_adt_id)
            {
                return area_flag_without_adt_id;
            }
            
            return AreaStorage::GetFlagByMapId(map_id);
        }

        /*if (_out_is_outdoors)
        {
            if (have_area_info)
            {
                *_out_is_outdoors = IsOutdoorWMO(mogp_flags, adt_id, root_id, group_id, wmo_entry, at_entry);
            }
            else
            {
                *_out_is_outdoors = true;
            }
        }*/
        
        // Unused
        //return area_flag;
    }
} // MapManagement::AreaManagement
