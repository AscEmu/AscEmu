/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <cstdint>

uint32_t const GrimrailDepotEncounterCount = 3;

enum GrimrailDepotData
{
    // Railmaster Rocketspark and Borka the Brute
    DATA_ROCKETSPARK_AND_BORKA = 0,  // 1715
    DATA_NITROGG               = 1,  // 1732
    DATA_TOVRA                 = 2   // 1736
};

enum GrimrailDepotCreatures
{
    BOSS_ROCKETSPARK      = 77803,
    BOSS_BORKA            = 86226,
    // second Borka template of the same encounter
    BOSS_BORKA_ALT        = 77816,
    BOSS_NITROGG          = 79545,
    BOSS_TOVRA            = 80005,

    // Nitrogg reinforcements
    NPC_GROMKAR_BOOMER    = 79720,
    NPC_GROMKAR_GRENADIER = 79739,
    NPC_GROMKAR_GUNNER    = 77483,
    NPC_IRON_INFANTRY     = 79888
};

enum GrimrailDepotGameObjects
{
    // sealed while Rocketspark and Borka fight
    GO_RAILYARD_SPIKED_GATE  = 237441,
    // opens when Rocketspark and Borka are down
    GO_RAILYARD_IRON_GATE    = 237442,
    // opens when Nitrogg falls
    GO_NITROGG_ASSAULT_DOOR  = 232131,
    // sealed while Tovra fights
    GO_TOVRA_SPIKED_GATE     = 237445
};

enum GrimrailDepotActions
{
    // the surviving half of the rail crew reacts
    ACTION_RAIL_PARTNER_FALLEN = 1
};

enum RocketsparkSpells
{
    SPELL_ROCKETSPARK_VX18_TARGET_ELIMINATOR = 162509,
    SPELL_ROCKETSPARK_X21_MISSILE_BARRAGE    = 162407,
    SPELL_ROCKETSPARK_NEW_PLAN               = 161091,
    SPELL_ROCKETSPARK_NEW_PLAN_MISSILE       = 163929
};

enum BorkaSpells
{
    SPELL_BORKA_SLAM                  = 161087,
    SPELL_BORKA_MAD_DASH              = 161102,
    SPELL_BORKA_UNMANAGED_AGGRESSION  = 161092
};

enum NitroggSpells
{
    // on foot
    SPELL_NITROGG_BLACKROCK_GRENADE  = 163539,
    SPELL_NITROGG_BLACKROCK_MORTAR   = 163550,
    // on the assault cannon
    SPELL_NITROGG_SUPPRESSIVE_FIRE   = 160683,
    SPELL_NITROGG_SLAG_BLAST         = 166565
};

enum NitroggReinforcementSpells
{
    SPELL_BOOMER_BLACKROCK_MORTAR     = 163550,
    SPELL_GRENADIER_BLACKROCK_GRENADE = 161150,
    SPELL_GUNNER_SHRAPNEL_BLAST       = 160943
};

enum TovraSpells
{
    SPELL_TOVRA_FREEZING_SNARE = 162080,
    SPELL_TOVRA_SPINNING_SPEAR = 162058
};
