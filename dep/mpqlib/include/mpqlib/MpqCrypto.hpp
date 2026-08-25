/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

// In-house replacement for StormLib's crypto helpers. Algorithms are Blizzard's
// well-documented MPQ encryption scheme (used identically by every MPQ reader,
// including AscEmu's own dep/libmpq); reimplemented here so ToolsCataMop no
// longer needs to link against the external StormLib project.

#pragma once

#include <array>
#include <cstdint>
#include <string_view>

namespace mpqlib
{
    // Blizzard's 0x500-entry decryption/hash table. Generated once, on first use,
    // via the published generation algorithm (same one AscEmu's dep/libmpq/tools/
    // crypt_buf_gen.c uses to produce its static table).
    class MpqCryptTable
    {
    public:
        static const std::array<uint32_t, 0x500>& get();

    private:
        static std::array<uint32_t, 0x500> generate();
    };

    // Hash keys used to index into the crypt table for different purposes.
    enum class MpqHashType : uint32_t
    {
        TableOffset = 0x000,
        NameA = 0x100,
        NameB = 0x200,
        FileKey = 0x300,
        KeyToCheck = 0x400
    };

    // Blizzard's hash function used for hash-table lookups, table decryption keys,
    // and file decryption keys.
    uint32_t hashString(std::string_view key, MpqHashType hashType);

    // Decrypts `data` (interpreted as an array of little-endian uint32_t) in place.
    void decryptBlock(void* data, size_t sizeInBytes, uint32_t key);

    // Brute-forces the decryption key of a block whose first decrypted uint32_t
    // is known in advance (used for the per-file packed sector-offset table,
    // where the first entry always equals the table's own byte size).
    bool detectFileKey(const void* data, size_t sizeInBytes, uint32_t knownFirstValue, uint32_t sectorSize, uint32_t& outKey);
}
