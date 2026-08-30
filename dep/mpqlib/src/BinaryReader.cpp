/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "mpqlib/BinaryReader.hpp"

#include <algorithm>
#include <cstring>

namespace mpqlib
{
    std::optional<BinaryReader> BinaryReader::fromMpq(MpqPatchChain& mpq, std::string_view filename)
    {
        std::vector<uint8_t> raw;
        if (!mpq.readFile(filename, raw))
            return std::nullopt;

        // uint8_t and std::byte share layout - a bulk memcpy here instead of
        // a per-element transform matters for multi-megabyte files.
        std::vector<std::byte> data(raw.size());
        std::memcpy(data.data(), raw.data(), raw.size());

        return BinaryReader(std::move(data));
    }

    BinaryReader::BinaryReader(std::vector<std::byte> data) noexcept : m_data(std::move(data))
    {
    }

    std::span<const std::byte> BinaryReader::subspan(size_t offset, size_t count) const
    {
        if (offset > m_data.size() || count > m_data.size() - offset)
            throw std::out_of_range("BinaryReader::subspan out of range");

        return std::span(m_data).subspan(offset, count);
    }

    void BinaryReader::seek(size_t offset) noexcept
    {
        m_cursor = std::min(offset, m_data.size());
    }

    void BinaryReader::skip(ptrdiff_t delta) noexcept
    {
        seek(static_cast<size_t>(static_cast<ptrdiff_t>(m_cursor) + delta));
    }

    size_t BinaryReader::read(void* dest, size_t byteCount) noexcept
    {
        size_t const available = m_data.size() - m_cursor;
        size_t const toRead = std::min(byteCount, available);

        std::memcpy(dest, m_data.data() + m_cursor, toRead);
        m_cursor += toRead;

        return toRead;
    }
}
