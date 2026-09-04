/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

// Scholomance (Mists revamp) - five bosses, fought in order per wowhead's dungeon guide:
// Instructor Chillheart -> Jandice Barov -> Rattlegore -> Lilian Voss -> Darkmaster Gandling.
// No prior scripting or dialogue data exists for this
// dungeon - abilities are built from wowhead's MoP Classic data. Entry ids for Jandice Barov,
// Lilian Voss and Gandling went through three different candidates during this project - see
// the session's cross-check summary - and are now confirmed against real placed spawns on
// this exact instance's map (1007) in an expanded reference database added
// later in the session: Jandice Barov=59184, Lilian Voss=58722 (a second entry, 59200, is
// also placed there - likely a phase-2/possessed form; not modeled separately), Gandling=59080
// (a second entry, 58875, is also placed there for the same reason). The earlier
// reused-Classic-id reading (10503/38895/1853) turned out to belong to the Classic Scholomance
// map (289) and an unrelated Eastern Kingdoms open-world placement (map 0) respectively, not
// to this Mists-revamp instance - that was a wrong correction, now reverted. Rattlegore
// (59153) has no placed spawn in that same reference database, but is independently confirmed
// as a real, distinctly-named NPC via wowhead's own dungeon strategy guide.

uint32_t const ScholomanceMopEncounterCount = 5;

enum ScholomanceMopData
{
    DATA_CHILLHEART    = 0,
    DATA_JANDICE_BAROV = 1,
    DATA_RATTLEGORE    = 2,
    DATA_LILIAN_VOSS   = 3,
    DATA_GANDLING      = 4
};

enum ScholomanceMopCreatures
{
    BOSS_CHILLHEART         = 58633,
    BOSS_JANDICE_BAROV      = 59184,
    BOSS_RATTLEGORE         = 59153,
    BOSS_LILIAN_VOSS        = 58722,
    BOSS_GANDLING           = 59080,

    // Trash - all three confirmed against an expanded reference database's own
    // placed spawns on this instance's map. None of their NPC pages carried ability data, so
    // their kits are thematically-reasonable rotations built from generic, verified-safe
    // spells rather than confirmed tooltips.
    NPC_KRASTINOVIAN_CARVER = 59368,
    NPC_SCHOLOMANCE_ACOLYTE = 58757,
    NPC_CANDLESTICK_MAGE    = 59467
};

enum KrastinovianCarverSpells
{
    SPELL_CARVER_CLEAVE        = 845,    // stand-in: reuses "Cleave" rank 1
    SPELL_CARVER_MORTAL_STRIKE = 12294  // stand-in: reuses "Mortal Strike" rank 1
};

enum ScholomanceAcolyteSpells
{
    SPELL_ACOLYTE_SHADOW_BOLT = 32860,  // stand-in: reuses the real "Shadow Bolt" spell
    SPELL_ACOLYTE_CURSE       = 30910   // stand-in: reuses a generic curse debuff
};

enum CandlestickMageSpells
{
    SPELL_CANDLESTICK_FIREBALL  = 133,    // stand-in: reuses "Fireball" rank 1
    SPELL_CANDLESTICK_FROSTBOLT = 116     // stand-in: reuses "Frostbolt" rank 1
};

// Chillheart's real fight is two phases: an ice-wall-race body phase, then a phylactery
// phase where fragments of her soul animate the room's tomes into adds; both the ice-wall
// hazard and the phylactery/tome-add phase are dropped in favor of a continuous rotation.
enum ChillheartSpells
{
    SPELL_CHILLHEART_WRACK_SOUL         = 111631,
    SPELL_CHILLHEART_ICE_WRATH          = 111610,
    SPELL_CHILLHEART_TOUCH_OF_THE_GRAVE = 111224,
    SPELL_CHILLHEART_FRIGID_GRASP       = 114886
};

enum JandiceBarovSpells
{
    SPELL_JANDICE_WONDROUS_RAPIDITY = 114062,
    SPELL_JANDICE_GRAVITY_FLUX      = 114035,
    SPELL_JANDICE_WHIRL_OF_ILLUSION = 113775
};

// Sourced from Scholomance's own wowhead dungeon strategy guide (Rattlegore's individual NPC
// page carries no ability data).
enum RattlegoreSpells
{
    SPELL_RATTLEGORE_BONE_ARMOR = 113996,
    SPELL_RATTLEGORE_BONE_SPIKE = 113999,
    SPELL_RATTLEGORE_RUSTING    = 113765,
    SPELL_RATTLEGORE_SOULFLAME  = 114007
};

// Sourced from Scholomance's own wowhead dungeon strategy guide (Lilian Voss's individual NPC
// page carries no ability data).
enum LilianVossSpells
{
    SPELL_LILIAN_DARK_SIMULACRUM        = 77606,
    SPELL_LILIAN_ARCANE_BOMB            = 113859,
    SPELL_LILIAN_POLYFORMIC_ACID_POTION = 114800
};

// Sourced from Scholomance's own wowhead dungeon strategy guide (Gandling's individual NPC
// page carries no ability data).
enum GandlingSpells
{
    SPELL_GANDLING_DEATHS_GRASP      = 111570,
    SPELL_GANDLING_DARK_BLAZE        = 111585,
    SPELL_GANDLING_BLAZING_SOUL      = 111642,
    SPELL_GANDLING_UNLEASHED_ANGUISH = 111649
};
