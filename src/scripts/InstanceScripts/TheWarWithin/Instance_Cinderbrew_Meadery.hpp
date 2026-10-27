/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <cstdint>

uint32_t const CinderbrewMeaderyEncounterCount = 4;

enum CinderbrewMeaderyData
{
    DATA_BREWMASTER_ALDRYR  = 0,    // 2900
    DATA_IPA                = 1,    // 2929
    DATA_GOLDIE_BARONBOTTOM = 2,    // 2930
    DATA_BENK_BUZZBEE       = 3     // 2931
};

enum CinderbrewMeaderyCreatures
{
    BOSS_BREWMASTER_ALDRYR      = 210271,
    NPC_ALDRYR_THIRSTY_PATRON   = 215819,

    BOSS_IPA                    = 222951,
    NPC_IPA_BREW_DROP           = 219301,

    BOSS_GOLDIE_BARONBOTTOM     = 218523,
    NPC_GOLDIE_CINDERBREW_BOMB  = 217837,

    BOSS_BENK_BUZZBEE           = 218002,
    NPC_BENK_RAVENOUS_CINDERBEE = 218016
};

enum CinderbrewMeaderyActions
{
    ACTION_IPA_BREW_ARRIVED     = 1,
    ACTION_GOLDIE_BOMB_IGNITE
};

enum AldryrSpells
{
    SPELL_ALDRYR_HAPPY_HOUR         = 442525,
    SPELL_ALDRYR_DISREGARD          = 442611,
    SPELL_ALDRYR_THROW_CINDERBREW   = 432182,
    SPELL_ALDRYR_BLAZING_BELCH      = 432198,
    SPELL_ALDRYR_KEG_SMASH          = 432229,
    SPELL_ALDRYR_PATRON_ROWDY_YELL  = 431896
};

enum IpaSpells
{
    SPELL_IPA_SPOUTING_STOUT            = 439365,
    SPELL_IPA_FILL_ER_UP                = 440104,
    SPELL_IPA_BURNING_FERMENTATION      = 439202,
    SPELL_IPA_BURNING_FERMENTATION_DOT  = 439325,
    SPELL_IPA_BOTTOMS_UPPERCUT          = 439031,
    SPELL_IPA_BREW_DROP_FROTHY          = 442122
};

enum GoldieSpells
{
    SPELL_GOLDIE_LET_IT_HAIL        = 435622,
    SPELL_GOLDIE_BURNING_RICOCHET   = 436640,
    SPELL_GOLDIE_CASH_CANNON        = 436592,
    SPELL_GOLDIE_BOMB_CINDER_BOOM   = 435788
};

enum BenkSpells
{
    SPELL_BENK_SNACK_TIME                   = 438025,
    SPELL_BENK_HONEY_MARINADE               = 440134,
    SPELL_BENK_FLUTTERING_WING              = 439524,
    SPELL_BENK_CINDERBEE_SHREDDING_STING    = 438971,
    SPELL_BENK_CINDERBEE_TAMED              = 439006
};
