/*
Copyright (c) 2014-2016 AscEmu Team <http://www.ascemu.org/>
This file is released under the MIT license. See README-MIT for more information.
*/

#ifndef _NEW_BYTE_BUFFER_HPP
#define _NEW_BYTE_BUFFER_HPP

#include "Common.h"
#include "Log.h"
#include "ByteConverter.hpp"
#include "../world/Server/WUtil.h"

#define DEF_BITS_1 uint8 _1
#define DEF_BITS_2 DEF_BITS_1, uint8 _2
#define DEF_BITS_3 DEF_BITS_2, uint8 _3
#define DEF_BITS_4 DEF_BITS_3, uint8 _4
#define DEF_BITS_5 DEF_BITS_4, uint8 _5
#define DEF_BITS_6 DEF_BITS_5, uint8 _6
#define DEF_BITS_7 DEF_BITS_6, uint8 _7
#define DEF_BITS_8 DEF_BITS_7, uint8 _8

#define DEF_BIT_VALS_1 _1
#define DEF_BIT_VALS_2 DEF_BIT_VALS_1, _2
#define DEF_BIT_VALS_3 DEF_BIT_VALS_2, _3
#define DEF_BIT_VALS_4 DEF_BIT_VALS_3, _4
#define DEF_BIT_VALS_5 DEF_BIT_VALS_4, _5
#define DEF_BIT_VALS_6 DEF_BIT_VALS_5, _6
#define DEF_BIT_VALS_7 DEF_BIT_VALS_6, _7
#define DEF_BIT_VALS_8 DEF_BIT_VALS_7, _8

class NewWoWGuid;

class ByteBufferException
{
public:
    ByteBufferException(bool _add, size_t _pos, size_t _esize, size_t _size)
        : add(_add), pos(_pos), esize(_esize), size(_size)
    {
        PrintPosError();
    }

    void PrintPosError() const
    {
        char const* traceStr;

#ifdef HAVE_ACE_STACK_TRACE_H
        ACE_Stack_Trace trace;
        traceStr = trace.c_str();
#else
        traceStr = NULL;
#endif

        sLog.outError(
            "Attempted to %s in ByteBuffer (pos: " SIZEFMTD " size: " SIZEFMTD ") "
            "value with size: " SIZEFMTD "%s%s",
            (add ? "put" : "get"), pos, size, esize,
            traceStr ? "\n" : "", traceStr ? traceStr : "");
    }
private:
    bool add;
    size_t pos;
    size_t esize;
    size_t size;
};

class BitStream
{
public:
    BitStream() : _rpos(0), _wpos(0) {}

    BitStream(uint32 val, size_t len) : _rpos(0), _wpos(0)
    {
        WriteBits(val, len);
    }

    BitStream(BitStream const& bs) : _rpos(bs._rpos), _wpos(bs._wpos), _data(bs._data) {}

    void Clear();
    uint8 GetBit(uint32 bit);
    uint8 ReadBit();
    void WriteBit(uint32 bit);
    template <typename T> void WriteBits(T value, size_t bits);
    bool Empty();
    void Reverse();
    void Print();

    size_t GetLength() { return _data.size(); }
    uint32 GetReadPosition() { return _rpos; }
    uint32 GetWritePosition() { return _wpos; }
    void SetReadPos(uint32 pos) { _rpos = pos; }

    uint8 const& operator[](uint32 const pos) const
    {
        return _data[pos];
    }

    uint8& operator[] (uint32 const pos)
    {
        return _data[pos];
    }

private:
    std::vector<uint8> _data;
    uint32 _rpos, _wpos;
};

template<class T>
struct Unused
{
    Unused() {}
};

class NewByteBuffer
{
    public:

        const static size_t DEFAULT_SIZE = 64;

        NewByteBuffer() : _rpos(0), _wpos(0), _bitpos(8), _curbitval(0) { _storage.reserve(DEFAULT_SIZE); }
        NewByteBuffer(size_t res) : _rpos(0), _wpos(0), _bitpos(8), _curbitval(0) { _storage.reserve(res); }
        NewByteBuffer(const NewByteBuffer &buf) : _rpos(buf._rpos), _wpos(buf._wpos), _storage(buf._storage),
            _bitpos(buf._bitpos), _curbitval(buf._curbitval) { }

        void clear()
        {
            _storage.clear();
            _rpos = 0;
            _wpos = 0;
            _curbitval = 0;
            _bitpos = 8;
        }

        template <typename T> NewByteBuffer& append(T value)
        {
            FlushBits();
            EndianConvert(value);
            return append((uint8*)&value, sizeof(value));
        }

        void FlushBits()
        {
            if (_bitpos == 8)
                return;

            append((uint8 *)&_curbitval, sizeof(uint8));
            _curbitval = 0;
            _bitpos = 8;
        }

        void ResetBitReader()
        {
            _bitpos = 8;
        }

        template <typename T> bool WriteBit(T bit)
        {
            --_bitpos;
            if (bit)
                _curbitval |= (1 << (_bitpos));

            if (_bitpos == 0)
            {
                _bitpos = 8;
                append((uint8 *)&_curbitval, sizeof(_curbitval));
                _curbitval = 0;
            }

            return (bit != 0);
        }

        bool ReadBit()
        {
            ++_bitpos;
            if (_bitpos > 7)
            {
                _curbitval = read<uint8>();
                _bitpos = 0;
            }

            return ((_curbitval >> (7 - _bitpos)) & 1) != 0;
        }

        template <typename T> void WriteBits(T value, size_t bits)
        {
            for (int32 i = bits - 1; i >= 0; --i)
                WriteBit((value >> i) & 1);
        }

        uint32 ReadBits(size_t bits)
        {
            uint32 value = 0;
            for (int32 i = bits - 1; i >= 0; --i)
                if (ReadBit())
                    value |= (1 << i);

            return value;
        }

        BitStream ReadBitStream(uint32 len)
        {
            BitStream b;
            for (uint32 i = 0; i < len; ++i)
                b.WriteBit(ReadBit());
            return b;
        }

        void WriteGuidMask(uint64 guid, uint8* maskOrder, uint8 maskCount, uint8 maskPos = 0)
        {
            uint8* guidByte = ((uint8*)&guid);

            for (uint8 i = 0; i < maskCount; i++)
                WriteBit(guidByte[maskOrder[i + maskPos]]);
        }

        void WriteGuidBytes(uint64 guid, uint8* byteOrder, uint8 byteCount, uint8 bytePos)
        {
            uint8* guidByte = ((uint8*)&guid);

            for (uint8 i = 0; i < byteCount; i++)
                if (guidByte[byteOrder[i + bytePos]])
                    (*this) << uint8(guidByte[byteOrder[i + bytePos]] ^ 1);
        }

        template<DEF_BITS_1>
        void ReadGuidMask(NewWoWGuid& guid);
        template<DEF_BITS_2>
        void ReadGuidMask(NewWoWGuid& guid);
        template<DEF_BITS_3>
        void ReadGuidMask(NewWoWGuid& guid);
        template<DEF_BITS_4>
        void ReadGuidMask(NewWoWGuid& guid);
        template<DEF_BITS_5>
        void ReadGuidMask(NewWoWGuid& guid);
        template<DEF_BITS_6>
        void ReadGuidMask(NewWoWGuid& guid);
        template<DEF_BITS_7>
        void ReadGuidMask(NewWoWGuid& guid);
        template<DEF_BITS_8>
        void ReadGuidMask(NewWoWGuid& guid);

        template<DEF_BITS_1>
        void WriteGuidMask(NewWoWGuid guid);
        template<DEF_BITS_2>
        void WriteGuidMask(NewWoWGuid guid);
        template<DEF_BITS_3>
        void WriteGuidMask(NewWoWGuid guid);
        template<DEF_BITS_4>
        void WriteGuidMask(NewWoWGuid guid);
        template<DEF_BITS_5>
        void WriteGuidMask(NewWoWGuid guid);
        template<DEF_BITS_6>
        void WriteGuidMask(NewWoWGuid guid);
        template<DEF_BITS_7>
        void WriteGuidMask(NewWoWGuid guid);
        template<DEF_BITS_8>
        void WriteGuidMask(NewWoWGuid guid);

        template<DEF_BITS_1>
        void ReadGuidBytes(NewWoWGuid& guid);
        template<DEF_BITS_2>
        void ReadGuidBytes(NewWoWGuid& guid);
        template<DEF_BITS_3>
        void ReadGuidBytes(NewWoWGuid& guid);
        template<DEF_BITS_4>
        void ReadGuidBytes(NewWoWGuid& guid);
        template<DEF_BITS_5>
        void ReadGuidBytes(NewWoWGuid& guid);
        template<DEF_BITS_6>
        void ReadGuidBytes(NewWoWGuid& guid);
        template<DEF_BITS_7>
        void ReadGuidBytes(NewWoWGuid& guid);
        template<DEF_BITS_8>
        void ReadGuidBytes(NewWoWGuid& guid);

        template<DEF_BITS_1>
        void WriteGuidBytes(NewWoWGuid guid);
        template<DEF_BITS_2>
        void WriteGuidBytes(NewWoWGuid guid);
        template<DEF_BITS_3>
        void WriteGuidBytes(NewWoWGuid guid);
        template<DEF_BITS_4>
        void WriteGuidBytes(NewWoWGuid guid);
        template<DEF_BITS_5>
        void WriteGuidBytes(NewWoWGuid guid);
        template<DEF_BITS_6>
        void WriteGuidBytes(NewWoWGuid guid);
        template<DEF_BITS_7>
        void WriteGuidBytes(NewWoWGuid guid);
        template<DEF_BITS_8>
        void WriteGuidBytes(NewWoWGuid guid);

        template <typename T> void put(size_t pos, T value)
        {
            EndianConvert(value);
            put(pos, (uint8 *)&value, sizeof(value));
        }

        NewByteBuffer &operator<<(uint8 value)
        {
            append<uint8>(value);
            return *this;
        }

        NewByteBuffer &operator<<(uint16 value)
        {
            append<uint16>(value);
            return *this;
        }

        NewByteBuffer &operator<<(uint32 value)
        {
            append<uint32>(value);
            return *this;
        }

        NewByteBuffer &operator<<(uint64 value)
        {
            append<uint64>(value);
            return *this;
        }

        NewByteBuffer &operator<<(int8 value)
        {
            append<int8>(value);
            return *this;
        }

        NewByteBuffer &operator<<(int16 value)
        {
            append<int16>(value);
            return *this;
        }

        NewByteBuffer &operator<<(int32 value)
        {
            append<int32>(value);
            return *this;
        }

        NewByteBuffer &operator<<(int64 value)
        {
            append<int64>(value);
            return *this;
        }

        NewByteBuffer &operator<<(float value)
        {
            append<float>(value);
            return *this;
        }

        NewByteBuffer &operator<<(double value)
        {
            append<double>(value);
            return *this;
        }

        NewByteBuffer &operator<<(const std::string &value)
        {
            append((uint8 const *)value.c_str(), value.length());
            append((uint8)0);
            return *this;
        }

        NewByteBuffer &operator<<(const char *str)
        {
            append((uint8 const *)str, str ? strlen(str) : 0);
            append((uint8)0);
            return *this;
        }

        NewByteBuffer &operator >> (bool &value)
        {
            value = read<char>() > 0 ? true : false;
            return *this;
        }

        NewByteBuffer &operator >> (uint8 &value)
        {
            value = read<uint8>();
            return *this;
        }

        NewByteBuffer &operator >> (uint16 &value)
        {
            value = read<uint16>();
            return *this;
        }

        NewByteBuffer &operator >> (uint32 &value)
        {
            value = read<uint32>();
            return *this;
        }

        NewByteBuffer &operator >> (uint64 &value)
        {
            value = read<uint64>();
            return *this;
        }

        NewByteBuffer &operator >> (int8 &value)
        {
            value = read<int8>();
            return *this;
        }

        NewByteBuffer &operator >> (int16 &value)
        {
            value = read<int16>();
            return *this;
        }

        NewByteBuffer &operator >> (int32 &value)
        {
            value = read<int32>();
            return *this;
        }

        NewByteBuffer &operator >> (int64 &value)
        {
            value = read<int64>();
            return *this;
        }

        NewByteBuffer &operator >> (float &value)
        {
            value = read<float>();
            return *this;
        }

        NewByteBuffer &operator >> (double &value)
        {
            value = read<double>();
            return *this;
        }

        NewByteBuffer &operator >> (std::string& value)
        {
            value.clear();
            while (rpos() < size())
            {
                char c = read<char>();
                if (c == 0)
                    break;
                value += c;
            }
            return *this;
        }

        template<class T>
        NewByteBuffer &operator >> (Unused<T> const&)
        {
            return read_skip<T>();
        }

        uint8& operator[](size_t const pos)
        {
            if (pos >= size())
                throw ByteBufferException(false, pos, 1, size());
            return _storage[pos];
        }

        uint8 const& operator[](size_t const pos) const
        {
            if (pos >= size())
                throw ByteBufferException(false, pos, 1, size());
            return _storage[pos];
        }

        size_t rpos() const { return _rpos; }

        size_t rpos(size_t rpos_)
        {
            _rpos = rpos_;
            return _rpos;
        }

        void rfinish()
        {
            _rpos = wpos();
        }

        size_t wpos() const { return _wpos; }

        size_t wpos(size_t wpos_)
        {
            _wpos = wpos_;
            return _wpos;
        }

        template<typename T>
        NewByteBuffer& read_skip()
        {
            read_skip(sizeof(T));
            return *this;
        }

        NewByteBuffer& read_skip(size_t skip)
        {
            ResetBitReader();
            if (_rpos + skip > size())
                throw ByteBufferException(false, _rpos, skip, size());
            _rpos += skip;

            return *this;
        }

        template <typename T> T read()
        {
            ResetBitReader();
            T r = read<T>(_rpos);
            _rpos += sizeof(T);
            return r;
        }

        template <typename T> T read(size_t pos) const
        {
            if (pos + sizeof(T) > size())
                throw ByteBufferException(false, pos, sizeof(T), size());
            T val = *((T const*)&_storage[pos]);
            EndianConvert(val);
            return val;
        }

        NewByteBuffer& read(uint8* dest, size_t len)
        {
            ResetBitReader();
            if (_rpos + len > size())
                throw ByteBufferException(false, _rpos, len, size());
            memcpy(dest, &_storage[_rpos], len);
            _rpos += len;

            return *this;
        }

        uint64 readPackGUID()
        {
            uint64 guid = 0;
            uint8 guidmark = 0;
            (*this) >> guidmark;

            for (int i = 0; i < 8; ++i)
            {
                if (guidmark & (uint8(1) << i))
                {
                    uint8 bit;
                    (*this) >> bit;
                    guid |= (uint64(bit) << (i * 8));
                }
            }

            return guid;
        }

        uint8 ReadUInt8()
        {
            uint8 u = 0;
            (*this) >> u;
            return u;
        }

        uint16 ReadUInt16()
        {
            uint16 u = 0;
            (*this) >> u;
            return u;
        }

        uint32 ReadUInt32()
        {
            uint32 u = 0;
            (*this) >> u;
            return u;
        }

        uint64 ReadUInt64()
        {
            uint64 u = 0;
            (*this) >> u;
            return u;
        }

        int8 ReadInt8()
        {
            int8 u = 0;
            (*this) >> u;
            return u;
        }

        int16 ReadInt16()
        {
            int16 u = 0;
            (*this) >> u;
            return u;
        }

        int32 ReadInt32()
        {
            uint32 u = 0;
            (*this) >> u;
            return u;
        }

        int64 ReadInt64()
        {
            int64 u = 0;
            (*this) >> u;
            return u;
        }

        std::string ReadString()
        {
            std::string s = "";
            (*this) >> s;
            return s;
        }

        std::string ReadString(uint32 count)
        {
            std::string out;
            uint32 start = rpos();
            while (rpos() < size() && rpos() < start + count)
                out += read<char>();

            return out;
        }

        NewByteBuffer& WriteStringData(const std::string& str)
        {
            FlushBits();
            return append((uint8 const*)str.c_str(), str.size());
        }

        bool ReadBoolean()
        {
            uint8 b = 0;
            (*this) >> b;
            return b > 0 ? true : false;
        }

        float ReadSingle()
        {
            float f = 0;
            (*this) >> f;
            return f;
        }

        const uint8 *contents() const { return &_storage[0]; }

        size_t size() const { return _storage.size(); }
        bool empty() const { return _storage.empty(); }

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

        NewByteBuffer& append(const std::string& str)
        {
            return append((uint8 const*)str.c_str(), str.size() + 1);
        }

        NewByteBuffer& append(const char* src, size_t cnt)
        {
            return append((const uint8 *)src, cnt);
        }

        template<class T> NewByteBuffer& append(const T* src, size_t cnt)
        {
            return append((const uint8 *)src, cnt * sizeof(T));
        }

        NewByteBuffer& append(const uint8* src, size_t cnt)
        {
            if (!cnt)
                return *this;

            ARCEMU_ASSERT(size() < 10000000);

            if (_storage.size() < _wpos + cnt)
                _storage.resize(_wpos + cnt);
            memcpy(&_storage[_wpos], src, cnt);
            _wpos += cnt;

            return *this;
        }

        NewByteBuffer& append(const NewByteBuffer& buffer)
        {
            if (buffer.wpos())
                return append(buffer.contents(), buffer.wpos());

            return *this;
        }

        NewByteBuffer& appendPackXYZ(float x, float y, float z)
        {
            uint32 packed = 0;
            packed |= ((int)(x / 0.25f) & 0x7FF);
            packed |= ((int)(y / 0.25f) & 0x7FF) << 11;
            packed |= ((int)(z / 0.25f) & 0x3FF) << 22;
            *this << packed;

            return *this;
        }

        NewByteBuffer& appendPackGUID(uint64 guid)
        {
            uint8 packGUID[8 + 1];
            packGUID[0] = 0;
            size_t size = 1;
            for (uint8 i = 0; guid != 0; ++i)
            {
                if (guid & 0xFF)
                {
                    packGUID[0] |= uint8(1 << i);
                    packGUID[size] = uint8(guid & 0xFF);
                    ++size;
                }

                guid >>= 8;
            }
            return append(packGUID, size);
        }

        void put(size_t pos, const uint8 *src, size_t cnt)
        {
            if (pos + cnt > size())
                throw ByteBufferException(true, pos, cnt, size());
            memcpy(&_storage[pos], src, cnt);
        }

        void print_storage() const
        {
            sLog.outDebug("STORAGE_SIZE: %lu", (unsigned long)size());
            for (uint32 i = 0; i < size(); ++i)
                sLog.outDebug("%u - ", read<uint8>(i));
            sLog.outDebug(" ");
        }

        void textlike() const
        {
            sLog.outDebug("STORAGE_SIZE: %lu", (unsigned long)size());
            for (uint32 i = 0; i < size(); ++i)
                sLog.outDebug("%c", read<uint8>(i));
            sLog.outDebug(" ");
        }

        void hexlike() const
        {
            uint32 j = 1, k = 1;
            sLog.outDebug("STORAGE_SIZE: %lu", (unsigned long)size());

            for (uint32 i = 0; i < size(); ++i)
            {
                if ((i == (j * 8)) && ((i != (k * 16))))
                {
                    if (read<uint8>(i) < 0x10)
                    {
                        sLog.outDebug("| 0%X ", read<uint8>(i));
                    }
                    else
                    {
                        sLog.outDebug("| %X ", read<uint8>(i));
                    }
                    ++j;
                }
                else if (i == (k * 16))
                {
                    if (read<uint8>(i) < 0x10)
                    {
                        sLog.outDebug("\n");

                        sLog.outDebug("0%X ", read<uint8>(i));
                    }
                    else
                    {
                        sLog.outDebug("\n");

                        sLog.outDebug("%X ", read<uint8>(i));
                    }

                    ++k;
                    ++j;
                }
                else
                {
                    if (read<uint8>(i) < 0x10)
                    {
                        sLog.outDebug("0%X ", read<uint8>(i));
                    }
                    else
                    {
                        sLog.outDebug("%X ", read<uint8>(i));
                    }
                }
            }
            sLog.outDebug("\n");
        }

    protected:

        size_t _rpos, _wpos, _bitpos;
        uint8 _curbitval;
        std::vector<uint8> _storage;
};

template <typename T>
inline NewByteBuffer &operator<<(NewByteBuffer &b, std::vector<T> v)
{
    b << (uint32)v.size();
    for (typename std::vector<T>::iterator i = v.begin(); i != v.end(); ++i)
    {
        b << *i;
    }
    return b;
}

template <typename T>
inline NewByteBuffer &operator >> (NewByteBuffer &b, std::vector<T> &v)
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

template <typename T>
inline NewByteBuffer &operator<<(NewByteBuffer &b, std::list<T> v)
{
    b << (uint32)v.size();
    for (typename std::list<T>::iterator i = v.begin(); i != v.end(); ++i)
    {
        b << *i;
    }
    return b;
}

template <typename T>
inline NewByteBuffer &operator >> (NewByteBuffer &b, std::list<T> &v)
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

template <typename K, typename V>
inline NewByteBuffer &operator<<(NewByteBuffer &b, std::map<K, V> &m)
{
    b << (uint32)m.size();
    for (typename std::map<K, V>::iterator i = m.begin(); i != m.end(); ++i)
    {
        b << i->first << i->second;
    }
    return b;
}

template <typename K, typename V>
inline NewByteBuffer &operator >> (NewByteBuffer &b, std::map<K, V> &m)
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

// TODO: Make a ByteBuffer.cpp and move all this inlining to it.
template<> inline std::string NewByteBuffer::read<std::string>()
{
    std::string tmp;
    *this >> tmp;
    return tmp;
}

template<>
inline NewByteBuffer& NewByteBuffer::read_skip<char*>()
{
    std::string temp;
    *this >> temp;

    return *this;
}

template<>
inline NewByteBuffer& NewByteBuffer::read_skip<char const*>()
{
    return read_skip<char*>();
}

template<>
inline NewByteBuffer& NewByteBuffer::read_skip<std::string>()
{
    return read_skip<char*>();
}

class BitConverter
{
    public:

        static uint8 ToUInt8(NewByteBuffer const& buff, size_t start = 0)
        {
            return buff.read<uint8>(start);
        }

        static uint16 ToUInt16(NewByteBuffer const& buff, size_t start = 0)
        {
            return buff.read<uint16>(start);
        }

        static uint32 ToUInt32(NewByteBuffer const& buff, size_t start = 0)
        {
            return buff.read<uint32>(start);
        }

        static uint64 ToUInt64(NewByteBuffer const& buff, size_t start = 0)
        {
            return buff.read<uint64>(start);
        }

        static int16 ToInt16(NewByteBuffer const& buff, size_t start = 0)
        {
            return buff.read<int16>(start);
        }

        static int32 ToInt32(NewByteBuffer const& buff, size_t start = 0)
        {
            return buff.read<int32>(start);
        }

        static int64 ToInt64(NewByteBuffer const& buff, size_t start = 0)
        {
            return buff.read<int64>(start);
        }
};

#endif  //_NEW_BYTE_BUFFER_HPP
