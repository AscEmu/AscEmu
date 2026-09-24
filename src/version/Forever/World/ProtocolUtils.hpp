#pragma once

#include <cstddef>
#include <cstdint>
#include <iomanip>
#include <sstream>
#include <string>

namespace AscEmu::Version::Forever
{

    inline std::string bytesToHex(const uint8_t* data, size_t size)
    {
        if (data == nullptr || size == 0)
            return "<empty>";

        std::ostringstream out;
        out << std::hex << std::uppercase << std::setfill('0');
        for (size_t i = 0; i < size; ++i)
        {
            if (i != 0)
                out << ' ';
            out << std::setw(2) << static_cast<unsigned>(data[i]);
        }
        return out.str();
    }
}
