/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <cstdint>

uint32_t const BloodmaulSlagMinesEncounterCount = 4;

enum BloodmaulSlagMinesData
{
    DATA_ROLTALL    = 0,  // 1652
    DATA_CRUSHTO    = 1,  // 1653
    DATA_GUGROKK    = 2,  // 1654
    // Forgemaster Gog'duh and Magmolatus
    DATA_MAGMOLATUS = 3   // 1655
};

enum BloodmaulSlagMinesCreatures
{
    BOSS_CRUSHTO             = 74787,
    BOSS_GOGDUH              = 74366,
    BOSS_MAGMOLATUS          = 74475,
    BOSS_ROLTALL             = 75786,
    BOSS_GUGROKK             = 74790,

    NPC_CAPTURED_MINER       = 83725,
    NPC_RUINATION            = 74570,
    NPC_CALAMITY             = 74571,
    NPC_FIERY_BOULDER_LEFT   = 75853,
    NPC_FIERY_BOULDER_MIDDLE = 75854,
    NPC_FIERY_BOULDER_RIGHT  = 75828,
    NPC_UNSTABLE_SLAG        = 74927
};

enum BloodmaulSlagMinesGameObjects
{
    // open when Magmolatus falls
    GO_ROLTALL_ENTRANCE_WALL = 224643,
    GO_ROLTALL_DRAW_BRIDGE   = 224487,
    // opens when Roltall falls
    GO_ROLTALL_EXIT_WALL     = 225693
};

enum BloodmaulSlagMinesActions
{
    ACTION_MAGMOLATUS_RELEASE = 1
};

enum CrushtoSpells
{
    SPELL_CRUSHTO_EARTH_CRUSH       = 153679,
    SPELL_CRUSHTO_FEROCIOUS_YELL    = 150759,
    SPELL_CRUSHTO_RAISE_THE_MINERS  = 150801,
    SPELL_CRUSHTO_WILD_SLAM         = 150753,
    SPELL_CRUSHTO_CRUSHING_LEAP     = 150745
};

enum GogduhSpells
{
    SPELL_GOGDUH_MAGMA_BARRAGE = 150004
};

enum MagmolatusSpells
{
    SPELL_MAGMOLATUS_WITHERING_FLAMES = 150032,
    SPELL_MAGMOLATUS_MOLTEN_IMPACT    = 150045,
    SPELL_MAGMOLATUS_SLAG_SMASH       = 150023
};

enum ForgeGuardianSpells
{
    SPELL_RUINATION_ROUGH_SMASH   = 149941,
    SPELL_RUINATION_SHATTER_EARTH = 150324,
    SPELL_CALAMITY_SCORCH         = 150290,
    SPELL_CALAMITY_DANCING_FLAMES = 149975,
    SPELL_CALAMITY_FIRESTORM      = 149997
};

enum RoltallSpells
{
    SPELL_ROLTALL_FIERY_BOULDER   = 153247,
    SPELL_ROLTALL_HEAT_WAVE       = 152940,
    SPELL_ROLTALL_BURNING_SLAG    = 152918,
    SPELL_ROLTALL_SCORCHING_AURA  = 167738,

    // cast by the rolling boulders
    SPELL_BOULDER_IMPACT          = 152742,
    SPELL_BOULDER_TRAMPLE         = 152843
};

enum GugrokkSpells
{
    SPELL_GUGROKK_MAGMA_ERUPTION  = 150764,
    SPELL_GUGROKK_MOLTEN_BLAST    = 150677,
    SPELL_GUGROKK_FLAME_BUFFET    = 163802,

    SPELL_SLAG_EMPOWERED_FLAMES   = 152091
};
