/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <cstdint>

uint32_t const DreamriftEncounterCount = 1;

enum DreamriftData
{
    DATA_CHIMAERUS  = 0     // 3306
};

enum DreamriftCreatures
{
    // Chimaerus the Undreamt God, spawned in the arena
    BOSS_CHIMAERUS                      = 256116,
    // second Chimaerus spawned further in
    BOSS_CHIMAERUS_ALT                  = 245569,

    // manifestations placed in the Rift
    NPC_RIFT_SWARMING_SHADE             = 245555,
    NPC_RIFT_COLOSSAL_HORROR            = 245556,
    NPC_RIFT_HAUNTING_ESSENCE           = 245575,

    // manifestations Chimaerus coalesces during the fight
    NPC_COALESCED_SWARMING_SHADE        = 248319,
    NPC_COALESCED_HAUNTING_ESSENCE      = 248444,
    NPC_COALESCED_COLOSSAL_HORROR       = 249341
};

enum DreamriftSharedSpells
{
    SPELL_DREAMRIFT_BERSERK                     = 26662
};

enum DreamriftChimaerusSpells
{
    // Stage One: Insatiable Hunger
    SPELL_CHIMAERUS_ALNDUST_UPHEAVAL            = 1262289,
    SPELL_CHIMAERUS_RIFT_EMERGENCE              = 1258610,
    SPELL_CHIMAERUS_INSATIABLE                  = 1252863,
    SPELL_CHIMAERUS_CANNIBALIZED_ESSENCE        = 1245844,
    SPELL_CHIMAERUS_CONSUMING_MIASMA            = 1257087,
    SPELL_CHIMAERUS_CAUSTIC_PHLEGM              = 1246653,
    SPELL_CHIMAERUS_RENDING_TEAR                = 1272726,
    SPELL_CHIMAERUS_CONSUME                     = 1245396,

    // Stage Two: To The Skies
    SPELL_CHIMAERUS_CORRUPTED_DEVASTATION       = 1245486,
    SPELL_CHIMAERUS_RAVENOUS_DIVE               = 1245406,
    SPELL_CHIMAERUS_SKY_CAUSTIC_PHLEGM          = 1246621,
    SPELL_CHIMAERUS_SKY_CONSUMING_MIASMA        = 1257085,
    SPELL_CHIMAERUS_RIFT_SHROUD                 = 1246132,

    // Coalesced Manifestations
    SPELL_MANIFESTATION_RIFT_SICKNESS           = 1250953,
    SPELL_HORROR_DISCORDANT_ROAR                = 1249207,
    SPELL_HORROR_COLOSSAL_STRIKES               = 1262020,
    SPELL_ESSENCE_FEARSOME_CRY                  = 1249017,
    SPELL_ESSENCE_ESSENCE_BOLT                  = 1261997
};
