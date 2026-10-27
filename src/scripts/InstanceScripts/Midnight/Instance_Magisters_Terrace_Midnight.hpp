/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <cstdint>

uint32_t const MagistersTerraceMidnightEncounterCount = 4;

enum MagistersTerraceMidnightData
{
    DATA_ARCANOTRON_CUSTOS  = 0,    // 3071
    DATA_SERANEL_SUNLASH    = 1,    // 3072
    DATA_GEMELLUS           = 2,    // 3073
    DATA_DEGENTRIUS         = 3     // 3074
};

enum MagistersTerraceMidnightCreatures
{
    BOSS_ARCANOTRON_CUSTOS      = 231861,
    NPC_CUSTOS_ENERGY_ORB       = 234775,

    BOSS_SERANEL_SUNLASH        = 231863,
    // second template of the same boss
    BOSS_SERANEL_SUNLASH_ALT    = 268337,

    BOSS_GEMELLUS               = 231864,
    NPC_GEMELLUS_CLONE          = 239636,
    NPC_GEMELLUS_CLONE_SECOND   = 239809,

    BOSS_DEGENTRIUS             = 231865
};

enum MagistersTerraceMidnightActions
{
    ACTION_CUSTOS_ORB_ABSORBED = 1,
    ACTION_GEMELLUS_ASTRAL_GRASP
};

enum ArcanotronCustosSpells
{
    SPELL_CUSTOS_REFUELING_PROTOCOL     = 474345,
    SPELL_CUSTOS_ARCANE_EMPOWERMENT     = 474407,
    SPELL_CUSTOS_UNSTABLE_ENERGY        = 1243905,
    SPELL_CUSTOS_ARCANE_EXPULSION       = 1214081,
    SPELL_CUSTOS_ETHEREAL_SHACKLES      = 1214038,
    SPELL_CUSTOS_REPULSING_SLAM         = 474496
};

enum SeranelSunlashSpells
{
    SPELL_SERANEL_SUPPRESSION_ZONE      = 1224903,
    SPELL_SERANEL_WAVE_OF_SILENCE       = 1225193,
    SPELL_SERANEL_RUNIC_MARK            = 1225792,
    SPELL_SERANEL_HASTENING_WARD        = 1248689
};

enum GemellusSpells
{
    SPELL_GEMELLUS_TRIPLICATE           = 1223847,
    SPELL_GEMELLUS_SYNAPTIC_NEXUS       = 1223936,
    SPELL_GEMELLUS_ASTRAL_GRASP         = 1224299,
    SPELL_GEMELLUS_COSMIC_STING         = 1284958,
    SPELL_GEMELLUS_VOID_SECRETIONS      = 1224100
};

enum DegentriusSpells
{
    SPELL_DEGENTRIUS_VOID_TORRENT           = 1214714,
    SPELL_DEGENTRIUS_UNSTABLE_VOID_ESSENCE  = 1215087,
    SPELL_DEGENTRIUS_VOID_DESTRUCTION       = 1215161,
    SPELL_DEGENTRIUS_DEVOURING_ENTROPY      = 1215897,
    SPELL_DEGENTRIUS_HULKING_FRAGMENT       = 1280113
};
