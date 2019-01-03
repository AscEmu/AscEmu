/*
 * AscEmu Framework based on ArcEmu MMORPG Server
 * Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
 * Copyright (C) 2008-2012 ArcEmu Team <http://www.ArcEmu.org/>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 *
 */

#pragma once

#include "CommonTypes.hpp"
#include "CommonHelpers.hpp"
#include "WoWGuid.h"
#include "LocationVector.h"

#include <cstdlib>
#include <string>
#include <vector>
#include <cstring>
#include <cassert>
#include <algorithm>
#include <list>
#include <map>
#include <ctime>

class SERVER_DECL ByteBuffer
{
    bool m_readFailure = false;
public:

    class error {};

    const static size_t DEFAULT_SIZE = 0x1000;

    bool hadReadFailure() const { return m_readFailure; }

        ByteBuffer() : _rpos(0), _wpos(0)
        {
            _storage.reserve(DEFAULT_SIZE);
            _bitpos = 8;
            _curbitval = 0;
        }

        ByteBuffer(size_t res) : _rpos(0), _wpos(0)
        {
            _storage.reserve(res);
            _bitpos = 8;
            _curbitval = 0;
        }

        ByteBuffer(const ByteBuffer & buf) : _rpos(buf._rpos), _wpos(buf._wpos), _bitpos(buf._bitpos), _curbitval(buf._curbitval), _storage(buf._storage)
        {}

        virtual ~ByteBuffer()
        {}

        void clear()
        {
            _storage.clear();
            _rpos = _wpos = 0;
            _bitpos = 8;
            _curbitval = 0;
        }

        template <typename T> void append(T value)
        {
            flushBits();
            append(reinterpret_cast<uint8_t *>(&value), sizeof(value));
        }

        void flushBits()
        {
            if (_bitpos == 8)
                return;

            append(static_cast<uint8_t *>(&_curbitval), sizeof(uint8_t));
            _curbitval = 0;
            _bitpos = 8;
        }

        bool writeBit(uint32_t bit)
        {
            --_bitpos;
            if (bit)
                _curbitval |= (1 << (_bitpos));

            if (_bitpos == 0)
            {
                _bitpos = 8;
                append(static_cast<uint8_t *>(&_curbitval), sizeof(_curbitval));
                _curbitval = 0;
            }

            return (bit != 0);
        }

        template <typename T> void writeBits(T value, size_t bits)
        {
            for (int32 i = static_cast<int32>(bits - 1); i >= 0; --i)
                writeBit((value >> i) & 1);
        }

        void WriteString(std::string const& str)
        {
            if (size_t len = str.length())
                append(str.c_str(), len);
        }

        void appendPackedTime(time_t time)
        {
            tm* lt = localtime(&time);
            append<uint32>((lt->tm_year - 100) << 24 | lt->tm_mon << 20 | (lt->tm_mday - 1) << 14 | lt->tm_wday << 11 | lt->tm_hour << 6 | lt->tm_min);
        }

        std::string ReadString(uint32_t length)
        {
            if (!length)
                return std::string();
            char* buffer = new char[length + 1]();
            read(reinterpret_cast<uint8_t*>(buffer), length);
            std::string retval = buffer;
            delete[] buffer;
            return retval;
        }

        uint8_t readUInt8()
        {
            uint8_t u = 0;
            (*this) >> u;
            return u;
        }

        bool readBit()
        {
            ++_bitpos;
            if (_bitpos > 7)
            {
                _bitpos = 0;
                _curbitval = read<uint8_t>();
            }
            bool bit = ((_curbitval >> (7 - _bitpos)) & 1) != 0;
            return bit;
        }

        void ReadByteMask(uint8_t& b)
        {
            b = readBit() ? 1 : 0;
        }

        void ReadByteSeq(uint8_t& b)
        {
            if (b != 0)
                b ^= read<uint8_t>();
        }

        void WriteByteMask(uint8_t b)
        {
            writeBit(b);
        }

        void WriteByteSeq(uint8_t b)
        {
            if (b != 0)
                append<uint8_t>(b ^ 1);
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

        template <typename T> void put(size_t pos, T value)
        {
            put(pos, reinterpret_cast<uint8_t*>(&value), sizeof(value));
        }

        ByteBuffer& operator << (bool value)
        {
            append<char>(static_cast<char>(value));
            return *this;
        }

        ByteBuffer& operator << (uint8_t value)
        {
            append<uint8_t>(value);
            return *this;
        }

        ByteBuffer& operator << (uint16_t value)
        {
            append<uint16_t>(value);
            return *this;
        }

        ByteBuffer& operator << (uint32_t value)
        {
            append<uint32_t>(value);
            return *this;
        }

        ByteBuffer& operator << (uint64_t value)
        {
            append<uint64_t>(value);
            return *this;
        }

        ByteBuffer& operator << (int8_t value)
        {
            append<int8_t>(value);
            return *this;
        }

        ByteBuffer& operator << (int16_t value)
        {
            append<int16_t>(value);
            return *this;
        }

        ByteBuffer& operator << (int32_t value)
        {
            append<int32_t>(value);
            return *this;
        }

        ByteBuffer& operator << (int64_t value)
        {
            append<int64_t>(value);
            return *this;
        }

        ByteBuffer& operator << (float value)
        {
            append<float>(value);
            return *this;
        }

        ByteBuffer& operator << (double value)
        {
            append<double>(value);
            return *this;
        }

        ByteBuffer& operator << (const std::string& value)
        {
            append((uint8_t*)value.c_str(), value.length());
            append(static_cast<uint8_t>(0));
            return *this;
        }

        ByteBuffer& operator << (const char* str)
        {
            append((uint8_t*)str, strlen(str));
            append(static_cast<uint8_t>(0));
            return *this;
        }

        ByteBuffer& operator << (const WoWGuid& value)
        {
            append<uint8_t>(value.GetNewGuidMask());
            append(const_cast<uint8_t*>(value.GetNewGuid()), value.GetNewGuidLen());
            return *this;
        }

        ByteBuffer& operator >> (bool& value)
        {
            value = read<char>() > 0 ? true : false;
            return *this;
        }

        ByteBuffer& operator >> (uint8_t& value)
        {
            value = read<uint8_t>();
            return *this;
        }
        ByteBuffer& operator >> (uint16_t& value)
        {
            value = read<uint16_t>();
            return *this;
        }

        ByteBuffer& operator >> (uint32_t& value)
        {
            value = read<uint32_t>();
            return *this;
        }

        ByteBuffer& operator >> (uint64_t& value)
        {
            value = read<uint64_t>();
            return *this;
        }

        ByteBuffer& operator >> (int8_t& value)
        {
            value = read<int8_t>();
            return *this;
        }

        ByteBuffer& operator >> (int16_t& value)
        {
            value = read<int16_t>();
            return *this;
        }

        ByteBuffer& operator >> (int32_t& value)
        {
            value = read<int32_t>();
            return *this;
        }

        ByteBuffer& operator >> (int64_t& value)
        {
            value = read<int64_t>();
            return *this;
        }

        ByteBuffer& operator >> (float& value)
        {
            value = read<float>();
            return *this;
        }

        ByteBuffer& operator >> (double& value)
        {
            value = read<double>();
            return *this;
        }

        ByteBuffer& operator >> (std::string& value)
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

        ByteBuffer& operator << (const LocationVector& vec)
        {
            append<float>(vec.x);
            append<float>(vec.y);
            append<float>(vec.z);

            return *this;
        }

        ByteBuffer& operator >> (LocationVector& vec)
        {
            vec.x = read<float>();
            vec.y = read<float>();
            vec.z = read<float>();

            return *this;
        }

        ByteBuffer& operator >> (WoWGuid& value)
        {
            uint8_t mask = read<uint8_t>();
            value.Init(static_cast<uint8_t>(mask));
            for (int i = 0; i < BitCount8(mask); i++)
            {
                uint8_t field = read<uint8_t>();
                value.AppendField(field);
            }
            return *this;
        }

        uint8_t operator[](size_t pos)
        {
            return read<uint8_t>(pos);
        }

        size_t rpos()
        {
            return _rpos;
        }

        size_t rpos(size_t rpos)
        {
            _rpos = rpos;
            return _rpos;
        }

        void rfinish()
        {
            _rpos = wpos();
        }

        size_t wpos()
        {
            return _wpos;
        }

        size_t wpos(size_t wpos)
        {
            _wpos = wpos;
            return _wpos;
        }

        template <typename T> T read()
        {
            T r = read<T>(_rpos);
            _rpos += sizeof(T);
            return r;
        }

        template <typename T> T read(size_t pos)
        {
            if (pos + sizeof(T) > size())
            {
                m_readFailure = true;
                return static_cast<T>(0);
            }
            else
            {
                return *((T*)&_storage[pos]);
            }
        }

        void read(uint8_t* dest, size_t len)
        {
            if (LIKELY(_rpos + len <= size()))
                memcpy(dest, &_storage[_rpos], len);
            else
                memset(dest, 0, len);
            _rpos += len;
        }

        uint32_t ReadPackedTime()
        {
            uint32_t packedDate = read<uint32_t>();
            tm lt = tm();

            lt.tm_min = packedDate & 0x3F;
            lt.tm_hour = (packedDate >> 6) & 0x1F;
            //lt.tm_wday = (packedDate >> 11) & 7;
            lt.tm_mday = ((packedDate >> 14) & 0x3F) + 1;
            lt.tm_mon = (packedDate >> 20) & 0xF;
            lt.tm_year = ((packedDate >> 24) & 0x1F) + 100;

            return uint32_t(mktime(&lt));
        }

        ByteBuffer& ReadPackedTime(uint32_t& time)
        {
            time = ReadPackedTime();
            return *this;
        }

        uint8_t* contents()
        {
            return _storage.data();
        }

        const uint8_t* contents() const
        {
            return &_storage[0];
        }

        inline size_t size() const
        {
            return _storage.size();
        }
        size_t remaining() const { return _storage.size() - _rpos; }

        bool isEmpty() const
        {
            return _storage.empty();
        }

        void resize(size_t newsize)
        {
            _storage.resize(newsize);
            _rpos = 0;
            _wpos = size();
        }

        void reserve(size_t ressize)
        {
            if (ressize > size())
                _storage.reserve(ressize);
        }

        void append(const char* src, size_t cnt)
        {
            return append(reinterpret_cast<const uint8_t*>(src), cnt);
        }

        void append(const uint8_t* src, size_t cnt)
        {
            if (!cnt)
                return;

            if (!src)
                return;

            assert(size() < 10000000);

            if (_storage.size() < _wpos + cnt)
                _storage.resize(_wpos + cnt);

            memcpy(&_storage[_wpos], src, cnt);
            _wpos += cnt;
        }

        void append(const ByteBuffer & buffer)
        {
            if (buffer.size() > 0)
                append(buffer.contents(), buffer.size());
        }

        void appendPackGUID(uint64_t guid)
        {
            size_t mask_position = wpos();
            *this << uint8_t(0);
            for (uint8_t i = 0; i < 8; i++)
            {
                if (guid & 0xFF)
                {
                    _storage[mask_position] |= (1 << i);
                    *this << uint8_t(guid & 0xFF);
                }

                guid >>= 8;
            }
        }

        uint64_t unpackGUID()
        {
            uint64_t guid = 0;
            uint8_t mask;
            uint8_t temp;
            *this >> mask;
            for (uint8_t i = 0; i < 8; ++i)
            {
                if (mask & (1 << i))
                {
                    *this >> temp;
                    guid |= uint64_t(temp << uint64_t(i << 3));
                }
            }
            return guid;
        }

        void put(size_t pos, const uint8_t* src, size_t cnt)
        {
            assert(pos + cnt <= size());
            memcpy(&_storage[pos], src, cnt);
        }

        void hexlike()
        {
            uint32_t j = 1, k = 1;
            printf("STORAGE_SIZE: %u\n", static_cast<unsigned int>(size()));
            for (uint32_t i = 0; i < size(); i++)
            {
                if ((i == (j * 8)) && ((i != (k * 16))))
                {
                    if (read<uint8_t>(i) <= 0x0F)
                    {
                        printf("| 0%X ", read<uint8_t>(i));
                    }
                    else
                    {
                        printf("| %X ", read<uint8_t>(i));
                    }

                    j++;
                }
                else if (i == (k * 16))
                {
                    rpos(rpos() - 16);
                    printf(" | ");

                    for (int x = 0; x < 16; x++)
                    {
                        printf("%c", read<uint8_t>(i - 16 + x));
                    }

                    if (read<uint8_t>(i) <= 0x0F)
                    {
                        printf("\n0%X ", read<uint8_t>(i));
                    }
                    else
                    {
                        printf("\n%X ", read<uint8_t>(i));
                    }

                    k++;
                    j++;
                }
                else
                {
                    if (read<uint8_t>(i) <= 0x0F)
                    {
                        printf("0%X ", read<uint8_t>(i));
                    }
                    else
                    {
                        printf("%X ", read<uint8_t>(i));
                    }
                }
            }
            printf("\n");
        }

        inline void reverse()
        {
            std::reverse(_storage.begin(), _storage.end());
        }

        inline void ResetRead()
        {
            _rpos = 0;
        }

        template<typename T>
        void read_skip()
        {
            read_skip(sizeof(T));
        }

        inline void read_skip(uint32_t byte_count)
        {
            _rpos += byte_count;
        }

    protected:
        size_t _rpos, _wpos, _bitpos;
        uint8_t _curbitval;
        std::vector<uint8_t> _storage;
};

///////////////////////////////////////////////////////////////////////////////

template <typename T> ByteBuffer & operator<<(ByteBuffer & b, std::vector<T> v)
{
    b << reinterpret_cast<uint32>(v.size());
    for (typename std::vector<T>::iterator i = v.begin(); i != v.end(); ++i)
    {
        b << *i;
    }
    return b;
}

template <typename T> ByteBuffer & operator>>(ByteBuffer & b, std::vector<T> &v)
{
    uint32 vsize;
    b >> vsize;
    v.clear();
    while (vsize--)
    {
        T t;
        b >> t;
        v.push_back(t);
    }
    return b;
}

template <typename T> ByteBuffer & operator<<(ByteBuffer & b, std::list<T> v)
{
    b << reinterpret_cast<uint32>(v.size());
    for (typename std::list<T>::iterator i = v.begin(); i != v.end(); ++i)
    {
        b << *i;
    }
    return b;
}

template <typename T> ByteBuffer & operator>>(ByteBuffer & b, std::list<T> &v)
{
    uint32 vsize;
    b >> vsize;
    v.clear();
    while (vsize--)
    {
        T t;
        b >> t;
        v.push_back(t);
    }
    return b;
}

template <typename K, typename V> ByteBuffer & operator<<(ByteBuffer & b, std::map<K, V> &m)
{
    b << reinterpret_cast<uint32>(m.size());
    for (typename std::map<K, V>::iterator i = m.begin(); i != m.end(); ++i)
    {
        b << i->first << i->second;
    }
    return b;
}

template <typename K, typename V> ByteBuffer & operator>>(ByteBuffer & b, std::map<K, V> &m)
{
    uint32 msize;
    b >> msize;
    m.clear();
    while (msize--)
    {
        K k;
        V v;
        b >> k >> v;
        m.insert(make_pair(k, v));
    }
    return b;
}
