/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "mpqlib/MpqCrypto.hpp"

#include <cctype>
#include <cstring>

namespace mpqlib
{
    std::array<uint32_t, 0x500> MpqCryptTable::generate()
    {
        std::array<uint32_t, 0x500> table{};

        uint32_t seed = 0x00100001;

        for (uint32_t index1 = 0; index1 < 0x100; ++index1)
        {
            uint32_t index2 = index1;
            for (uint32_t i = 0; i < 5; ++i, index2 += 0x100)
            {
                seed = (seed * 125 + 3) % 0x2AAAAB;
                const uint32_t temp1 = (seed & 0xFFFF) << 0x10;

                seed = (seed * 125 + 3) % 0x2AAAAB;
                const uint32_t temp2 = (seed & 0xFFFF);

                table[index2] = temp1 | temp2;
            }
        }

        return table;
    }

    const std::array<uint32_t, 0x500>& MpqCryptTable::get()
    {
        static const std::array<uint32_t, 0x500> table = generate();
        return table;
    }

    uint32_t hashString(std::string_view key, MpqHashType hashType)
    {
        const auto& table = MpqCryptTable::get();
        const uint32_t offset = static_cast<uint32_t>(hashType);

        uint32_t seed1 = 0x7FED7FED;
        uint32_t seed2 = 0xEEEEEEEE;

        for (const char c : key)
        {
            const uint32_t ch = static_cast<uint32_t>(std::toupper(static_cast<unsigned char>(c)));
            seed1 = table[offset + ch] ^ (seed1 + seed2);
            seed2 = ch + seed1 + seed2 + (seed2 << 5) + 3;
        }

        return seed1;
    }

    void decryptBlock(void* data, size_t sizeInBytes, uint32_t key)
    {
        const auto& table = MpqCryptTable::get();
        auto* words = static_cast<uint32_t*>(data);

        uint32_t seed2 = 0xEEEEEEEE;

        for (size_t remaining = sizeInBytes; remaining >= 4; remaining -= 4, ++words)
        {
            seed2 += table[0x400 + (key & 0xFF)];
            const uint32_t decrypted = *words ^ (key + seed2);
            key = ((~key << 0x15) + 0x11111111) | (key >> 0x0B);
            seed2 = decrypted + seed2 + (seed2 << 5) + 3;
            *words = decrypted;
        }
    }

    bool detectFileKey(const void* data, size_t sizeInBytes, uint32_t knownFirstValue, uint32_t sectorSize, uint32_t& outKey)
    {
        if (sizeInBytes < 8)
            return false;

        const auto& table = MpqCryptTable::get();
        const auto* words = static_cast<const uint32_t*>(data);

        const uint32_t temp = (words[0] ^ knownFirstValue) - 0xEEEEEEEE;

        for (uint32_t i = 0; i < 0x100; ++i)
        {
            uint32_t seed1 = temp - table[0x400 + i];
            uint32_t seed2 = 0xEEEEEEEE;

            seed2 += table[0x400 + (seed1 & 0xFF)];
            uint32_t decrypted0 = words[0] ^ (seed1 + seed2);

            if (decrypted0 != knownFirstValue)
                continue;

            const uint32_t fileSeed = seed1 + 1;

            seed1 = ((~seed1 << 0x15) + 0x11111111) | (seed1 >> 0x0B);
            seed2 = decrypted0 + seed2 + (seed2 << 5) + 3;
            seed2 += table[0x400 + (seed1 & 0xFF)];
            const uint32_t decrypted1 = words[1] ^ (seed1 + seed2);

            // The second decrypted value is not known exactly, but no compressed
            // sector can be larger than the archive's sector size.
            if ((decrypted1 - decrypted0) <= sectorSize)
            {
                outKey = fileSeed;
                return true;
            }
        }

        return false;
    }
}
