/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <cstdint>

uint32_t const BlackRookHoldEncounterCount = 4;

enum BlackRookHoldData
{
    DATA_AMALGAM_OF_SOULS   = 0,  // 1832
    DATA_ILLYSANNA          = 1,  // 1833
    DATA_SMASHSPITE         = 2,  // 1834
    // Kur'talos Ravencrest, Latosius and Dantalionax
    DATA_KURTALOS           = 3   // 1835
};

enum BlackRookHoldCreatures
{
    BOSS_AMALGAM_OF_SOULS       = 98542,
    NPC_RESTLESS_SOUL           = 99664,

    BOSS_ILLYSANNA              = 98696,
    NPC_SOUL_TORN_VANGUARD      = 100485,
    NPC_RISEN_ARCANIST          = 100486,

    BOSS_SMASHSPITE             = 98949,
    NPC_FEL_BAT                 = 100759,

    BOSS_KURTALOS               = 98965,
    BOSS_LATOSIUS               = 98970,
    BOSS_DANTALIONAX            = 99611,
    NPC_IMAGE_OF_LATOSIUS       = 101028,
    NPC_SOUL_OF_RAVENCREST      = 101054
};

enum BlackRookHoldGameObjects
{
    // crypt of the Amalgam
    GO_AMALGAM_DOOR_1           = 247403,
    GO_AMALGAM_DOOR_2           = 247404,
    GO_AMALGAM_DOOR_3           = 247405,
    GO_AMALGAM_DOOR_4           = 247406,
    GO_AMALGAM_EXIT             = 247407,

    GO_ILLYSANNA_DOOR_1         = 247399,
    GO_ILLYSANNA_DOOR_2         = 247401,
    GO_ILLYSANNA_DOOR_3         = 247400,

    GO_SMASHSPITE_DOOR          = 247402,
    GO_SMASHSPITE_EXIT_1        = 245104,
    GO_SMASHSPITE_EXIT_2        = 245105,

    GO_KURTALOS_DOOR            = 247498
};

enum BlackRookHoldActions
{
    ACTION_LATOSIUS_BETRAYAL    = 1,
    ACTION_KURTALOS_SOUL_SAPPED
};

enum BlackRookHoldAmalgamSpells
{
    SPELL_AMALGAM_SWIRLING_SCYTHE   = 195254,
    SPELL_AMALGAM_SOUL_ECHOES       = 194966,
    SPELL_AMALGAM_REAP_SOUL         = 194956,
    SPELL_AMALGAM_CALL_SOULS        = 196078,
    SPELL_AMALGAM_SOUL_BURST        = 196587,
    SPELL_AMALGAM_SOULGORGE         = 196930
};

enum BlackRookHoldIllysannaSpells
{
    SPELL_ILLYSANNA_BRUTAL_GLAIVE   = 197546,
    SPELL_ILLYSANNA_VENGEFUL_SHEAR  = 197418,
    SPELL_ILLYSANNA_DARK_RUSH_MARK  = 197478,
    SPELL_ILLYSANNA_DARK_RUSH       = 197484,
    SPELL_ILLYSANNA_LEAP            = 197622,
    SPELL_ILLYSANNA_EYE_BEAMS       = 197674
};

enum BlackRookHoldSmashspiteSpells
{
    SPELL_SMASHSPITE_EARTHSHAKING_STOMP = 198073,
    SPELL_SMASHSPITE_BRUTAL_HAYMAKER    = 198245,
    SPELL_SMASHSPITE_HATEFUL_GAZE       = 198079,
    SPELL_SMASHSPITE_HATEFUL_CHARGE     = 198250,
    SPELL_FEL_BAT_FEL_VOMIT             = 198446
};

enum BlackRookHoldKurtalosSpells
{
    SPELL_KURTALOS_UNERRING_SHEAR       = 198635,
    SPELL_KURTALOS_WHIRLING_BLADE       = 198641,

    SPELL_LATOSIUS_SHADOW_BOLT          = 198833,
    SPELL_LATOSIUS_DARK_BLAST           = 198820,
    SPELL_LATOSIUS_SAP_SOUL             = 198961,

    SPELL_DANTALIONAX_SHADOW_BOLT_VOLLEY = 202019,
    SPELL_DANTALIONAX_CLOUD_OF_HYPNOSIS  = 199143,
    SPELL_DANTALIONAX_DREADLORDS_GUILE   = 199193,
    SPELL_DANTALIONAX_STINGING_SWARM     = 201733,
    SPELL_IMAGE_DARK_OBLITERATION        = 199567,

    SPELL_RAVENCREST_GHOST_VISUAL        = 199243,
    SPELL_RAVENCREST_LEGACY              = 199368
};
