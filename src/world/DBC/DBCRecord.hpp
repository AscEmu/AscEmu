/**
 * AscEmu Framework based on ArcEmu MMORPG Server
 * Copyright (C) 2014-2015 AscEmu Team <http://www.ascemu.org>
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
 */

#ifndef _DBC_RECORD_H
#define _DBC_RECORD_H

#include "Common.h"

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

#endif // _DBC_RECORD_H