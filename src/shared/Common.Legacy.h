/*
 * AscEmu Framework based on ArcEmu MMORPG Server
 * Copyright (c) 2014-2018 AscEmu Team <http://www.ascemu.org>
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
 */

#ifndef _COMMON_H
#define _COMMON_H

/* Define these if you are creating a repack */
/*
#define REPACK "Moocow's Repack"
#define REPACK_AUTHOR "Trelorn"
#define REPACK_WEBSITE "www.google.com"*/

enum TimeVariables
{
    TIME_SECOND = 1,
    TIME_MINUTE = TIME_SECOND * 60,
    TIME_HOUR   = TIME_MINUTE * 60,
    TIME_DAY    = TIME_HOUR * 24,
    TIME_MONTH  = TIME_DAY * 30,
    TIME_YEAR   = TIME_MONTH * 12
};

enum MsTimeVariables
{
    MSTIME_SECOND   = 1000,
    MSTIME_6SECONDS = MSTIME_SECOND * 6,
    MSTIME_MINUTE   = MSTIME_SECOND * 60,
    MSTIME_HOUR     = MSTIME_MINUTE * 60,
    MSTIME_DAY      = MSTIME_HOUR * 24
};

#include "AscemuServerDefines.hpp"

#include <cstdlib>
#include <cstdio>

#include <cstdarg>
#include <ctime>
#include <cmath>
#include <cerrno>

#if defined( __WIN32__ ) || defined( WIN32 ) || defined( _WIN32 )
    #define WIN32_LEAN_AND_MEAN
    #define NOMINMAX
    #include <windows.h>
    #undef NOMINMAX
#else
    #define MAX_PATH 1024
#endif

#include "Network/NetworkIncludes.hpp"

// current platform and compiler
#define PLATFORM_WIN32 0
#define PLATFORM_UNIX  1
#define PLATFORM_APPLE 2

#define UNIX_FLAVOUR_LINUX 1
#define UNIX_FLAVOUR_BSD 2
#define UNIX_FLAVOUR_OTHER 3
#define UNIX_FLAVOUR_OSX 4

#if defined( __WIN32__ ) || defined( WIN32 ) || defined( _WIN32 )
    #define PLATFORM PLATFORM_WIN32
#elif defined(__APPLE__)
    #define PLATFORM PLATFORM_APPLE
#else
    #define PLATFORM PLATFORM_UNIX
#endif

#define COMPILER_MICROSOFT 0
#define COMPILER_GNU       1
#define COMPILER_CLANG     3

#ifdef _MSC_VER
    #define COMPILER COMPILER_MICROSOFT
#elif defined(__GNUC__)
    #define COMPILER COMPILER_GNU
#elif defined(__clang__)
    #define COMPILER COMPILER_CLANG
#else
    #pragma error "FATAL ERROR: Unknown compiler."
#endif

#if _WIN32
#define PLATFORM_TEXT "Win32"
#elif __APPLE__
    #define PLATFORM_TEXT "OSX"
#elif defined(BSD)
    #define PLATFORM_TEXT "BSD"
#elif defined(__linux__)
    #define PLATFORM_TEXT "Linux"
#endif

#ifdef _DEBUG
    #define CONFIG "Debug"
#else
    #define CONFIG "Release"
#endif

#if defined(__x86_64__) || defined(__x86_64) || defined(__amd64__) || defined(_WIN64)
    #define ARCH "X64"
#else
    #define ARCH "X86"
#endif

#ifdef USE_EPOLL
    #define CONFIG_USE_EPOLL
#endif

#ifdef USE_KQUEUE
    #define CONFIG_USE_KQUEUE
#endif

#ifdef USE_POLL
    #define CONFIG_USE_POLL
#endif

#include <cstdlib>
#include <set>
#include <list>
#include <string>
#include <map>
#include <queue>
#include <sstream>
#include <algorithm>
#include <cstring>
#include <climits>

#include <unordered_map>
#include <unordered_set>

#include "CommonHelpers.hpp"
#include "CommonTypes.hpp"

// Include all threading files
#include <cassert>
#include "Threading/LegacyThreading.h"

#include "Threading/ConditionVariable.h"

#include "CRefcounter.h"

#if COMPILER == COMPILER_MICROSOFT
    #define I64FMT "%016I64X"
    #define I64FMTD "%I64u"
    #define SI64FMTD "%I64d"
    #define snprintf _snprintf
    #define atoll __atoi64
#else
    #define stricmp strcasecmp
    #define strnicmp strncasecmp
    #define I64FMT "%016llX"
    #define I64FMTD "%llu"
    #define SI64FMTD "%lld"
#endif

#define atol(a) strtoul( a, NULL, 10)

#if COMPILER == COMPILER_MICROSOFT
    #define MS_FLOAT_CONTROL
    #pragma float_control(push)
    #pragma float_control(precise, on)
#endif

// fast int abs
static inline int int32abs(const int value)
{
    return (value ^ (value >> 31)) - (value >> 31);
}

// fast int abs and recast to unsigned
static inline uint32 int32abs2uint32(const int value)
{
    return (uint32)(value ^ (value >> 31)) - (value >> 31);
}

/// Fastest Method of float2int32
static inline int float2int32(const float value)
{
#if !defined(_WIN64) && COMPILER == COMPILER_MICROSOFT && !defined(USING_BIG_ENDIAN)
    int i;
    __asm
    {
        fld value
        frndint
        fistp i
    }
    return i;
#else
    union { int asInt[2]; double asDouble; } n;
    n.asDouble = value + 6755399441055744.0;

    return n.asInt [0];
#endif
}

/// Fastest Method of long2int32
static inline int long2int32(const double value)
{
#if !defined(_WIN64) && COMPILER == COMPILER_MICROSOFT && !defined(USING_BIG_ENDIAN)
    int i;
    __asm
    {
        fld value
        frndint
        fistp i
    }
    return i;
#else
    union { int asInt[2]; double asDouble; } n;
    n.asDouble = value + 6755399441055744.0;

    return n.asInt [0];
#endif
}

#ifdef MS_FLOAT_CONTROL
    #pragma float_control(pop)
#endif

#ifndef WIN32
    #include <sys/timeb.h>
#endif

#ifndef WIN32
    #define FALSE 0
    #define TRUE  1
#endif

inline void reverse_array(uint8* pointer, size_t count)
{
    size_t x;
    uint8* temp = (uint8*)malloc(count);
    memcpy(temp, pointer, count);
    for(x = 0; x < count; ++x)
        pointer[x] = temp[count - x - 1];
    free(temp);
}

// returns true if the ip hits the mask, otherwise false
inline static bool ParseCIDRBan(unsigned int IP, unsigned int Mask, unsigned int MaskBits)
{
    // CIDR bans are a compacted form of IP / Submask
    // So 192.168.1.0/255.255.255.0 would be 192.168.1.0/24
    // IP's in the 192.168l.1.x range would be hit, others not.
    unsigned char* source_ip = (unsigned char*)&IP;
    unsigned char* mask = (unsigned char*)&Mask;
    int full_bytes = MaskBits / 8;
    int leftover_bits = MaskBits % 8;
    //int byte;

    // sanity checks for the data first
    if (MaskBits > 32)
        return false;

    // this is the table for comparing leftover bits
    static const unsigned char leftover_bits_compare[9] =
    {
        0x00,            // 00000000
        0x80,            // 10000000
        0xC0,            // 11000000
        0xE0,            // 11100000
        0xF0,            // 11110000
        0xF8,            // 11111000
        0xFC,            // 11111100
        0xFE,            // 11111110
        0xFF,            // 11111111 - This one isn't used
    };

    // if we have any full bytes, compare them with memcpy
    if (full_bytes > 0)
    {
        if (memcmp(source_ip, mask, full_bytes) != 0)
            return false;
    }

    // compare the left over bits
    if (leftover_bits > 0)
    {
        if ((source_ip[full_bytes] & leftover_bits_compare[leftover_bits]) !=
                (mask[full_bytes] & leftover_bits_compare[leftover_bits]))
        {
            // one of the bits does not match
            return false;
        }
    }

    // all of the bits match that were testable
    return true;
}

inline static unsigned int MakeIP(const char* str)
{
    unsigned int bytes[4];
    unsigned int res;
    if (sscanf(str, "%u.%u.%u.%u", &bytes[0], &bytes[1], &bytes[2], &bytes[3]) != 4)
        return 0;

    res = bytes[0] | (bytes[1] << 8) | (bytes[2] << 16) | (bytes[3] << 24);
    return res;
}

#include "DynLib.hpp"
#include "FindFiles.hpp"
#include "SysInfo.hpp"
#include "PerformanceCounter.hpp"

#ifndef EOL
#ifdef WIN32
#define EOL "\r\n"
#else
#define EOL "\n"
#endif
#endif

#ifndef EOL_SIZE
#ifdef WIN32
#define EOL_SIZE 2
#else
#define EOL_SIZE 1
#endif
#endif

#endif      //_COMMON_H
