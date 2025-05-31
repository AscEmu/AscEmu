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
        typedef std::list<std::unique_ptr<char[]>> StringPoolList;

    public:
        WDBContainer() : m_row_count(0), m_field_count(0), m_data_table(nullptr)
        {
            m_index_table.as_t = nullptr;
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

        void setFormat(std::unique_ptr<const char[]> _format)
        {
            m_format = std::move(_format);
        }

        char const* getFormat() const
        {
            return m_format.get();
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
                if (!dbc_loader.load(_dbcFilename, m_format.get()))
                    return false;

            if (fileSuffix == ".db2")
                if (!dbc_loader.loadDb2(_dbcFilename, m_format.get()))
                    return false;

            m_field_count = dbc_loader.getNumColumns();

            m_data_table = dbc_loader.autoProduceData(m_format.get(), m_row_count, m_index_table.as_char);

            m_string_pool_list.emplace_back(dbc_loader.autoProduceStrings(m_format.get(), m_data_table.get()));

            return m_index_table.as_t != nullptr;
        }

        bool loadStringsFrom(char const* _dbcFilename)
        {
            // DBCs must already be loaded using Load
            if (m_index_table.as_t == nullptr)
                return false;

            WDB::WDBLoader dbc_loader;
            // Only continue if Load was successful
            if (!dbc_loader.load(_dbcFilename, m_format.get()))
                return false;

            m_string_pool_list.emplace_back(dbc_loader.autoProduceStrings(m_format.get(), m_data_table.get()));
            return true;
        }

        void clear()
        {
            if (m_index_table.as_t == nullptr)
                return;

            m_index_table.as_t = nullptr;
            m_data_table = nullptr;

            while (!m_string_pool_list.empty())
            {
                m_string_pool_list.pop_front();
            }

            m_row_count = 0;
        }

    private:
        std::unique_ptr<const char[]> m_format;
        uint32_t m_row_count;
        uint32_t m_field_count;

        union IndexTableUnion
        {
            T** as_t;
            std::unique_ptr<char*[]> as_char;

            IndexTableUnion() : as_t(nullptr) {}
            ~IndexTableUnion() {}
        } m_index_table;

        std::unique_ptr<char[]> m_data_table;
        StringPoolList m_string_pool_list;

        WDBContainer(WDBContainer const& _right) = delete;
        WDBContainer& operator=(WDBContainer const& _right) = delete;
    };
}
