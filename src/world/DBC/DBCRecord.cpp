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

#include "DBCRecord.hpp"

namespace DBC
{
    DBCRecord::DBCRecord(unsigned char* offset) : m_offset(offset)
    {
        /* No Body */
    }

    const float DBCRecord::GetFloat(size_t field, uint32 field_count, const uint32 field_offset)
    {
        assert(field < field_count);
        float val = *reinterpret_cast<float*>(m_offset + field_offset);
        return val;
    }

    const uint32 DBCRecord::GetUInt32(size_t field, uint32 field_count, const uint32 field_offset)
    {
        assert(field < field_count);
        uint32 val = *reinterpret_cast<uint32*>(m_offset + field_offset);
        return val;
    }

    const uint8 DBCRecord::GetUInt8(size_t field, uint32 field_count, const uint32 field_offset)
    {
        assert(field < field_count);
        return *reinterpret_cast<uint8*>(m_offset + field_offset);
    }

    const char* DBCRecord::GetString(size_t field, uint32 field_count, const uint32 field_offset, uint32 string_size, unsigned char* string_table)
    {
        assert(field < field_count);
        size_t string_offset = GetUInt32(field, field_count, field_offset);
        assert(string_offset < string_size);
        return reinterpret_cast<char*>(string_table + string_offset);
    }
}