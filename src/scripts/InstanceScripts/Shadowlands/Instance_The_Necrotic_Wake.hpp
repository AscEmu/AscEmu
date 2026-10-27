/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <cstdint>

uint32_t const TheNecroticWakeEncounterCount = 4;

enum TheNecroticWakeData
{
    DATA_BLIGHTBONE             = 0,    // 2387
    DATA_AMARTH                 = 1,    // 2388
    DATA_SURGEON_STITCHFLESH    = 2,    // 2389
    DATA_NALTHOR                = 3     // 2390
};

enum TheNecroticWakeCreatures
{
    // Blightbone
    BOSS_BLIGHTBONE                     = 162691,
    NPC_BLIGHTBONE_CARRION_WORM         = 164702,
    // second Carrion Worm template of the dungeon
    NPC_BLIGHTBONE_CARRION_WORM_ALT     = 200035,

    // Amarth, The Harvester
    BOSS_AMARTH                         = 162692,
    NPC_AMARTH_REANIMATED_WARRIOR       = 164427,
    NPC_AMARTH_REANIMATED_MAGE          = 164414,

    // Surgeon Stitchflesh
    BOSS_SURGEON_STITCHFLESH            = 162689,
    NPC_STITCHFLESHS_CREATION           = 164578,

    // Nalthor the Rimebinder
    BOSS_NALTHOR_THE_RIMEBINDER         = 162693,
    NPC_NALTHOR_ZOLRAMUS_SIPHONER       = 164815
};

enum BlightboneSpells
{
    SPELL_BLIGHTBONE_HEAVING_RETCH      = 320596,
    SPELL_BLIGHTBONE_FETID_GAS          = 320637,
    SPELL_BLIGHTBONE_CRUNCH             = 320655,

    SPELL_CARRION_WORM_BLOOD_GORGE      = 320614,
    SPELL_CARRION_WORM_CARRION_ERUPTION = 320631
};

enum AmarthSpells
{
    SPELL_AMARTH_LAND_OF_THE_DEAD       = 321226,
    SPELL_AMARTH_FINAL_HARVEST          = 321247,
    SPELL_AMARTH_UNHOLY_FRENZY          = 320012,
    SPELL_AMARTH_NECROTIC_BOLT          = 320170,

    SPELL_REANIMATED_MAGE_FROSTBOLT     = 333602
};

enum StitchfleshSpells
{
    SPELL_STITCHFLESH_AWAKEN_CREATION   = 320358,
    SPELL_STITCHFLESH_EMBALMING_ICHOR   = 327664,
    SPELL_STITCHFLESH_STITCHNEEDLE      = 320200,
    SPELL_STITCHFLESH_SEVER_FLESH       = 334488,
    SPELL_STITCHFLESH_ESCAPE            = 320359,

    SPELL_CREATION_MEAT_HOOK            = 322681,
    SPELL_CREATION_MUTILATE             = 320376,
    SPELL_CREATION_FESTERING_ROT        = 334321
};

enum NalthorSpells
{
    SPELL_NALTHOR_COMET_STORM           = 320784,
    SPELL_NALTHOR_ICEBOUND_AEGIS        = 321368,
    SPELL_NALTHOR_FROZEN_BINDS          = 320788,
    SPELL_NALTHOR_ICY_SHARD             = 320771,
    SPELL_NALTHOR_DARK_EXILE            = 321894,

    SPELL_SIPHONER_ENFEEBLE             = 322274
};
