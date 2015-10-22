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

#ifndef _DBC_LOADER_H
#define _DBC_LOADER_H

#include "DBCRecord.hpp"

namespace DBC
{
    enum DbcFieldFormat
    {
        FT_NA = 'x',                                              //not used or unknown, 4 byte size
        FT_NA_BYTE = 'X',                                         //not used or unknown, byte
        FT_STRING = 's',                                          //char*
        FT_FLOAT = 'f',                                           //float
        FT_INT = 'i',                                             //uint32
        FT_BYTE = 'b',                                            //uint8
        FT_SORT = 'd',                                            //sorted by this field, field is not included
        FT_IND = 'n',                                             //the same, but parsed to data
        FT_LOGIC = 'l',                                           //Logical (boolean)
        FT_SQL_PRESENT = 'p',                                     //Used in sql format to mark column present in sql dbc
        FT_SQL_ABSENT = 'a'                                       //Used in sql format to mark column absent in sql dbc
    };

    class DBCLoader
    {
    protected:
        uint32 m_record_size;
        uint32 m_record_count;
        uint32 m_field_count;
        uint32 m_string_size;
        uint32* m_fields_offset;
        unsigned char* m_data;
        unsigned char* m_string_table;

    public:
        DBC::DBCRecord GetRecord(size_t record_id);
        const uint32 GetNumRows();
        const uint32 GetRowSize();
        const uint32 GetNumColumns();
        const uint32 GetOffset(size_t id);

        const bool IsLoaded();

        char* AutoProduceData(const char* dbc_format, uint32& record_count, char**& index_table, uint32 sql_record_count, uint32 sql_highest_index, char *& sql_data_table);
        char* AutoProduceStrings(const char* dbc_format, char* data_table);
        static uint32 GetFormatRecordSize(const char* dbc_format, int32* index_pos = NULL);
        bool Load(const char* dbc_filename, const char* dbc_format);

        DBCLoader();
        ~DBCLoader();

        DBCLoader(DBCLoader const& right) = delete;
        DBCLoader& operator=(DBCLoader const& right) = delete;
    };
}

#endif // _DBC_LOADER_H