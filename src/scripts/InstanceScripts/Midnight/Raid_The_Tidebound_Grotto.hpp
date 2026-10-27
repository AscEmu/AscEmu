/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <cstdint>

uint32_t const TideboundGrottoEncounterCount = 1;

enum TideboundGrottoData
{
    DATA_NYMRISSA_WAVECALLER    = 0     // 3379
};

enum TideboundGrottoCreatures
{
    BOSS_NYMRISSA_WAVECALLER        = 252959,

    // murlocs lured by the Alluring Bubble
    NPC_BUBBLEFIN_SHORERUNNER       = 253032,
    NPC_BUBBLEFIN_BERSERKER         = 253180
};

enum TideboundGrottoSharedSpells
{
    SPELL_TIDEBOUND_BERSERK         = 26662
};

enum TideboundNymrissaSpells
{
    SPELL_NYMRISSA_ALLURING_BUBBLE      = 1257717,
    SPELL_NYMRISSA_SWIRLING_WHIRLPOOLS  = 1258668,
    SPELL_NYMRISSA_POP                  = 1258150,
    SPELL_NYMRISSA_CHILLING_FROST       = 1313393,
    SPELL_NYMRISSA_WATER_JET            = 1258901,
    SPELL_NYMRISSA_ICEBLADE_FLURRY      = 1282937,
    SPELL_NYMRISSA_ABYSSAL_RAIN         = 1260837,
    SPELL_NYMRISSA_WAVECALLERS_MIGHT    = 1277386,

    SPELL_BERSERKER_PULSING_TIDES       = 1258894
};
