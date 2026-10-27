/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <cstdint>

uint32_t const AraKaraEncounterCount = 3;

enum AraKaraData
{
    DATA_KIKATAL_THE_HARVESTER  = 0,    // 2901
    DATA_ANUBZEKT               = 1,    // 2906
    DATA_AVANOXX                = 2     // 2926
};

enum AraKaraCreatures
{
    BOSS_KIKATAL_THE_HARVESTER      = 215407,
    // second template of Ki'katal the Harvester
    BOSS_KIKATAL_THE_HARVESTER_ALT  = 231649,
    NPC_BLOODWORKER                 = 215826,
    NPC_BLACK_BLOOD                 = 215968,

    BOSS_ANUBZEKT                   = 215405,

    BOSS_AVANOXX                    = 213179,
    NPC_STARVED_CRAWLER             = 218961
};

enum AraKaraGameObjects
{
    // spike walls on both sides of Anub'zekt's arena
    GO_ANUBZEKT_NORTH_SPIKE_WALL    = 455374,
    GO_ANUBZEKT_SOUTH_SPIKE_WALL    = 455375
};

enum KikatalSpells
{
    SPELL_KIKATAL_GRASPING_BLOOD        = 432031,
    SPELL_KIKATAL_COSMIC_SINGULARITY    = 432117,
    SPELL_KIKATAL_VENOM_VOLLEY          = 432227,
    SPELL_KIKATAL_ERUPTING_WEBS         = 432130
};

enum AnubzektSpells
{
    SPELL_ANUBZEKT_EYE_OF_THE_SWARM     = 433766,
    SPELL_ANUBZEKT_INFESTATION          = 433740,
    SPELL_ANUBZEKT_BURROW_CHARGE        = 433677,
    SPELL_ANUBZEKT_IMPALE               = 433425
};

enum AvanoxxSpells
{
    SPELL_AVANOXX_ALERTING_SHRILL       = 438476,
    SPELL_AVANOXX_GOSSAMER_ONSLAUGHT    = 438473,
    SPELL_AVANOXX_VORACIOUS_BITE        = 438471,

    SPELL_CRAWLER_HUNGER                = 438490
};
