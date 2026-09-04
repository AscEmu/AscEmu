/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

// Baradin Hold - three rotating single-boss encounters (Cataclysm 4.1 - 4.3 tiers).
// Boss identities and mechanics verified against wowhead.

uint32_t const BaradinHoldEncounterCount = 3;

enum BaradinHoldData
{
    DATA_ARGALOTH = 0,
    DATA_OCCUTHAR = 1,
    DATA_ALIZABAL = 2
};

enum BaradinHoldCreatures
{
    BOSS_ARGALOTH        = 47120,
    NPC_FEL_FLAMES       = 47829,

    BOSS_OCCUTHAR        = 52363,
    NPC_EYE_OF_OCCUTHAR  = 52389,
    NPC_FOCUS_FIRE_DUMMY = 52369,

    BOSS_ALIZABAL        = 55869,

    NPC_DISCIPLE_OF_HATE = 56350
};

enum DiscipleOfHateSpells
{
    SPELL_DISCIPLE_OF_HATE_RUN_THROUGH     = 105859,
    SPELL_DISCIPLE_OF_HATE_WHIRL_OF_BLADES = 105855
};

enum BaradinHoldGameObjects
{
    GO_ARGALOTH_DOOR = 207619,
    GO_OCCUTHAR_DOOR = 208953,
    GO_ALIZABAL_DOOR = 209849
};

enum ArgalothSpells
{
    SPELL_ARGALOTH_METEOR_SLASH_VISUAL     = 88949,
    SPELL_ARGALOTH_CONSUMING_DARKNESS_10   = 88954,
    SPELL_ARGALOTH_CONSUMING_DARKNESS_25   = 95173,
    SPELL_ARGALOTH_METEOR_SLASH_10         = 88942,
    SPELL_ARGALOTH_METEOR_SLASH_25         = 95172,
    SPELL_ARGALOTH_FEL_FIRESTORM           = 88972,
    SPELL_ARGALOTH_FEL_FIRESTORM_TRIGGERED = 88973,
    SPELL_ARGALOTH_BERSERK                 = 47008,

    SPELL_FEL_FLAMES                       = 88999
};

enum OccutharSpells
{
    SPELL_OCCUTHAR_SEARING_SHADOWS      = 96913,
    SPELL_OCCUTHAR_FOCUSED_FIRE_TRIGGER = 96872,
    SPELL_OCCUTHAR_FOCUSED_FIRE         = 96884,
    SPELL_OCCUTHAR_EYES_OF_OCCUTHAR     = 96920,
    SPELL_OCCUTHAR_GAZE_OF_OCCUTHAR     = 96942,
    SPELL_OCCUTHAR_BERSERK              = 47008
};

enum AlizabalSpells
{
    SPELL_ALIZABAL_SKEWER                 = 104936,
    SPELL_ALIZABAL_BLADE_DANCE            = 105828,
    SPELL_ALIZABAL_BLADE_DANCE_CHARGE     = 105726,
    SPELL_ALIZABAL_BLADE_DANCE_ROOT       = 105784,
    SPELL_ALIZABAL_SEETHING_HATE_10       = 105065,
    SPELL_ALIZABAL_SEETHING_HATE_25       = 108090,
    SPELL_ALIZABAL_SEETHING_HATE_PERIODIC = 105067,
    SPELL_ALIZABAL_BERSERK                = 47008
};
