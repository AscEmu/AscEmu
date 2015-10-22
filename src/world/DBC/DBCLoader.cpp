/*
 * AscEmu Framework based on ArcEmu MMORPG Server
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
 *
 */

#include "DBCLoader.hpp"

namespace DBC
{
    DBCLoader::DBCLoader() : m_record_size(0), m_record_count(0), m_field_count(0), m_string_size(0), m_fields_offset(NULL), m_data(NULL), m_string_table(NULL)
    {
        /* No Body */
    }

    DBCLoader::~DBCLoader()
    {
        delete[] m_data;
        delete[] m_fields_offset;
    }

    DBC::DBCRecord DBCLoader::GetRecord(size_t record_id)
    {
        assert(m_data);
        return DBC::DBCRecord(m_data + record_id * m_record_size);
    }

    const uint32 DBC::DBCLoader::GetNumRows()
    {
        return m_record_count;
    }

    const uint32 DBC::DBCLoader::GetRowSize()
    {
        return m_record_size;
    }

    const uint32 DBC::DBCLoader::GetNumColumns()
    {
        return m_field_count;
    }

    const uint32 DBC::DBCLoader::GetOffset(size_t id)
    {
        if (m_fields_offset != NULL && id < m_field_count)
        {
            return m_fields_offset[id];
        }

        return 0;
    }

    uint32 DBCLoader::GetFormatRecordSize(const char* dbc_format, int32* index_pos /* = NULL */)
    {
        uint32 record_size = 0;
        int32 i = -1;

        for (uint32 x = 0; dbc_format[x]; ++x)
        {
            switch (dbc_format[x])
            {
            case DBC::DbcFieldFormat::FT_FLOAT:
                record_size += sizeof(float);
                break;
            case DBC::DbcFieldFormat::FT_INT:
                record_size += sizeof(uint32);
                break;
            case DBC::DbcFieldFormat::FT_STRING:
                record_size += sizeof(char*);
                break;
            case DBC::DbcFieldFormat::FT_SORT:
                i = x;
                break;
            case DBC::DbcFieldFormat::FT_IND:
                i = x;
                record_size += sizeof(uint32);
                break;
            case DBC::DbcFieldFormat::FT_BYTE:
                record_size += sizeof(uint8);
                break;
            case DBC::DbcFieldFormat::FT_NA:
            case DBC::DbcFieldFormat::FT_NA_BYTE:
                break;
            case FT_LOGIC:
                ASSERT(false && "Attempted to load DBC files that do not have field types that match what is in the core. Check DBC\\DBCStructures.hpp or your DBC files.");
                break;
            default:
                ASSERT(false && "Unknown field format character in DBC\\DBCStructures.hpp");
                break;
            }
        }

        if (index_pos)
        {
            *index_pos = i;
        }

        return record_size;
    }

    bool DBCLoader::Load(const char* dbc_filename, const char* dbc_format)
    {
        uint32 header;
        if (m_data)
        {
            delete[] m_data;
            m_data = NULL;
        }

        auto file = fopen(dbc_filename, "rb");
        if (!file)
        {
            return false;
        }

        /* Number of records */
        if (fread(&header, 4, 1, file) != 1)
        {
            fclose(file);
            return false;
        }

        /* 'WDBC' magic string */
        if (header != 0x43424457)
        {
            fclose(file);
            return false;
        }

        /* Number of records */
        if (fread(&m_record_count, 4, 1, file) != 1)
        {
            fclose(file);
            return false;
        }

        /* Number of fields */
        if (fread(&m_field_count, 4, 1, file) != 1)
        {
            fclose(file);
            return false;
        }

        /* Size of an individual record */
        if (fread(&m_record_size, 4, 1, file) != 1)
        {
            fclose(file);
            return false;
        }

        /* String size */
        if (fread(&m_string_size, 4, 1, file) != 1)
        {
            fclose(file);
            return false;
        }

        m_fields_offset = new uint32[m_field_count];
        m_fields_offset[0] = 0;

        for (uint32 i = 1; i < m_field_count; ++i)
        {
            m_fields_offset[i] = m_fields_offset[i - 1];
            /* Byte fields */
            if (dbc_format[i - 1] == 'b' || dbc_format[i - 1] == 'X')
            {
                m_fields_offset[i] += sizeof(uint8);
            }
            /* 4 byte fields (int32, float, strings) */
            else
            {
                m_fields_offset[i] += sizeof(uint32);
            }
        }

        m_data = new unsigned char[m_record_size * m_record_count + m_string_size];
        m_string_table = m_data + m_record_size * m_record_count;

        if (fread(m_data, m_record_size * m_record_count + m_string_size, 1, file) != 1)
        {
            fclose(file);
            return false;
        }

        fclose(file);
        return true;
    }

    char* DBCLoader::AutoProduceData(const char* dbc_format, uint32& record_count, char**& index_table, uint32 sql_record_count, uint32 sql_highest_index, char *& sql_data_table)
    {
        if (strlen(dbc_format) != m_field_count) return NULL;

        /* Get struct size and index position */
        int i;
        uint32 record_size = this->GetFormatRecordSize(dbc_format, &i);

        if (i >= 0)
        {
            uint32 max_index = 0;
            for (uint32 y = 0; y < m_record_count; ++y)
            {
                uint32 index = this->GetRecord(y).GetUInt32(i, m_field_count, this->GetOffset(i));
                if (index > max_index)
                {
                    max_index = index;
                }
            }

            if (sql_highest_index > max_index)
            {
                max_index = sql_highest_index;
            }

            ++max_index;
            record_count = max_index;
            index_table = new char*[max_index];
            memset(index_table, 0, max_index * sizeof(char*));
        }
        else
        {
            record_count = m_record_count + sql_record_count;
            index_table = new char*[m_record_count + sql_record_count];
        }

        char* data_table = new char[(m_record_count + sql_record_count) * record_size];
        uint32 offset = 0;

        for (uint32 y = 0; y < m_record_count; ++y)
        {
            if (i >= 0)
            {
                index_table[this->GetRecord(y).GetUInt32(i, m_field_count, this->GetOffset(i))] = &data_table[offset];
            }
            else
            {
                index_table[y] = &data_table[offset];
            }

            for (uint32 x = 0; x < m_field_count; ++x)
            {
                switch (dbc_format[x])
                {
                case DBC::DbcFieldFormat::FT_FLOAT:
                    *((float*)(&data_table[offset])) = this->GetRecord(y).GetFloat(x, m_field_count, this->GetOffset(x));
                    offset += sizeof(float);
                    break;
                case DBC::DbcFieldFormat::FT_IND:
                case DBC::DbcFieldFormat::FT_INT:
                    *((uint32*)(&data_table[offset])) = this->GetRecord(y).GetUInt32(x, m_field_count, this->GetOffset(x));
                    offset += sizeof(uint32);
                    break;
                case DBC::DbcFieldFormat::FT_BYTE:
                    *((uint8*)(&data_table[offset])) = this->GetRecord(y).GetUInt8(x, m_field_count, this->GetOffset(x));
                    offset += sizeof(uint8);
                    break;
                case DBC::DbcFieldFormat::FT_STRING:
                    /* Non-empty or "" strings are replaced in DBC::DBCLoader::AutoProduceStrings */
                    *((char**)(&data_table[offset])) = NULL;
                    offset += sizeof(char*);
                    break;
                case DBC::DbcFieldFormat::FT_NA:
                case DBC::DbcFieldFormat::FT_NA_BYTE:
                case DBC::DbcFieldFormat::FT_SORT:
                    break;
                case FT_LOGIC:
                    ASSERT(false && "Attempted to load DBC files that do not have field types that match what is in the core. Check DBC\\DBCStructures.hpp or your DBC files.");
                    break;
                default:
                    ASSERT(false && "Unknown field format character in DBC\\DBCStructures.hpp");
                    break;
                }
            }
        }

        sql_data_table = data_table + offset;
        return data_table;
    }

    char* DBCLoader::AutoProduceStrings(const char* dbc_format, char* data_table)
    {
        if (strlen(dbc_format) != m_field_count) return NULL;

        char* string_pool = new char[m_string_size];
        memcpy(string_pool, m_string_table, m_string_size);

        uint32 offset = 0;

        for (uint32 y = 0; y < m_record_count; ++y)
        {
            for (uint32 x = 0; x < m_field_count; ++x)
            {
                switch (dbc_format[x])
                {
                case DBC::DbcFieldFormat::FT_FLOAT:
                    offset += sizeof(float);
                    break;
                case DBC::DbcFieldFormat::FT_IND:
                case DBC::DbcFieldFormat::FT_INT:
                    offset += sizeof(uint32);
                    break;
                case DBC::DbcFieldFormat::FT_BYTE:
                    offset += sizeof(uint8);
                    break;
                case DBC::DbcFieldFormat::FT_STRING:
                {
                    /* Fills only unfilled entries */
                    char** slot = (char**)(&data_table[offset]);
                    if (!*slot || !**slot)
                    {
                        const char* st = this->GetRecord(y).GetString(x, m_field_count, this->GetOffset(x), m_string_size, m_string_table);
                        *slot = string_pool + (st - (const char*)m_string_table);
                    }
                    offset += sizeof (char*);
                    break;
                }
                case DBC::DbcFieldFormat::FT_NA:
                case DBC::DbcFieldFormat::FT_NA_BYTE:
                case DBC::DbcFieldFormat::FT_SORT:
                    break;
                case FT_LOGIC:
                    ASSERT(false && "Attempted to load DBC files that do not have field types that match what is in the core. Check DBC\\DBCStructures.hpp or your DBC files.");
                    break;
                default:
                    ASSERT(false && "Unknown field format character in DBC\\DBCStructures.hpp");
                    break;
                }
            }
        }

        return string_pool;
    }
}