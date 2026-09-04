/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "Storage/WDB/WDBStores.hpp"

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
    typedef std::map<uint32_t, uint32_t> AreaIdByMapId;

    // Temporary 
    typedef std::map<uint32_t, uint32_t> MapEntryPair;

    class AreaStorage
    {
    protected:
        inline static WDB::WDBStore<WDB::Structures::AreaTableEntry> const* m_areaContainer;
        inline static MapEntryPair m_map_storage;
        inline static AreaIdByMapId m_areaIdByMapIdCollection;

    public:
        static void initialise(WDB::WDBStore<WDB::Structures::AreaTableEntry> const* container);
        static MapEntryPair* getMapCollection();

        static WDB::WDBStore<WDB::Structures::AreaTableEntry> const* getStorage();

        /* Get Area */
        //static WDB::Structures::AreaTableEntry const* GetAreaByPosition(uint32_t map_id, float x, float y, float z);
        //static WDB::Structures::AreaTableEntry const* GetAreaByFlagAndMapId(uint32_t area_flag, uint32_t map_id);
        static WDB::Structures::AreaTableEntry const* getAreaById(uint32_t areaId);
        static WDB::Structures::AreaTableEntry const* getAreaByMapId(uint32_t mapId);
        static WDB::Structures::AreaTableEntry const* getExactArea(WorldMap* worldMap, LocationVector const& pos, uint32_t phaseMask);

        /* Get ID */
        static uint32_t getIdByMapId(uint32_t mapId);

        /* Misc */
        static bool IsOutdoor(uint32_t mapId, float x, float y, float z);
        static bool IsOutdoorWMO(uint32_t mogpFlags, int32_t /*adtId*/, int32_t /*rootId*/, int32_t /*groupId*/, WDB::Structures::WMOAreaTableEntry const* wmoEntry, WDB::Structures::AreaTableEntry const* atEntry);

        static WDB::Structures::AreaTableEntry const* AreaStorage::getAreaByPosition(uint32_t adtAreaId, float tileMapHeight, bool haveAreaInfo, uint32_t mogpFlags, int32_t adtId, int32_t rootId, int32_t groupId, uint32_t mapId, float x, float y, float z);
    };
} // </ MapManagement::AreaManagement>
