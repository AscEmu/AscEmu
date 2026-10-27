/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <cstdint>

uint32_t const DarkflameCleftEncounterCount = 4;

enum DarkflameCleftData
{
    DATA_THE_CANDLE_KING = 0,   // 2787
    DATA_THE_DARKNESS    = 1,   // 2788
    DATA_BLAZIKON        = 2,   // 2826
    DATA_OL_WAXBEARD     = 3    // 2829
};

enum DarkflameCleftCreatures
{
    BOSS_THE_CANDLE_KING        = 208745,
    NPC_WAX_STATUE              = 209603,

    BOSS_THE_DARKNESS           = 210797,
    // second template of The Darkness
    BOSS_THE_DARKNESS_ALT       = 208747,
    NPC_WRIGGLING_DARKSPAWN     = 213008,
    NPC_DARKNESS_CANDLE         = 219866,

    BOSS_BLAZIKON               = 208743,

    BOSS_OL_WAXBEARD            = 210153,
    // second template of Ol' Waxbeard
    BOSS_OL_WAXBEARD_ALT        = 210149,
    NPC_MENIAL_LABORER          = 210148
};

enum DarkflameCleftGameObjects
{
    // catacombs gate behind The Candle King
    GO_CANDLE_KING_GATE         = 430177,
    // both ends of Blazikon's candle hall
    GO_BLAZIKON_NORTH_DOOR      = 430173,
    GO_BLAZIKON_SOUTH_DOOR      = 430175
};

enum CandleKingSpells
{
    SPELL_CANDLE_KING_EERIE_MOLDS       = 420659,
    SPELL_CANDLE_KING_CURSED_WAX        = 421648,
    SPELL_CANDLE_KING_DARKFLAME_PICKAXE = 421277,
    SPELL_CANDLE_KING_PARANOID_MIND     = 426145,
    SPELL_CANDLE_KING_THROW_DARKFLAME   = 420696
};

enum DarknessSpells
{
    SPELL_DARKNESS_RISING_GLOOM         = 426943,
    SPELL_DARKNESS_CALL_DARKSPAWN       = 427157,
    SPELL_DARKNESS_DRAIN_LIGHT          = 427176,
    SPELL_DARKNESS_UMBRAL_SLASH         = 427100,
    SPELL_DARKNESS_SHADOWBLAST          = 427011
};

enum BlazikonSpells
{
    SPELL_BLAZIKON_ENKINDLING_INFERNO   = 423099,
    SPELL_BLAZIKON_WICKLIGHTER_BARRAGE  = 421638,
    SPELL_BLAZIKON_INCITE_FLAMES        = 424223,
    SPELL_BLAZIKON_DOUSING_BREATH       = 425394,
    SPELL_BLAZIKON_BLAZING_STORMS       = 443835
};

enum OlWaxbeardSpells
{
    SPELL_WAXBEARD_KOL_TO_ARMS          = 421875,
    SPELL_WAXBEARD_RECKLESS_CHARGE      = 422150,
    SPELL_WAXBEARD_CAVE_IN              = 422270,
    SPELL_WAXBEARD_LURING_CANDLEFLAME   = 422162,
    SPELL_WAXBEARD_ROCK_BUSTER          = 422245,

    SPELL_LABORER_CRUDE_WEAPONS         = 422682
};
