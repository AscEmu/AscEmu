/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <cstdint>
#include <cstring>
#include <memory>
#include <optional>
#include <span>
#include <stdexcept>
#include <string_view>
#include <vector>

namespace MapExtractor
{
    class ChunkNode
    {
    public:
        ChunkNode(std::shared_ptr<std::vector<uint8_t>> data, size_t offset, size_t size)
            : m_data(std::move(data)), m_offset(offset), m_size(size) { }

        template <typename T>
        T const& as() const
        {
            if (m_size < sizeof(T))
                throw std::runtime_error("chunk is smaller than requested structure");
            return *reinterpret_cast<T const*>(m_data->data() + m_offset);
        }

        [[nodiscard]] size_t size() const noexcept { return m_size; }
        [[nodiscard]] uint8_t const* data() const noexcept { return m_data->data() + m_offset; }

        [[nodiscard]] ChunkNode const* find(std::string_view tag) const;
        [[nodiscard]] std::vector<ChunkNode const*> findAll(std::string_view tag) const;

    private:
        friend class ChunkTree;
        static bool matches(uint8_t const* p, std::string_view logicalTag);
        void scanChildren(std::span<const std::string_view> recognizedTags);

        std::shared_ptr<std::vector<uint8_t>> m_data;
        size_t m_offset{};
        size_t m_size{};
        std::vector<std::unique_ptr<ChunkNode>> m_children;
    };

    class ChunkTree
    {
    public:
        static std::optional<ChunkTree> load(std::vector<uint8_t> data, std::span<const std::string_view> recognizedTags);

        [[nodiscard]] ChunkNode const* find(std::string_view tag) const;
        [[nodiscard]] std::vector<ChunkNode const*> findAll(std::string_view tag) const;

    private:
        std::shared_ptr<std::vector<uint8_t>> m_data;
        std::vector<std::unique_ptr<ChunkNode>> m_nodes;
    };
}
