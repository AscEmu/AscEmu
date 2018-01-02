/*
Copyright (c) 2014-2018 AscEmu Team <http://www.ascemu.org/>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

enum procFlags : uint32_t
{
    PROC_NULL                         = 0x0,
    PROC_ON_ANY_HOSTILE_ACTION        = 0x1,
    PROC_ON_GAIN_EXPIERIENCE          = 0x2,
    PROC_ON_MELEE_ATTACK              = 0x4,
    PROC_ON_CRIT_HIT_VICTIM           = 0x8,
    PROC_ON_CAST_SPELL                = 0x10,
    PROC_ON_PHYSICAL_ATTACK_VICTIM    = 0x20,
    PROC_ON_RANGED_ATTACK             = 0x40,
    PROC_ON_RANGED_CRIT_ATTACK        = 0x80,
    PROC_ON_PHYSICAL_ATTACK           = 0x100,
    PROC_ON_MELEE_ATTACK_VICTIM       = 0x200,
    PROC_ON_SPELL_HIT                 = 0x400,
    PROC_ON_RANGED_CRIT_ATTACK_VICTIM = 0x800,
    PROC_ON_CRIT_ATTACK               = 0x1000,
    PROC_ON_RANGED_ATTACK_VICTIM      = 0x2000,
    PROC_ON_PRE_DISPELL_AURA_VICTIM   = 0x4000,
    PROC_ON_SPELL_LAND_VICTIM         = 0x8000,
    PROC_ON_CAST_SPECIFIC_SPELL       = 0x10000,
    PROC_ON_SPELL_HIT_VICTIM          = 0x20000,
    PROC_ON_SPELL_CRIT_HIT_VICTIM     = 0x40000,
    PROC_ON_TARGET_DIE                = 0x80000,
    PROC_ON_ANY_DAMAGE_VICTIM         = 0x100000,
    PROC_ON_TRAP_TRIGGER              = 0x200000,
    PROC_ON_AUTO_SHOT_HIT             = 0x400000,
    PROC_ON_ABSORB                    = 0x800000,
    PROC_ON_RESIST_VICTIM             = 0x1000000,
    PROC_ON_DODGE_VICTIM              = 0x2000000,
    PROC_ON_DIE                       = 0x4000000,
    PROC_REMOVEONUSE                  = 0x8000000,
    PROC_MISC                         = 0x10000000,
    PROC_ON_BLOCK_VICTIM              = 0x20000000,
    PROC_ON_SPELL_CRIT_HIT            = 0x40000000,
    PROC_TARGET_SELF                  = 0x80000000,
};
