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

#include <cstddef>
#include <cstdint>
#include <map>
#include <memory>
#include <optional>
#include <span>
#include <stdexcept>
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
        // Throws std::runtime_error (rather than assert()) when the node's
        // matched byte span is too small for T: ChunkTree's byte-scan is a
        // heuristic that tolerates non-chunk bytes, so an opportunistic
        // false-positive tag match (a recognized 4-byte tag value occurring
        // by coincidence inside unrelated payload data, e.g. raw height-map
        // floats) is expected, real-world input, not a programmer error -
        // assert() would either silently read out of bounds in a Release
        // build (NDEBUG strips it) or abort the whole process in Debug.
        // Callers are expected to catch this per chunk/tile and skip it.
        template<typename T>
        const T& as() const
        {
            static_assert(std::is_trivially_copyable_v<T>);
            if (sizeof(T) > m_bytes.size())
                throw std::runtime_error("ChunkNode::as<T>(): chunk too small for requested type (need "
                    + std::to_string(sizeof(T)) + " bytes, have " + std::to_string(m_bytes.size()) + ")");
            return *reinterpret_cast<const T*>(m_bytes.data());
        }

        // Exactly one child with this tag, else nullptr (absent or ambiguous).
        const ChunkNode* find(std::string_view tag) const;

        // Total bytes matched for this node (its own [tag][size] header plus
        // payload) - lets a caller sanity-check a chunk's real, scanned size
        // against what it's about to read via as<T>() before doing so,
        // rather than trusting a size field from elsewhere in the file
        // (e.g. a parent chunk's own header) that isn't guaranteed to agree
        // with what's actually on disk at this chunk's own location.
        size_t size() const { return m_bytes.size(); }

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
