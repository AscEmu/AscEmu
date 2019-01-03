/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

enum PowerType : int16_t
{
    POWER_TYPE_HEALTH      = -2,
    POWER_TYPE_MANA        = 0,
    POWER_TYPE_RAGE        = 1,
    POWER_TYPE_FOCUS       = 2,
    POWER_TYPE_ENERGY      = 3,
    POWER_TYPE_HAPPINESS   = 4,
#if VERSION_STRING >= WotLK
    POWER_TYPE_RUNES       = 5,
    POWER_TYPE_RUNIC_POWER = 6,
#endif
#if VERSION_STRING >= Cata
    POWER_TYPE_SOUL_SHARDS = 7,
    POWER_TYPE_ECLIPSE     = 8,
    POWER_TYPE_HOLY_POWER  = 9,
    POWER_TYPE_ALTERNATIVE = 10,
#endif
    POWER_TYPE_STEAM       = 61,
    POWER_TYPE_PYRITE      = 41,
    POWER_TYPE_HEAT        = 101,
    POWER_TYPE_OOZE        = 121,
    POWER_TYPE_BLOOD       = 141,
    POWER_TYPE_WRATH       = 142
};
