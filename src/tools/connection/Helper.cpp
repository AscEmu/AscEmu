/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Helper.hpp"

#include <algorithm>
#include <cctype>
#include <cstring>
#include <stdexcept>
#include <string>

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

    uint32_t getBuildNumber(std::span<const uint8_t> _data)
    {
        // <Version>6.2.4.21742</Version>: the digits behind the last dot
        static constexpr char marker[] = "<Version>";
        constexpr size_t markerLength = sizeof(marker) - 1;

        for (size_t i = 0; i + markerLength < _data.size(); ++i)
        {
            if (std::memcmp(_data.data() + i, marker, markerLength) != 0)
                continue;

            size_t end = i + markerLength;
            while (end < _data.size() && end - i < 64 && _data[end] != '<')
                ++end;

            std::string version(reinterpret_cast<const char*>(_data.data() + i + markerLength), end - i - markerLength);
            // the manifest pads the version with blanks and a line break
            std::erase_if(version, [](unsigned char c) { return std::isspace(c) != 0; });
            const auto dot = version.rfind('.');
            const std::string build = dot == std::string::npos ? version : version.substr(dot + 1);
            if (build.empty() || build.size() > 6 || !std::all_of(build.begin(), build.end(), [](unsigned char c) { return std::isdigit(c) != 0; }))
                continue;

            return static_cast<uint32_t>(std::stoul(build));
        }

        return 0;
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
