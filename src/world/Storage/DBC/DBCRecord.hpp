/*
Copyright (c) 2014-2021 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "Common.hpp"

namespace DBC
{
    class DBCRecord
    {
    protected:
        unsigned char* m_offset;

    public:
        DBCRecord(unsigned char* offset);

        float GetFloat(size_t field, uint32_t field_count, uint32_t field_offset) const;
        uint32_t GetUInt32(size_t field, uint32_t field_count, uint32_t field_offset) const;
        uint8_t GetUInt8(size_t field, uint32_t field_count, uint32_t field_offset) const;
        char* GetString(size_t field, uint32_t field_count, uint32_t field_offset, uint32_t string_size, unsigned char* string_table) const;
    };
}
