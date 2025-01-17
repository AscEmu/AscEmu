/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "WDBLoader.hpp"

#include <list>

namespace WDB
{
    template <class T>
    class WDBContainer
    {
        typedef std::list<char*> StringPoolList;

    public:
        WDBContainer() : m_row_count(0), m_field_count(0), m_data_table(nullptr)
        {
            m_index_table.as_t = NULL;
        }

        ~WDBContainer()
        {
            this->clear();
        }

        T const* lookupEntry(uint32_t _id) const
        {
            if (_id >= m_row_count)
                return nullptr;

            return m_index_table.as_t[_id];
        }

        T const* assertEntry(uint32_t _id) const
        {
            T const* entry = lookupEntry(_id);
            ASSERT(entry);
            return entry;
        }

        uint32_t getNumRows() const
        {
            return m_row_count;
        }

        void setFormat(const char* _format)
        {
            m_format = _format;
        }

        char const* getFormat() const
        {
            return m_format;
        }

        uint32_t getFieldCount() const
        {
            return m_field_count;
        }

        bool load(char const* _dbcFilename)
        {
            WDB::WDBLoader dbc_loader;

            std::string fileSuffix = _dbcFilename;
            fileSuffix = fileSuffix.substr(fileSuffix.size() - 4);

            // Only continue if load is successful
            if (fileSuffix == ".dbc")
                if (!dbc_loader.load(_dbcFilename, m_format))
                    return false;

            if (fileSuffix == ".db2")
                if (!dbc_loader.loadDb2(_dbcFilename, m_format))
                    return false;

            m_field_count = dbc_loader.getNumColumns();

            m_data_table = reinterpret_cast<T*>(dbc_loader.autoProduceData(m_format, m_row_count, m_index_table.as_char));

            m_string_pool_list.push_back(dbc_loader.autoProduceStrings(m_format, reinterpret_cast<char*>(m_data_table)));

            return m_index_table.as_t != NULL;
        }

        bool loadStringsFrom(char const* _dbcFilename)
        {
            // DBCs must already be loaded using Load
            if (!m_index_table.as_t)
                return false;

            WDB::WDBLoader dbc_loader;
            // Only continue if Load was successful
            if (!dbc_loader.load(_dbcFilename, m_format))
                return false;

            m_string_pool_list.push_back(dbc_loader.autoProduceStrings(m_format, reinterpret_cast<char*>(m_data_table)));
            return true;
        }

        void clear()
        {
            if (!m_index_table.as_t)
                return;

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
        uint32_t m_row_count;
        uint32_t m_field_count;

        union
        {
            T** as_t;
            char** as_char;
        } m_index_table;

        T* m_data_table;
        StringPoolList m_string_pool_list;

        WDBContainer(WDBContainer const& _right) = delete;
        WDBContainer& operator=(WDBContainer const& _right) = delete;
    };
}
