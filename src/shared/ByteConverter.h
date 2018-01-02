/*
Copyright (c) 2014-2018 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <Common.hpp>
#include <algorithm>

namespace ByteConverter
{
    template<size_t T>
    inline void convert(char* val)
    {
        std::swap(*val, *(val + T - 1));
        convert < T - 2 >(val + 1);
    }

    template<> inline void convert<0>(char*) {}
    template<> inline void convert<1>(char*) {}

    template<typename T>
    inline void apply(T* val) { convert<sizeof(T)>((char*)(val)); }
}

template<typename T> inline void convertEndian(T&) {}
template<typename T> inline void convertEndianReverse(T& val) { ByteConverter::apply<T>(&val); }

template<typename T> void convertEndian(T*);
template<typename T> void convertEndianReverse(T*);

inline void convertEndian(uint8_t&) {}
inline void convertEndian(int8_t&) {}
inline void convertEndianReverse(uint8_t&) {}
inline void convertEndianReverse(int8_t&) {}
