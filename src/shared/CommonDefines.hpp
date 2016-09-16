/*
Copyright (c) 2014-2016 AscEmu Team <http://www.ascemu.org/>
This file is released under the MIT license. See README-MIT for more information.
*/

#ifndef _COMMON_DEFINES_HPP
#define _COMMON_DEFINES_HPP

// platforms that already define M_PI in math.h
#ifdef M_PI
#undef M_PI
#endif

#define M_PI       3.14159265358979323846
#define M_H_PI     1.57079632679489661923
#define M_Q_PI     0.785398163397448309615
#define M_PI_FLOAT 3.14159f

// time defines

/* Some minor documentation about the time field
    minute's = 0x0000003F   00000000000000000000000000111111
    hour's   = 0x000007C0   00000000000000000000011111000000
    weekdays = 0x00003800   00000000000000000011100000000000
    days     = 0x000FC000   00000000000011111100000000000000
    months   = 0x00F00000   00000000111100000000000000000000
    years    = 0x1F000000   00011111000000000000000000000000
    unk      = 0xE0000000   11100000000000000000000000000000
*/

#define MINUTE_BITMASK      0x0000003F
#define HOUR_BITMASK        0x000007C0
#define WEEKDAY_BITMASK     0x00003800
#define DAY_BITMASK         0x000FC000
#define MONTH_BITMASK       0x00F00000
#define YEAR_BITMASK        0x1F000000
#define UNK_BITMASK         0xE0000000

#define MINUTE_SHIFTMASK    0
#define HOUR_SHIFTMASK      6
#define WEEKDAY_SHIFTMASK   11
#define DAY_SHIFTMASK       14
#define MONTH_SHIFTMASK     20
#define YEAR_SHIFTMASK      24
#define UNK_SHIFTMASK       29

#endif  //_COMMON_DEFINES_HPP
