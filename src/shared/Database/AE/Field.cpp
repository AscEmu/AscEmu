/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "../Field.hpp"
#include <charconv>
#include <cstdlib>
#include <limits>
#include <string>
#include <string_view>

namespace
{
    template <typename T>
    T parseUnsigned(std::string_view value, T fallback = 0)
    {
        if (value.empty())
            return fallback;

        std::uint64_t parsed = 0;
        const auto [ptr, ec] = std::from_chars(value.data(), value.data() + value.size(), parsed);
        if (ec != std::errc{} || ptr != value.data() + value.size())
            return fallback;

        if (parsed > static_cast<std::uint64_t>(std::numeric_limits<T>::max()))
            return fallback;

        return static_cast<T>(parsed);
    }

    template <typename T>
    T parseSigned(std::string_view value, T fallback = 0)
    {
        if (value.empty())
            return fallback;

        std::int64_t parsed = 0;
        const auto [ptr, ec] = std::from_chars(value.data(), value.data() + value.size(), parsed);
        if (ec != std::errc{} || ptr != value.data() + value.size())
            return fallback;

        if (parsed < static_cast<std::int64_t>(std::numeric_limits<T>::min()) ||
            parsed > static_cast<std::int64_t>(std::numeric_limits<T>::max()))
            return fallback;

        return static_cast<T>(parsed);
    }

    float parseFloat(std::string_view value)
    {
        if (value.empty())
            return 0.0f;

        std::string owned(value);
        char* end = nullptr;
        const float parsed = std::strtof(owned.c_str(), &end);
        if (end != owned.c_str() + owned.size())
            return 0.0f;

        return parsed;
    }

    bool iequals(std::string_view lhs, std::string_view rhs)
    {
        if (lhs.size() != rhs.size())
            return false;

        for (std::size_t i = 0; i < lhs.size(); ++i)
        {
            const unsigned char a = static_cast<unsigned char>(lhs[i]);
            const unsigned char b = static_cast<unsigned char>(rhs[i]);
            if (std::tolower(a) != std::tolower(b))
                return false;
        }

        return true;
    }
}

bool Field::isSet() const { return m_value != nullptr; }

void Field::setValue(char* value)
{
    m_value = value;
    m_length = 0;
}

void Field::setValue(const char* value, std::size_t length)
{
    m_value = value;
    m_length = value != nullptr ? length : 0;
}

const char* Field::asCString() const { return m_value != nullptr ? m_value : ""; }

std::string_view Field::asStringView() const
{
    if (m_value == nullptr)
        return {};

    if (m_length == 0)
        return std::string_view(m_value);

    return std::string_view(m_value, m_length);
}

float Field::asFloat() const { return parseFloat(asStringView()); }

bool Field::asBool() const
{
    const auto value = asStringView();
    if (value.empty())
        return false;

    if (iequals(value, "true") || iequals(value, "yes"))
        return true;

    if (iequals(value, "false") || iequals(value, "no"))
        return false;

    return asUint32(true) != 0U;
}

uint8_t Field::asUint8(bool) const { return parseUnsigned<uint8_t>(asStringView(), 0U); }
int8_t Field::asInt8() const { return parseSigned<int8_t>(asStringView(), 0); }

uint16_t Field::asUint16(bool) const { return parseUnsigned<uint16_t>(asStringView(), 0U); }
int16_t Field::asInt16() const { return parseSigned<int16_t>(asStringView(), 0); }

uint32_t Field::asUint32(bool) const { return parseUnsigned<uint32_t>(asStringView(), 0U); }
int32_t Field::asInt32() const { return parseSigned<int32_t>(asStringView(), 0); }

uint64_t Field::asUint64(bool) const { return parseUnsigned<uint64_t>(asStringView(), 0ULL); }
int64_t Field::asInt64() const { return parseSigned<int64_t>(asStringView(), 0LL); }

