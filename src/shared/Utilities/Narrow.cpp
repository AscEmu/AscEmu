/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Utilities/Narrow.hpp"
#include <cmath>

namespace Util
{
    //////////////////////////////////////////////////////////////////////////////////////////
    // Narrowing
    uint32_t MAKE_PAIR32(uint16_t l, uint16_t h)
    {
        return uint32_t(l | (uint32_t(h) << 16));
    }

    int32_t float2int32(float value)
    {
        return static_cast<int32_t>(std::round(value));
    }

    // Fastest Method of long2int32
    int32_t long2int32(const double value)
    {
        return static_cast<int32_t>(std::lround(value));
    }

    //////////////////////////////////////////////////////////////////////////////////////////
    // Narrowing std::string and cstring to specific types
    // bool conversion
    bool stringToBool(const std::string& _str) { return safeStringToSigned<bool>(_str); }
    bool stringToBool(const char* _cstr) { return safeStringToSigned<bool>(_cstr); }

    // uint8_t conversion
    uint8_t stringToUint8(const std::string& _str, bool _silencedError) { return safeStringToUnsigned<uint8_t>(_str, _silencedError); }
    uint8_t stringToUint8(const char* _cstr, bool _silencedError) { return safeStringToUnsigned<uint8_t>(_cstr, _silencedError); }

    // int8_t conversion
    int8_t stringToInt8(const std::string& _str) { return safeStringToSigned<int8_t>(_str); }
    int8_t stringToInt8(const char* _cstr) { return safeStringToSigned<int8_t>(_cstr); }

    // uint16_t conversion
    uint16_t stringToUint16(const std::string& _str, bool _silencedError) { return safeStringToUnsigned<uint16_t>(_str, _silencedError); }
    uint16_t stringToUint16(const char* _cstr, bool _silencedError) { return safeStringToUnsigned<uint16_t>(_cstr, _silencedError); }

    // int16_t conversion
    int16_t stringToInt16(const std::string& _str) { return safeStringToSigned<int16_t>(_str); }
    int16_t stringToInt16(const char* _cstr) { return safeStringToSigned<int16_t>(_cstr); }

    // uint32_t conversion
    uint32_t stringToUint32(const std::string& _str, bool _silencedError) { return safeStringToUnsigned<uint32_t>(_str, _silencedError); }
    uint32_t stringToUint32(const char* _cstr, bool _silencedError) { return safeStringToUnsigned<uint32_t>(_cstr, _silencedError); }

    // int32_t conversion
    int32_t stringToInt32(const std::string& _str) { return safeStringToSigned<int32_t>(_str); }
    int32_t stringToInt32(const char* _cstr) { return safeStringToSigned<int32_t>(_cstr); }

    // uint64_t conversion
    uint64_t stringToUint64(const std::string& _str, bool _silencedError) { return safeStringToUnsigned<uint64_t>(_str, _silencedError); }
    uint64_t stringToUint64(const char* _cstr, bool _silencedError) { return safeStringToUnsigned<uint64_t>(_cstr, _silencedError); }

    // int64_t conversion
    int64_t stringToInt64(const std::string& _str) { return safeStringToSigned<int64_t>(_str); }
    int64_t stringToInt64(const char* cstr) { return safeStringToSigned<int64_t>(cstr); }

    // float conversion
    float stringToFloat(const std::string& _str) { return safeStringToSigned<float>(_str); }
    float stringToFloat(const char* _cstr) { return safeStringToSigned<float>(_cstr); }

    // double conversion
    double stringToDouble(const std::string& _str) { return safeStringToSigned<double>(_str); }
    double stringToDouble(const char* _cstr) { return safeStringToSigned<double>(_cstr); }
}
