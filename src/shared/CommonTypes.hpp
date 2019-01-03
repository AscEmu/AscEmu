/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <cstdint>

typedef int64_t int64;
typedef int32_t int32;
typedef int16_t int16;
typedef int8_t int8;
typedef uint64_t uint64;
typedef uint32_t uint32;
typedef uint16_t uint16;
typedef uint8_t uint8;

#ifdef _WIN32
    #ifndef SCRIPTLIB
        #define SERVER_DECL __declspec(dllexport)
        #define SCRIPT_DECL __declspec(dllimport)
    #else
        #define SERVER_DECL __declspec(dllimport)
        #define SCRIPT_DECL __declspec(dllexport)
    #endif // SCRIPTLIB
    #define DECL_LOCAL
#elif defined(__GNUC__) || defined(__clang__)
    #define SERVER_DECL __attribute__((visibility ("default")))
    #define SCRIPT_DECL __attribute__((visibility ("default")))
    #define DECL_LOCAL __attribute__((visibility ("hidden")))
#else
    #define SERVER_DECL
    #define SCRIPT_DECL
    #define DECL_LOCAL
#endif  // _WIN32
