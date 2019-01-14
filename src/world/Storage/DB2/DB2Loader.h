/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "../shared/ByteConverter.h"
#include "Common.hpp"
#include "../DBC/DBCGlobals.hpp"
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
                        convertEndian(val);
                        return val;
                    }
                    uint32 getUInt(size_t field) const
                    {
                        assert(field < file.fieldCount);
                        uint32 val = *reinterpret_cast<uint32*>(offset + file.GetOffset(field));
                        convertEndian(val);
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

            Record getRecord(size_t id);
            uint32_t GetNumRows() const { return recordCount; }
            uint32_t GetCols() const { return fieldCount; }
            uint32_t GetOffset(size_t id) const { return (fieldsOffset != NULL && id < fieldCount) ? fieldsOffset[id] : 0; }
            bool IsLoaded() const { return (data != NULL); }
            char* AutoProduceData(const char* fmt, uint32_t& count, char**& indexTable);
            char* AutoProduceStringsArrayHolders(const char* fmt, char* dataTable);
            char* AutoProduceStrings(const char* fmt, char* dataTable, DBC::LocaleConstant loc);
            static uint32_t GetFormatRecordSize(const char * format, int32_t* index_pos = NULL);
            static uint32_t GetFormatStringsFields(const char * format);

        private:

            uint32_t recordSize;
            uint32_t recordCount;
            uint32_t fieldCount;
            uint32_t stringSize;
            uint32_t *fieldsOffset;
            unsigned char *data;
            unsigned char *stringTable;

            uint32_t tableHash;
            uint32_t build;

            int unk1;
            int unk2;
            int unk3;
            int locale;
            int unk5;
    };
}
