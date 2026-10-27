/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <cstdint>

uint32_t const UnbindingOfKithixEncounterCount = 1;

enum UnbindingOfKithixData
{
    DATA_KITHIX     = 0     // 3513
};

enum UnbindingOfKithixCreatures
{
    BOSS_KITHIX                             = 259664,

    // Voidswarm aqir, not in the 12.1.0 creature templates yet: 0 keeps them unused
    NPC_KITHIX_VENOMOUS_HULK_PLACEHOLDER    = 0,
    NPC_KITHIX_VOIDWEAVER_PLACEHOLDER       = 0,
    NPC_KITHIX_BOMBER_BEETLE_PLACEHOLDER    = 0
};

enum UnbindingOfKithixSharedSpells
{
    SPELL_KITHIX_BERSERK                    = 26662
};

enum UnbindingKithixSpells
{
    // Stage One: Darkness' Domain
    SPELL_KITHIX_VOIDSWARM                  = 1301511,
    SPELL_KITHIX_SUFFOCATING_DARKNESS       = 1302951,
    SPELL_KITHIX_ABYSSAL_GRASP              = 1303406,
    SPELL_KITHIX_DARK_FRENZY                = 1303260,
    SPELL_KITHIX_EXTINGUISH                 = 1304424,
    SPELL_KITHIX_UNSPEAKABLE_HORRORS        = 1304045,
    SPELL_KITHIX_TWISTED_APPENDAGE          = 1303681,
    SPELL_KITHIX_DARK_DEVASTATION           = 1304930,
    SPELL_KITHIX_COMMANDING_PRESENCE        = 1305008,

    // Stage Two: Last Light
    SPELL_KITHIX_STYGIAN_HOWL               = 1301110,
    SPELL_KITHIX_NULL_GATE                  = 1308873,
    SPELL_KITHIX_NEGATION                   = 1318467,

    // Voidswarm
    SPELL_HULK_VENOM_ROAR                   = 1302884,
    SPELL_VOIDWEAVER_VOIDWEAVE              = 1302333,
    SPELL_VOIDWEAVER_NIGHTFALL              = 1302705,
    SPELL_BEETLE_FIXATE                     = 1302295,
    SPELL_BEETLE_GUT_REACTION               = 1302117
};
