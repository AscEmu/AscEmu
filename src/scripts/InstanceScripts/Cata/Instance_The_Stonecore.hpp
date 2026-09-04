/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

// The Stonecore - four bosses, fought in a strictly linear order (Corborus -> Slabhide ->
// Ozruk -> High Priestess Azil per wowhead). Identities and mechanics verified against
// wowhead.

uint32_t const TheStonecoreEncounterCount = 4;

enum TheStonecoreData
{
    DATA_CORBORUS            = 0,
    DATA_SLABHIDE            = 1,
    DATA_OZRUK               = 2,
    DATA_HIGH_PRIESTESS_AZIL = 3
};

enum TheStonecoreGameObjects
{
    // Both "Rock Wall" spawns share this entry and sit just past Corborus at the start of the
    // dungeon; they open together once Corborus is dead, clearing the path deeper in.
    GO_ROCK_WALL = 204381
};

enum TheStonecoreCreatures
{
    BOSS_CORBORUS               = 43438,
    BOSS_SLABHIDE               = 43214,
    BOSS_OZRUK                  = 42188,
    BOSS_HIGH_PRIESTESS_AZIL    = 42333,

    NPC_STONECORE_RIFT_CONJURER = 42691,
    NPC_STONECORE_BRUISER       = 42692,
    NPC_STONECORE_WARBRINGER    = 42696,
    NPC_STONECORE_MAGMALORD     = 42789,
    NPC_STONECORE_FLAYER        = 42808,
    NPC_CRYSTALSPAWN_GIANT      = 42810,
    NPC_STONECORE_BERSERKER     = 43430,
    NPC_STONECORE_EARTHSHAPER   = 43537
};

enum TheStonecoreTrashSpells
{
    SPELL_RIFT_CONJURER_ARCANE_BOLT   = 80279,
    SPELL_RIFT_CONJURER_ARCANE_BUBBLE = 80308,

    // Reference also drives an action-list-linked charge ability (source id 4269200) that
    // has no direct spell cast in AI scripting data; not ported.
    SPELL_STONECORE_BRUISER_SMASH     = 80195,

    SPELL_WARBRINGER_SUNDER_ARMOR     = 80158,
    SPELL_WARBRINGER_GORE             = 15496,

    SPELL_MAGMALORD_LAVA_BURST        = 80151,
    SPELL_MAGMALORD_MAGMA_SHIELD      = 80038,

    // Reference's other two AI scripting data rows for this NPC drive non-cast SmartAI actions
    // (distance/phase control) we have no equivalent for; not ported.
    SPELL_STONECORE_FLAYER_REND       = 79922,

    SPELL_CRYSTALSPAWN_GIANT_SHATTER  = 81008,

    SPELL_BERSERKER_WHIRLWIND         = 81574,
    SPELL_BERSERKER_ENRAGE            = 81568,

    // Several AI scripting data rows for this NPC drive phase/say/link actions with no direct
    // spell cast; only the four clear ability casts below are ported.
    SPELL_EARTHSHAPER_STONE_SPIKES    = 81576,
    SPELL_EARTHSHAPER_QUAKE           = 81459,
    SPELL_EARTHSHAPER_EMBEDDED_SPIKE  = 81530,
    SPELL_EARTHSHAPER_UPHEAVAL        = 81463
};

enum CorborusSpells
{
    SPELL_CORBORUS_DAMPENING_WAVE  = 82415,
    SPELL_CORBORUS_CRYSTAL_BARRAGE = 86881
};

enum SlabhideSpells
{
    SPELL_SLABHIDE_LAVA_FISSURE = 80803,
    SPELL_SLABHIDE_SAND_BLAST   = 80807
};

enum OzrukSpells
{
    SPELL_OZRUK_ELEMENTIUM_BULWARK      = 78939,
    SPELL_OZRUK_GROUND_SLAM             = 78903,
    SPELL_OZRUK_ELEMENTIUM_SPIKE_SHIELD = 78835,
    SPELL_OZRUK_ENRAGE                  = 80467
};

enum AzilSpells
{
    SPELL_AZIL_CURSE_OF_BLOOD      = 79345,
    SPELL_AZIL_FORCE_GRIP          = 79351,
    SPELL_AZIL_SUMMON_GRAVITY_WELL = 79340
};
