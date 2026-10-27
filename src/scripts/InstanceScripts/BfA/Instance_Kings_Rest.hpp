/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <cstdint>

uint32_t const KingsRestEncounterCount = 4;

enum KingsRestData
{
    DATA_GOLDEN_SERPENT    = 0, // 2139
    // Kula the Butcher, Aka'ali the Conqueror and Zanazal the Wise
    DATA_COUNCIL_OF_TRIBES = 1, // 2140
    DATA_MCHIMBA           = 2, // 2142
    DATA_KING_DAZAR        = 3  // 2143
};

enum KingsRestCreatures
{
    BOSS_GOLDEN_SERPENT    = 135322,
    NPC_ANIMATED_GOLD      = 135406,

    BOSS_KULA_THE_BUTCHER  = 135475,
    BOSS_AKAALI            = 135470,
    BOSS_ZANAZAL_THE_WISE  = 135472,
    NPC_EXPLOSIVE_TOTEM    = 135764,
    NPC_THUNDERING_TOTEM   = 135761,
    NPC_TORRENT_TOTEM      = 135765,
    NPC_EARTHWALL_TOTEM    = 135759,

    BOSS_MCHIMBA           = 134993,

    BOSS_KING_DAZAR        = 136160,
    NPC_REBAN              = 136984,
    NPC_TZALA              = 136976
};

enum KingsRestGameObjects
{
    GO_SERPENT_ROOM_DOOR   = 287391,
    GO_SERPENT_EXIT_DOOR   = 287392,
    GO_COUNCIL_GATE        = 287503,
    GO_MCHIMBA_GATE        = 287395,
    GO_DAZAR_GATE          = 289644
};

enum KingsRestActions
{
    ACTION_GOLD_HEED_CALL  = 1,
    ACTION_CHAMPION_STEP_IN
};

enum GoldenSerpentSpells
{
    SPELL_SERPENT_SPIT_GOLD         = 265773,
    SPELL_SERPENT_SERPENTINE_GUST   = 265781,
    SPELL_SERPENT_TAIL_THRASH       = 265910,
    SPELL_SERPENT_LUCRES_CALL       = 265923,
    SPELL_GOLD_MOLTEN_GOLD          = 265915,
    SPELL_GOLD_LUSTER               = 265991
};

enum CouncilOfTribesSpells
{
    SPELL_KULA_SEVERING_AXE            = 266231,
    SPELL_KULA_WHIRLING_AXES           = 266206,

    SPELL_AKAALI_DEBILITATING_BACKHAND = 266237,
    SPELL_AKAALI_BARREL_THROUGH        = 266951,
    SPELL_AKAALI_BARREL_CHARGE         = 266941,
    SPELL_AKAALI_BARREL_IMPACT         = 266940,

    SPELL_ZANAZAL_LIGHTNING_BOLT       = 267308,
    SPELL_ZANAZAL_POISON_NOVA          = 267273,
    SPELL_ZANAZAL_CALL_OF_ELEMENTS     = 267060,
    SPELL_TOTEM_EXPLODE                = 267077,
    SPELL_TOTEM_THUNDERING_CRASH       = 267257
};

enum MchimbaSpells
{
    SPELL_MCHIMBA_BURN_CORRUPTION   = 267639,
    SPELL_MCHIMBA_DRAIN_FLUIDS      = 267618,
    SPELL_MCHIMBA_DESSICATION       = 267626
};

enum KingDazarSpells
{
    SPELL_DAZAR_BLADE_COMBO         = 268586,
    SPELL_DAZAR_GALE_SLASH          = 268403,
    SPELL_DAZAR_QUAKING_LEAP        = 268932,
    SPELL_REBAN_HUNTING_LEAP        = 270502,
    SPELL_TZALA_DEATHLY_ROAR        = 269369
};
