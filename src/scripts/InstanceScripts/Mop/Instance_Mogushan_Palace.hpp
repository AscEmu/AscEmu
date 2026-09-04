/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

// Mogu'shan Palace - three encounters (verified against wowhead's MoP Classic dungeon
// guide): Trial of the Kings (Kuai the Brute, Ming the Cunning and Haiyan the Unstoppable -
// only one is active at a time in the real fight, submitting instead of dying before the
// next one activates; ported here as three independent bosses sharing one boss-state slot
// instead, since we have no "submit and hand off" mechanic), Gekkan, and Xin the
// Weaponmaster. No prior scripting or dialogue data exists
// for this dungeon. This project's own gameobject reference import does carry a real
// "Ancient Guo-Lai Door" template (this dungeon's own normal-difficulty name is "Guo-Lai
// Halls" - see below) - it has no placed spawn/position data in that same import, so the
// door is wired up in code but will only actually appear once a real spawn-placement entry
// places it.

enum MogushanPalaceGameObjects
{
    // Opens once the Trial of the Kings is defeated, matching the dungeon's linear layout.
    GO_ANCIENT_GUOLAI_DOOR = 211662
};

uint32_t const MogushanPalaceEncounterCount = 3;

enum MogushanPalaceData
{
    DATA_TRIAL_OF_THE_KINGS   = 0,
    DATA_GEKKAN               = 1,
    DATA_XIN_THE_WEAPONMASTER = 2
};

enum MogushanPalaceCreatures
{
    BOSS_KUAI_THE_BRUTE         = 61442,
    BOSS_MING_THE_CUNNING       = 61444,
    BOSS_HAIYAN_THE_UNSTOPPABLE = 61445,
    BOSS_GEKKAN                 = 61243,
    BOSS_XIN_THE_WEAPONMASTER   = 61398,

    // Trash - this dungeon has no placed spawn data in either reference database (unlike most
    // other Mop instances); this is a real, named creature template found by proximity to the
    // confirmed boss entries, but its actual presence in this specific dungeon (rather than
    // the surrounding Vale of Eternal Blossoms open world, which shares this id range) is not
    // spawn-confirmed. Wowhead's own dungeon guide describes this as a short, low-density
    // dungeon ("not a lot of mobs"), consistent with only a light trash pass being warranted.
    NPC_MOGU_DEFIER             = 61304
};

enum MoguDefierSpells
{
    SPELL_MOGU_DEFIER_CLEAVE = 845,    // stand-in: reuses "Cleave" rank 1
    SPELL_MOGU_DEFIER_ENRAGE = 8599    // stand-in: reuses the generic "Enrage" self-buff
};

enum KuaiSpells
{
    SPELL_KUAI_RAVAGE    = 119946,
    SPELL_KUAI_SHOCKWAVE = 119922
};

enum MingSpells
{
    SPELL_MING_WHIRLING_DERVISH = 119981,
    SPELL_MING_MAGNETIC_FIELD   = 120100,
    SPELL_MING_LIGHTNING_BOLT   = 96891
};

enum HaiyanSpells
{
    SPELL_HAIYAN_CONFLAGRATE    = 17962,
    SPELL_HAIYAN_METEOR         = 120195,
    SPELL_HAIYAN_TRAUMATIC_BLOW = 123655
};

enum GekkanSpells
{
    SPELL_GEKKAN_SHANK           = 118963,
    SPELL_GEKKAN_FIRE_BOLT       = 118936,
    SPELL_GEKKAN_DARK_BOLT       = 118917,
    SPELL_GEKKAN_HEX_OF_LETHARGY = 118903,
    SPELL_GEKKAN_CLEANSING_FLAME = 118940
};

enum XinSpells
{
    SPELL_XIN_GROUND_SLAM     = 119684,
    SPELL_XIN_CIRCLE_OF_FLAME = 17447,
    SPELL_XIN_INCITING_ROAR   = 122959
};
