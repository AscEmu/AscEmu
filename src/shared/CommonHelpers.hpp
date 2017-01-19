/*
 * AscEmu Framework based on ArcEmu MMORPG Server
 * Copyright (C) 2014-2017 AscEmu Team <http://www.ascemu.org>
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

#ifndef _COMMON_HELPERS_HPP
#define _COMMON_HELPERS_HPP

#if defined ( __GNUC__ ) || defined(__clang__)
#    define LIKELY( _x ) \
        __builtin_expect( ( _x ), 1 )
#    define UNLIKELY( _x ) \
         __builtin_expect( ( _x ), 0 )
#else
#    define LIKELY( _x ) \
        _x
#    define UNLIKELY( _x ) \
        _x
#endif

#endif  //_COMMON_HELPERS_HPPP
