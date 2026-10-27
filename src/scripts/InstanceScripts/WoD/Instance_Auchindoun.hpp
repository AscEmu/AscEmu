/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <cstdint>

uint32_t const AuchindounEncounterCount = 4;

enum AuchindounData
{
    DATA_AZZAKEL  = 0,  // 1678
    DATA_NYAMI    = 1,  // 1685
    DATA_KAATHAR  = 2,  // 1686
    DATA_TERONGOR = 3   // 1714
};

enum AuchindounCreatures
{
    BOSS_KAATHAR            = 86217,
    BOSS_KAATHAR_SPAWNED    = 75839,  // entry of the placed spawn
    BOSS_NYAMI              = 76177,
    BOSS_AZZAKEL            = 75927,
    BOSS_TERONGOR           = 77734,

    // Torn Spirits adds
    NPC_MALEFIC_DEFENDER    = 76283,
    NPC_TWISTED_MAGUS       = 76296,
    NPC_SPITEFUL_ARBITER    = 76284,

    // Claws of Argus demons
    NPC_BLAZING_TRICKSTER   = 79511,
    NPC_FELGUARD            = 76259,
    NPC_CACKLING_PYROMANIAC = 76260,

    // heroic add of Teron'gor
    NPC_FELBORNE_ABYSSAL    = 77905
};

enum AuchindounGameObjects
{
    // opens when Kaathar falls
    GO_KAATHAR_HOLY_BARRIER = 230399,
    // opens when Azzakel falls
    GO_AZZAKEL_FEL_BARRIER  = 230400
};

enum KaatharSpells
{
    SPELL_KAATHAR_SANCTIFIED_STRIKE = 152954,
    SPELL_KAATHAR_HALLOWED_GROUND   = 154526,
    SPELL_KAATHAR_HOLY_SHIELD       = 153002,
    SPELL_KAATHAR_CONSECRATED_LIGHT = 153006,
    SPELL_KAATHAR_FATE              = 157465
};

enum NyamiSpells
{
    SPELL_NYAMI_MIND_SPIKE       = 157043,
    SPELL_NYAMI_SHADOW_WORD_PAIN = 154477,
    SPELL_NYAMI_SOUL_VESSEL      = 155327,
    SPELL_NYAMI_TORN_SPIRITS     = 153994
};

enum NyamiAddSpells
{
    SPELL_DEFENDER_CRUSADER_STRIKE = 176931,
    SPELL_MAGUS_ARCANE_BOLT        = 154235,
    SPELL_MAGUS_ARCANE_BOMB        = 157652,
    SPELL_ARBITER_HAMMER           = 154218,
    SPELL_ARBITER_RADIANT_FURY     = 154261
};

enum AzzakelSpells
{
    SPELL_AZZAKEL_FEL_LASH          = 153234,
    SPELL_AZZAKEL_CURTAIN_OF_FLAME  = 153392,
    SPELL_AZZAKEL_MALEVOLENT_CRUSH  = 153499,
    SPELL_AZZAKEL_FEL_POOL          = 153500,
    SPELL_AZZAKEL_CLAWS_OF_ARGUS    = 153764
};

enum TerongorSpells
{
    // shared
    SPELL_TERONGOR_SHADOW_BOLT          = 156829,
    SPELL_TERONGOR_CORRUPTION           = 156842,
    SPELL_TERONGOR_RAIN_OF_FIRE         = 156974,
    SPELL_TERONGOR_DRAIN_LIFE           = 156854,
    SPELL_TERONGOR_CURSE_OF_EXHAUSTION  = 164841,
    SPELL_TERONGOR_CHAOS_BOLT           = 156975,

    // affliction
    SPELL_TERONGOR_AGONY                = 156925,
    SPELL_TERONGOR_UNSTABLE_AFFLICTION  = 156954,
    SPELL_TERONGOR_SEED_OF_MALEVOLENCE  = 156921,

    // destruction
    SPELL_TERONGOR_IMMOLATE             = 156964,
    SPELL_TERONGOR_INCINERATE           = 156963,
    SPELL_TERONGOR_CONFLAGRATE          = 154083,

    // demonology
    SPELL_TERONGOR_DOOM                 = 156965,
    SPELL_TERONGOR_DEMONIC_LEAP         = 157039,
    SPELL_TERONGOR_TOUCH_OF_CHAOS       = 169028,

    // school transformations at 75%
    SPELL_TERONGOR_AFFLICTION_FORM      = 156863,
    SPELL_TERONGOR_DESTRUCTION_FORM     = 156866,
    SPELL_TERONGOR_DEMONOLOGY_FORM      = 156919
};
