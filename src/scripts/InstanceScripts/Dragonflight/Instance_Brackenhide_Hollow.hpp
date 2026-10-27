/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <cstdint>

uint32_t const BrackenhideHollowEncounterCount = 4;

enum BrackenhideHollowData
{
    DATA_GUTSHOT              = 0, // 2567
    DATA_TREEMOUTH            = 1, // 2568
    DATA_DECATRIARCH_WRATHEYE = 2, // 2569
    // Rira Hackclaw, Gashtooth and Tricktotem
    DATA_HACKCLAWS_WAR_BAND   = 3  // 2570
};

enum BrackenhideHollowCreatures
{
    BOSS_GUTSHOT              = 186116,
    NPC_ROTFANG_HYENA         = 194745,

    BOSS_TREEMOUTH            = 186120,
    NPC_DECAYING_SLIME        = 192481,

    BOSS_DECATRIARCH_WRATHEYE = 186121,
    NPC_ROTBURST_TOTEM        = 190381,

    BOSS_RIRA_HACKCLAW        = 186122,
    BOSS_GASHTOOTH            = 186124,
    BOSS_TRICKTOTEM           = 186125,
    NPC_HEXTRICK_TOTEM        = 193352
};

enum BrackenhideHollowActions
{
    ACTION_WARBAND_HEXTRICK_TOTEM = 1,
    ACTION_WARBAND_DECAYED_SENSES
};

enum BrackenhideHollowGutshotSpells
{
    SPELL_GUTSHOT_GUT_SHOT              = 384343,
    SPELL_GUTSHOT_ENSNARING_TRAP        = 384148,
    SPELL_GUTSHOT_MASTERS_CALL          = 384638,
    SPELL_GUTSHOT_MEAT_TOSS             = 384416,
    SPELL_GUTSHOT_FEEDING_FRENZY        = 384764,

    SPELL_HYENA_BOUNDING_LEAP           = 384558,
    SPELL_HYENA_BOUNDING_LEAP_NORMAL    = 454527,
    SPELL_HYENA_CRIPPLING_BITE          = 384575
};

enum BrackenhideHollowTreemouthSpells
{
    SPELL_TREEMOUTH_VINE_WHIP           = 377559,
    SPELL_TREEMOUTH_DECAY_SPRAY         = 376811,
    SPELL_TREEMOUTH_INFECTIOUS_SPIT     = 377864,
    SPELL_TREEMOUTH_GRASPING_VINES      = 376933,
    SPELL_TREEMOUTH_CONSUME             = 377222,

    SPELL_SLIME_BURST                   = 378055,
    SPELL_SLIME_GUSHING_OOZE            = 381770
};

enum BrackenhideHollowWratheyeSpells
{
    SPELL_WRATHEYE_DECAYSTRIKE                = 373917,
    SPELL_WRATHEYE_CHOKING_ROTCLOUD           = 376170,
    SPELL_WRATHEYE_CHOKING_ROTCLOUD_NORMAL    = 454584,
    SPELL_WRATHEYE_DECAYING_STRENGTH          = 374186,
    SPELL_WRATHEYE_DECAYING_STRENGTH_NORMAL   = 454585,

    SPELL_TOTEM_ROTTING_BURST                 = 373939,
    SPELL_TOTEM_ROTTING_BURST_NORMAL          = 454583
};

enum BrackenhideHollowHackclawsWarBandSpells
{
    SPELL_RIRA_SAVAGE_CHARGE                  = 381419,
    SPELL_RIRA_BLADESTORM                     = 377827,
    SPELL_RIRA_CLEAVE                         = 377807,

    SPELL_GASHTOOTH_DECAYED_SENSES            = 381379,
    SPELL_GASHTOOTH_DECAYED_SENSES_NORMAL     = 454471,
    SPELL_GASHTOOTH_GASH_FRENZY               = 378020,
    SPELL_GASHTOOTH_GASH_FRENZY_NORMAL        = 454472,
    SPELL_GASHTOOTH_MARKED_FOR_BUTCHERY       = 378229,

    SPELL_TRICKTOTEM_GREATER_HEALING_RAPIDS   = 377950,
    SPELL_TRICKTOTEM_BLOODFRENZY              = 377965,
    SPELL_TRICKTOTEM_EARTH_BOLT               = 378155,

    SPELL_HEXTRICK_TOTEM_HEXTRICK             = 381466
};
