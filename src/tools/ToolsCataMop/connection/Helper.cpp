/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Helper.hpp"

#include <stdexcept>

namespace cp
{
    static uint16_t readU16Le(std::span<const uint8_t> _data, size_t _off)
    {
        if (_off + 2 > _data.size())
            throw std::out_of_range("readU16Le");
		
        return static_cast<uint16_t>(_data[_off]) |
            (static_cast<uint16_t>(_data[_off + 1]) << 8);
    }

    static uint32_t readU32Le(std::span<const uint8_t> _data, size_t _off)
    {
        if (_off + 4 > _data.size())
            throw std::out_of_range("readU32Le");
		
        return static_cast<uint32_t>(_data[_off]) |
            (static_cast<uint32_t>(_data[_off + 1]) << 8) |
            (static_cast<uint32_t>(_data[_off + 2]) << 16) |
            (static_cast<uint32_t>(_data[_off + 3]) << 24);
    }

    BinaryType getBinaryType(std::span<const uint8_t> _data)
    {
        if (_data.size() < 4)
            return BinaryType::None;

        const auto mz = readU16Le(_data, 0);

        // "MZ" => PE
        if (mz == 0x5A4D)
        {
            if (_data.size() < 0x3C + 4)
                throw std::runtime_error("Truncated PE header");

            const auto pe_off = readU32Le(_data, 0x3C);
            if (pe_off + 6 > _data.size())
                throw std::runtime_error("Invalid PE offset");

            const auto pe_magic = readU32Le(_data, pe_off);
            if (pe_magic != 0x00004550)
                throw std::runtime_error("Not a PE file");

            const auto machine = readU16Le(_data, pe_off + 4);
            return static_cast<BinaryType>(static_cast<std::uint32_t>(machine));
        }

        // Mach-O (read UInt32 little-endian)
        const auto magic = readU32Le(_data, 0);
        return static_cast<BinaryType>(magic);
    }
} // namespace cp
