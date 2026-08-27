/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "mpqlib/DBCFile.hpp"

#include <algorithm>
#include <array>
#include <cstdint>

namespace
{
    struct DbcHeader
    {
        std::array<char, 4> magic;
        uint32_t recordCount;
        uint32_t fieldCount;
        uint32_t recordSize;
        uint32_t stringSize;
    };

    static_assert(sizeof(DbcHeader) == 20);
}

DBCFile::Exception::Exception(const std::string& message) : m_message(message)
{
}

DBCFile::Exception::~Exception() = default;

const std::string& DBCFile::Exception::getMessage() const
{
    return m_message;
}

DBCFile::NotFound::NotFound() : Exception("Key was not found")
{
}

DBCFile::Cursor::Cursor(std::span<const std::byte> recordBytes, std::span<const std::byte> stringTable) noexcept :
    m_row(recordBytes, stringTable)
{
}

DBCFile::Cursor& DBCFile::Cursor::operator++()
{
    auto const bytes = m_row.m_recordBytes;
    m_row.m_recordBytes = std::span<const std::byte>(bytes.data() + bytes.size(), bytes.size());
    return *this;
}

DBCFile::DBCFile(mpqlib::MpqPatchChain& mpq, const std::string& filename) :
    m_mpq(mpq), m_filename(filename), m_recordSize(0), m_recordCount(0), m_fieldCount(0), m_stringSize(0)
{
}

DBCFile::~DBCFile() = default;

bool DBCFile::open()
{
    auto reader = mpqlib::BinaryReader::fromMpq(m_mpq, m_filename);
    if (!reader || reader->size() < kHeaderSize)
        return false;

    auto const& header = reader->at<DbcHeader>(0);
    if (header.magic != std::array<char, 4>{ 'W', 'D', 'B', 'C' })
        return false;

    if (static_cast<size_t>(header.fieldCount) * sizeof(uint32_t) != header.recordSize)
        return false;

    size_t const dataSize = static_cast<size_t>(header.recordSize) * header.recordCount + header.stringSize;
    if (reader->size() < kHeaderSize + dataSize)
        return false;

    m_recordSize = header.recordSize;
    m_recordCount = header.recordCount;
    m_fieldCount = header.fieldCount;
    m_stringSize = header.stringSize;
    m_reader = std::move(reader);
    return true;
}

std::span<const std::byte> DBCFile::stringTableSpan() const
{
    return m_reader->subspan(kHeaderSize + m_recordSize * m_recordCount, m_stringSize);
}

DBCFile::Row DBCFile::getRow(size_t id) const
{
    assert(m_reader);
    return Row(m_reader->subspan(kHeaderSize + id * m_recordSize, m_recordSize), stringTableSpan());
}

size_t DBCFile::getRowCount() const
{
    return m_recordCount;
}

size_t DBCFile::getFieldCount() const
{
    return m_fieldCount;
}

size_t DBCFile::getMaxId() const
{
    assert(m_reader);

    size_t maxId = 0;
    for (size_t i = 0; i < m_recordCount; ++i)
        maxId = std::max<size_t>(maxId, getRow(i).getUInt(0));

    return maxId;
}

DBCFile::Cursor DBCFile::begin() const
{
    assert(m_reader);
    if (m_recordCount == 0)
        return end();

    return Cursor(m_reader->subspan(kHeaderSize, m_recordSize), stringTableSpan());
}

DBCFile::Cursor DBCFile::end() const
{
    assert(m_reader);
    auto const strings = stringTableSpan();
    return Cursor(std::span<const std::byte>(strings.data(), 0), strings);
}
