/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

// Terrace of Endless Spring - four bosses: Protector Kaolan, Tsulong, Lei Shi, Sha of Fear.
// No prior scripting or dialogue data exists for this raid
// - all kits below are built from wowhead's MoP Classic data instead. This project's own
// gameobject reference import does carry a real "Celestial Door" template (fits this raid's
// Vale-of-Eternal-Blossoms/celestial theme; see below) - it has no placed spawn/position data
// in that same import, so the door is wired up in code but will only actually appear once a
// real spawn-placement entry places it.

enum TerraceOfEndlessSpringGameObjects
{
    // Opens once Protector Kaolan is defeated, matching the raid's linear layout.
    GO_CELESTIAL_DOOR = 223501
};

uint32_t const TerraceOfEndlessSpringEncounterCount = 4;

enum TerraceOfEndlessSpringData
{
    DATA_PROTECTOR_KAOLAN = 0,
    DATA_TSULONG          = 1,
    DATA_LEI_SHI          = 2,
    DATA_SHA_OF_FEAR      = 3
};

enum TerraceOfEndlessSpringCreatures
{
    BOSS_PROTECTOR_KAOLAN   = 60583,
    BOSS_TSULONG            = 62442,
    BOSS_LEI_SHI            = 62983,
    BOSS_SHA_OF_FEAR        = 60999,

    // Trash - both confirmed against this raid's own real creature spawn data; both real
    // abilities sourced directly from their own wowhead NPC pages.
    NPC_ANIMATED_PROTECTOR  = 62995,
    NPC_CORRUPTED_PROTECTOR = 63275
};

enum AnimatedProtectorSpells
{
    SPELL_ANIMATED_PROTECTOR_PROTECT = 123505,
    SPELL_ANIMATED_PROTECTOR_CLEAVE  = 845    // stand-in: reuses "Cleave" rank 1
};

enum CorruptedProtectorSpells
{
    SPELL_CORRUPTED_PROTECTOR_DISPERSE = 123610,
    SPELL_CORRUPTED_PROTECTOR_CLEAVE   = 845    // stand-in: reuses "Cleave" rank 1
};

// Protector Kaolan's real fight is a corruption-management encounter shared with two add
// healers (Elder Asani/Elder Regail) who can purify or embrace his Sha corruption for
// different buffs; that choice mechanic and the two adds aren't ported, only Kaolan's own
// direct kit is.
enum ProtectorKaolanSpells
{
    SPELL_KAOLAN_TOUCH_OF_SHA            = 117519,
    SPELL_KAOLAN_DEFILED_GROUND          = 117986,
    SPELL_KAOLAN_OVERWHELMING_CORRUPTION = 117351
};

// Tsulong alternates day (holy) and night (shadow) phases in the real fight; both halves of
// his kit are kept but run continuously rather than being phase-gated.
enum TsulongSpells
{
    SPELL_TSULONG_DREAD_SHADOWS = 122768,
    SPELL_TSULONG_SHADOW_BREATH = 122752,
    SPELL_TSULONG_SUNBEAM       = 122789,
    SPELL_TSULONG_SUN_BREATH    = 123105
};

// Lei Shi's real fight is a stealth/hide-and-seek encounter with totems that must be
// protected or destroyed; simplified to her direct damage kit only.
enum LeiShiSpells
{
    SPELL_LEISHI_SPRAY     = 123121,
    SPELL_LEISHI_SCARY_FOG = 123705
};

// Sourced directly from Sha of Fear's own wowhead NPC page.
enum ShaOfFearSpells
{
    SPELL_SHAOFFEAR_BREATH_OF_FEAR = 119414,
    SPELL_SHAOFFEAR_THRASH         = 77758,
    SPELL_SHAOFFEAR_OMINOUS_CACKLE = 119593,
    SPELL_SHAOFFEAR_DEATH_BLOSSOM  = 119887
};
