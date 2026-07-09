/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "Platform/SymbolVisibility.hpp"
#include "WoWGuid.hpp"
#include "Utilities/LocationVector.hpp"

#include <cstdlib>
#include <memory>
#include <string>
#include <vector>
#include <cstring>
#include <cassert>
#include <algorithm>
#include <list>
#include <map>
#include <ctime>
#include <limits>
#include <type_traits>
#include <utility>
#include <concepts>
#include <array>
#include <bit>

namespace byteBufferDetail
{
    template <typename T>
    inline constexpr bool isBinarySerializable = std::is_trivially_copyable_v<T> && !std::is_pointer_v<T>;

    [[nodiscard]] inline uint32_t checkedContainerSize(size_t value)
    {
        assert(value <= static_cast<size_t>(std::numeric_limits<uint32_t>::max()));
        return static_cast<uint32_t>(value);
    }

    template <typename T>
    using Decayed = std::remove_cv_t<std::remove_reference_t<T>>;

    template <typename T>
    inline constexpr bool isPacketScalar =
        std::same_as<Decayed<T>, uint8_t> || std::same_as<Decayed<T>, uint16_t> ||
        std::same_as<Decayed<T>, uint32_t> || std::same_as<Decayed<T>, uint64_t> ||
        std::same_as<Decayed<T>, int8_t> || std::same_as<Decayed<T>, int16_t> ||
        std::same_as<Decayed<T>, int32_t> || std::same_as<Decayed<T>, int64_t> ||
        std::same_as<Decayed<T>, float> || std::same_as<Decayed<T>, double>;

    template <typename T>
    concept PacketScalar = isPacketScalar<T>;

    [[nodiscard]] inline tm toLocalTime(std::time_t timeValue)
    {
        tm result{};
#if defined(_WIN32)
        localtime_s(&result, &timeValue);
#else
        localtime_r(&timeValue, &result);
#endif
        return result;
    }
}

class SERVER_DECL ByteBuffer
{
public:
    ByteBuffer() : m_readPosition(0), m_writePosition(0), m_bitPosition(8), m_currentBitValue(0)
    {
        m_vectorStorage.reserve(defaultSize);
    }

    ByteBuffer(size_t res) : m_readPosition(0), m_writePosition(0), m_bitPosition(8), m_currentBitValue(0)
    {
        m_vectorStorage.reserve(res);
    }

    ByteBuffer(const ByteBuffer& other) : m_readFailure(other.m_readFailure), m_readPosition(other.m_readPosition),
        m_writePosition(other.m_writePosition), m_bitPosition(other.m_bitPosition),
        m_currentBitValue(other.m_currentBitValue), m_vectorStorage(other.m_vectorStorage)
    {}

    virtual ~ByteBuffer() = default;

    ///////////////////////////////////////////////////////////////////////////////
    // helper functions
    void clear()
    {
        m_vectorStorage.clear();
        m_readPosition = 0;
        m_writePosition = 0;
        m_bitPosition = 8;
        m_currentBitValue = 0;
        m_readFailure = false;
    }

    inline void reverse() { std::reverse(m_vectorStorage.begin(), m_vectorStorage.end()); }

    bool isEmpty() const { return m_vectorStorage.empty(); }

    void resize(size_t newsize)
    {
        m_vectorStorage.resize(newsize);
        m_readPosition = 0;
        m_writePosition = size();
    }

    void reserve(size_t ressize)
    {
        if (ressize > m_vectorStorage.capacity())
            m_vectorStorage.reserve(ressize);
    }

    uint8_t* contents() { return m_vectorStorage.data(); }

    const uint8_t* contents() const { return m_vectorStorage.data(); }

    inline size_t size() const { return m_vectorStorage.size(); }

    size_t remaining() const
    {
        return m_readPosition <= m_vectorStorage.size() ? (m_vectorStorage.size() - m_readPosition) : 0;
    }

    size_t rpos() { return m_readPosition; }

    size_t rpos(size_t rpos)
    {
        m_readPosition = rpos;
        return m_readPosition;
    }

    void rfinish() {  m_readPosition = wpos(); }

    size_t wpos() { return m_writePosition; }

    size_t wpos(size_t wpos)
    {
        m_writePosition = wpos;
        return m_writePosition;
    }

    size_t bitwpos() const { return m_writePosition * 8 + 8 - m_bitPosition; }

    size_t bitwpos(size_t newPos)
    {
        m_writePosition = newPos / 8;
        m_bitPosition = 8 - (newPos % 8);
        return m_writePosition * 8 + 8 - m_bitPosition;
    }

    bool hadReadFailure() const { return m_readFailure; }

    ///////////////////////////////////////////////////////////////////////////////
    // error handling
    class error {};

    ///////////////////////////////////////////////////////////////////////////////
    // write functions

    template <typename T>
    void append(T value)
    {
        static_assert(byteBufferDetail::isBinarySerializable<T>, "ByteBuffer::append(T) requires a trivially copyable type");

        flushBits();
        append(reinterpret_cast<const uint8_t*>(std::addressof(value)), sizeof(T));
    }

    template <byteBufferDetail::PacketScalar T>
    ByteBuffer& operator<<(T value)
    {
        using ValueType = byteBufferDetail::Decayed<T>;
        append<ValueType>(static_cast<ValueType>(value));
        return *this;
    }

    ByteBuffer& operator<<(const LocationVector& vec)
    {
        append<float>(vec.x);
        append<float>(vec.y);
        append<float>(vec.z);

        return *this;
    }

    void append(const WoWGuid& value)
    {
        flushBits();
        append<uint8_t>(value.getNewGuidMask());
        append(value.getNewGuid(), value.getNewGuidLen());
    }

    ByteBuffer& operator<<(const WoWGuid& value)
    {
        append(value);
        return *this;
    }

    void append(const char* src, size_t cnt)
    {
        return append(reinterpret_cast<const uint8_t*>(src), cnt);
    }

    template <class T>
    void append(const T* src, size_t cnt)
    {
        if (src == nullptr || cnt == 0)
            return;

        append(reinterpret_cast<const uint8_t*>(src), cnt * sizeof(T));
    }

    void append(const uint8_t* src, size_t cnt)
    {
        if (cnt == 0 || src == nullptr)
        {
            return;
        }

        assert(size() < 10000000);

        if (m_writePosition > std::numeric_limits<size_t>::max() - cnt)
        {
            throw error{};
        }

        const size_t requiredSize = m_writePosition + cnt;
        if (m_vectorStorage.size() < requiredSize)
        {
            m_vectorStorage.resize(requiredSize);
        }

        std::memcpy(m_vectorStorage.data() + m_writePosition, src, cnt);
        m_writePosition = requiredSize;
    }

    void append(const ByteBuffer& buffer)
    {
        if (buffer.size() > 0)
            append(buffer.contents(), buffer.size());
    }

    void appendCString(const char* str, size_t length)
    {
        if (length != 0)
            append(reinterpret_cast<const uint8_t*>(str), length);

        append(static_cast<uint8_t>(0));
    }

    ByteBuffer& operator<<(const std::string& value)
    {
        appendCString(value.c_str(), value.length());
        return *this;
    }

    ByteBuffer& operator<<(const char* str)
    {
        if (str == nullptr)
        {
            append(static_cast<uint8_t>(0));
            return *this;
        }

        appendCString(str, std::strlen(str));
        return *this;
    }

    void writeString(const std::string& value)
    {
        if (!value.empty())
            append(reinterpret_cast<const uint8_t*>(value.data()), value.size());
    }

    void appendPackXYZ(float x, float y, float z)
    {
        uint32_t packed = 0;
        packed |= ((int)(x / 0.25f) & 0x7FF);
        packed |= ((int)(y / 0.25f) & 0x7FF) << 11;
        packed |= ((int)(z / 0.25f) & 0x3FF) << 22;
        *this << packed;
    }

    void appendPackedTime(std::time_t timeValue)
    {
        const tm localTime = byteBufferDetail::toLocalTime(timeValue);

        append<uint32_t>(
            ((static_cast<uint32_t>(localTime.tm_year - 100) & 0x1Fu) << 24u) |
            ((static_cast<uint32_t>(localTime.tm_mon) & 0x0Fu) << 20u) |
            ((static_cast<uint32_t>(localTime.tm_mday - 1) & 0x3Fu) << 14u) |
            ((static_cast<uint32_t>(localTime.tm_wday) & 0x07u) << 11u) |
            ((static_cast<uint32_t>(localTime.tm_hour) & 0x1Fu) << 6u) |
            (static_cast<uint32_t>(localTime.tm_min) & 0x3Fu));
    }

    void appendPackGuid(uint64_t guid)
    {
        const size_t maskPosition = wpos();
        *this << static_cast<uint8_t>(0);

        for (uint8_t i = 0; i < 8; ++i)
        {
            if ((guid & 0xFFu) != 0)
            {
                m_vectorStorage[maskPosition] |= static_cast<uint8_t>(1u << i);
                *this << static_cast<uint8_t>(guid & 0xFFu);
            }

            guid >>= 8u;
        }
    }

    bool writeBit(uint32_t bit)
    {
        --m_bitPosition;
        if (bit)
            m_currentBitValue |= (1 << (m_bitPosition));

        if (m_bitPosition == 0)
        {
            m_bitPosition = 8;
            append(static_cast<uint8_t*>(&m_currentBitValue), sizeof(m_currentBitValue));
            m_currentBitValue = 0;
        }

        return (bit != 0);
    }

    template <typename T>
    void writeBits(T value, size_t bits)
    {
        for (int32_t i = static_cast<int32_t>(bits - 1); i >= 0; --i)
            writeBit((value >> i) & 1);
    }

    void writeByteMask(uint8_t b)
    {
        writeBit(b);
    }

    void writeByteSeq(uint8_t b)
    {
        if (b != 0)
            append<uint8_t>(b ^ 1);
    }

    void flushBits()
    {
        if (m_bitPosition == 8)
            return;

        append(&m_currentBitValue, sizeof(m_currentBitValue));
        m_currentBitValue = 0;
        m_bitPosition = 8;
    }

    ///////////////////////////////////////////////////////////////////////////////
    // read functions

    template <typename T>
    T read()
    {
        static_assert(byteBufferDetail::isBinarySerializable<T>, "ByteBuffer::read<T>() requires a trivially copyable type");

        const T value = read<T>(m_readPosition);
        m_readPosition += sizeof(T);
        return value;
    }

    template <byteBufferDetail::PacketScalar T>
    ByteBuffer& operator>>(T& value)
    {
        using ValueType = byteBufferDetail::Decayed<T>;
        value = static_cast<T>(read<ValueType>());
        return *this;
    }

    ByteBuffer& operator>>(std::string& value)
    {
        value.clear();
        while (true)
        {
            char c = read<char>();
            if (c == 0)
                break;
            value += c;
        }
        return *this;
    }

    ByteBuffer& operator>>(LocationVector& vec)
    {
        vec.x = read<float>();
        vec.y = read<float>();
        vec.z = read<float>();

        return *this;
    }

    ByteBuffer& operator>>(WoWGuid& _guid)
    {
        uint8_t mask = read<uint8_t>();
        _guid.init(static_cast<uint8_t>(mask));
        for (int i = 0; i < BitCount8(mask); i++)
        {
            uint8_t field = read<uint8_t>();
            _guid.appendField(field);
        }
        return *this;
    }

    template <typename T>
    T read(size_t pos)
    {
        static_assert(byteBufferDetail::isBinarySerializable<T>, "ByteBuffer::read<T>(size_t) requires a trivially copyable type");

        if (pos > size() || sizeof(T) > size() - pos)
        {
            m_readFailure = true;
            return T{};
        }

        T value{};
        std::memcpy(std::addressof(value), m_vectorStorage.data() + pos, sizeof(T));
        return value;
    }

    void read(uint8_t* dest, size_t len)
    {
        if (len == 0)
            return;

        if (dest == nullptr)
        {
            m_readFailure = true;
            m_readPosition += len;
            return;
        }

        if (m_readPosition <= size() && len <= size() - m_readPosition) [[likely]]
        {
            std::memcpy(dest, m_vectorStorage.data() + m_readPosition, len);
        }
        else [[unlikely]]
        {
            m_readFailure = true;
            std::memset(dest, 0, len);
        }

        m_readPosition += len;
    }

    [[nodiscard]] uint32_t readPackedTime()
    {
        const uint32_t packedDate = read<uint32_t>();

        tm localTime{};
        localTime.tm_min = packedDate & 0x3Fu;
        localTime.tm_hour = (packedDate >> 6u) & 0x1Fu;
        localTime.tm_mday = ((packedDate >> 14u) & 0x3Fu) + 1;
        localTime.tm_mon = (packedDate >> 20u) & 0x0Fu;
        localTime.tm_year = ((packedDate >> 24u) & 0x1Fu) + 100;
        localTime.tm_isdst = -1;

        return static_cast<uint32_t>(std::mktime(&localTime));
    }

    ByteBuffer& readPackedTime(uint32_t& value)
    {
        value = readPackedTime();
        return *this;
    }

    std::string readString(uint32_t length)
    {
        if (length == 0)
            return {};

        std::string value(length, '\0');
        read(reinterpret_cast<uint8_t*>(value.data()), length);
        return value;
    }

    [[nodiscard]] uint64_t unpackGuid()
    {
        uint64_t guid = 0;
        uint8_t mask = 0;
        uint8_t fieldValue = 0;

        *this >> mask;

        for (uint8_t i = 0; i < 8; ++i)
        {
            if ((mask & (1u << i)) != 0)
            {
                *this >> fieldValue;
                guid |= (static_cast<uint64_t>(fieldValue) << static_cast<uint64_t>(i * 8u));
            }
        }

        return guid;
    }

    uint8_t readUInt8()
    {
        uint8_t u = 0;
        (*this) >> u;
        return u;
    }

    bool readBit()
    {
        ++m_bitPosition;
        if (m_bitPosition > 7)
        {
            m_bitPosition = 0;
            m_currentBitValue = read<uint8_t>();
        }
        bool bit = ((m_currentBitValue >> (7 - m_bitPosition)) & 1) != 0;
        return bit;
    }

    void readByteMask(uint8_t& b)
    {
        b = readBit() ? 1 : 0;
    }

    void readByteSeq(uint8_t& b)
    {
        if (b != 0)
            b ^= read<uint8_t>();
    }

    uint32_t readBits(int32_t bits)
    {
        uint32_t value = 0;
        for (int32_t i = bits - 1; i >= 0; --i)
        {
            if (readBit())
            {
                value |= (1 << i);
            }
        }
        return value;
    }

    ByteBuffer& operator<<(bool value)
    {
        append<char>(static_cast<char>(value));
        return *this;
    }

    ByteBuffer& operator>>(bool& value)
    {
        value = read<char>() > 0 ? true : false;
        return *this;
    }

    void hexlike()
    {
        uint32_t j = 1, k = 1;

        fmt::println("STORAGE_SIZE: {}", static_cast<unsigned int>(size()));

        for (uint32_t i = 0; i < size(); i++)
        {
            if ((i == (j * 8)) && ((i != (k * 16))))
            {
                if (read<uint8_t>(i) <= 0x0F)
                    fmt::print("| 0{:X} ", read<uint8_t>(i));
                else
                    fmt::print("| {:X} ", read<uint8_t>(i));

                ++j;
            }
            else if (i == (k * 16))
            {
                rpos(rpos() - 16);
                fmt::print(" | ");

                for (int x = 0; x < 16; x++)
                    fmt::print("{}", static_cast<char>(read<uint8_t>(i - 16 + x)));

                if (read<uint8_t>(i) <= 0x0F)
                    fmt::print("\n0{:X} ", read<uint8_t>(i));
                else
                    fmt::print("\n{:X} ", read<uint8_t>(i));

                ++k;
                ++j;
            }
            else
            {
                if (read<uint8_t>(i) <= 0x0F)
                    fmt::print("0{:X} ", read<uint8_t>(i));
                else
                    fmt::print("{:X} ", read<uint8_t>(i));
            }
        }

        fmt::println("");
    }

    inline void resetRead()
    {
        m_readPosition = 0;
        m_readFailure = false;
    }

    template <typename T>
    void readSkip()
    {
        readSkip(static_cast<uint32_t>(sizeof(T)));
    }

    inline void readSkip(uint32_t byteCount)
    {
        if (byteCount > remaining())
        {
            m_readFailure = true;
            m_readPosition = size();
            return;
        }

        m_readPosition += byteCount;
    }

    uint8_t operator[](size_t pos) { return read<uint8_t>(pos); }

    void put(size_t pos, const uint8_t* src, size_t cnt)
    {
        if (cnt == 0)
            return;

        assert(src != nullptr);
        assert(pos <= size());
        assert(cnt <= size() - pos);

        if (src == nullptr || pos > size() || cnt > size() - pos)
            return;

        std::memcpy(m_vectorStorage.data() + pos, src, cnt);
    }

    template <typename T>
    void put(size_t pos, T value)
    {
        static_assert(byteBufferDetail::isBinarySerializable<T>, "ByteBuffer::put(size_t, T) requires a trivially copyable type");
        put(pos, reinterpret_cast<const uint8_t*>(std::addressof(value)), sizeof(T));
    }

    template <typename T>
    void putBits(size_t bitPosition, T value, uint32_t bitCount)
    {
        for (uint32_t i = 0; i < bitCount; ++i)
        {
            const size_t writeBytePosition = (bitPosition + i) / 8;
            const size_t writeBitPosition = (bitPosition + i) % 8;

            if ((value >> (bitCount - i - 1)) & 1)
                m_vectorStorage[writeBytePosition] |= static_cast<uint8_t>(1u << (7u - writeBitPosition));
            else
                m_vectorStorage[writeBytePosition] &= static_cast<uint8_t>(~(1u << (7u - writeBitPosition)));
        }
    }

private:
    bool m_readFailure = false;
    static constexpr size_t defaultSize = 0x1000;

protected:
    size_t m_readPosition;
    size_t m_writePosition;
    size_t m_bitPosition;
    uint8_t m_currentBitValue;
    std::vector<uint8_t> m_vectorStorage;
};

///////////////////////////////////////////////////////////////////////////////

template <typename T>
ByteBuffer& operator<<(ByteBuffer& b, const std::vector<T>& v)
{
    b << byteBufferDetail::checkedContainerSize(v.size());

    for (const auto& value : v)
        b << value;

    return b;
}

template <typename T>
ByteBuffer& operator>>(ByteBuffer& b, std::vector<T>& v)
{
    uint32_t vsize = 0;
    b >> vsize;

    v.clear();
    v.reserve(vsize);

    while (vsize-- > 0)
    {
        T value{};
        b >> value;
        v.push_back(std::move(value));
    }

    return b;
}

template <typename T>
ByteBuffer& operator<<(ByteBuffer& b, const std::list<T>& v)
{
    b << byteBufferDetail::checkedContainerSize(v.size());

    for (const auto& value : v)
        b << value;

    return b;
}

template <typename T>
ByteBuffer& operator>>(ByteBuffer& b, std::list<T>& v)
{
    uint32_t vsize = 0;
    b >> vsize;

    v.clear();

    while (vsize-- > 0)
    {
        T value{};
        b >> value;
        v.push_back(std::move(value));
    }

    return b;
}

template <typename K, typename V>
ByteBuffer& operator<<(ByteBuffer& b, const std::map<K, V>& m)
{
    b << byteBufferDetail::checkedContainerSize(m.size());

    for (const auto& [key, value] : m)
        b << key << value;

    return b;
}

template <typename K, typename V>
ByteBuffer& operator>>(ByteBuffer& b, std::map<K, V>& m)
{
    uint32_t msize = 0;
    b >> msize;

    m.clear();

    while (msize-- > 0)
    {
        K key{};
        V value{};
        b >> key >> value;
        m.emplace(std::move(key), std::move(value));
    }

    return b;
}

namespace
{
    [[nodiscard]] inline auto packFastGuid(std::uint64_t oldGuid)
        -> std::pair<std::array<std::uint8_t, 9>, std::uint32_t>
    {
        std::array<std::uint8_t, 9> packed{};
        const auto bytes = std::bit_cast<std::array<std::uint8_t, 8>>(oldGuid);

        std::uint8_t guidMask = 0;
        std::uint32_t outSize = 1;

        for (std::size_t i = 0; i < bytes.size(); ++i)
        {
            if (bytes[i] == 0)
                continue;

            packed[outSize++] = bytes[i];
            guidMask |= static_cast<std::uint8_t>(1u << i);
        }

        packed[0] = guidMask;
        return {packed, outSize};
    }
}

static inline void FastGUIDPack(ByteBuffer& buf, const std::uint64_t& oldguid)
{
    const auto [packed, packedSize] = packFastGuid(oldguid);
    buf.append(packed.data(), packedSize);
}

// Warning: this presumes that all GUIDs can be compressed to at least 1 byte.
static inline unsigned int FastGUIDPack(const std::uint64_t& oldguid, unsigned char* buffer, std::uint32_t pos)
{
    const auto [packed, packedSize] = packFastGuid(oldguid);
    std::memcpy(buffer + pos, packed.data(), packedSize);
    return packedSize;
}
