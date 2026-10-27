/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <cstdint>

uint32_t const MarchOnQuelDanasEncounterCount = 2;

enum MarchOnQuelDanasData
{
    DATA_BELOREN            = 0,    // 3182
    DATA_MIDNIGHT_FALLS     = 1     // 3183
};

enum MarchOnQuelDanasCreatures
{
    // Belo'ren, Child of Al'ar
    BOSS_BELOREN                    = 240387,
    NPC_BELOREN_LIGHT_EMBER         = 246729,
    NPC_BELOREN_VOID_EMBER          = 246728,
    // second ember templates of the same name
    NPC_BELOREN_LIGHT_EMBER_ALT     = 259938,
    NPC_BELOREN_VOID_EMBER_ALT      = 259945,

    // Midnight Falls
    BOSS_LURA                       = 240391,
    NPC_LURA_MIDNIGHT_CRYSTAL       = 250615,
    NPC_LURA_DUSK_CRYSTAL           = 250616,
    NPC_LURA_DAWN_CRYSTAL           = 251180
};

enum MarchOnQuelDanasSharedSpells
{
    SPELL_MARCH_QUEL_DANAS_BERSERK          = 26662
};

enum MarchOnQuelDanasBelorenSpells
{
    // Stage One: Phoenix Reborn
    SPELL_BELOREN_VOIDLIGHT_CONVERGENCE     = 1242515,
    SPELL_BELOREN_IMMORTAL_FLAME            = 1243320,
    SPELL_BELOREN_EMBERS_OF_BELOREN         = 1241282,
    SPELL_BELOREN_RADIANT_ECHOES            = 1242981,
    SPELL_BELOREN_GUARDIANS_EDICT           = 1260763,
    SPELL_BELOREN_ETERNAL_BURNS             = 1244344,
    SPELL_BELOREN_INFUSED_QUILLS            = 1242260,
    SPELL_BELOREN_DEATH_DROP                = 1246709,

    // Stage Two: Ashen Shell
    SPELL_BELOREN_REBIRTH                   = 1241313,
    SPELL_BELOREN_ASHEN_BENEDICTION         = 1262573,
    SPELL_BELOREN_INCUBATION_OF_FLAMES      = 1242792,

    SPELL_LIGHT_EMBER_LIGHT_DIVE            = 1241292,
    SPELL_LIGHT_EMBER_LIGHT_BLAST           = 1264696,
    SPELL_LIGHT_EMBER_LIGHT_ERUPTION        = 1243852,
    SPELL_VOID_EMBER_VOID_DIVE              = 1241339,
    SPELL_VOID_EMBER_VOID_BLAST             = 1264698,
    SPELL_VOID_EMBER_VOID_ERUPTION          = 1243854
};

enum MarchOnQuelDanasLuraSpells
{
    // Stage One: Final Tolls
    SPELL_LURA_THE_DARKWELL                 = 1282027,
    SPELL_LURA_DARK_QUASAR                  = 1279420,
    SPELL_LURA_HEAVENS_GLAIVES              = 1253915,
    SPELL_LURA_DEATHS_DIRGE                 = 1244412,
    SPELL_LURA_DISINTEGRATION               = 1251649,
    SPELL_LURA_HEAVENS_LANCE                = 1263970,
    SPELL_LURA_SHATTERED_SKY                = 1249796,

    // Intermission: Total Eclipse
    SPELL_LURA_TOTAL_ECLIPSE                = 1260261,
    SPELL_LURA_STARSPLINTER                 = 1282441,
    SPELL_LURA_ECLIPSE_DARK_QUASAR          = 1285561,

    // Stage Two: The Dark Reactor
    SPELL_LURA_INTO_THE_DARKWELL            = 1282034,
    SPELL_LURA_GALVANIZE                    = 1284525,
    SPELL_LURA_CORE_HARVEST                 = 1282412,
    SPELL_LURA_DARK_MELTDOWN                = 1281194,

    // Stage Three: Midnight Falls
    SPELL_LURA_THE_DARK_ARCHANGEL           = 1250898,
    SPELL_LURA_MIDNIGHT                     = 1266622,
    SPELL_LURA_BLACK_TIDE                   = 1263253,
    SPELL_LURA_DARK_CONSTELLATION           = 1266388,
    SPELL_LURA_DEATHS_REQUIEM               = 1273158,
    SPELL_LURA_LIGHT_SIPHON                 = 1266897,

    SPELL_MIDNIGHT_CRYSTAL_COSMIC_FRACTURE  = 1251789,
    SPELL_DUSK_CRYSTAL_DIMMING              = 1252974,
    SPELL_DAWN_CRYSTAL_RADIANCE             = 1282458,
    SPELL_DAWN_CRYSTAL_GLIMMERING           = 1254398,
    SPELL_DAWN_CRYSTAL_TEARS_OF_LURA        = 1254262
};
