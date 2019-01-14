/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "Common.hpp"
#include "Storage/DBC/DBCStorage.hpp"
#include "Storage/DBC/DBCStructures.hpp"

namespace MapManagement
{
    namespace AreaManagement
    {
        typedef std::map<uint32, uint32> AreaFlagByAreaID;
        typedef std::map<uint32, uint32> AreaFlagByMapID;

        // Temporary 
        typedef std::map<uint32, uint32> MapEntryPair;

        class AreaStorage
        {
            protected:

                static DBC::DBCStorage<DBC::Structures::AreaTableEntry>* m_storage;
                static MapEntryPair m_map_storage;
                static AreaFlagByAreaID m_area_flag_by_id_collection;
                static AreaFlagByMapID m_area_flag_by_map_id_collection;

            public:

                static void Initialise(DBC::DBCStorage<DBC::Structures::AreaTableEntry>* dbc_storage);
                static MapEntryPair* GetMapCollection();

                static DBC::DBCStorage<DBC::Structures::AreaTableEntry>* GetStorage();

                /* Get Area */
                //static DBC::Structures::AreaTableEntry const* GetAreaByPosition(uint32 map_id, float x, float y, float z);
                //static DBC::Structures::AreaTableEntry const* GetAreaByFlagAndMapId(uint32 area_flag, uint32 map_id);
                static DBC::Structures::AreaTableEntry const* GetAreaById(uint32 area_id);
                static DBC::Structures::AreaTableEntry const* GetAreaByFlag(uint32 area_flag);
                static DBC::Structures::AreaTableEntry const* GetAreaByMapId(uint32 map_id);

                /* Get Flag */
                static int32 GetFlagById(uint32 area_id);
                static uint32 GetFlagByMapId(uint32 map_id);

                /* Get ID */
                static uint32 GetIdByFlag(uint32 area_flag);
                static uint32 GetIdByMapId(uint32 map_id);

                /* Get ID & Zone */
                static void GetZoneAndIdByFlag(uint32& zone_id, uint32& area_id, uint16 area_flag, uint32 map_id);

                /* Misc */
                static bool IsOutdoor(uint32 mapId, float x, float y, float z);
                static bool IsOutdoorWMO(uint32 mogpFlags, int32 /*adtId*/, int32 /*rootId*/, int32 /*groupId*/, DBC::Structures::WMOAreaTableEntry const* wmoEntry, DBC::Structures::AreaTableEntry const* atEntry);
                
                static const uint32 GetFlagByPosition(uint32 area_flag_without_adt_id, bool have_area_info, uint32 mogp_flags, int32 adt_id, int32 root_id, int32 group_id, uint32 map_id, float x, float y, float z, bool* _out_is_outdoors = nullptr);
        };
    } // </ AreaManagementNamespace>
} // </ MapManagementNamespace>
