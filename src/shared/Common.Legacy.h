/*
 * AscEmu Framework based on ArcEmu MMORPG Server
 * Copyright (c) 2014-2024 AscEmu Team <http://www.ascemu.org>
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

// Include all threading files

#include <signal.h>

#ifndef WIN32
#include <sched.h>
#include <sys/resource.h>
#endif

#include "Threading/LegacyThreading.h"
#include "Threading/ConditionVariable.h"

// current compiler (old)
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

#if COMPILER == COMPILER_MICROSOFT
    #define strnicmp _strnicmp
#else
    #define strnicmp strncasecmp
#endif

#if COMPILER == COMPILER_MICROSOFT
    #define MS_FLOAT_CONTROL
    #pragma float_control(push)
    #pragma float_control(precise, on)
#endif


#ifdef MS_FLOAT_CONTROL
    #pragma float_control(pop)
#endif

#ifndef WIN32
    #include <sys/timeb.h>
#endif


#endif      //_COMMON_H
