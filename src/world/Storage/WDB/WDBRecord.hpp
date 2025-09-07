/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <cstdint>
#include <cstddef>

namespace WDB
{
    class WDBRecord
    {
    protected:
        unsigned char* m_offset;

    public:
        WDBRecord(unsigned char* offset);

        float getFloat(size_t field, uint32_t field_count, uint32_t field_offset) const;
        uint32_t getUInt32(size_t field, uint32_t field_count, uint32_t field_offset) const;
        uint8_t getUInt8(size_t field, uint32_t field_count, uint32_t field_offset) const;
        char* getString(size_t field, uint32_t field_count, uint32_t field_offset, uint32_t string_size, unsigned char* string_table) const;
    };
}
