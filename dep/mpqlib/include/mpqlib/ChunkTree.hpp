/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

// Scans a loaded file for a caller-supplied set of recognized chunk tags and
// builds the (possibly nested) tree of matches it finds - tolerates
// interleaved bytes that aren't a recognized tag, unlike a strict sequential
// [tag][size][payload] walk (see ChunkReader for that simpler case). Useful
// for formats where chunks of interest are surrounded by other, unlisted
// data, or where a chunk (e.g. ADT's MCNK) nests further recognized chunks
// inside its own payload.

#pragma once

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <map>
#include <memory>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <type_traits>
#include <vector>

#include "mpqlib/BinaryReader.hpp"
#include "mpqlib/MpqPatchChain.hpp"

namespace mpqlib
{
    class ChunkTree;

    // One recognized chunk: its own [tag][size] header plus payload, viewed
    // in place - Blizzard's chunk structs put fcc/size first, so overlaying
    // one directly onto a node's bytes (via as<T>()) reads both the header
    // and the payload fields that follow it.
    class ChunkNode
    {
    public:
        template<typename T>
        const T& as() const
        {
            static_assert(std::is_trivially_copyable_v<T>);
            assert(sizeof(T) <= m_bytes.size());
            return *reinterpret_cast<const T*>(m_bytes.data());
        }

        // Exactly one child with this tag, else nullptr (absent or ambiguous).
        const ChunkNode* find(std::string_view tag) const;

    private:
        explicit ChunkNode(std::span<const std::byte> bytes) noexcept : m_bytes(bytes) {}

        std::span<const std::byte> m_bytes;
        std::multimap<std::string, std::unique_ptr<ChunkNode>> m_children;

        friend class ChunkTree;
    };

    class ChunkTree
    {
    public:
        // Each recognized tag alongside its on-disk (reversed) byte pattern
        // packed into a uint32_t, precomputed once per load() so the scan
        // loop can do a single load + integer compare per candidate instead
        // of re-deriving byte comparisons at every position it looks at.
        // Public only because the scan helper that consumes it needs to name
        // the type; not meant to be constructed by callers of load().
        struct RecognizedTag
        {
            std::string_view name;
            uint32_t reversedMagic;
        };

        static std::optional<ChunkTree> load(MpqPatchChain& mpq, std::string_view filename,
            std::span<const std::string_view> recognizedTags);

        ChunkTree(ChunkTree const&) = delete;
        ChunkTree& operator=(ChunkTree const&) = delete;
        ChunkTree(ChunkTree&&) = default;
        ChunkTree& operator=(ChunkTree&&) = default;

        // Exactly one top-level chunk with this tag, else nullptr.
        const ChunkNode* find(std::string_view tag) const;
        // Every top-level chunk with this tag, in encounter order.
        std::vector<const ChunkNode*> findAll(std::string_view tag) const;

    private:
        ChunkTree() noexcept;

        static std::vector<RecognizedTag> buildRecognizedTags(std::span<const std::string_view> tags);

        static std::multimap<std::string, std::unique_ptr<ChunkNode>> scanChildren(BinaryReader const& reader,
            size_t nodeStart, uint32_t parentDeclaredSize, std::span<const RecognizedTag> recognizedTags);

        BinaryReader m_reader;
        std::multimap<std::string, std::unique_ptr<ChunkNode>> m_roots;
    };
}
