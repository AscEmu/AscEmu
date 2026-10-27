/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <cstdint>

uint32_t const MistsOfTirnaScitheEncounterCount = 3;

enum MistsOfTirnaScitheData
{
    DATA_MISTCALLER             = 0,    // 2392
    DATA_TREDOVA                = 1,    // 2393
    DATA_INGRA_MALOCH           = 2     // 2397
};

enum MistsOfTirnaScitheCreatures
{
    // Mistcaller
    BOSS_MISTCALLER                     = 164501,
    NPC_MISTCALLER_ILLUSIONARY_CLONE    = 165108,
    NPC_MISTCALLER_ILLUSIONARY_VULPIN   = 165251,

    // Tred'ova
    BOSS_TREDOVA                        = 164517,
    NPC_TREDOVA_GORMLING_LARVA          = 165560,
    NPC_TREDOVA_COCOON_OF_LAKALI        = 165512,

    // Ingra Maloch
    BOSS_INGRA_MALOCH                   = 164567,
    NPC_DROMAN_OULFARRAN                = 164804,
    // further Droman Oulfarran templates of the dungeon
    NPC_DROMAN_OULFARRAN_ALT            = 170218,
    NPC_DROMAN_OULFARRAN_ALT2           = 170229
};

enum MistcallerSpells
{
    SPELL_MISTCALLER_GUESSING_GAME      = 321471,
    SPELL_MISTCALLER_DODGE_BALL         = 321834,
    SPELL_MISTCALLER_PATTY_CAKE         = 321828,
    SPELL_MISTCALLER_FREEZE_TAG         = 321873,

    SPELL_CLONE_PENALIZING_BURST        = 321669,
    SPELL_CLONE_OOPSIE                  = 321837,
    SPELL_VULPIN_FREEZE_TAG_FIXATION    = 321891,
    SPELL_VULPIN_FREEZING_BURST         = 321893
};

enum TredovaSpells
{
    SPELL_TREDOVA_CONSUMPTION           = 322450,
    SPELL_TREDOVA_GORGING_SHIELD        = 322527,
    SPELL_TREDOVA_ANIMA_REJECTION       = 322465,
    SPELL_TREDOVA_ANIMA_SHEDDING        = 326263,
    SPELL_TREDOVA_ACCELERATED_INCUBATION = 322550,
    SPELL_TREDOVA_MIND_LINK             = 322614,
    SPELL_TREDOVA_MARKED_PREY           = 322563,
    SPELL_TREDOVA_ACID_EXPULSION        = 322655,

    SPELL_LARVA_DECOMPOSITION_POOL      = 326308
};

enum IngraMalochSpells
{
    SPELL_MALOCH_SOUL_SHACKLE           = 321005,
    SPELL_MALOCH_SPIRIT_BOLT            = 323057,
    SPELL_MALOCH_EMBRACE_DARKNESS       = 323149,

    SPELL_DROMAN_BEWILDERING_POLLEN     = 323137,
    SPELL_DROMAN_TEARS_OF_THE_FOREST    = 323177,
    SPELL_DROMAN_DROMANS_WRATH          = 323059
};
