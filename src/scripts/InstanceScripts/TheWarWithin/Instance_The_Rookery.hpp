/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <cstdint>

uint32_t const TheRookeryEncounterCount = 3;

enum TheRookeryData
{
    DATA_KYRIOSS                = 0,    // 2816
    DATA_VOIDSTONE_MONSTROSITY  = 1,    // 2836
    DATA_STORMGUARD_GORREN      = 2     // 2861
};

enum TheRookeryCreatures
{
    BOSS_KYRIOSS                = 209230,

    BOSS_VOIDSTONE_MONSTROSITY  = 207207,
    NPC_STORMRIDER_VOKMAR       = 215967,
    NPC_VOIDSTONE_FRAGMENT      = 227518,
    NPC_VOIDSTONE_AWAKENED      = 213741,

    BOSS_STORMGUARD_GORREN      = 207205,
    // second template of Stormguard Gorren
    BOSS_STORMGUARD_GORREN_ALT  = 218708
};

enum KyriossSpells
{
    SPELL_KYRIOSS_LIGHTNING_TORRENT     = 444250,
    SPELL_KYRIOSS_CRASHING_THUNDER      = 1214325,
    SPELL_KYRIOSS_WILD_LIGHTNING        = 474017,
    SPELL_KYRIOSS_LIGHTNING_DASH        = 419871
};

enum VoidstoneMonstrositySpells
{
    SPELL_MONSTROSITY_NULL_UPHEAVAL     = 423305,
    SPELL_MONSTROSITY_VOID_SHELL        = 445262,
    SPELL_MONSTROSITY_UNLEASH_CORRUPTION = 429487,
    SPELL_MONSTROSITY_OBLIVION_WAVE     = 445457,
    SPELL_MONSTROSITY_ENTROPY           = 423393,

    SPELL_FRAGMENT_RESHAPE              = 428269,
    SPELL_AWAKENED_CORRUPTION_OVERLOAD  = 429029,
    SPELL_VOKMAR_STORMS_VENGEANCE       = 424371
};

enum StormguardGorrenSpells
{
    SPELL_GORREN_CHAOTIC_CORRUPTION     = 424739,
    SPELL_GORREN_DARK_GRAVITY           = 425048,
    SPELL_GORREN_CRUSH_REALITY          = 424958
};
