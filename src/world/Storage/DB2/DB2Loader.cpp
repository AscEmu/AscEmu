/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "DB2Loader.h"
#include "../DBC/DBCGlobals.hpp"

DB2::DB2FileLoader::DB2FileLoader()
{
    data = NULL;
    fieldsOffset = NULL;
}

bool DB2::DB2FileLoader::Load(const char *filename, const char *fmt)
{
    uint32_t header = 48;
    if (data)
    {
        delete[] data;
        data = NULL;
    }

    FILE* f = fopen(filename, "rb");
    if (!f)
        return false;

    if (fread(&header, 4, 1, f) != 1)       // Signature
    {
        fclose(f);
        return false;
    }

    convertEndian(header);

    if (header != 0x32424457)
    {
        fclose(f);
        return false;                       //'WDB2'
    }

    if (fread(&recordCount, 4, 1, f) != 1)  // Number of records
    {
        fclose(f);
        return false;
    }

    convertEndian(recordCount);

    if (fread(&fieldCount, 4, 1, f) != 1)   // Number of fields
    {
        fclose(f);
        return false;
    }

    convertEndian(fieldCount);

    if (fread(&recordSize, 4, 1, f) != 1)   // Size of a record
    {
        fclose(f);
        return false;
    }

    convertEndian(recordSize);

    if (fread(&stringSize, 4, 1, f) != 1)   // String size
    {
        fclose(f);
        return false;
    }

    convertEndian(stringSize);

    if (fread(&tableHash, 4, 1, f) != 1)    // Table hash
    {
        fclose(f);
        return false;
    }

    convertEndian(tableHash);

    if (fread(&build, 4, 1, f) != 1)        // Build
    {
        fclose(f);
        return false;
    }

    convertEndian(build);

    if (fread(&unk1, 4, 1, f) != 1)         // Unknown WDB2
    {
        fclose(f);
        return false;
    }

    convertEndian(unk1);

    if (fread(&unk2, 4, 1, f) != 1)         // Unknown WDB2
    {
        fclose(f);
        return false;
    }

    convertEndian(unk2);

    if (fread(&unk3, 4, 1, f) != 1)         // Unknown WDB2
    {
        fclose(f);
        return false;
    }

    convertEndian(unk3);

    if (fread(&locale, 4, 1, f) != 1)       // Locales
    {
        fclose(f);
        return false;
    }

    convertEndian(locale);

    if (fread(&unk5, 4, 1, f) != 1)         // Unknown WDB2
    {
        fclose(f);
        return false;
    }

    convertEndian(unk5);

    fieldsOffset = new uint32_t[fieldCount];
    fieldsOffset[0] = 0;
    for (uint32_t i = 1; i < fieldCount; i++)
    {
        fieldsOffset[i] = fieldsOffset[i - 1];
        if (fmt[i - 1] == 'b' || fmt[i - 1] == 'X') // byte fields
            fieldsOffset[i] += 1;
        else                                // 4 byte fields (int32/float/strings)
            fieldsOffset[i] += 4;
    }

    data = new unsigned char[recordSize*recordCount + stringSize];
    stringTable = data + recordSize*recordCount;

    if (fread(data, recordSize * recordCount + stringSize, 1, f) != 1)
    {
        fclose(f);
        return false;
    }

    fclose(f);
    return true;
}

DB2::DB2FileLoader::~DB2FileLoader()
{
    if (data)
        delete[] data;
    if (fieldsOffset)
        delete[] fieldsOffset;
}

DB2::DB2FileLoader::Record DB2::DB2FileLoader::getRecord(size_t id)
{
    assert(data);
    return Record(*this, data + id*recordSize);
}

uint32 DB2::DB2FileLoader::GetFormatRecordSize(const char * format, int32_t* index_pos)
{
    uint32_t recordsize = 0;
    int32_t i = -1;
    for (uint32_t x = 0; format[x]; ++x)
    {
        switch (format[x])
        {
            case FT_FLOAT:
                recordsize += sizeof(float);
                break;
            case FT_INT:
                recordsize += sizeof(uint32_t);
                break;
            case FT_STRING:
                recordsize += sizeof(char*);
                break;
            case FT_SORT:
                i = x;
                break;
            case FT_IND:
                i = x;
                recordsize += sizeof(uint32_t);
                break;
            case FT_BYTE:
                recordsize += sizeof(uint8_t);
                break;
            case FT_LOGIC:
                assert(false && "2 files not have logic field type");
                break;
            case FT_NA:
            case FT_NA_BYTE:
                break;
            default:
                assert(false && "unknown format character");
                break;
        }
    }

    if (index_pos)
        *index_pos = i;

    return recordsize;
}

uint32 DB2::DB2FileLoader::GetFormatStringsFields(const char * format)
{
    uint32_t stringfields = 0;
    for (uint32_t x = 0; format[x]; ++x)
        if (format[x] == FT_STRING)
            ++stringfields;

    return stringfields;
}

char* DB2::DB2FileLoader::AutoProduceData(const char* format, uint32_t& records, char**& indexTable)
{
    typedef char * ptr;
    if (strlen(format) != fieldCount)
        return NULL;

    //get struct size and index pos
    int32_t i;
    uint32_t recordsize = GetFormatRecordSize(format, &i);

    if (i >= 0)
    {
        uint32_t maxi = 0;
        //find max index
        for (uint32_t y = 0; y < recordCount; y++)
        {
            uint32_t ind = getRecord(y).getUInt(i);
            if (ind > maxi)
                maxi = ind;
        }

        ++maxi;
        records = maxi;
        indexTable = new ptr[maxi];
        memset(indexTable, 0, maxi * sizeof(ptr));
    }
    else
    {
        records = recordCount;
        indexTable = new ptr[recordCount];
    }

    char* dataTable = new char[recordCount*recordsize];

    uint32_t offset = 0;

    for (uint32_t y = 0; y < recordCount; ++y)
    {
        if (i >= 0)
        {
            indexTable[getRecord(y).getUInt(i)] = &dataTable[offset];
        }
        else
            indexTable[y] = &dataTable[offset];

        for (uint32_t x = 0; x < fieldCount; ++x)
        {
            switch (format[x])
            {
                case FT_FLOAT:
                    *((float*)(&dataTable[offset])) = getRecord(y).getFloat(x);
                    offset += sizeof(float);
                    break;
                case FT_IND:
                case FT_INT:
                    *((uint32_t*)(&dataTable[offset])) = getRecord(y).getUInt(x);
                    offset += sizeof(uint32_t);
                    break;
                case FT_BYTE:
                    *((uint8_t*)(&dataTable[offset])) = getRecord(y).getUInt8(x);
                    offset += sizeof(uint8_t);
                    break;
                case FT_STRING:
                    *((char**)(&dataTable[offset])) = NULL;
                    offset += sizeof(char*);
                    break;
                case FT_LOGIC:
                    assert(false && "DBC files not have logic field type");
                    break;
                case FT_NA:
                case FT_NA_BYTE:
                case FT_SORT:
                    break;
                default:
                    assert(false && "unknown format character");
                    break;
            }
        }
    }

    return dataTable;
}

static char const* const nullStr = "";

char* DB2::DB2FileLoader::AutoProduceStringsArrayHolders(const char* format, char* dataTable)
{
    if (strlen(format) != fieldCount)
        return NULL;

    size_t stringFields = GetFormatStringsFields(format);
    size_t stringHolderSize = sizeof(char*) * DBC::C_TOTAL_LOCALES;
    size_t stringHoldersRecordPoolSize = stringFields * stringHolderSize;
    size_t stringHoldersPoolSize = stringHoldersRecordPoolSize * recordCount;

    char* stringHoldersPool = new char[stringHoldersPoolSize];

    for (size_t i = 0; i < stringHoldersPoolSize / sizeof(char*); ++i)
        ((char const**)stringHoldersPool)[i] = nullStr;

    uint32_t offset = 0;

    for (uint32_t y = 0; y < recordCount; y++)
    {
        uint32_t stringFieldNum = 0;

        for (uint32_t x = 0; x < fieldCount; x++)
            switch (format[x])
            {
                case FT_FLOAT:
                case FT_IND:
                case FT_INT:
                    offset += 4;
                    break;
                case FT_BYTE:
                    offset += 1;
                    break;
                case FT_STRING:
                {
                    char const*** slot = (char const***)(&dataTable[offset]);
                    *slot = (char const**)(&stringHoldersPool[stringHoldersRecordPoolSize * y + stringHolderSize*stringFieldNum]);

                    ++stringFieldNum;
                    offset += sizeof(char*);
                    break;
                }
                case FT_NA:
                case FT_NA_BYTE:
                case FT_SORT:
                    break;
                default:
                    assert(false && "unknown format character");
            }
    }

    return stringHoldersPool;
}

char* DB2::DB2FileLoader::AutoProduceStrings(const char* format, char* dataTable, DBC::LocaleConstant loc)
{
    if (strlen(format) != fieldCount)
        return NULL;

    char* stringPool = new char[stringSize];
    memcpy(stringPool, stringTable, stringSize);

    uint32_t offset = 0;

    for (uint32_t y = 0; y < recordCount; ++y)
    {
        for (uint32_t x = 0; x < fieldCount; ++x)
        {
            switch (format[x])
            {
                case FT_FLOAT:
                    offset += sizeof(float);
                    break;
                case FT_IND:
                case FT_INT:
                    offset += sizeof(uint32_t);
                    break;
                case FT_BYTE:
                    offset += sizeof(uint8_t);
                    break;
                case FT_STRING:
                {
                    char** holder = *((char***)(&dataTable[offset]));
                    char** slot = &holder[loc];

                    if (*slot == nullStr)
                    {
                        const char * st = getRecord(y).getString(x);
                        *slot = stringPool + (st - (const char*)stringTable);
                    }

                    offset += sizeof(char*);
                    break;
                }
                case FT_LOGIC:
                    assert(false && "DBC files not have logic field type");
                    break;
                case FT_NA:
                case FT_NA_BYTE:
                case FT_SORT:
                    break;
                default:
                    assert(false && "unknown format character");
                    break;
            }
        }
    }

    return stringPool;
}
