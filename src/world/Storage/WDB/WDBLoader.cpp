/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "WDBLoader.hpp"
#include <cassert>

#include "AEVersion.hpp"
#include "WDBFormat.hpp"
#include "Debugging/Errors.h"

namespace WDB
{
    WDBLoader::WDBLoader() : m_record_size(0), m_record_count(0), m_field_count(0), m_string_size(0), m_fields_offset(nullptr), m_data(nullptr), m_string_table(nullptr)
    { }

    WDBLoader::~WDBLoader() = default;

    WDB::WDBRecord WDBLoader::getRecord(size_t record_id) const
    {
        assert(m_data);
        return {m_data.get() + record_id * m_record_size};
    }

    uint32_t WDB::WDBLoader::getNumRows() const
    {
        return m_record_count;
    }

    uint32_t WDB::WDBLoader::getRowSize() const
    {
        return m_record_size;
    }

    uint32_t WDB::WDBLoader::getNumColumns() const
    {
        return m_field_count;
    }

    uint32_t WDB::WDBLoader::getOffset(size_t _id) const
    {
        if (m_fields_offset != nullptr && _id < m_field_count)
            return m_fields_offset[_id];

        return 0;
    }

    int WDBLoader::getVersionIdForAEVersion()
    {
        switch (VERSION_STRING)
        {
            case Classic:
                return 0;
            case TBC:
                return 1;
            case WotLK:
                return 2;
            case Cata:
                return 3;
            case Mop:
                return 4;
            default:
                return 0;
        }
    }

    bool WDBLoader::hasFormat(std::string _dbcFile)
    {
        std::string fileName = _dbcFile;
        fileName.replace(fileName.size() - 1, 1, "c");

        for (auto formats : dbcFieldDefines)
            if (formats.first == _dbcFile || formats.first == fileName)
                return true;

        return false;
    }

    std::string WDBLoader::getFormat(std::string _dbcFile)
    {
        std::string fileName = _dbcFile;
        fileName.replace(fileName.size() - 1, 1, "c");

        for (auto formats : dbcFieldDefines)
        {
            if (formats.first == _dbcFile || formats.first == fileName)
            {
                std::string format = formats.second.format[getVersionIdForAEVersion()];
                return format;
            }
        }

        return "";
    }

    uint32_t WDBLoader::getFormatRecordSize(const char* _dbcFormat, int32_t* _indexPos /* = NULL */)
    {
        uint32_t recordSize = 0;
        int32_t i = -1;

        for (uint32_t x = 0; _dbcFormat[x]; ++x)
        {
            switch (_dbcFormat[x])
            {
                case WDB::DbcFieldFormat::FT_FLOAT:
                    recordSize += sizeof(float);
                    break;
                case WDB::DbcFieldFormat::FT_INT:
                    recordSize += sizeof(uint32_t);
                    break;
                case WDB::DbcFieldFormat::FT_STRING:
                    recordSize += sizeof(char*);
                    break;
                case WDB::DbcFieldFormat::FT_SORT:
                    i = x;
                    break;
                case WDB::DbcFieldFormat::FT_IND:
                    i = x;
                    recordSize += sizeof(uint32_t);
                    break;
                case WDB::DbcFieldFormat::FT_BYTE:
                    recordSize += sizeof(uint8_t);
                    break;
                case WDB::DbcFieldFormat::FT_NA:
                case WDB::DbcFieldFormat::FT_NA_BYTE:
                    break;
                case FT_LOGIC:
                    ASSERT(false && "Attempted to load DBC files that do not have field types that match what is in the core. Check DBC\\DBCStructures.hpp or your DBC files.");
                    break;
                default:
                    ASSERT(false && "Unknown field format character in DBC\\DBCStructures.hpp");
                    break;
            }
        }

        if (_indexPos)
            *_indexPos = i;

        return recordSize;
    }

    bool WDBLoader::load(const char* _dbcFilename, const char* _dbcFormat)
    {
        uint32_t header;
        if (m_data)
        {
            m_data = nullptr;
        }

        const auto file = fopen(_dbcFilename, "rb");
        if (!file)
            return false;

        // Signature
        if (fread(&header, 4, 1, file) != 1)
        {
            fclose(file);
            return false;
        }

        // 'WDBC' magic string
        if (header != 0x43424457)
        {
            fclose(file);
            return false;
        }

        // Number of records
        if (fread(&m_record_count, 4, 1, file) != 1)
        {
            fclose(file);
            return false;
        }

        // Number of fields
        if (fread(&m_field_count, 4, 1, file) != 1)
        {
            fclose(file);
            return false;
        }

        // Size of an individual record
        if (fread(&m_record_size, 4, 1, file) != 1)
        {
            fclose(file);
            return false;
        }

        // String size
        if (fread(&m_string_size, 4, 1, file) != 1)
        {
            fclose(file);
            return false;
        }

        m_fields_offset = std::make_unique<uint32_t[]>(m_field_count);
        m_fields_offset[0] = 0;

        for (uint32_t i = 1; i < m_field_count; ++i)
        {
            m_fields_offset[i] = m_fields_offset[i - 1];
            // Byte fields
            if (_dbcFormat[i - 1] == 'b' || _dbcFormat[i - 1] == 'X')
            {
                m_fields_offset[i] += sizeof(uint8_t);
            }
            // 4 byte fields (int32_t, float, strings)
            else
            {
                m_fields_offset[i] += sizeof(uint32_t);
            }
        }

        m_data = std::make_unique<unsigned char[]>(m_record_size * m_record_count + m_string_size);
        m_string_table = m_data.get() + m_record_size * m_record_count;

        if (fread(m_data.get(), m_record_size * m_record_count + m_string_size, 1, file) != 1)
        {
            fclose(file);
            return false;
        }

        fclose(file);
        return true;
    }

    bool WDBLoader::loadDb2(const char* _dbcFilename, const char* _dbcFormat)
    {
        uint32_t header = 48;
        if (m_data)
        {
            m_data = nullptr;
        }

        const auto file = fopen(_dbcFilename, "rb");
        if (!file)
            return false;

        // Signature
        if (fread(&header, 4, 1, file) != 1)
        {
            fclose(file);
            return false;
        }

        //'WDB2'
        if (header != 0x32424457)
        {
            fclose(file);
            return false;
        }

        // Number of records
        if (fread(&m_record_count, 4, 1, file) != 1)
        {
            fclose(file);
            return false;
        }

        // Number of fields
        if (fread(&m_field_count, 4, 1, file) != 1)
        {
            fclose(file);
            return false;
        }

        // Size of a record
        if (fread(&m_record_size, 4, 1, file) != 1)
        {
            fclose(file);
            return false;
        }

        // String size
        if (fread(&m_string_size, 4, 1, file) != 1)
        {
            fclose(file);
            return false;
        }

        // Table hash
        if (fread(&m_tableHash, 4, 1, file) != 1)
        {
            fclose(file);
            return false;
        }

        // Build
        if (fread(&m_build, 4, 1, file) != 1)
        {
            fclose(file);
            return false;
        }

        // Unknown WDB2
        if (fread(&m_unk1, 4, 1, file) != 1)
        {
            fclose(file);
            return false;
        }

        // Unknown WDB2
        if (fread(&m_unk2, 4, 1, file) != 1)
        {
            fclose(file);
            return false;
        }

        // Unknown WDB2
        if (fread(&m_unk3, 4, 1, file) != 1)
        {
            fclose(file);
            return false;
        }

        // Locales
        if (fread(&m_locale, 4, 1, file) != 1)
        {
            fclose(file);
            return false;
        }

        // Unknown WDB2
        if (fread(&m_unk5, 4, 1, file) != 1)
        {
            fclose(file);
            return false;
        }

        m_fields_offset = std::make_unique<uint32_t[]>(m_field_count);
        m_fields_offset[0] = 0;
        for (uint32_t i = 1; i < m_field_count; i++)
        {
            m_fields_offset[i] = m_fields_offset[i - 1];
            if (_dbcFormat[i - 1] == 'b' || _dbcFormat[i - 1] == 'X') // byte fields
                m_fields_offset[i] += 1;
            else                                // 4 byte fields (int32_t/float/strings)
                m_fields_offset[i] += 4;
        }

        m_data = std::make_unique<unsigned char[]>(m_record_size * m_record_count + m_string_size);
        m_string_table = m_data.get() + m_record_size * m_record_count;

        if (fread(m_data.get(), m_record_size * m_record_count + m_string_size, 1, file) != 1)
        {
            fclose(file);
            return false;
        }

        fclose(file);
        return true;
    }

    std::unique_ptr<char[]> WDBLoader::autoProduceData(const char* _dbcFormat, uint32_t& _recordCount, std::unique_ptr<char*[]>& _indexTable)
    {
        if (strlen(_dbcFormat) != m_field_count) return nullptr;

        /* Get struct size and index position */
        int i;
        const uint32_t recordSize = this->getFormatRecordSize(_dbcFormat, &i);

        if (i >= 0)
        {
            uint32_t maxIndex = 0;
            for (uint32_t y = 0; y < m_record_count; ++y)
            {
                uint32_t index = this->getRecord(y).getUInt32(i, m_field_count, this->getOffset(i));
                if (index > maxIndex)
                    maxIndex = index;
            }

            ++maxIndex;
            _recordCount = maxIndex;
            _indexTable = std::make_unique<char*[]>(maxIndex);
            memset(_indexTable.get(), 0, maxIndex * sizeof(char*));
        }
        else
        {
            _recordCount = m_record_count;
            _indexTable = std::make_unique<char*[]>(m_record_count);
        }

        auto data_table = std::make_unique<char[]>(m_record_count * recordSize);
        uint32_t offset = 0;

        for (uint32_t y = 0; y < m_record_count; ++y)
        {
            if (i >= 0)
                _indexTable[this->getRecord(y).getUInt32(i, m_field_count, this->getOffset(i))] = &data_table[offset];
            else
                _indexTable[y] = &data_table[offset];

            for (uint32_t x = 0; x < m_field_count; ++x)
            {
                switch (_dbcFormat[x])
                {
                    case WDB::DbcFieldFormat::FT_FLOAT:
                        *((float*)(&data_table[offset])) = this->getRecord(y).getFloat(x, m_field_count, this->getOffset(x));
                        offset += sizeof(float);
                        break;
                    case WDB::DbcFieldFormat::FT_IND:
                    case WDB::DbcFieldFormat::FT_INT:
                        *((uint32_t*)(&data_table[offset])) = this->getRecord(y).getUInt32(x, m_field_count, this->getOffset(x));
                        offset += sizeof(uint32_t);
                        break;
                    case WDB::DbcFieldFormat::FT_BYTE:
                        *((uint8_t*)(&data_table[offset])) = this->getRecord(y).getUInt8(x, m_field_count, this->getOffset(x));
                        offset += sizeof(uint8_t);
                        break;
                    case WDB::DbcFieldFormat::FT_STRING:
                        /* Non-empty or "" strings are replaced in WDB::DBCLoader::AutoProduceStrings */
                        *((char**)(&data_table[offset])) = nullptr;
                        offset += sizeof(char*);
                        break;
                    case WDB::DbcFieldFormat::FT_NA:
                    case WDB::DbcFieldFormat::FT_NA_BYTE:
                    case WDB::DbcFieldFormat::FT_SORT:
                        break;
                    case FT_LOGIC:
                        ASSERT(false && "Attempted to load DBC files that do not have field types that match what is in the core. Check WDBStructures.hpp or your DBC files.");
                        break;
                    default:
                        ASSERT(false && "Unknown field format character in WDBStructures.hpp");
                        break;
                }
            }
        }

        return data_table;
    }

    std::unique_ptr<char[]> WDBLoader::autoProduceStrings(const char* _dbcFormat, char* _dataTable)
    {
        if (strlen(_dbcFormat) != m_field_count)
            return nullptr;

        auto string_pool = std::make_unique<char[]>(m_string_size);
        memcpy(string_pool.get(), m_string_table, m_string_size);

        uint32_t offset = 0;

        for (uint32_t y = 0; y < m_record_count; ++y)
        {
            for (uint32_t x = 0; x < m_field_count; ++x)
            {
                switch (_dbcFormat[x])
                {
                    case WDB::DbcFieldFormat::FT_FLOAT:
                        offset += sizeof(float);
                        break;
                    case WDB::DbcFieldFormat::FT_IND:
                    case WDB::DbcFieldFormat::FT_INT:
                        offset += sizeof(uint32_t);
                        break;
                    case WDB::DbcFieldFormat::FT_BYTE:
                        offset += sizeof(uint8_t);
                        break;
                    case WDB::DbcFieldFormat::FT_STRING:
                    {
                        /* Fills only unfilled entries */
                        char** slot = (char**)(&_dataTable[offset]);
                        if (!*slot || !**slot)
                        {
                            const char* st = this->getRecord(y).getString(x, m_field_count, this->getOffset(x), m_string_size, m_string_table);
                            *slot = string_pool.get() + (st - (const char*)m_string_table);
                        }
                        offset += sizeof (char*);
                        break;
                    }
                    case WDB::DbcFieldFormat::FT_NA:
                    case WDB::DbcFieldFormat::FT_NA_BYTE:
                    case WDB::DbcFieldFormat::FT_SORT:
                        break;
                    case FT_LOGIC:
                        ASSERT(false && "Attempted to load DBC files that do not have field types that match what is in the core. Check WDBStructures.hpp or your DBC files.");
                        break;
                    default:
                        ASSERT(false && "Unknown field format character in WDBStructures.hpp");
                        break;
                }
            }
        }

        return string_pool;
    }
}
