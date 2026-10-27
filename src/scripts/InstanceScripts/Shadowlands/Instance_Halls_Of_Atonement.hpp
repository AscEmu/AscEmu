/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <cstdint>

uint32_t const HallsOfAtonementEncounterCount = 4;

enum HallsOfAtonementData
{
    DATA_ECHELON                    = 0,    // 2380
    DATA_LORD_CHAMBERLAIN           = 1,    // 2381
    DATA_HALKIAS                    = 2,    // 2401
    DATA_HIGH_ADJUDICATOR_ALEEZ     = 3     // 2403
};

enum HallsOfAtonementCreatures
{
    // Echelon
    BOSS_ECHELON                        = 164185,
    NPC_ECHELON_UNDYING_STONEFIEND      = 164363,

    // Lord Chamberlain
    BOSS_LORD_CHAMBERLAIN               = 164218,
    NPC_CHAMBERLAIN_SINSTONE_STATUE     = 165737,

    // Halkias, the Sin-Stained Goliath
    BOSS_HALKIAS                        = 165408,

    // High Adjudicator Aleez
    BOSS_HIGH_ADJUDICATOR_ALEEZ         = 165410,
    NPC_ALEEZ_GHASTLY_PARISHIONER       = 165913
};

enum EchelonSpells
{
    SPELL_ECHELON_STONE_CALL            = 319733,
    SPELL_ECHELON_FLESH_TO_STONE        = 319603,
    SPELL_ECHELON_STONE_SHATTERING_LEAP = 319592,
    SPELL_ECHELON_BLOOD_TORRENT         = 319702,

    SPELL_STONEFIEND_VOLATILE_TRANSFORMATION = 328125,
    SPELL_STONEFIEND_STONE_FORM         = 319724
};

enum LordChamberlainSpells
{
    SPELL_CHAMBERLAIN_CHORUS            = 341245,
    SPELL_CHAMBERLAIN_TELEKINETIC_ONSLAUGHT = 329113,
    SPELL_CHAMBERLAIN_TELEKINETIC_REPULSION = 323129,
    SPELL_CHAMBERLAIN_RITUAL_OF_WOE     = 323393,
    SPELL_CHAMBERLAIN_TELEKINETIC_TOSS  = 323143,
    SPELL_CHAMBERLAIN_UNLEASHED_SUFFERING = 323236,
    SPELL_CHAMBERLAIN_ERUPTING_TORMENT  = 1236964
};

enum HalkiasSpells
{
    SPELL_HALKIAS_REFRACTED_SINLIGHT    = 322913,
    SPELL_HALKIAS_CRUMBLING_SLAM        = 322936,
    SPELL_HALKIAS_HEAVE_DEBRIS          = 322943,
    SPELL_HALKIAS_SINLIGHT_VISIONS      = 322977
};

enum HighAdjudicatorAleezSpells
{
    SPELL_ALEEZ_ANIMA_BOLT              = 323538,
    SPELL_ALEEZ_ANIMA_FOUNTAIN          = 329340,

    SPELL_PARISHIONER_HAUNTING_FIXATION = 323650,
    SPELL_PARISHIONER_EPHEMERAL_VISAGE  = 323741,
    SPELL_PARISHIONER_PULSE_FROM_BEYOND = 323852
};
