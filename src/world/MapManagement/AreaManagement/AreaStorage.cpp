/*
 * AscEmu Framework based on ArcEmu MMORPG Server
 * Copyright (C) 2008-2011 <http://www.ArcEmu.org/>
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

#include "AreaStorage.hpp"

namespace MapManagement
{
    namespace AreaManagement
    {
        DBC::DBCStorage<DBC::Structures::AreaTableEntry>* AreaStorage::m_storage;
        MapEntryPair AreaStorage::m_map_storage;
        std::vector<WMOTriple*> AreaStorage::m_wmo_triple_collection;
        AreaFlagByAreaID AreaStorage::m_area_flag_by_id_collection;
        AreaFlagByMapID AreaStorage::m_area_flag_by_map_id_collection;

        MapEntryPair* AreaStorage::GetMapCollection()
        {
            return &AreaStorage::m_map_storage;
        }

        void AreaStorage::Initialise(DBC::DBCStorage<DBC::Structures::AreaTableEntry>* dbc_storage)
        {
            m_storage = dbc_storage;

            /* Preload this stuff to make lookups easier elsewhere in code */
            for (auto i = 0; i < m_storage->GetNumRows(); ++i)
            {
                if (auto area = m_storage->LookupEntry(i))
                {
                    m_area_flag_by_id_collection.insert(std::map<uint16, uint32>::value_type(uint16(area->id), area->explore_flag));
                    if (area->zone == 0 && area->map_id != 0 && area->map_id != 1 && area->map_id != 530 && area->map_id != 571)
                    {
                        m_area_flag_by_map_id_collection.insert(std::map<uint32, uint32>::value_type(area->map_id, area->explore_flag));
                    }
                }
            }

        }

        void AreaStorage::AddWMOTripleEntry(int32 group_id, int32 root_id, int32 adt_id, uint32 area_id)
        {
            auto triple = new WMOTriple();
            triple->group_id = group_id;
            triple->root_id = root_id;
            triple->adt_id = adt_id;
            triple->area_id = area_id;
            m_wmo_triple_collection.push_back(triple);
        }

        DBC::DBCStorage<DBC::Structures::AreaTableEntry>* AreaStorage::GetStorage()
        {
            return m_storage;
        }

        int32 AreaStorage::GetFlagById(uint32 area_id)
        {
            auto iter = m_area_flag_by_id_collection.find(area_id);
            if (iter == m_area_flag_by_id_collection.end())
                return -1;

            return iter->second;
        }

        uint32 AreaStorage::GetFlagByMapId(uint32 map_id)
        {
            auto iter = m_area_flag_by_map_id_collection.find(map_id);
            if (iter == m_area_flag_by_map_id_collection.end())
                return 0;
            else
                return iter->second;
        }

        DBC::Structures::AreaTableEntry const* AreaStorage::GetAreaByFlag(uint32 area_flag)
        {
            return m_storage->LookupEntry(area_flag);
        }

        DBC::Structures::AreaTableEntry const* AreaStorage::GetAreaByMapId(uint32 map_id)
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

        ::DBC::Structures::AreaTableEntry const* AreaStorage::GetAreaById(uint32 area_id)
        {
            int32 area_flag = AreaStorage::GetFlagById(area_id);
            if (area_flag < 0)
                return NULL;

            return m_storage->LookupEntry(area_flag);
        }
        
        const void AreaStorage::GetZoneAndIdByPosition(TerrainHolder* terrain, uint32 map_id, float x, float y, float z, uint32& _out_zone_id, uint32& _out_area_id)
        {
            auto area_flag = AreaStorage::GetFlagByPosition(terrain, map_id, x, y, z, nullptr);
            AreaStorage::GetZoneAndIdByFlag(_out_zone_id, _out_area_id, area_flag, map_id);
        }

        void AreaStorage::GetZoneAndIdByFlag(uint32& zone_id, uint32& area_id, uint16 area_flag, uint32 map_id)
        {
            auto area = AreaStorage::GetAreaByFlag(area_flag);
            if (!area)
            {
                area = AreaStorage::GetAreaByMapId(map_id);
            }

            area_id = area ? area->id : 0;
            zone_id = area ? ((area->zone != 0) ? area->zone : area->id) : 0;
        }
        /*
        bool AreaStorage::IsOutdoorWMO(uint32 mogp_flags, int32 adt_id, int32 root_id, int32 group_id, WMOAreaTableEntry const* wmo_entry, ::DBC::Structures::AreaTableEntry const* at_entry)
        {
            bool outdoor = true;

            if (wmo_entry && at_entry)
            {
                if (at_entry->flags & 0x04000000) // AREA_FLAG_OUTSIDE
                {
                    return true;
                }
                if (at_entry->flags & 0x02000000) // AREA_FLAG_INSIDE
                {
                    return false;
                }
            }

            outdoor = (mogp_flags & 0x8) != 0;

            if (wmo_entry)
            {
                if (wmo_entry->flags & 0x4)
                {
                    return true;
                }
                if (wmo_entry->flags & 0x2)
                {
                    outdoor = false;
                }
            }

            return outdoor;
        }*/

        uint32 AreaStorage::GetIdByFlag(uint32 area_flag)
        {
            auto area = AreaStorage::GetAreaByFlag(area_flag);
            if (area)
                return area->id;
            else
                return 0;
        }

        uint32 AreaStorage::GetIdByMapId(uint32 map_id)
        {
            auto area = AreaStorage::GetAreaByMapId(map_id);
            if (area)
                return area->id;
            else
                return 0;
        }

        uint32 AreaStorage::GetIdByPosition(TerrainHolder* terrain, uint32 map_id, float x, float y, float z)
        {
            auto area_flag = AreaStorage::GetFlagByPosition(terrain, map_id, x, y, z, nullptr);
            auto area = AreaStorage::GetAreaByFlag(area_flag);
            if (!area)
            {
                area = AreaStorage::GetAreaByMapId(map_id);
            }
            return area->id;
        }

        WMOTriple* AreaStorage::GetWMOTriple(int32 group_id, int32 root_id, int32 adt_id)
        {
            for (auto triple : m_wmo_triple_collection)
            {
                if (triple->group_id == group_id)
                {
                    if (triple->root_id == root_id)
                    {
                        if (triple->adt_id == adt_id)
                        {
                            return triple;
                        }
                    }
                }
            }

            return nullptr;
        }

        const uint16 AreaStorage::GetFlagByPosition(TerrainHolder* terrain, uint32 map_id, float x, float y, float z, bool* _out_is_outdoors)
        {
            uint32 mogp_flags;
            int32 adt_id, root_id, group_id;
            ::DBC::Structures::AreaTableEntry const* at_entry = nullptr;
            bool have_area_info = false;

            if (terrain->GetAreaInfo(x, y, z, mogp_flags, adt_id, root_id, group_id))
            {
                have_area_info = true;
                auto wmo_triple = AreaStorage::GetWMOTriple(root_id, adt_id, group_id);
                if (wmo_triple)
                {
                    at_entry = AreaStorage::GetAreaById(wmo_triple->area_id);
                }
            }

            uint16 area_flag;

            if (at_entry)
            {
                area_flag = at_entry->explore_flag;
            }
            else
            {
                auto tile = terrain->GetTile(x, y);
                if (tile)
                {
                    area_flag = tile->m_map.GetArea(x, y);
                }

                if (!tile || area_flag == 0)
                {
                    area_flag = AreaStorage::GetFlagByMapId(map_id);
                }
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
            return area_flag;
        }
    } // </ AreaManagementNamespace>
} // </ MapManagementNamespace>