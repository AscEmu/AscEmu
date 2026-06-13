/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <cstdint>

    #include <string_view>
    #include <cstddef>

class Field
{
public:
    bool isSet() const;
    void setValue(char* value);
    void setValue(const char* value, std::size_t length);

    const char* asCString() const;
    std::string_view asStringView() const;

    float asFloat() const;
    bool asBool() const;

    uint8_t asUint8(bool _silencedError = false) const;
    int8_t asInt8() const;

    uint16_t asUint16(bool _silencedError = false) const;
    int16_t asInt16() const;

    uint32_t asUint32(bool _silencedError = false) const;
    int32_t asInt32() const;

    uint64_t asUint64(bool _silencedError = false) const;
    int64_t asInt64() const;

private:
    const char* m_value = nullptr;
    std::size_t m_length = 0;
};
