/*
 * AscEmu Framework based on ArcEmu MMORPG Server
 * Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
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

#define DEF_VALUE_NOT_SET 0xDEADBEEF

#define GM_TICKET_MY_MASTER_COMPATIBLE

#endif      //_ASCEMU_SERVER_DEFINES_HPP
