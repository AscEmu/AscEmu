/*
 * AscEmu Framework based on ArcEmu MMORPG Server
 * Copyright (c) 2014-2022 AscEmu Team <http://www.ascemu.org>
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

#define MAX_PATH 1024

#include "git_version.h"
#include <signal.h>

#ifndef WIN32
#include <sched.h>
#include <sys/resource.h>
#endif

#include "DynLib.hpp"
#include "SysInfo.hpp"
#include "PerformanceCounter.hpp"

#include <cstdio>
#include <cstdarg>
#include <ctime>
#include <cmath>
#include <cerrno>

#include "Network/NetworkIncludes.hpp"

// current platform and compiler
#define PLATFORM_WINDOWS 0
#define PLATFORM_UNIX    1
#define PLATFORM_APPLE   2
#define PLATFORM_INTEL   3

#if defined( _WIN64 )
#  define PLATFORM PLATFORM_WINDOWS
#elif defined( __WIN32__ ) || defined( WIN32 ) || defined( _WIN32 )
#  define PLATFORM PLATFORM_WINDOWS
#elif defined( __APPLE_CC__ )
#  define PLATFORM PLATFORM_APPLE
#elif defined( __INTEL_COMPILER )
#  define PLATFORM PLATFORM_INTEL
#else
#  define PLATFORM PLATFORM_UNIX
#endif

#define COMPILER_MICROSOFT 0
#define COMPILER_GNU       1
#define COMPILER_BORLAND   2
#define COMPILER_INTEL     3

#ifdef _MSC_VER
#  define COMPILER COMPILER_MICROSOFT
#elif defined( __BORLANDC__ )
#  define COMPILER COMPILER_BORLAND
#elif defined( __INTEL_COMPILER )
#  define COMPILER COMPILER_INTEL
#elif defined( __GNUC__ )
#  define COMPILER COMPILER_GNU
#else
#  pragma error "FATAL ERROR: Unknown compiler."
#endif

#ifdef WIN32
    #define LIBMASK ".dll";
#else
    #ifndef __APPLE__
        #define LIBMASK ".so";
    #else
        #define LIBMASK ".dylib";
    #endif
#endif

#ifdef _DEBUG
    #define CONFIG "Debug"
#else
    #define CONFIG "Release"
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

#if COMPILER == COMPILER_MICROSOFT
    #define I64FMT "%016I64X"
    #define I64FMTD "%I64u"
    #define SI64FMTD "%I64d"
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

// Fastest Method of float2int32
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

// Fastest Method of long2int32
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

#endif      //_COMMON_H
