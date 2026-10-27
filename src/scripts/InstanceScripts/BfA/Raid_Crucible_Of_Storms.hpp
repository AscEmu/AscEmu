/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <cstdint>

uint32_t const CrucibleOfStormsEncounterCount = 2;

enum CrucibleOfStormsData
{
    // Zaxasj the Speaker and Fa'thuul the Feared
    DATA_RESTLESS_CABAL     = 0,    // 2269
    DATA_UUNAT              = 1     // 2273
};

enum CrucibleOfStormsCreatures
{
    // The Restless Cabal
    BOSS_ZAXASJ_THE_SPEAKER             = 144755,
    BOSS_FATHUUL_THE_FEARED             = 144754,
    NPC_CABAL_VISAGE_FROM_BEYOND        = 144996,
    NPC_CABAL_ELDRITCH_ABOMINATION      = 145053,

    // Uu'nat, Harbinger of the Void
    BOSS_UUNAT                          = 145371,
    NPC_UUNAT_UNDYING_GUARDIAN          = 146829,
    NPC_UUNAT_PRIMORDIAL_MINDBENDER     = 146940,
    NPC_UUNAT_SWARMING_VOIDSPAWN        = 146945,
    NPC_UUNAT_UNKNOWABLE_TERROR         = 147024
};

enum CrucibleOfStormsSharedSpells
{
    SPELL_CRUCIBLE_BERSERK              = 26662
};

enum CrucibleCabalSpells
{
    // Zaxasj the Speaker
    SPELL_ZAXASJ_APHOTIC_BLAST          = 282386,
    SPELL_ZAXASJ_CEREBRAL_ASSAULT       = 282589,
    SPELL_ZAXASJ_DARK_HERALD            = 282561,

    // Fa'thuul the Feared
    SPELL_FATHUUL_SHEAR_MIND            = 282384,
    SPELL_FATHUUL_CRUSHING_DOUBT        = 282432,
    SPELL_FATHUUL_ELDRITCH_REVELATION   = 282617,

    // relic powers released at 75%, 50% and 25%
    SPELL_RELIC_EMBRACE_OF_THE_VOID     = 282738,
    SPELL_RELIC_CUSTODY_OF_THE_DEEP     = 282750,
    SPELL_RELIC_STORM_OF_ANNIHILATION   = 282742,

    SPELL_VISAGE_TERRIFYING_ECHO        = 282517,
    SPELL_VISAGE_COALESCE_VOID          = 282524,
    SPELL_ABOMINATION_WITNESS_THE_END   = 282621
};

enum CrucibleUunatSpells
{
    SPELL_UUNAT_TOUCH_OF_THE_END        = 284851,
    SPELL_UUNAT_OBLIVION_TEAR           = 285185,
    SPELL_UUNAT_VOID_CRASH              = 285410,
    SPELL_UUNAT_MADDENING_EYES          = 285345,
    SPELL_UUNAT_PIERCING_GAZE           = 285367,
    SPELL_UUNAT_GIFT_OBSCURITY          = 285453,
    SPELL_UUNAT_GIFT_HYSTERIA           = 285638,
    SPELL_UUNAT_GIFT_LUNACY             = 285685,
    SPELL_UUNAT_INSATIABLE_TORMENT      = 285652,

    SPELL_GUARDIAN_SHROUD_OF_WHISPERS   = 285310,
    SPELL_GUARDIAN_UNNATURAL_REGENERATION = 285333,
    SPELL_MINDBENDER_CONSUME_ESSENCE    = 285427,
    SPELL_TERROR_UNKNOWABLE_TERROR      = 285562
};
