/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "Storage/WDB/WDBContainer.hpp"

#include <map>

class LocationVector;
class WorldMap;

namespace WDB::Structures
{
    struct AreaTableEntry;
    struct WMOAreaTableEntry;
}

namespace MapManagement::AreaManagement
{
    typedef std::map<uint32_t, uint32_t> AreaFlagByAreaID;
    typedef std::map<uint32_t, uint32_t> AreaFlagByMapID;

    // Temporary 
    typedef std::map<uint32_t, uint32_t> MapEntryPair;

    class AreaStorage
    {
    protected:
        static WDB::WDBContainer<WDB::Structures::AreaTableEntry>* m_storage;
        static MapEntryPair m_map_storage;
        static AreaFlagByAreaID m_area_flag_by_id_collection;
        static AreaFlagByMapID m_area_flag_by_map_id_collection;

    public:
        static void Initialise(WDB::WDBContainer<WDB::Structures::AreaTableEntry>* dbc_storage);
        static MapEntryPair* GetMapCollection();

        static WDB::WDBContainer<WDB::Structures::AreaTableEntry>* GetStorage();

        /* Get Area */
        //static WDB::Structures::AreaTableEntry const* GetAreaByPosition(uint32_t map_id, float x, float y, float z);
        //static WDB::Structures::AreaTableEntry const* GetAreaByFlagAndMapId(uint32_t area_flag, uint32_t map_id);
        static WDB::Structures::AreaTableEntry const* GetAreaById(uint32_t area_id);
        static WDB::Structures::AreaTableEntry const* GetAreaByFlag(uint32_t area_flag);
        static WDB::Structures::AreaTableEntry const* GetAreaByMapId(uint32_t map_id);
        static WDB::Structures::AreaTableEntry const* getExactArea(WorldMap* worldMap, LocationVector pos, uint32_t phaseMask);

        /* Get Flag */
        static int32_t GetFlagById(uint32_t area_id);
        static uint32_t GetFlagByMapId(uint32_t map_id);

        /* Get ID */
        static uint32_t GetIdByFlag(uint32_t area_flag);
        static uint32_t GetIdByMapId(uint32_t map_id);

        /* Get ID & Zone */
        static void GetZoneAndIdByFlag(uint32_t& zone_id, uint32_t& area_id, uint16_t area_flag, uint32_t map_id);

        /* Misc */
        static bool IsOutdoor(uint32_t mapId, float x, float y, float z);
        static bool IsOutdoorWMO(uint32_t mogpFlags, int32_t /*adtId*/, int32_t /*rootId*/, int32_t /*groupId*/, WDB::Structures::WMOAreaTableEntry const* wmoEntry, WDB::Structures::AreaTableEntry const* atEntry);
        
        static const uint32_t GetFlagByPosition(uint32_t area_flag_without_adt_id, uint32_t tileMapHeight, bool have_area_info, uint32_t mogp_flags, int32_t adt_id, int32_t root_id, int32_t group_id, uint32_t map_id, float x, float y, float z, bool* _out_is_outdoors = nullptr);
    };
} // </ MapManagement::AreaManagement>
