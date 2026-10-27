/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <cstdint>

uint32_t const DarkheartThicketEncounterCount = 4;

enum DarkheartThicketData
{
    DATA_ARCHDRUID_GLAIDALIS = 0, // 1836
    DATA_OAKHEART            = 1, // 1837
    DATA_DRESARON            = 2, // 1838
    DATA_SHADE_OF_XAVIUS     = 3  // 1839
};

enum DarkheartThicketCreatures
{
    BOSS_ARCHDRUID_GLAIDALIS = 96512,
    NPC_DRUIDIC_PRESERVER    = 100403,

    BOSS_OAKHEART            = 103344,

    BOSS_DRESARON            = 99200,
    NPC_CORRUPTED_DRAGON_EGG = 101072,
    NPC_CORRUPTED_EGG        = 100533,
    NPC_HATESPAWN_WHELPLING  = 101074,

    BOSS_SHADE_OF_XAVIUS     = 99192
};

enum DarkheartThicketGameObjects
{
    // sixteen roots ring the grove, first and last entry of the range
    GO_GLAIDALIS_GROVE_WALL_FIRST = 254984,
    GO_GLAIDALIS_GROVE_WALL_LAST  = 254999,

    GO_DRESARON_LAIR_DOOR_1       = 254849,
    GO_DRESARON_LAIR_DOOR_2       = 254850,

    GO_XAVIUS_ROOT_WALL           = 254855
};

enum DarkheartThicketGlaidalisSpells
{
    SPELL_GLAIDALIS_PRESERVER_LEAP   = 197709,
    SPELL_GLAIDALIS_GRIEVOUS_LEAP    = 196346,
    SPELL_GLAIDALIS_GRIEVOUS_TEAR    = 196376,
    SPELL_GLAIDALIS_RAMPAGE_FORM     = 198360,
    SPELL_GLAIDALIS_RAMPAGE_SWIPE    = 198376,
    SPELL_GLAIDALIS_RAMPAGE_CHARGE   = 198379,
    SPELL_GLAIDALIS_NIGHTFALL        = 198400
};

enum DarkheartThicketOakheartSpells
{
    SPELL_OAKHEART_SHATTERED_EARTH   = 204666,
    SPELL_OAKHEART_STRANGLING_ROOTS  = 204574,
    SPELL_OAKHEART_NIGHTMARE_BREATH  = 204667,
    SPELL_OAKHEART_CRUSHING_GRIP     = 204611,
    SPELL_OAKHEART_UPROOT            = 212786
};

enum DarkheartThicketDresaronSpells
{
    SPELL_DRESARON_BREATH_OF_CORRUPTION = 191325,
    SPELL_DRESARON_EARTHSHAKING_ROAR    = 199389,
    SPELL_DRESARON_DOWN_DRAFT           = 199345
};

enum DarkheartThicketXaviusSpells
{
    SPELL_XAVIUS_FESTERING_RIP          = 200182,
    SPELL_XAVIUS_NIGHTMARE_BOLT         = 204808,
    SPELL_XAVIUS_FEED_ON_THE_WEAK       = 200238,
    SPELL_XAVIUS_INDUCED_PARANOIA       = 200359,
    SPELL_XAVIUS_APOCALYPTIC_NIGHTMARE  = 200050
};
