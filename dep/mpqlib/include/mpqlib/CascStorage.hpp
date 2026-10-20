/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

// Read access to a CASC client installation (6.x and later), the counterpart of
// MpqPatchChain for the MPQ clients. Wraps the CascLib storage handle: a tool
// opens the client root once and pulls whole files out of it by name (6.x and
// 7.x still address world data by path) or by file data id (the only way for
// 8.1 and later). Every read copies the complete file into memory - the
// consumers (DB2 tables, ADT/WDT chunk trees) need all of it anyway - and is
// serialised through one mutex, so the callers may convert files in parallel
// without worrying about CascLib's own thread safety.

#pragma once

#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <memory>
#include <mutex>
#include <optional>
#include <string_view>
#include <vector>

namespace mpqlib
{
    // CascLib's CASC_LOCALE_* values the tools need, mirrored here so that
    // nothing outside this translation unit has to include CascLib.h.
    //
    // Open the storage with kCascLocaleNone: CascLib then loads the root with
    // the locale the client has installed (the Tags of .build.info). With a
    // wider mask the root lists one entry per locale for every localised
    // file (Map.db2, AreaTable.db2, Spell.db2, ...), CascLib keeps the first
    // one by file data id, and that is usually a locale whose content is not
    // on disk - the file then opens as "not found".
    inline constexpr uint32_t kCascLocaleNone = 0x00000000;
    inline constexpr uint32_t kCascLocaleEnUs = 0x00000002;
    inline constexpr uint32_t kCascLocaleEnGb = 0x00000200;
    inline constexpr uint32_t kCascLocaleAllWow = 0x0001F3F6;
    inline constexpr uint32_t kCascLocaleAll = 0xFFFFFFFF;

    class CascStorage
    {
    public:
        // product is the code name from .build.info ("wow", "wowt", ...) - a
        // client root can hold several products, the name picks one.
        static std::unique_ptr<CascStorage> open(std::filesystem::path const& clientRoot, std::string_view product, uint32_t localeMask);

        ~CascStorage();
        CascStorage(CascStorage const&) = delete;
        CascStorage& operator=(CascStorage const&) = delete;

        // The build number CascLib found in the storage's build config, 0 when unknown.
        uint32_t buildNumber() const;

        std::optional<std::vector<std::byte>> readFile(std::string_view name, uint32_t localeMask = kCascLocaleAllWow) const;
        std::optional<std::vector<std::byte>> readFile(uint32_t fileDataId, uint32_t localeMask = kCascLocaleAllWow) const;

        // Only the first count bytes of a file (fewer when the file is shorter).
        std::optional<std::vector<std::byte>> readFilePrefix(uint32_t fileDataId, size_t count, uint32_t localeMask = kCascLocaleAllWow) const;

        bool hasFile(std::string_view name, uint32_t localeMask = kCascLocaleAllWow) const;
        bool hasFile(uint32_t fileDataId, uint32_t localeMask = kCascLocaleAllWow) const;

        // The raw CascLib storage handle, for the one caller that enumerates
        // the whole storage itself (the DB2 full scan).
        void* nativeHandle() const noexcept { return m_handle; }

        static std::string_view errorText(uint32_t cascError);
        static uint32_t lastError();

    private:
        explicit CascStorage(void* handle) noexcept;

        std::optional<std::vector<std::byte>> readOpenedFile(void* fileHandle) const;

        void* m_handle;
        mutable std::mutex m_mutex;
    };
}
