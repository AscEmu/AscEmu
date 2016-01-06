/**
 * AscEmu Framework based on ArcEmu MMORPG Server
 * Copyright (C) 2014-2016 AscEmu Team <http://www.ascemu.org>
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

#ifndef _MAP_MANAGEMENT_AREA_MANAGEMENT_AREA_STORAGE_H
#define _MAP_MANAGEMENT_AREA_MANAGEMENT_AREA_STORAGE_H

#include "Common.h"
#include "DBC/DBCStorage.hpp"
#include "DBC/DBCStructures.hpp"

namespace MapManagement
{
    namespace AreaManagement
    {
        typedef std::map<uint16, uint32> AreaFlagByAreaID;
        typedef std::map<uint32, uint32> AreaFlagByMapID;

        // Temporary 
        typedef std::map<uint32, uint32> MapEntryPair;
        struct WMOTriple
        {
            int32 group_id;
            int32 root_id;
            int32 adt_id;
            uint32 area_id;
        };

        class AreaStorage
        {
            protected:

                static DBC::DBCStorage<DBC::Structures::AreaTableEntry>* m_storage;
                static MapEntryPair m_map_storage;
                static std::vector<WMOTriple*> m_wmo_triple_collection;
                static AreaFlagByAreaID m_area_flag_by_id_collection;
                static AreaFlagByMapID m_area_flag_by_map_id_collection;

            public:

                static void Initialise(DBC::DBCStorage<DBC::Structures::AreaTableEntry>* dbc_storage);
                static MapEntryPair* GetMapCollection();

                static DBC::DBCStorage<DBC::Structures::AreaTableEntry>* GetStorage();
                static void AddWMOTripleEntry(int32 group_id, int32 root_id, int32 adt_id, uint32 area_id);
                static WMOTriple* GetWMOTriple(int32 group_id, int32 root_id, int32 adt_id);

                /* Get Area */
                //static DBC::Structures::AreaTableEntry const* GetAreaByPosition(uint32 map_id, float x, float y, float z);
                //static DBC::Structures::AreaTableEntry const* GetAreaByFlagAndMapId(uint32 area_flag, uint32 map_id);
                static DBC::Structures::AreaTableEntry const* GetAreaById(uint32 area_id);
                static DBC::Structures::AreaTableEntry const* GetAreaByFlag(uint32 area_flag);
                static DBC::Structures::AreaTableEntry const* GetAreaByMapId(uint32 map_id);
                //static DBC::Structures::AreaTableEntry const* GetAreaById(uint32 area_id);

                /* Get Flag */
                static int32 GetFlagById(uint32 area_id);
                static uint32 GetFlagByMapId(uint32 map_id);

                /* Get ID */
                static uint32 GetIdByFlag(uint32 area_flag);
                static uint32 GetIdByMapId(uint32 map_id);

                /* Get ID & Zone */
                static void GetZoneAndIdByFlag(uint32& zone_id, uint32& area_id, uint16 area_flag, uint32 map_id);

                /* Misc */
                /* This was removed due to compile issues on Linux - will be redone when code is better structured */
                /*static bool IsOutdoorWMO(uint32 mogp_flags, int32 adt_id, int32 root_id, int32 group_id, WMOAreaTableEntry const* wmo_entry, ::DBC::Structures::AreaTableEntry const* at_entry);*/
                static const uint16 GetFlagByPosition(uint16 area_flag_without_adt_id, bool have_area_info, uint32 mogp_flags, int32 adt_id, int32 root_id, int32 group_id, uint32 map_id, float x, float y, float z, bool* _out_is_outdoors = nullptr);
        };
    } // </ AreaManagementNamespace>
} // </ MapManagementNamespace>

#endif // _MAP_MANAGEMENT_AREA_MANAGEMENT_AREA_STORAGE_H
