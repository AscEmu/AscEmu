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

#include "Threading/ConditionVariable.h"

#ifdef USE_EPOLL
    #define CONFIG_USE_EPOLL
#endif

#ifdef USE_KQUEUE
    #define CONFIG_USE_KQUEUE
#endif

#ifdef USE_POLL
    #define CONFIG_USE_POLL
#endif

#ifndef WIN32
    #include <sys/timeb.h>
#endif


#endif      //_COMMON_H
