/*
Copyright (c) 2014-2016 AscEmu Team <http://www.ascemu.org/>
This file is released under the MIT license. See README-MIT for more information.
*/

#ifndef BYTECONVERTER_HPP
#define BYTECONVERTER_HPP


#include <Common.h>
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
    template<> inline void convert<1>(char*) {} // igncentral byte

    template<typename T>
    inline void apply(T* val) { convert<sizeof(T)>((char*)(val)); }
}

template<typename T> inline void EndianConvert(T&) {}
template<typename T> inline void EndianConvertReverse(T& val) { ByteConverter::apply<T>(&val); }

template<typename T> void EndianConvert(T*);         // gnr link error
template<typename T> void EndianConvertReverse(T*);  // gnr link error

inline void EndianConvert(uint8&) {}
inline void EndianConvert(int8&) {}
inline void EndianConvertReverse(uint8&) {}
inline void EndianConvertReverse(int8&) {}

#endif  //BYTECONVERTER_HPP

