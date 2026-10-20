/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "WDBLoader.hpp"

#include <cstring>
#include <list>
#include <memory>
#include <type_traits>

namespace WDB
{
    template <class T>
    class WDBContainer
    {
        typedef std::list<std::unique_ptr<char[]>> StringPoolList;

    public:
        WDBContainer() noexcept : m_row_count(0), m_field_count(0), m_data_table(nullptr)
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

            if (m_manual_index_table)
                return m_manual_index_table[_id];

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

        // Entries written by the DB2 loader instead of the dbc loader: the
        // records are zeroed raw storage like the dbc data table, so the entry
        // types stay the plain structs the legacy loader casts to. lookupEntry()
        // and getNumRows() keep their maxId + 1 semantics.
        void reserveManualEntries(uint32_t _maxId, uint32_t _count)
        {
            static_assert(std::is_trivially_destructible_v<T>, "manual entries are raw zeroed storage");

            clear();

            m_row_count = _maxId + 1;
            m_manual_count = _count;
            m_manual_used = 0;
            m_manual_data = std::make_unique<char[]>(static_cast<size_t>(_count) * sizeof(T) + sizeof(T));
            std::memset(m_manual_data.get(), 0, static_cast<size_t>(_count) * sizeof(T) + sizeof(T));
            m_manual_index_table = std::make_unique<T*[]>(m_row_count);
            for (uint32_t i = 0; i < m_row_count; ++i)
                m_manual_index_table[i] = nullptr;
        }

        // nullptr when the id is out of range, already used or the storage is full
        T* addManualEntry(uint32_t _id)
        {
            if (!m_manual_index_table || _id >= m_row_count || m_manual_index_table[_id] != nullptr || m_manual_used >= m_manual_count)
                return nullptr;

            T* entry = reinterpret_cast<T*>(m_manual_data.get() + static_cast<size_t>(m_manual_used) * sizeof(T));
            ++m_manual_used;
            m_manual_index_table[_id] = entry;
            return entry;
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
            m_manual_index_table.reset();
            m_manual_data.reset();
            m_manual_count = 0;
            m_manual_used = 0;

            if (m_index_table.as_t != nullptr)
            {
                m_index_table.as_t = nullptr;
                m_data_table = nullptr;

                while (!m_string_pool_list.empty())
                {
                    m_string_pool_list.pop_front();
                }
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

        std::unique_ptr<char[]> m_manual_data;
        std::unique_ptr<T*[]> m_manual_index_table;
        uint32_t m_manual_count = 0;
        uint32_t m_manual_used = 0;

        WDBContainer(WDBContainer const& _right) = delete;
        WDBContainer& operator=(WDBContainer const& _right) = delete;
    };
}
