/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

// Loads one file out of an MpqPatchChain into memory and hands out bounds-
// checked views over it - random-access (a header or record at a known
// offset) as well as sequential (a moving read cursor for streaming chunk
// formats). One shared buffer primitive instead of every format-specific
// reader (DBC records, ADT/WDT chunks, the old MPQFile stream wrapper)
// managing its own raw allocation and pointer arithmetic.

#pragma once

#include <cstddef>
#include <optional>
#include <span>
#include <stdexcept>
#include <string>
#include <string_view>
#include <type_traits>
#include <vector>

#include "mpqlib/MpqPatchChain.hpp"

namespace mpqlib
{
    class BinaryReader
    {
    public:
        static std::optional<BinaryReader> fromMpq(MpqPatchChain& mpq, std::string_view filename);

        explicit BinaryReader(std::vector<std::byte> data) noexcept;

        size_t size() const noexcept { return m_data.size(); }
        bool empty() const noexcept { return m_data.empty(); }

        std::span<const std::byte> bytes() const noexcept { return m_data; }

        // Throws std::out_of_range if [offset, offset + count) isn't fully in bounds.
        std::span<const std::byte> subspan(size_t offset, size_t count) const;

        // Views a POD header in place - zero-copy, but only well-defined when
        // offset actually satisfies alignof(T) (true for anything read at
        // offset 0, since the backing allocation is suitably aligned for any
        // type; not guaranteed at an arbitrary mid-buffer offset). Callers
        // reading at a data-dependent offset should copy out instead.
        template<typename T>
        const T& at(size_t offset) const
        {
            static_assert(std::is_trivially_copyable_v<T>);
            return *reinterpret_cast<const T*>(subspan(offset, sizeof(T)).data());
        }

        // Sequential cursor, for formats read as a stream rather than by offset.
        size_t tell() const noexcept { return m_cursor; }
        bool atEnd() const noexcept { return m_cursor >= m_data.size(); }
        void seek(size_t offset) noexcept;
        void skip(ptrdiff_t delta) noexcept;

        // Copies up to byteCount bytes starting at the cursor into dest, advances
        // the cursor by that amount, and returns how many bytes were actually
        // available (may be less than requested at the end of the buffer).
        size_t read(void* dest, size_t byteCount) noexcept;

    private:
        std::vector<std::byte> m_data;
        size_t m_cursor = 0;
    };
}
