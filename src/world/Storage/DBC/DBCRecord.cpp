/*
Copyright (c) 2014-2018 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
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