/*
Copyright (c) 2014-2021 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "DBCRecord.hpp"

namespace DBC
{
    DBCRecord::DBCRecord(unsigned char* offset) : m_offset(offset)
    {
        /* No Body */
    }

    float DBCRecord::GetFloat(size_t field, uint32_t field_count, const uint32_t field_offset) const
    {
        assert(field < field_count);
        float val = *reinterpret_cast<float*>(m_offset + field_offset);
        return val;
    }

    uint32_t DBCRecord::GetUInt32(size_t field, uint32_t field_count, const uint32_t field_offset) const
    {
        assert(field < field_count);
        uint32_t val = *reinterpret_cast<uint32_t*>(m_offset + field_offset);
        return val;
    }

    uint8 DBCRecord::GetUInt8(size_t field, uint32_t field_count, const uint32_t field_offset) const
    {
        assert(field < field_count);
        return *reinterpret_cast<uint8*>(m_offset + field_offset);
    }

    char* DBCRecord::GetString(size_t field, uint32_t field_count, const uint32_t field_offset, uint32_t string_size, unsigned char* string_table) const
    {
        assert(field < field_count);
        const size_t string_offset = GetUInt32(field, field_count, field_offset);
        assert(string_offset < string_size);
        return reinterpret_cast<char*>(string_table + string_offset);
    }
}