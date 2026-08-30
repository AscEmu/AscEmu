/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

// Single-archive MPQ reader, replacing StormLib for AscEmu's Cata/Mop tools.
//
// Scope: read-only access via the classic hash table + block table, which is
// what every real Cata/Mop client archive (base archives and wow-update-*
// patch archives alike) still carries alongside the newer HET/BET tables -
// verified byte-for-byte against real 5.8.4 and 4.3.4 client files before
// writing this. HET/BET parsing is intentionally not implemented: none of
// the archives AscEmu's tools need to read require it, and adding it later
// (if a future archive genuinely turns out to be classic-table-less) is a
// self-contained, additive extension of this class.
//
// The classic hash/block tables themselves can be stored compressed on disk
// in format-4 archives (notably the Cache/patch-*.MPQ hotfix archives) - the
// v4 header's HashTableSize64/BlockTableSize64 fields give the real on-disk
// byte count, which is handled transparently in open().

#pragma once

#include <cstdint>
#include <fstream>
#include <memory>
#include <mutex>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace mpqlib
{
    enum MpqFileFlags : uint32_t
    {
        MPQ_FILE_IMPLODE = 0x00000100,     // legacy single-codec PKWARE DCL implode
        MPQ_FILE_COMPRESS = 0x00000200,    // one or more chained codecs, sector-prefixed
        MPQ_FILE_ENCRYPTED = 0x00010000,
        MPQ_FILE_FIX_KEY = 0x00020000,     // decryption key depends on the file's offset in the archive
        MPQ_FILE_SINGLE_UNIT = 0x01000000, // file is stored as a single sector, not split into block-sized sectors
        MPQ_FILE_SECTOR_CRC = 0x04000000,  // sector offset table carries one extra CRC entry
        MPQ_FILE_EXISTS = 0x80000000,
    };

    struct MpqFileInfo
    {
        uint32_t blockIndex = 0;
        uint32_t filePos = 0;
        uint32_t packedSize = 0;
        uint32_t unpackedSize = 0;
        uint32_t flags = 0;
    };

    class MpqArchive
    {
    public:
        explicit MpqArchive(std::string path);
        ~MpqArchive();

        MpqArchive(MpqArchive&&) noexcept;
        MpqArchive& operator=(MpqArchive&&) noexcept;
        MpqArchive(const MpqArchive&) = delete;
        MpqArchive& operator=(const MpqArchive&) = delete;

        // True once a valid MPQ header + hash/block table has been loaded.
        bool isOpen() const { return m_isOpen; }

        const std::string& path() const { return m_path; }

        std::optional<MpqFileInfo> findFile(std::string_view fileName) const;

        // Reads and fully decompresses a file into `out`. Returns false on any
        // I/O, decryption, or decompression failure (out is left unspecified).
        bool readFile(const MpqFileInfo& info, std::vector<uint8_t>& out) const;
        bool readFile(std::string_view fileName, std::vector<uint8_t>& out) const;

        // Lists every file name from the archive's internal "(listfile)", if
        // present. Archives without one (rare for WoW client data) return empty.
        std::vector<std::string> listFiles() const;

    private:
        struct HashEntry
        {
            uint32_t nameHashA;
            uint32_t nameHashB;
            uint16_t locale;
            uint16_t platform;
            uint32_t blockIndex;
        };

        struct BlockEntry
        {
            uint32_t filePos;
            uint32_t packedSize;
            uint32_t unpackedSize;
            uint32_t flags;
        };

        bool open();
        bool readSectorOffsetTable(const MpqFileInfo& info, std::vector<uint32_t>& offsets, uint32_t& fileKey) const;
        bool readMaybeCompressedTable(uint64_t tablePos, uint64_t cmpSize, void* out, size_t naturalSize, const char* keyName) const;

        std::string m_path;
        mutable std::ifstream m_file;
        bool m_isOpen = false;

        // Guards m_file's seek+read sequence in readFile()/readSectorOffsetTable() -
        // multiple threads calling readFile() concurrently on the same archive
        // instance (e.g. several worker threads each converting a different ADT
        // tile out of the same world.MPQ) would otherwise race on the shared
        // stream's position. A unique_ptr rather than a plain std::mutex member
        // because std::mutex isn't movable and MpqPatchChain stores archives in a
        // std::vector that needs to move elements on growth.
        mutable std::unique_ptr<std::mutex> m_fileMutex;

        uint32_t m_hashTableCount = 0;
        uint32_t m_blockTableCount = 0;
        uint32_t m_sectorSize = 0;

        std::vector<HashEntry> m_hashTable;
        std::vector<BlockEntry> m_blockTable;
    };
}
