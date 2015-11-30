/*
 * AscEmu Framework based on ArcEmu MMORPG Server
 * Copyright (C) 2014-2015 AscEmu Team <http://www.ascemu.org>
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

#ifndef _COMMON_TYPES_HPP
#define _COMMON_TYPES_HPP

#include <cstdint>

typedef int64_t int64;
typedef int32_t int32;
typedef int16_t int16;
typedef int8_t int8;
typedef uint64_t uint64;
typedef uint32_t uint32;
typedef uint16_t uint16;
typedef uint8_t uint8;

#ifdef WIN32
#ifndef SCRIPTLIB
#define SERVER_DECL __declspec(dllexport)
#define SCRIPT_DECL __declspec(dllimport)
#else
#define SERVER_DECL __declspec(dllimport)
#define SCRIPT_DECL __declspec(dllexport)
#endif
#define DECL_LOCAL
#elif defined __GNUC__ && __GNUC__ >= 4
#define SERVER_DECL __attribute__((visibility ("default")))
#define SCRIPT_DECL __attribute__((visibility ("default")))
#define DECL_LOCAL __attribute__((visibility ("hidden")))
#else
#define SERVER_DECL
#define SCRIPT_DECL
#define DECL_LOCAL
#endif

#endif  //_COMMON_TYPES_HPP
