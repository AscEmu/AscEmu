/*
Copyright (c) 2014-2016 AscEmu Team <http://www.ascemu.org/>
This file is released under the MIT license. See README-MIT for more information.
*/

#ifndef _DB2_LOADER_HPP
#define _DB2_LOADER_HPP

#include "../shared/ByteConverter.hpp"
#include "Common.h"
#include <cassert>

namespace DB2
{
    enum
    {
        FT_NA = 'x',                // ignore/ default, 4 byte size, in Source String means field is ignored, in Dest String means field is filled with default value
        FT_NA_BYTE = 'X',           // ignore/ default, 1 byte size, see above
        FT_NA_FLOAT = 'F',          // ignore/ default,  float size, see above
        FT_NA_POINTER = 'p',        // fill default value into dest, pointer size, Use this only with static data (otherwise mem-leak)
        FT_STRING = 's',            // char*
        FT_FLOAT = 'f',             // float
        FT_INT = 'i',               // uint32
        FT_BYTE = 'b',              // uint8
        FT_SORT = 'd',              // sorted by this field, field is not included
        FT_IND = 'n',               // the same,but parsed to data
        FT_LOGIC = 'l'              // Logical (boolean)
    };

    class DB2FileLoader
    {
        public:

            DB2FileLoader();
            ~DB2FileLoader();

            bool Load(const char *filename, const char *fmt);

            class Record
            {
                public:

                    float getFloat(size_t field) const
                    {
                        assert(field < file.fieldCount);
                        float val = *reinterpret_cast<float*>(offset + file.GetOffset(field));
                        EndianConvert(val);
                        return val;
                    }
                    uint32 getUInt(size_t field) const
                    {
                        assert(field < file.fieldCount);
                        uint32 val = *reinterpret_cast<uint32*>(offset + file.GetOffset(field));
                        EndianConvert(val);
                        return val;
                    }
                    uint8 getUInt8(size_t field) const
                    {
                        assert(field < file.fieldCount);
                        return *reinterpret_cast<uint8*>(offset + file.GetOffset(field));
                    }

                    const char *getString(size_t field) const
                    {
                        assert(field < file.fieldCount);
                        size_t stringOffset = getUInt(field);
                        assert(stringOffset < file.stringSize);
                        return reinterpret_cast<char*>(file.stringTable + stringOffset);
                    }

                private:

                    Record(DB2FileLoader &file_, unsigned char *offset_) : offset(offset_), file(file_) {}
                    unsigned char *offset;
                    DB2FileLoader &file;

                    friend class DB2FileLoader;

            };

            // Get record by id
            Record getRecord(size_t id);
            /// Get begin iterator over records

            uint32 GetNumRows() const { return recordCount; }
            uint32 GetCols() const { return fieldCount; }
            uint32 GetOffset(size_t id) const { return (fieldsOffset != NULL && id < fieldCount) ? fieldsOffset[id] : 0; }
            bool IsLoaded() const { return (data != NULL); }
            char* AutoProduceData(const char* fmt, uint32& count, char**& indexTable);
            char* AutoProduceStringsArrayHolders(const char* fmt, char* dataTable);
            char* AutoProduceStrings(const char* fmt, char* dataTable, DBC::LocaleConstant loc);
            static uint32 GetFormatRecordSize(const char * format, int32 * index_pos = NULL);
            static uint32 GetFormatStringsFields(const char * format);

        private:

            uint32 recordSize;
            uint32 recordCount;
            uint32 fieldCount;
            uint32 stringSize;
            uint32 *fieldsOffset;
            unsigned char *data;
            unsigned char *stringTable;

            // WDB2 / WCH2 fields
            uint32 tableHash;    // WDB2
            uint32 build;        // WDB2

            int unk1;            // WDB2 (Unix time in WCH2)
            int unk2;            // WDB2
            int unk3;            // WDB2 (index table)
            int locale;          // WDB2
            int unk5;            // WDB2
    };
}

#endif  //_DB2_LOADER_HPP
