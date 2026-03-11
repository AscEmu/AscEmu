/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <cstdint>

enum PowerType : int16_t
{
    POWER_TYPE_HEALTH           = -2,
    POWER_TYPE_MANA             = 0,
    POWER_TYPE_RAGE             = 1,
    POWER_TYPE_FOCUS            = 2,
    POWER_TYPE_ENERGY           = 3,
#if VERSION_STRING < Cata
    POWER_TYPE_HAPPINESS        = 4,
#elif VERSION_STRING == Mop
    POWER_TYPE_LIGHT_FORCE      = 4, // MoP: A part of the Force that should be used by monks (removed in MoP Beta)
#else
    POWER_TYPE_UNK4             = 4,
#endif
#if VERSION_STRING >= WotLK
    POWER_TYPE_RUNES            = 5,
    POWER_TYPE_RUNIC_POWER      = 6,
#endif
#if VERSION_STRING >= Cata
    POWER_TYPE_SOUL_SHARDS      = 7,
    POWER_TYPE_ECLIPSE          = 8,
    POWER_TYPE_HOLY_POWER       = 9,
    POWER_TYPE_ALTERNATIVE      = 10, // Every player class has this, used in quests
#endif
#if VERSION_STRING >= Mop
    POWER_TYPE_DARK_FORCE       = 11, // MoP: A part of the Force that should be used by monks (removed in MoP Beta)
    POWER_TYPE_CHI              = 12, // MoP: Used by monks
    POWER_TYPE_SHADOW_ORBS      = 13, // MoP: Used by shadow priests
    POWER_TYPE_BURNING_EMBERS   = 14, // MoP: Used by destruction warlocks
    POWER_TYPE_DEMONIC_FURY     = 15, // MoP: Used by demonology warlocks
    POWER_TYPE_ARCANE_CHARGES   = 16, // MoP: Used by arcane mages
#endif
    TOTAL_PLAYER_POWER_TYPES
};
