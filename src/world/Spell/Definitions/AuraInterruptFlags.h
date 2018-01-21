/*
Copyright (c) 2014-2018 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

enum AuraInterruptFlags
{
    AURA_INTERRUPT_NULL                       = 0x00000000,
    AURA_INTERRUPT_ON_HOSTILE_SPELL_INFLICTED = 0x00000001, // Remove on being hit by hostile spell
    AURA_INTERRUPT_ON_ANY_DAMAGE_TAKEN        = 0x00000002, // Remove on any damage taken
    AURA_INTERRUPT_ON_CAST                    = 0x00000004, // Remove on any spell cast
    AURA_INTERRUPT_ON_MOVEMENT                = 0x00000008, // Remove on moving
    AURA_INTERRUPT_ON_TURNING                 = 0x00000010, // Remove on turning
    AURA_INTERRUPT_ON_JUMP                    = 0x00000020, // Remove on jumping
    AURA_INTERRUPT_ON_DISMOUNT                = 0x00000040, // Remove on dismounting
    AURA_INTERRUPT_ON_ENTER_WATER             = 0x00000080, // Remove when entering water
    AURA_INTERRUPT_ON_LEAVE_WATER             = 0x00000100, // Remove when leaving water, could be AURA_INTERRUPT_ON_LEAVE_CURRENT_SURFACE
    AURA_INTERRUPT_ON_UNUSED2                 = 0x00000200, // Something to do with sheath state
    AURA_INTERRUPT_ON_UNK4                    = 0x00000400, // On gossip NPC?
    AURA_INTERRUPT_ON_UNK5                    = 0x00000800,
    AURA_INTERRUPT_ON_START_ATTACK            = 0x00001000, // Remove on melee attack
    AURA_INTERRUPT_ON_CAST_SPELL              = 0x00002000, // Remove on spell cast
    AURA_INTERRUPT_ON_UNUSED3                 = 0x00004000,
    AURA_INTERRUPT_ON_CAST_SPELL_UNK          = 0x00008000, // Not sure what this one is, but definitely not on spell cast
    AURA_INTERRUPT_ON_UNK7                    = 0x00010000,
    AURA_INTERRUPT_ON_MOUNT                   = 0x00020000, // Remove on mounting up
    AURA_INTERRUPT_ON_STAND_UP                = 0x00040000, // Remove on stand up, for spells like drinking/eating
    AURA_INTERRUPT_ON_LEAVE_AREA              = 0x00080000, // Remove on leaving zone or being teleported
    AURA_INTERRUPT_ON_INVINCIBLE              = 0x00100000, // Remove on becoming invincible
    AURA_INTERRUPT_ON_STEALTH                 = 0x00200000, // Remove on entering stealth
    AURA_INTERRUPT_ON_UNK8                    = 0x00400000,
    AURA_INTERRUPT_ON_PVP_ENTER               = 0x00800000, // Remove on getting PvP flagged
    AURA_INTERRUPT_ON_DIRECT_DAMAGE           = 0x01000000, // Remove on taking direct damage
    AURA_INTERRUPT_ON_AFTER_CAST_SPELL        = 0x80000000,
};
