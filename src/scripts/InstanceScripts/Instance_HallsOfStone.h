/*
Copyright (c) 2014-2018 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

enum CreatureEntry
{
    //Dark Rune Stormcaller
    CN_DR_STORMCALLER       = 27984,

    //Iron Golem Custodian
    CN_GOLEM_CUSTODIAN      = 27985,

    //Dark Rune Protector
    CN_DR_PROTECTOR         = 27983,

    //Lesser Air Elemental
    CN_LASSER_AIR_ELEMENTAL = 28384,

    //Dark Rune Worker
    CN_DR_WORKER            = 27961,

    //Dark Rune Warrior
    CN_DR_WARRIOR           = 27960,

    //Dark Rune Theurgist
    CN_DR_THEURGIST         = 27963,

    //Dark Rune Shaper
    CN_DR_SHAPER            = 27965,

    //Dark Rune Scholar
    CN_DR_SCHOLAR           = 27964,

    //Dark Rune Giant
    CN_DR_GIANT             = 27969,

    //Raging Construct
    CN_RAGING_CONSTRUCT     = 27970,

    //Lightning Construct
    CN_LIGHTNING_CONSTRUCT  = 27972,

    //Forged Iron Trogg
    CN_FI_TRAGG             = 27979,

    //Maiden of Grief
    BOSS_MAIDEN_OF_GRIEF    = 27975,

    // Krystallus
    BOSS_KRYSTALLUS         = 27977
};

enum CreatureSpells
{
    //Dark Rune Stormcaller
    STORMCALLER_LIGHTNINGBOLT   = 12167,
    STORMCALLER_SHADOWWORD      = 15654,

    //Iron Golem Custodian
    CUSTODIAN_CRUSH_ARMOR       = 33661,    //Suden armor?
    CUSTODIAN_GROUND_SMASH      = 12734,    //STUN

    //Dark Rune Protector
    PROTECTOR_CHARGE            = 22120,
    PROTECTOR_CLAVE             = 42724,

    //Lesser Air Elemental
    ELEMENTAL_LIGHTNING_BOLT    = 15801,

    //Dark Rune Worker
    WORKER_ENRAGE               = 51499,    //not really enrage :) 
    WORKER_PIERCE_ARMOR         = 46202,

    //Dark Rune Warrior
    WARRIOR_CLAVE               = 42724,
    WARRIOR_HEROIC_STRIKE       = 53395,

    //Dark Rune Theurgist
    THEURGIST_BLAST_WAVE        = 22424,    //Cast on self 12466
    THEURGIST_FIREBOLT          = 12466,    //Random target?
    THEURGIST_IRON_MIGHT        = 51484,    //Cast on self, some kind of enrage.

    //Dark Rune Shaper
    SHAPER_RAY                  = 51496,    //Debuff

    //Dark Rune Scholar
    SCHOLAR_SILANCE             = 51612,    //not rly silance but something like it :)

    //Dark Rune Giant
    GIANT_FIST                  = 51494,    //also some kind of enrage
    GIANT_STOMP                 = 51493,    //Knockback

    //Raging Construct
    RAGING_CLAVE                = 28168,
    RAGING_POTENT_JOLT          = 51819,    // he should stack this in about every 6 seconds or something

    //Lightning Construct
    LIGHTN_CHAIN_LIGHTNING      = 52383,
    LIGHTN_ELECTRICAL_OVERLOAD  = 52341,    //explode?

    //Forged Iron Trogg
    TRAGG_SHOCK                 = 50900,

    //Maiden of Grief
    MAIDEN_PILLAR_OF_WOE        = 50761,    //apply at long/min range (all in that void zone should get it)
    MAIDEN_SHOCK_OF_SORROW      = 50760,
    MAIDEN_STORM_OF_GRIEF       = 50752,

    // Krystallus
    KRYSTALLUS_BOULDER_TOSS     = 50843,
    KRYSTALLUS_SHATTER          = 50810,
    KRYSTALLUS_STOMP            = 50868
};

enum CreatureSay
{

};
