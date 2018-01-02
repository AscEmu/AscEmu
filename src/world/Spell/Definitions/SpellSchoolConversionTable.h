/*
Copyright (c) 2014-2018 AscEmu Team <http://www.ascemu.org/>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once
#include "School.h"

// converting schools for 2.4.0 client
static const uint32 g_spellSchoolConversionTable[SCHOOL_COUNT] =
{
    1,  // SCHOOL_NORMAL
    2,  // SCHOOL_HOLY
    4,  // SCHOOL_FIRE
    8,  // SCHOOL_NATURE
    16, // SCHOOL_FROST
    32, // SCHOOL_SHADOW
    64, // SCHOOL_ARCANE
};
