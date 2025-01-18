/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

// wotlk
//#define HIGHEST_FACTION = 46

#define UNIT_SUMMON_SLOTS 6

#define SPELL_GROUPS 96                                                             // This is actually on 64 bits !
#define DIMINISHING_GROUP_COUNT 15

#define MIN_MELEE_REACH                     2.0f
#define NOMINAL_MELEE_RANGE                 5.0f
#define MELEE_RANGE                         (NOMINAL_MELEE_RANGE - MIN_MELEE_REACH * 2) //center to center for players
