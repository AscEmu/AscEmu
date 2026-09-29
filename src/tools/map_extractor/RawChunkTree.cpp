/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "RawChunkTree.hpp"

#include <algorithm>
#include <array>

namespace MapExtractor
{
    namespace
    {
        uint32_t readU32(uint8_t const* p)
        {
            uint32_t value{};
            std::memcpy(&value, p, sizeof(value));
            return value;
        }
    }

    bool ChunkNode::matches(uint8_t const* p, std::string_view logicalTag)
    {
        if (logicalTag.size() != 4)
            return false;
        return p[0] == static_cast<uint8_t>(logicalTag[3])
            && p[1] == static_cast<uint8_t>(logicalTag[2])
            && p[2] == static_cast<uint8_t>(logicalTag[1])
            && p[3] == static_cast<uint8_t>(logicalTag[0]);
    }

    void ChunkNode::scanChildren(std::span<const std::string_view> recognizedTags)
    {
        // Child chunks inside MCNK are referenced by offsets in the header, but
        // the old extractor already relied on a byte scan. Keep that behaviour,
        // while requiring a complete [fourcc,size,payload] to fit in the parent.
        size_t const begin = m_offset + 8;
        size_t const end = m_offset + m_size;
        if (end <= begin + 8)
            return;

        for (size_t pos = begin; pos + 8 <= end; ++pos)
        {
            bool recognized = false;
            for (std::string_view tag : recognizedTags)
            {
                if (tag == "MCNK")
                    continue;
                if (matches(m_data->data() + pos, tag))
                {
                    recognized = true;
                    break;
                }
            }
            if (!recognized)
                continue;

            uint32_t const payloadSize = readU32(m_data->data() + pos + 4);
            size_t const totalSize = static_cast<size_t>(payloadSize) + 8;
            if (totalSize < 8 || pos + totalSize > end)
                continue;

            m_children.emplace_back(std::make_unique<ChunkNode>(m_data, pos, totalSize));
            pos += totalSize - 1;
        }
    }

    ChunkNode const* ChunkNode::find(std::string_view tag) const
    {
        for (auto const& child : m_children)
            if (matches(child->data(), tag))
                return child.get();
        return nullptr;
    }

    std::vector<ChunkNode const*> ChunkNode::findAll(std::string_view tag) const
    {
        std::vector<ChunkNode const*> result;
        for (auto const& child : m_children)
            if (matches(child->data(), tag))
                result.push_back(child.get());
        return result;
    }

    std::optional<ChunkTree> ChunkTree::load(std::vector<uint8_t> data, std::span<const std::string_view> recognizedTags)
    {
        if (data.size() < 8)
            return std::nullopt;

        ChunkTree tree;
        tree.m_data = std::make_shared<std::vector<uint8_t>>(std::move(data));

        size_t pos = 0;
        while (pos + 8 <= tree.m_data->size())
        {
            uint32_t const payloadSize = readU32(tree.m_data->data() + pos + 4);
            size_t const totalSize = static_cast<size_t>(payloadSize) + 8;
            if (totalSize < 8 || pos + totalSize > tree.m_data->size())
                break;

            bool recognized = false;
            for (std::string_view tag : recognizedTags)
            {
                if (ChunkNode::matches(tree.m_data->data() + pos, tag))
                {
                    recognized = true;
                    break;
                }
            }

            if (recognized)
            {
                auto node = std::make_unique<ChunkNode>(tree.m_data, pos, totalSize);
                node->scanChildren(recognizedTags);
                tree.m_nodes.emplace_back(std::move(node));
            }

            pos += totalSize;
        }

        return tree;
    }

    ChunkNode const* ChunkTree::find(std::string_view tag) const
    {
        for (auto const& node : m_nodes)
            if (ChunkNode::matches(node->data(), tag))
                return node.get();
        return nullptr;
    }

    std::vector<ChunkNode const*> ChunkTree::findAll(std::string_view tag) const
    {
        std::vector<ChunkNode const*> result;
        for (auto const& node : m_nodes)
            if (ChunkNode::matches(node->data(), tag))
                result.push_back(node.get());
        return result;
    }
}
