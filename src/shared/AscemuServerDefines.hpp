/*
 * AscEmu Framework based on ArcEmu MMORPG Server
 * Copyright (C) 2014-2017 AscEmu Team <http://www.ascemu.org>
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

#pragma once

#include "git_version.h"
#include <signal.h>

#ifndef WIN32
#include <sched.h>
#include <sys/resource.h>
#endif

#include "../shared/ascemu_getopt.h"

// Minimum supported version.
#define CLIENT_MIN_BUILD 12340

// Maximum supported version.
#define CLIENT_MAX_BUILD 12340

// Cache supported version.
#define CLIENT_CACHE_VERSION 12340

// Default value is not set.
#define DEF_VALUE_NOT_SET 0xDEADBEEF

// Enable improved ticket system.
#define GM_TICKET_MY_MASTER_COMPATIBLE
