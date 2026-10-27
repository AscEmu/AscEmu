/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <cstdint>

uint32_t const TheArcwayEncounterCount = 5;

enum TheArcwayData
{
    DATA_CORSTILAX          = 0,  // 1825
    DATA_NALTIRA            = 1,  // 1826
    DATA_IVANYR             = 2,  // 1827
    DATA_GENERAL_XAKAL      = 3,  // 1828
    DATA_ADVISOR_VANDROS    = 4   // 1829
};

enum TheArcwayCreatures
{
    BOSS_CORSTILAX              = 98205,
    NPC_SUPPRESSION_PROTOCOL    = 99628,
    NPC_DESTABILIZED_ORB        = 110863,

    BOSS_NALTIRA                = 98207,
    NPC_VICIOUS_MANAFANG        = 110966,

    BOSS_IVANYR                 = 98203,
    NPC_NIGHT_CRYSTAL           = 98734,

    BOSS_GENERAL_XAKAL          = 98206,
    NPC_FEL_FISSURE             = 100342,
    NPC_DREAD_FELBAT            = 100393,

    BOSS_ADVISOR_VANDROS        = 98208,
    NPC_CHRONO_SHARD            = 102849
};

enum TheArcwayGameObjects
{
    // open after Ivanyr
    GO_ARCWAY_DOOR_D_BOTTOM = 250273,
    GO_ARCWAY_DOOR_G        = 250275,
    GO_ARCWAY_DOOR_I        = 250635,

    // open after Corstilax
    GO_ARCWAY_DOOR_J        = 250636,
    GO_ARCWAY_DOOR_L        = 250637,

    // open after Nal'tira
    GO_ARCWAY_DOOR_A        = 250272,
    GO_ARCWAY_DOOR_B        = 250278,
    GO_ARCWAY_DOOR_K        = 250276
};

enum CorstilaxSpells
{
    SPELL_CORSTILAX_SUPPRESSION_PROTOCOL    = 196047,
    SPELL_CORSTILAX_PROTOCOL_FIXATE         = 196068,
    SPELL_CORSTILAX_PROTOCOL_BEAM           = 196070,
    SPELL_CORSTILAX_QUARANTINE              = 195791,
    SPELL_CORSTILAX_CLEANSING_FORCE         = 196115,
    SPELL_CORSTILAX_DESTABILIZED_ORB        = 220481,
    SPELL_DESTABILIZED_ORB_AURA             = 220482,
    SPELL_CORSTILAX_NIGHTWELL_ENERGY        = 195362
};

enum NaltiraSpells
{
    SPELL_NALTIRA_BLINK_STRIKES_VISUAL  = 199864,
    SPELL_NALTIRA_BLINK_STRIKES_JUMP    = 199810,
    SPELL_NALTIRA_BLINK_STRIKES_HIT     = 199811,
    SPELL_NALTIRA_NETHER_VENOM          = 199956,
    SPELL_NALTIRA_TANGLED_WEB           = 200227
};

enum IvanyrSpells
{
    SPELL_IVANYR_ARCANE_BLAST           = 196357,
    SPELL_IVANYR_VOLATILE_MAGIC         = 196562,
    SPELL_IVANYR_OVERCHARGE_MANA        = 196392,
    SPELL_IVANYR_NETHER_LINK            = 196804,
    SPELL_IVANYR_WITHERING_CONSUMPTION  = 196549,
    SPELL_IVANYR_CONSUME_ESSENCE        = 196877
};

enum XakalSpells
{
    SPELL_XAKAL_FEL_FISSURE         = 197776,
    SPELL_FEL_FISSURE_AREA          = 197542,
    SPELL_XAKAL_SHADOW_SLASH        = 212028,
    SPELL_XAKAL_WICKED_SLAM         = 197810,

    SPELL_FELBAT_BOMBARDMENT        = 197787,
    SPELL_FELBAT_THIRST_FOR_BLOOD   = 220533
};

enum VandrosSpells
{
    SPELL_VANDROS_ACCELERATING_BLAST    = 203176,
    SPELL_VANDROS_CHRONO_SHARDS         = 203254,
    SPELL_VANDROS_FORCE_BOMB            = 202974,
    SPELL_VANDROS_UNSTABLE_MANA         = 220871,

    SPELL_CHRONO_SHARD_PULSE_VISUAL     = 203835,
    SPELL_CHRONO_SHARD_TIME_SPLIT       = 203833
};
