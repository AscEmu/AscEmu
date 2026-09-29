/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <cstdint>
#include <filesystem>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace MapExtractor::Forever
{
    struct ClientBuildInfo
    {
        std::filesystem::path storageRoot;
        std::string product;
        std::string buildKey;
        uint32_t build{};
    };

    class CascStorage
    {
    public:
        CascStorage() = default;
        ~CascStorage();
        CascStorage(CascStorage const&) = delete;
        CascStorage& operator=(CascStorage const&) = delete;

        bool open(ClientBuildInfo const& info, uint32_t localeMask, std::string& error);
        void close();
        [[nodiscard]] bool isOpen() const noexcept { return m_handle != nullptr; }
        [[nodiscard]] uint32_t build() const noexcept { return m_build; }
        [[nodiscard]] std::string const& product() const noexcept { return m_product; }

        bool readFile(uint32_t fileDataId, std::vector<uint8_t>& data, uint32_t localeMask = 0) const;
        bool extractFile(uint32_t fileDataId, std::filesystem::path const& destination, uint32_t localeMask = 0) const;
        std::vector<uint32_t> enumerateFileDataIds() const;
        bool readDb2Identity(uint32_t fileDataId, uint32_t& tableHash, uint32_t& layoutHash, uint32_t localeMask = 0) const;

    private:
        void* m_handle{};
        uint32_t m_build{};
        std::string m_product;
    };

    std::optional<ClientBuildInfo> detectClient(std::filesystem::path startPath, std::string& error);
}
