/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "mpqlib/ChunkTree.hpp"

#include <algorithm>
#include <cstdint>
#include <cstring>

namespace mpqlib
{
    namespace
    {
        // Single load + integer compare per candidate, matching the cost of
        // the original's plain uint32_t equality check against a lookup
        // table - this runs on every byte position during the scans below
        // (the caller's loop condition already guarantees offset+4 is in
        // range). memcpy rather than a reinterpret_cast read: offset is a
        // data-dependent scan position, not guaranteed 4-byte aligned.
        bool matchRecognizedTag(std::span<const std::byte> bytes, size_t offset,
            std::span<const ChunkTree::RecognizedTag> recognizedTags, std::string_view& matched)
        {
            uint32_t raw;
            std::memcpy(&raw, bytes.data() + offset, sizeof(raw));

            for (auto const& candidate : recognizedTags)
            {
                if (raw == candidate.reversedMagic)
                {
                    matched = candidate.name;
                    return true;
                }
            }

            return false;
        }

        uint32_t readDeclaredSize(std::span<const std::byte> bytes, size_t offset)
        {
            uint32_t size = 0;
            std::memcpy(&size, bytes.data() + offset, sizeof(size));
            return size;
        }
    }

    std::vector<ChunkTree::RecognizedTag> ChunkTree::buildRecognizedTags(std::span<const std::string_view> tags)
    {
        std::vector<RecognizedTag> result;
        result.reserve(tags.size());

        for (std::string_view tag : tags)
        {
            char const reversed[4] = { tag[3], tag[2], tag[1], tag[0] };
            uint32_t magic = 0;
            std::memcpy(&magic, reversed, sizeof(magic));
            result.push_back(RecognizedTag{ tag, magic });
        }

        return result;
    }

    // Scans [nodeStart + 8, nodeStart + parentDeclaredSize) for further
    // recognized chunks nested inside a chunk's own payload - note the upper
    // bound is relative to the *header start*, not the payload end, so the
    // last 8 bytes of a payload are never checked as a potential child
    // header, and a child is only kept if its own declared size is smaller
    // than its parent's. Both quirks come from the original implementation
    // this replaces; preserved here for identical output.
    std::multimap<std::string, std::unique_ptr<ChunkNode>> ChunkTree::scanChildren(BinaryReader const& reader,
        size_t nodeStart, uint32_t parentDeclaredSize, std::span<const RecognizedTag> recognizedTags)
    {
        std::multimap<std::string, std::unique_ptr<ChunkNode>> children;
        auto const bytes = reader.bytes();
        size_t offset = nodeStart + 8;
        size_t const scanEnd = nodeStart + parentDeclaredSize;

        while (offset < scanEnd)
        {
            std::string_view tag;
            if (!matchRecognizedTag(bytes, offset, recognizedTags, tag))
            {
                ++offset;
                continue;
            }

            uint32_t const size = readDeclaredSize(bytes, offset + 4);
            if (size < parentDeclaredSize)
            {
                size_t const available = bytes.size() - offset;
                size_t const nodeLength = std::min<size_t>(8u + size, available);

                auto node = std::unique_ptr<ChunkNode>(new ChunkNode(bytes.subspan(offset, nodeLength)));
                node->m_children = scanChildren(reader, offset, size, recognizedTags);
                children.emplace(std::string(tag), std::move(node));
            }

            offset += 8 + size;
        }

        return children;
    }

    ChunkTree::ChunkTree() noexcept : m_reader(std::vector<std::byte>{})
    {
    }

    std::optional<ChunkTree> ChunkTree::load(MpqPatchChain& mpq, std::string_view filename,
        std::span<const std::string_view> recognizedTags)
    {
        auto reader = BinaryReader::fromMpq(mpq, filename);
        if (!reader)
            return std::nullopt;

        ChunkTree tree;
        tree.m_reader = std::move(*reader);

        auto const tags = buildRecognizedTags(recognizedTags);
        auto const bytes = tree.m_reader.bytes();
        size_t offset = 0;
        while (offset + 8 <= bytes.size())
        {
            std::string_view tag;
            if (!matchRecognizedTag(bytes, offset, tags, tag))
            {
                ++offset;
                continue;
            }

            uint32_t const size = readDeclaredSize(bytes, offset + 4);
            if (size <= bytes.size())
            {
                size_t const available = bytes.size() - offset;
                size_t const nodeLength = std::min<size_t>(8u + size, available);

                auto node = std::unique_ptr<ChunkNode>(new ChunkNode(bytes.subspan(offset, nodeLength)));
                node->m_children = scanChildren(tree.m_reader, offset, size, tags);
                tree.m_roots.emplace(std::string(tag), std::move(node));
            }

            offset += 8 + size;
        }

        return tree;
    }

    const ChunkNode* ChunkNode::find(std::string_view tag) const
    {
        auto range = m_children.equal_range(std::string(tag));
        if (std::distance(range.first, range.second) == 1)
            return range.first->second.get();

        return nullptr;
    }

    const ChunkNode* ChunkTree::find(std::string_view tag) const
    {
        auto range = m_roots.equal_range(std::string(tag));
        if (std::distance(range.first, range.second) == 1)
            return range.first->second.get();

        return nullptr;
    }

    std::vector<const ChunkNode*> ChunkTree::findAll(std::string_view tag) const
    {
        std::vector<const ChunkNode*> result;
        auto range = m_roots.equal_range(std::string(tag));
        for (auto it = range.first; it != range.second; ++it)
            result.push_back(it->second.get());

        return result;
    }
}
