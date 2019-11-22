/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

enum School : uint8_t
{
    SCHOOL_NORMAL = 0,  // Physical
    SCHOOL_HOLY   = 1,
    SCHOOL_FIRE   = 2,
    SCHOOL_NATURE = 3,
    SCHOOL_FROST  = 4,
    SCHOOL_SHADOW = 5,
    SCHOOL_ARCANE = 6,
    TOTAL_SPELL_SCHOOLS
};

enum SchoolMask : uint8_t
{
    SCHOOL_MASK_NORMAL = 1,  // Physical
    SCHOOL_MASK_HOLY   = 2,
    SCHOOL_MASK_FIRE   = 4,
    SCHOOL_MASK_NATURE = 8,
    SCHOOL_MASK_FROST  = 16,
    SCHOOL_MASK_SHADOW = 32,
    SCHOOL_MASK_ARCANE = 64
};
