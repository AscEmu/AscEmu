/*
Copyright (c) 2014-2018 AscEmu Team <http://www.ascemu.org/>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

enum SpellTypes
{
    SPELL_TYPE_NONE             = 0x0,
    SPELL_TYPE_SEAL             = 0x1,
    SPELL_TYPE_ASPECT           = 0x2,
    SPELL_TYPE_BLESSING         = 0x4,
    SPELL_TYPE_CURSE            = 0x8,
    SPELL_TYPE_STING            = 0x10,
    SPELL_TYPE_ARMOR            = 0x20,
    SPELL_TYPE_AURA             = 0x40,
    SPELL_TYPE_MARK_GIFT        = 0x80,
    SPELL_TYPE_TRACK            = 0x100,
    SPELL_TYPE_HUNTER_TRAP      = 0x200,
    SPELL_TYPE_MAGE_INTEL       = 0x400,
    SPELL_TYPE_MAGE_MAGI        = 0x800,
    SPELL_TYPE_MAGE_WARDS       = 0x1000,
    SPELL_TYPE_PRIEST_SH_PPROT  = 0x2000,
    SPELL_TYPE_SHIELD           = 0x4000,
    SPELL_TYPE_FORTITUDE        = 0x8000,
    SPELL_TYPE_SPIRIT           = 0x10000,
    SPELL_TYPE_MAGE_AMPL_DUMP   = 0x20000,
    SPELL_TYPE_WARLOCK_IMMOLATE = 0x40000,
    SPELL_TYPE_ELIXIR_BATTLE    = 0x80000,
    SPELL_TYPE_ELIXIR_GUARDIAN  = 0x100000,
    SPELL_TYPE_ELIXIR_FLASK     = SPELL_TYPE_ELIXIR_BATTLE | SPELL_TYPE_ELIXIR_GUARDIAN,
    SPELL_TYPE_HUNTER_MARK      = 0x200000,
    SPELL_TYPE_WARRIOR_SHOUT    = 0x400000,
    SPELL_TYPE_QUIVER_HASTE     = 0x800000,
    SPELL_TYPE_CORRUPTION       = 0x1000000,
    SPELL_TYPE_HAND             = 0x2000000,
};
