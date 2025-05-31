/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "WDBRecord.hpp"

#include <memory>
#include <string>

namespace WDB
{
    enum DbcFieldFormat
    {
        FT_NA = 'x',                                              // not used or unknown, 4 byte size
        FT_NA_BYTE = 'X',                                         // not used or unknown, byte
        FT_STRING = 's',                                          // char*
        FT_FLOAT = 'f',                                           // float
        FT_INT = 'i',                                             // uint32_t
        FT_BYTE = 'b',                                            // uint8_t
        FT_SORT = 'd',                                            // sorted by this field, field is not included
        FT_IND = 'n',                                             // the same, but parsed to data
        FT_LOGIC = 'l',                                           // Logical (boolean)
        FT_SQL_PRESENT = 'p',                                     // Used in sql format to mark column present in sql dbc
        FT_SQL_ABSENT = 'a'                                       // Used in sql format to mark column absent in sql dbc
    };

    class WDBLoader
    {
    public:
        WDBLoader();
        ~WDBLoader();

        WDBLoader(WDBLoader const& right) = delete;
        WDBLoader& operator=(WDBLoader const& right) = delete;

        bool load(const char* _dbcFilename, const char* _dbcFormat);
        bool loadDb2(const char* _dbcFilename, const char* _dbcFormat);

        std::unique_ptr<char[]> autoProduceData(const char* _dbcFormat, uint32_t& _recordCount, std::unique_ptr<char*[]>& _indexTable);
        std::unique_ptr<char[]> autoProduceStrings(const char* _dbcFormat, char* _dataTable);
        static int getVersionIdForAEVersion();
        static bool hasFormat(std::string _dbcFile);
        static std::string getFormat(std::string _dbcFile);
        static uint32_t getFormatRecordSize(const char* _dbcFormat, int32_t* _indexPos = NULL);

        WDB::WDBRecord getRecord(size_t record_id) const;
        uint32_t getNumRows() const;
        uint32_t getRowSize() const;
        uint32_t getNumColumns() const;
        uint32_t getOffset(size_t _id) const;

    protected:
        uint32_t m_record_size;
        uint32_t m_record_count;
        uint32_t m_field_count;
        uint32_t m_string_size;
        std::unique_ptr<uint32_t[]> m_fields_offset;
        std::unique_ptr<unsigned char[]> m_data;
        unsigned char* m_string_table;

        // db2 fields
        uint32_t m_tableHash{};
        uint32_t m_build{};

        int m_unk1{};
        int m_unk2{};
        int m_unk3{};
        int m_locale{};
        int m_unk5{};
    };
}
