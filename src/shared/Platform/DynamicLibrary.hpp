/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <filesystem>
#include <string>
#include <string_view>
#include <vector>

namespace AscEmu::Platform
{
    class DynamicLibrary final
    {
    public:
        using NativeHandle = void*;

        DynamicLibrary() = default;
        explicit DynamicLibrary(std::filesystem::path path);

        ~DynamicLibrary();

        DynamicLibrary(const DynamicLibrary&) = delete;
        DynamicLibrary& operator=(const DynamicLibrary&) = delete;

        DynamicLibrary(DynamicLibrary&& other) noexcept;
        DynamicLibrary& operator=(DynamicLibrary&& other) noexcept;

        [[nodiscard]] bool load();
        void close() noexcept;

        [[nodiscard]] void* symbolAddress(std::string_view symbolName) noexcept;

        template <typename SymbolType>
        [[nodiscard]] SymbolType symbol(std::string_view symbolName) noexcept
        {
            return reinterpret_cast<SymbolType>(symbolAddress(symbolName));
        }

        [[nodiscard]] bool isLoaded() const noexcept;
        [[nodiscard]] bool hasError() const noexcept;

        [[nodiscard]] const std::filesystem::path& path() const noexcept;
        [[nodiscard]] const std::string& lastError() const noexcept;

        void setPath(std::filesystem::path path);

        [[nodiscard]] static bool hasLibraryExtension(const std::filesystem::path& path);
        [[nodiscard]] static bool isLibraryFile(const std::filesystem::directory_entry& entry);

    private:
        void resetError() noexcept;
        void setError(std::string message);

        [[nodiscard]] static std::string getLastNativeError();

        std::filesystem::path m_path;
        NativeHandle m_handle = nullptr;
        std::string m_lastError;
    };
}