/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <cstdint>

enum SpellProcFlags : uint32_t
{
    PROC_NULL                                       = 0x0,
    PROC_ON_DEATH                                   = 0x1,
    PROC_ON_KILL                                    = 0x2,      // Gain experience or honorable kill

    PROC_ON_DONE_MELEE_HIT                          = 0x4,      // Melee auto attack
    PROC_ON_TAKEN_MELEE_HIT                         = 0x8,      // Melee auto attack
    PROC_ON_DONE_MELEE_SPELL_HIT                    = 0x10,     // Spell which uses a melee weapon
    PROC_ON_TAKEN_MELEE_SPELL_HIT                   = 0x20,     // Spell which uses a melee weapon

    PROC_ON_DONE_RANGED_HIT                         = 0x40,     // Ranged auto attack
    PROC_ON_TAKEN_RANGED_HIT                        = 0x80,     // Ranged auto attack
    PROC_ON_DONE_RANGED_SPELL_HIT                   = 0x100,    // Spell which uses a ranged weapon
    PROC_ON_TAKEN_RANGED_SPELL_HIT                  = 0x200,    // Spell which uses a ranged weapon

    PROC_ON_DONE_POSITIVE_SPELL_DAMAGE_CLASS_NONE   = 0x400,    // Positive spell with no damage class
    PROC_ON_TAKEN_POSITIVE_SPELL_DAMAGE_CLASS_NONE  = 0x800,    // Positive spell with no damage class
    PROC_ON_DONE_NEGATIVE_SPELL_DAMAGE_CLASS_NONE   = 0x1000,   // Negative spell with no damage class
    PROC_ON_TAKEN_NEGATIVE_SPELL_DAMAGE_CLASS_NONE  = 0x2000,   // Negative spell with no damage class

    PROC_ON_DONE_POSITIVE_SPELL_DAMAGE_CLASS_MAGIC  = 0x4000,   // Positive spell with magic damage class
    PROC_ON_TAKEN_POSITIVE_SPELL_DAMAGE_CLASS_MAGIC = 0x8000,   // Positive spell with magic damage class
    PROC_ON_DONE_NEGATIVE_SPELL_DAMAGE_CLASS_MAGIC  = 0x10000,  // Negative spell with magic damage class
    PROC_ON_TAKEN_NEGATIVE_SPELL_DAMAGE_CLASS_MAGIC = 0x20000,  // Negative spell with magic damage class

    PROC_ON_DONE_PERIODIC                           = 0x40000,  // Periodic damage or heal
    PROC_ON_TAKEN_PERIODIC                          = 0x80000,  // Periodic damage or heal

    PROC_ON_TAKEN_ANY_DAMAGE                        = 0x100000, // Any damage
    PROC_ON_TRAP_ACTIVATION                         = 0x200000, // Hunter trap activation
    PROC_ON_DONE_OFFHAND_ATTACK                     = 0x400000, // Offhand hit
    PROC_ON_TAKEN_OFFHAND_ATTACK                    = 0x800000, // Offhand hit
};

// Custom proc flags for extra checks
enum SpellExtraProcFlags : uint32_t
{
    EXTRA_PROC_NULL                   = 0x0,
    EXTRA_PROC_ON_MAIN_HAND_HIT_ONLY  = 0x1, // used with main hand weapon's enchantments and 'chance on hit' effects
    EXTRA_PROC_ON_OFF_HAND_HIT_ONLY   = 0x2, // used with off hand weapon's enchantments and 'chance on hit' effects
    EXTRA_PROC_ON_CRIT_ONLY           = 0x4, // melee, ranged or spell crit
};
