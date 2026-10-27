/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <cstdint>

uint32_t const WindrunnerSpireEncounterCount = 4;

enum WindrunnerSpireData
{
    DATA_EMBERDAWN          = 0,    // 3056
    // Kalis and Latch
    DATA_DERELICT_DUO       = 1,    // 3057
    DATA_COMMANDER_KROLUK   = 2,    // 3058
    DATA_RESTLESS_HEART     = 3     // 3059
};

enum WindrunnerSpireCreatures
{
    BOSS_EMBERDAWN                  = 231606,

    BOSS_KALIS                      = 231626,
    BOSS_LATCH                      = 231629,

    BOSS_COMMANDER_KROLUK           = 231631,
    NPC_HAUNTING_GRUNT              = 232446,
    // second template of the same warparty member
    NPC_HAUNTING_GRUNT_ALT          = 258868,
    NPC_SPECTRAL_AXETHROWER         = 232447,
    NPC_SPECTRAL_AXETHROWER_ALT     = 232148,

    BOSS_RESTLESS_HEART             = 231636
};

enum WindrunnerSpireActions
{
    ACTION_DUO_PARTNER_FALLEN = 1
};

enum EmberdawnSpells
{
    SPELL_EMBERDAWN_BURNING_GALE        = 465904,
    SPELL_EMBERDAWN_FLAMING_UPDRAFT     = 466556,
    SPELL_EMBERDAWN_IGNITED_EMBERS      = 467120,
    SPELL_EMBERDAWN_FLAMING_TWISTERS    = 469633,
    SPELL_EMBERDAWN_SEARING_BEAK        = 466064
};

enum DerelictDuoSpells
{
    SPELL_DUO_BROKEN_BOND               = 1219551,

    SPELL_KALIS_SHADOW_BOLT             = 472724,
    SPELL_KALIS_DEBILITATING_SHRIEK     = 472736,

    SPELL_LATCH_HEAVING_YANK            = 472795,
    SPELL_LATCH_HEAVING_CHOP            = 474075,
    SPELL_LATCH_SPLATTERING_SPEW        = 472745,
    SPELL_LATCH_GUNK_SPLATTER           = 472777,
    SPELL_LATCH_BONE_HACK               = 472888
};

enum CommanderKrolukSpells
{
    SPELL_KROLUK_RAMPAGE                = 467620,
    SPELL_KROLUK_RECKLESS_LEAP          = 472081,
    SPELL_KROLUK_RALLYING_BELLOW        = 468070,
    SPELL_KROLUK_BLADESTORM             = 470963,
    SPELL_KROLUK_SHIELD_WALL            = 1250851,

    SPELL_GRUNT_INTERCEPTING_CHARGE     = 467815,
    SPELL_AXETHROWER_THROW_AXE          = 1217094
};

enum RestlessHeartSpells
{
    SPELL_HEART_BULLSEYE_WINDBLAST      = 468429,
    SPELL_HEART_ARROW_RAIN              = 472556,
    SPELL_HEART_BOLT_GALE               = 474528,
    SPELL_HEART_TEMPEST_SLASH           = 472662
};
