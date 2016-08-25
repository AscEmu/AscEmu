/*
Copyright (c) 2014-2016 AscEmu Team <http://www.ascemu.org/>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "NewByteBuffer.hpp"

void BitStream::Clear()
{
    _data.clear();
    _rpos = 0;
    _wpos = 0;
}

uint8 BitStream::GetBit(uint32 bit)
{
    ARCEMU_ASSERT(_data.size() > bit);
    return _data[bit];
}

uint8 BitStream::ReadBit()
{
    ARCEMU_ASSERT(_data.size() < _rpos);
    uint8 b = _data[_rpos];
    ++_rpos;
    return b;
}

void BitStream::WriteBit(uint32 bit)
{
    _data.push_back(bit ? uint8(1) : uint8(0));
    ++_wpos;
}

template <typename T> void BitStream::WriteBits(T value, size_t bits)
{
    for (int32 i = bits - 1; i >= 0; --i)
        WriteBit((value >> i) & 1);
}

bool BitStream::Empty()
{
    return _data.empty();
}

void BitStream::Reverse()
{
    uint32 len = GetLength();
    std::vector<uint8> b = _data;
    Clear();

    for (uint32 i = len; i > 0; --i)
        WriteBit(b[i - 1]);
}

void BitStream::Print()
{
    std::stringstream ss;
    ss << "BitStream: ";
    for (uint32 i = 0; i < GetLength(); ++i)
        ss << uint32(GetBit(i)) << " ";

    sLog.outDebug("%s", ss.str().c_str());
}

