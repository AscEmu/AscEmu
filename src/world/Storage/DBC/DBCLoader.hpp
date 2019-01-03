/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

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

            DBC::DBCRecord GetRecord(size_t record_id) const;
            uint32 GetNumRows() const;
            uint32 GetRowSize() const;
            uint32 GetNumColumns() const;
            uint32 GetOffset(size_t id) const;

            bool IsLoaded();

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
