/*
Copyright (c) 2014-2018 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "Common.hpp"
#include "DBCSQL.hpp"
#include "DBCLoader.hpp"
#include "Database/Field.h"

namespace DBC
{
    template <class T>
    class DBCStorage
    {
        private:

            typedef std::list<char*> StringPoolList;
        public:

            DBCStorage(char const* f) : m_format(f), m_row_count(0), m_field_count(0), m_data_table(NULL)
            {
                m_index_table.as_t = NULL;
            }

            ~DBCStorage()
            {
                this->Clear();
            }

            T const* LookupEntry(uint32 id) const
            {
                if (id >= m_row_count)
                {
                    return NULL;
                }

                return m_index_table.as_t[id];
            }

            T const* AssertEntry(uint32 id) const
            {
                T const* entry = LookupEntry(id);
                ASSERT(entry);
                return entry;
            }

            const uint32 GetNumRows()
            {
                return m_row_count;
            }

            char const* GetFormat() const
            {
                return m_format;
            }

            const uint32 GetFieldCount()
            {
                return m_field_count;
            }

            bool Load(char const* dbc_filename, DBC::SQL::SqlDbc* sql)
            {
                DBC::DBCLoader dbc_loader;
                // Only continue if load is successful
                if (!dbc_loader.Load(dbc_filename, m_format))
                {
                    return false;
                }

                uint32 sql_record_count = 0;
                uint32 sql_highest_index = 0;
                // SQL not yet implemented
                auto result = 0;
                // Load data from SQL
                if (sql)
                {
                    assert(false && "SqlDbc not yet implemented");
                }

                char* sql_data_table = NULL;
                m_field_count = dbc_loader.GetNumColumns();

                m_data_table = reinterpret_cast<T*>(dbc_loader.AutoProduceData(m_format, m_row_count, m_index_table.as_char, sql_record_count, sql_highest_index, sql_data_table));

                m_string_pool_list.push_back(dbc_loader.AutoProduceStrings(m_format, reinterpret_cast<char*>(m_data_table)));

                if (result)
                {
                    assert(false && "SqlDbc not yet implemented");
                }

                return m_index_table.as_t != NULL;
            }

            bool LoadStringsFrom(char const* dbc_filename)
            {
                // DBCs must already be loaded using Load
                if (!m_index_table.as_t) return false;

                DBC::DBCLoader dbc_loader;
                // Only continue if Load was successful
                if (!dbc_loader.Load(dbc_filename, m_format)) return false;

                m_string_pool_list.push_back(dbc_loader.AutoProduceStrings(m_format, reinterpret_cast<char*>(m_data_table)));
                return true;
            }

            void Clear()
            {
                if (!m_index_table.as_t) return;

                delete[] reinterpret_cast<char*>(m_index_table.as_t);
                m_index_table.as_t = NULL;
                delete[] reinterpret_cast<char*>(m_data_table);
                m_data_table = NULL;

                while (!m_string_pool_list.empty())
                {
                    delete[] m_string_pool_list.front();
                    m_string_pool_list.pop_front();
                }

                m_row_count = 0;
            }

        private:

            char const* m_format;
            uint32 m_row_count;
            uint32 m_field_count;

            union
            {
                T** as_t;
                char** as_char;
            } m_index_table;

            T* m_data_table;
            StringPoolList m_string_pool_list;

            DBCStorage(DBCStorage const& right) = delete;
            DBCStorage& operator=(DBCStorage const& right) = delete;
    };
}
