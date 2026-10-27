/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <cstdint>

uint32_t const TempleOfSethralissEncounterCount = 4;

enum TempleOfSethralissData
{
    // Adderis and Aspix
    DATA_ADDERIS_AND_ASPIX       = 0, // 2124
    DATA_MEREKTHA                = 1, // 2125
    DATA_GALVAZZT                = 2, // 2126
    // healing event, ends when the Avatar is restored
    DATA_AVATAR_OF_SETHRALISS    = 3  // 2127
};

enum TempleOfSethralissCreatures
{
    BOSS_ADDERIS                 = 133379,
    BOSS_ASPIX                   = 133944,

    BOSS_MEREKTHA                = 133384,
    NPC_SAND_CRUSTED_STRIKER     = 134390,
    // second striker template of the later waves
    NPC_SAND_CRUSTED_STRIKER_ALT = 135846,

    BOSS_GALVAZZT                = 133389,
    NPC_ENERGY_CORE              = 135445,

    NPC_AVATAR_OF_SETHRALISS     = 133392,
    NPC_HOODOO_HEXER             = 136250,
    NPC_HEART_GUARDIAN           = 139946,
    NPC_PLAGUE_DOCTOR            = 139949,
    NPC_PLAGUE_TOAD              = 137233
};

enum TempleOfSethralissGameObjects
{
    GO_ADDERIS_ASPIX_BARRIER     = 292551,
    GO_MEREKTHA_BARRIER          = 290906,
    GO_GALVAZZT_BARRIER          = 292414
};

enum TempleOfSethralissActions
{
    ACTION_TWIN_TAKE_SHIELD      = 1,
    ACTION_TWIN_PARTNER_FELL,
    ACTION_GALVAZZT_CORE_CHARGE,
    ACTION_AVATAR_RITUAL_BEGIN
};

enum AdderisAspixSpells
{
    SPELL_TWIN_LIGHTNING_SHIELD     = 263246,
    SPELL_ADDERIS_GUST              = 263775,
    SPELL_ADDERIS_ARCING_BLADE      = 263234,
    SPELL_ADDERIS_PEAL_OF_THUNDER   = 263365,
    SPELL_ADDERIS_ARC_DASH          = 263425,
    SPELL_ASPIX_JOLT                = 263318,
    SPELL_ASPIX_CONDUCTION          = 263371,
    SPELL_ASPIX_CYCLONE_STRIKE      = 261773,
    SPELL_ASPIX_STATIC_SHOCK        = 263257
};

enum MerekthaSpells
{
    SPELL_MEREKTHA_NOXIOUS_BREATH   = 272657,
    SPELL_MEREKTHA_BLINDING_SAND    = 263914,
    SPELL_MEREKTHA_BURROW           = 264206
};

enum GalvazztSpells
{
    SPELL_GALVAZZT_CONSUME_CHARGE   = 266512,
    SPELL_CORE_ENERGY_CORE          = 265977,
    SPELL_CORE_ARC                  = 265986,
    SPELL_CORE_GALVANIZE            = 266923
};

enum AvatarOfSethralissSpells
{
    SPELL_HEXER_TAINT               = 273677,
    SPELL_HEXER_LAVA_BURST          = 274642,
    SPELL_HEXER_FLAME_SHOCK         = 268013,
    SPELL_TOAD_PLAGUE               = 269686
};
