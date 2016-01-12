/*
 * AscEmu Framework based on ArcEmu MMORPG Server
 * Copyright (C) 2014-2016 AscEmu Team <http://www.ascemu.org>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _ASCEMU_SERVER_DEFINES_HPP
#define _ASCEMU_SERVER_DEFINES_HPP


#include "git_version.h"
#include <signal.h>

#ifndef WIN32
#include <sched.h>
#include <sys/resource.h>
#endif

#include "../shared/ascemu_getopt.h"

#define WORLD_BANNER "<< AscEmu %s/%s-%s (%s) :: World Server >>"
#define LOGON_BANNER "<< AscEmu %s/%s-%s (%s) :: Logon Server >>"

#define DEF_VALUE_NOT_SET 0xDEADBEEF

#ifndef _VERSION
#define _VERSION "3.3.5a"
#endif


/// Use memory mapping for map files for faster access (let OS take care of caching)
/// (currently only available under windows)
/// Only recommended under X64 builds, X86 builds will most likely run out of address space.
/// Default: Disabled
//#define USE_MEMORY_MAPPING_FOR_MAPS


/// Enable/Disable achievement mgr
/// Default: Enabled
/// To disable add // before #define below
#define ENABLE_ACHIEVEMENTS


/// Enable/disable movement compression.
/// This allows the server to compress long-range creatures movement into a buffer and then flush
/// it periodically, compressed with deflate. This can make a large difference to server bandwidth.
/// Currently this sort of compression is only used for player and creature movement, although
/// it may be expanded in the future.
/// Default: disabled
//#define ENABLE_COMPRESSED_MOVEMENT 1
//#define ENABLE_COMPRESSED_MOVEMENT_FOR_PLAYERS 1
//#define ENABLE_COMPRESSED_MOVEMENT_FOR_CREATURES 1

#endif      //_ASCEMU_SERVER_DEFINES_HPP
