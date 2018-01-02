/*
Copyright (c) 2014-2018 AscEmu Team <http://www.ascemu.org/>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

enum SPELL_MODIFIER_TYPE
{
    SMT_DAMAGE_DONE       = 0,
    SMT_DURATION          = 1,
    SMT_THREAT_REDUCED    = 2,
    SMT_EFFECT_1          = 3,
    SMT_CHARGES           = 4,
    SMT_RANGE             = 5,
    SMT_RADIUS            = 6,
    SMT_CRITICAL          = 7,
    SMT_MISC_EFFECT       = 8,
    SMT_NONINTERRUPT      = 9,
    SMT_CAST_TIME         = 10,
    SMT_COOLDOWN_DECREASE = 11,
    SMT_EFFECT_2          = 12,
    // UNUSED_13
    SMT_COST              = 14,
    SMT_CRITICAL_DAMAGE   = 15,
    SMT_HITCHANCE         = 16,
    SMT_ADDITIONAL_TARGET = 17,
    SMT_TRIGGER           = 18,
    SMT_AMPTITUDE         = 19,
    SMT_JUMP_REDUCE       = 20,
    SMT_GLOBAL_COOLDOWN   = 21,
    SMT_SPELL_VALUE_PCT   = 22,
    SMT_EFFECT_3          = 23,
    SMT_PENALTY           = 24,
    // UNUSED_25
    // UNUSED_26
    SMT_EFFECT_BONUS      = 27,
    SMT_RESIST_DISPEL     = 28,
};
