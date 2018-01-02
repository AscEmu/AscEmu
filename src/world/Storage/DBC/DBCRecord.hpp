/*
Copyright (c) 2014-2018 AscEmu Team <http://www.ascemu.org>
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

            const float GetFloat(size_t field, uint32 field_count, const uint32 field_offset);
            const uint32 GetUInt32(size_t field, uint32 field_count, const uint32 field_offset);
            const uint8 GetUInt8(size_t field, uint32 field_count, const uint32 field_offset);
            const char* GetString(size_t field, uint32 field_count, const uint32 field_offset, uint32 string_size, unsigned char* string_table);
    };
}
