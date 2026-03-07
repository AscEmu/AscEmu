/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "BinaryTypes.hpp"

#include <filesystem>
#include <span>
#include <vector>
#include <cstdint>
#include <optional>

namespace cp
{
    class Patcher
    {
    public:
        explicit Patcher(std::filesystem::path _file);

        Patcher(const Patcher&) = delete;
        Patcher& operator=(const Patcher&) = delete;

        Patcher(Patcher&&) noexcept = default;
        Patcher& operator=(Patcher&&) noexcept = default;

        ~Patcher() noexcept;

        const std::filesystem::path& binaryPath() const noexcept { return m_binary; }
        void setBinaryPath(std::filesystem::path p) { m_binary = std::move(p); }

        bool getIsInitialized() const noexcept { return m_initialized; }
        BinaryType type() const noexcept { return m_type; }

        // pattern: 0x00 bytes act as wildcards
        void patch(std::span<const uint8_t> _replacement, std::span<const uint8_t> _pattern);

        void finish() noexcept { m_success = true; }

    private:
        std::optional<size_t> searchOffset(std::span<const uint8_t> _pattern) const;

        std::filesystem::path m_binary;

        std::vector<uint8_t> m_data;

        bool m_initialized{false};
        bool m_success{false};

        BinaryType m_type{BinaryType::None};
    };

} // namespace cp
