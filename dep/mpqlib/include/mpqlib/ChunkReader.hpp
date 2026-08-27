/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

// Walks Blizzard's [4-byte reversed tag][uint32 size][payload] chunk sequence
// - the format ADT/WDT/WMO files share - so callers don't each hand-roll the
// same read-reverse-dispatch-advance loop over a raw cursor.

#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <span>
#include <string_view>

#include "mpqlib/BinaryReader.hpp"

namespace mpqlib
{
    struct Chunk
    {
        std::string_view tag;
        std::span<const std::byte> payload;
    };

    class ChunkReader
    {
    public:
        explicit ChunkReader(BinaryReader const& reader, size_t startOffset = 0) noexcept :
            m_reader(reader), m_offset(startOffset)
        {
        }

        // Advances past the current chunk and fills out with the next one.
        // Returns false (out left untouched) once the remaining bytes can't
        // hold another full chunk header + payload.
        bool next(Chunk& out);

    private:
        BinaryReader const& m_reader;
        size_t m_offset;
        std::array<char, 4> m_tagStorage{};
    };
}
