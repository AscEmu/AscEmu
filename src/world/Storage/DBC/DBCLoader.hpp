/*
Copyright (c) 2014-2023 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <string>

#include "DBCRecord.hpp"

namespace DBC
{
    enum DbcFieldFormat
    {
        FT_NA = 'x',                                              // not used or unknown, 4 byte size
        FT_NA_BYTE = 'X',                                         // not used or unknown, byte
        FT_STRING = 's',                                          // char*
        FT_FLOAT = 'f',                                           // float
        FT_INT = 'i',                                             // uint32
        FT_BYTE = 'b',                                            // uint8
        FT_SORT = 'd',                                            // sorted by this field, field is not included
        FT_IND = 'n',                                             // the same, but parsed to data
        FT_LOGIC = 'l',                                           // Logical (boolean)
        FT_SQL_PRESENT = 'p',                                     // Used in sql format to mark column present in sql dbc
        FT_SQL_ABSENT = 'a'                                       // Used in sql format to mark column absent in sql dbc
    };

    class DBCLoader
    {
    protected:
        uint32_t m_record_size;
        uint32_t m_record_count;
        uint32_t m_field_count;
        uint32_t m_string_size;
        uint32_t* m_fields_offset;
        unsigned char* m_data;
        unsigned char* m_string_table;

        //db2 fields
        uint32_t m_tableHash;
        uint32_t m_build;

        int m_unk1;
        int m_unk2;
        int m_unk3;
        int m_locale;
        int m_unk5;

    public:
        DBC::DBCRecord GetRecord(size_t record_id) const;
        uint32_t GetNumRows() const;
        uint32_t GetRowSize() const;
        uint32_t GetNumColumns() const;
        uint32_t GetOffset(size_t id) const;

        bool IsLoaded();

        char* AutoProduceData(const char* dbc_format, uint32_t& record_count, char**& index_table);
        char* AutoProduceStrings(const char* dbc_format, char* data_table);
        static int getVersionIdForAEVersion();
        static bool hasFormat(std::string _dbcFile);
        static std::string GetFormat(std::string _dbcFile);
        static uint32_t GetFormatRecordSize(const char* dbc_format, int32_t* index_pos = NULL);
        bool Load(const char* dbc_filename, const char* dbc_format);
        bool LoadDB2(const char* dbc_filename, const char* dbc_format);

        DBCLoader();
        ~DBCLoader();

        DBCLoader(DBCLoader const& right) = delete;
        DBCLoader& operator=(DBCLoader const& right) = delete;
    };
}
