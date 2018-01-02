/*
Copyright (c) 2014-2018 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

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
