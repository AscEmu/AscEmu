/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <array>
#include <algorithm>
#include <bit>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <span>
#include <string>
#include <type_traits>

#include "mpqlib/BinaryReader.hpp"
#include "mpqlib/MpqPatchChain.hpp"

class DBCFile
{
public:
    DBCFile(mpqlib::MpqPatchChain& mpq, const std::string& filename);
    ~DBCFile();

    // Open database. It must be opened before it can be used.
    bool open();

    class Exception
    {
    public:
        explicit Exception(const std::string& message);
        virtual ~Exception();
        const std::string& getMessage() const;

    private:
        std::string m_message;
    };

    class NotFound : public Exception
    {
    public:
        NotFound();
    };

    class Cursor;
    class Row
    {
    public:
        // Read per field on every row while iterating a DBC - kept inline.
        float getFloat(size_t field) const { return readField<float>(field); }
        unsigned int getUInt(size_t field) const { return readField<unsigned int>(field); }
        int getInt(size_t field) const { return readField<int>(field); }

        const char* getString(size_t field) const
        {
            auto const offset = getUInt(field);
            assert(offset < m_stringTable.size());
            return reinterpret_cast<const char*>(m_stringTable.data() + offset);
        }

    private:
        Row(std::span<const std::byte> recordBytes, std::span<const std::byte> stringTable) noexcept :
            m_recordBytes(recordBytes), m_stringTable(stringTable)
        {
        }

        // Field-punning helper for the accessors above - a template, so it
        // has to live in the header regardless of the "no inline bodies" rule.
        template<typename T>
        T readField(size_t field) const
        {
            static_assert(std::is_trivially_copyable_v<T> && sizeof(T) == sizeof(uint32_t));
            assert((field + 1) * sizeof(T) <= m_recordBytes.size());

            std::array<std::byte, sizeof(T)> raw{};
            std::ranges::copy(m_recordBytes.subspan(field * sizeof(T), sizeof(T)), raw.begin());
            return std::bit_cast<T>(raw);
        }

        std::span<const std::byte> m_recordBytes;
        std::span<const std::byte> m_stringTable;

        friend class DBCFile;
        friend class DBCFile::Cursor;

        Row& operator=(Row const& right) = delete;
    };

    class Cursor
    {
    public:
        Cursor(std::span<const std::byte> recordBytes, std::span<const std::byte> stringTable) noexcept;

        Cursor& operator++();

        const Row& operator*() const { return m_row; }
        const Row* operator->() const { return &m_row; }

        bool operator==(const Cursor& other) const { return m_row.m_recordBytes.data() == other.m_row.m_recordBytes.data(); }
        bool operator!=(const Cursor& other) const { return m_row.m_recordBytes.data() != other.m_row.m_recordBytes.data(); }

    private:
        Row m_row;

        Cursor& operator=(Cursor const& right) = delete;
    };

    Row getRow(size_t id) const;
    Cursor begin() const;
    Cursor end() const;
    size_t getRowCount() const;
    size_t getFieldCount() const;
    size_t getMaxId() const;

private:
    static constexpr size_t kHeaderSize = 20;

    std::span<const std::byte> stringTableSpan() const;

    mpqlib::MpqPatchChain& m_mpq;
    std::string m_filename;
    size_t m_recordSize;
    size_t m_recordCount;
    size_t m_fieldCount;
    size_t m_stringSize;
    std::optional<mpqlib::BinaryReader> m_reader;
};
