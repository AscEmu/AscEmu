/*
Copyright (c) 2014-2020 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

enum
{
    CN_VOID_REAVER = 19516,

    VOID_REAVER_POUNDING = 34164,
    //VOID_REAVER_ARCANE_ORB = 34190;
    VOID_REAVER_ARCANE_ORB_TRIGGER = 34172,
    VOID_REAVER_KNOCK_AWAY = 25778,
    VOID_REAVER_ENRAGE = 27680, // Needs checking (as it can be wrong [or maybe IS wrong])
    CN_SOLARIAN = 18805,
    CN_SOLARIUMAGENT = 18925,
    CN_SOLARIUMPRIEST = 18806,
    CN_SOLARIUM_SPOT_LIGHT = 15631,
    SOLARIAN_WRATH_OF_THE_ASTROMANCER = 42783, // Infuses an enemy with Arcane power, causing them to harm nearby allies for 5400 to 6600. Arcane damage after 6 sec.
    SOLARIAN_WRATH_OF_THE_ASTROMANCER_BOMB = 42787, // The actual spell that triggers the explosion with arcane damage and slow fall
    SOLARIAN_ARCANE_MISSILES = 33031, // Launches magical missiles at an enemy, inflicting Arcane damage each second for 3 sec. Trigger spell (3000 arcane damage)
    SOLARIAN_BLINDING_LIGHT = 33009, // Hits everyone in the raid for 2280 to 2520 arcane damage. 20sec cooldown.
    SOLARIAN_SOLARIANS_TRANSFORM = 39117, // Transforms into void walker.
    SOLARIAN_VOID_BOLT = 39329, // The Void Walker casts this every 10 seconds. It deals 4394 to 5106 shadow damage to the target with the highest aggro.
    SOLARIAN_PSYCHIC_SCREAM = 34322, // Fears up to 5 targets in melee range.
    SOLARIUMPRIEST_GREATER_HEAL = 38580, // Heals 23125 to 26875 any friendly target
    SOLARIUMPRIEST_HOLY_SMITE = 31740, // Deals 553 to 747 holy damage

    //////////////////////////////////////////////////////////////////////////////////////////
    // Thaladred the Darkener AI(1st advisor)
    CN_DARKENER = 20064,
    DARKENER_PSYCHIC_BLOW = 36966,
    DARKENER_SILENCE = 29943,

    CN_SANGUINAR = 20060,
    SANGUINAR_BELLOWING = 36922,

    //////////////////////////////////////////////////////////////////////////////////////////
    // Grand Astromancer Capernian AI (3rd advisor)
    CN_CAPERNIAN = 20062,
    CAPERNIAN_CONFLAGRATION = 37018,
    CAPERNIAN_FIREBALL = 36971,
    CAPERNIAN_ARCANE_BURST = 36970,

    CN_TELONICUS = 20063,
    TELONICUS_BOMB = 37036,
    TELONICUS_REMOTE_TOY = 37027, // doesn't seems to work like it should

    //////////////////////////////////////////////////////////////////////////////////////////
    // Flame Strike AI
    CN_FLAME_STRIKE_TRIGGER = 21369,
    FLAME_STRIKE_TRIGGER_FLAME_STRIKE = 36731,
    FLAME_STRIKE_TRIGGER_FLAME_STRIKE_EFFECT = 36730,

    //////////////////////////////////////////////////////////////////////////////////////////
    // Phoenix AI
    CN_PHOENIX = 21362,
    PHOENIX_BURN = 36721,
    PHOENIX_REBIRTH = 35369, // used as instant cast - but it does not show animation now (maybe it would be good to move it to trigger?)

    //////////////////////////////////////////////////////////////////////////////////////////
    // Phoenix Egg AI
    CN_PHOENIX_EGG = 21364,

    NETHERSTRAND_LONGBOW = 21268,
    DEVASTATION = 21269,
    COSMIC_INFUSER = 21270,
    INFINITY_BLADE = 21271,
    WARP_SLICER = 21272,
    PHASESHIFT_BULWARK = 21273,
    STAFF_OF_DISINTEGRATION = 21274,

    // Prince Kael'Thas
    CN_KAELTHAS = 19622,

    // Common spells
    KAELTHAS_FIREBALL = 36805, // prolly wrong id
    KAELTHAS_ARCANE_DISRUPTION = 36834,
    KAELTHAS_SHOCK_BARRIER = 36815, // timed

    //////////////////////////////////////////////////////////////////////////////////////////
    // Phase 4 spells
    KAELTHAS_FLAME_STRIKE_SUMMON = 36735,
    KAELTHAS_PHOENIX = 36723,
    KAELTHAS_PYROBLAST = 36819, // timed
    KAELTHAS_MIND_CONTROL = 36797, // timed

    //////////////////////////////////////////////////////////////////////////////////////////
    // Phase 5 spells
    //KAELTHAS_GRAVITY_LAPSE = 35966, // timed
    //KAELTHAS_NETHER_VAPOR = 35859,
    KAELTHAS_NETHER_BEAM = 35873, // timed along with lapse

    //KAELTHAS_GRAVITY1 = 34480, // knockback + aura
    //KAELTHAS_GRAVITY2 = 35941, // explosion effect

    KAELTHAS_SUMMON_WEAPONS = 36976, // casting effect

    //REMOVE_INFUSER = 39498,
    //REMOVE_DEVASTATION = 39499,
    //REMOVE_INFINITY = 39500,
    //REMOVE_LONGBOW = 39501,
    //REMOVE_BULWARK = 39502,
    //REMOVE_STAFF = 39503,
    //REMOVE_SLICER = 39504,
};

enum CreatureEntry
{
};

enum CreatureSpells
{
};
