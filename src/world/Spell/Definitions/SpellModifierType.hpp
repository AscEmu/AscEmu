/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <cstdint>

enum SpellModifierType : uint8_t
{
    SPELLMOD_DAMAGE_DONE       = 0,
    SPELLMOD_DURATION          = 1,
    SPELLMOD_THREAT_REDUCED    = 2,
    SPELLMOD_EFFECT_1          = 3,
    SPELLMOD_CHARGES           = 4,
    SPELLMOD_RANGE             = 5,
    SPELLMOD_RADIUS            = 6,
    SPELLMOD_CRITICAL          = 7,
    SPELLMOD_ALL_EFFECTS       = 8,
    SPELLMOD_NONINTERRUPT      = 9,
    SPELLMOD_CAST_TIME         = 10,
    SPELLMOD_COOLDOWN_DECREASE = 11,
    SPELLMOD_EFFECT_2          = 12,
    // UNUSED_13
    SPELLMOD_COST              = 14,
    SPELLMOD_CRITICAL_DAMAGE   = 15,
    SPELLMOD_HITCHANCE         = 16,
    SPELLMOD_ADDITIONAL_TARGET = 17,
    SPELLMOD_TRIGGER           = 18,
    SPELLMOD_AMPTITUDE         = 19,
    SPELLMOD_JUMP_REDUCE       = 20,
    SPELLMOD_GLOBAL_COOLDOWN   = 21,
    SPELLMOD_PERIODIC_DAMAGE   = 22,
    SPELLMOD_EFFECT_3          = 23,
    SPELLMOD_PENALTY           = 24,
    // UNUSED_25
    // UNUSED_26
    SPELLMOD_EFFECT_BONUS      = 27,
    SPELLMOD_RESIST_DISPEL     = 28,
    MAX_SPELLMOD_TYPE
};
