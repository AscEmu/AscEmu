/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

// Stormstout Brewery - three bosses (Ook-Ook, Hoptallus, Yan-Zhu the Uncasked), ported from
// the reference implementation's scripts where
// available. Yan-Zhu has no reference script there (encounter-tracking only) - his abilities
// come from wowhead's MoP Classic data instead. The "Banana Barrel"/"Angry Bopper" trash
// gauntlets that summon Ook-Ook/Hoptallus via an alternate-power minigame and area triggers
// aren't ported; both bosses are just regular pre-spawned encounters here. The reference
// instance script has no door GameObjects, but this project's own gameobject
// reference import does carry a real "Panda Brewery Door" template (see below) - it has no
// placed spawn/position data in that same import, so the door is wired up in code but will
// only actually appear once a real spawn-placement entry places it.

enum StormstoutBreweryGameObjects
{
    // Confirmed as a real MoP-build gameobject template ("Panda Brewery Door") in this
    // project's reference import; no placed spawn/position data exists for it yet - see
    // header comment. Opens once Ook-Ook is defeated, matching the dungeon's linear layout
    // (Grain Cellar -> Brewhall/Great Wheel -> Tasting Room).
    GO_PANDA_BREWERY_DOOR = 209938
};

uint32_t const StormstoutBreweryEncounterCount = 3;

enum StormstoutBreweryData
{
    DATA_OOK_OOK   = 0,
    DATA_HOPTALLUS = 1,
    DATA_YAN_ZHU   = 2
};

enum StormstoutBreweryCreatures
{
    BOSS_OOK_OOK             = 56637,
    BOSS_HOPTALLUS           = 56717,
    BOSS_YAN_ZHU             = 59479,

    // Confirmed against this instance's own real creature spawn data (30 pre-placed instances
    // of this entry in the dungeon) - the previous NPC_HOPLING_1..5/NPC_HOPPER_1..2 entries
    // used here didn't match any real spawn in this map and have been dropped.
    NPC_GOLDEN_HOPLING       = 59824,

    // Trash - both confirmed against this instance's own real creature spawn data. Neither
    // NPC page carried ability data, so their kits are thematically-reasonable rotations built
    // from generic, verified-safe spells rather than confirmed tooltips.
    NPC_HOZEN_PARTY_ANIMAL   = 56927,
    NPC_ANCESTRAL_BREWMASTER = 59075
};

enum HozenPartyAnimalSpells
{
    SPELL_HOZEN_PARTY_ANIMAL_CLEAVE = 845,    // stand-in: reuses "Cleave" rank 1
    SPELL_HOZEN_PARTY_ANIMAL_ENRAGE = 8599    // stand-in: reuses the generic "Enrage" self-buff
};

enum AncestralBrewmasterSpells
{
    SPELL_BREWMASTER_LESSER_HEAL = 2050,   // stand-in: reuses "Lesser Heal" rank 1
    SPELL_BREWMASTER_RENEW       = 139     // stand-in: reuses "Renew" rank 1
};

enum OokOokSpells
{
    SPELL_OOKOOK_GROUND_POUND  = 106807,
    SPELL_OOKOOK_GOING_BANANAS = 106651
};

enum HoptallusSpells
{
    SPELL_HOPTALLUS_FURLWIND      = 112992,
    SPELL_HOPTALLUS_CARROT_BREATH = 112944,
    SPELL_HOPTALLUS_SCREECH       = 114367
};

// Yan-Zhu the Uncasked - no reference C++ script exists upstream, so this kit is built from
// wowhead's MoP Classic ability list instead: Bloat (grows a random player until they burst,
// knocking back everyone to either side), Blackout Brew (stacking Frost DoT that roots at 10
// stacks), Brew Bolt (ranged attack), Wall of Suds (ground hazard zone).
enum YanZhuSpells
{
    SPELL_YANZHU_BLOAT         = 106546,
    SPELL_YANZHU_BLACKOUT_BREW = 106851,
    SPELL_YANZHU_BREW_BOLT     = 114548,
    SPELL_YANZHU_WALL_OF_SUDS  = 114466
};
