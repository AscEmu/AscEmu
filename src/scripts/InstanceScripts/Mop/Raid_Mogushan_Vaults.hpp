/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

// Mogu'shan Vaults - seven encounter slots: The Stone Guard, Feng the Accursed, Gara'jal the
// Spiritbinder, The Spirit Kings, Elegon, Will of the Emperor. No prior scripting or dialogue
// data exists for this raid; all kits below are built
// from wowhead's MoP Classic data. Will of the Emperor is fought as a pair of animated mogu
// constructs (Qin-xi/Jan-xi, found via the encounter's own wowhead strategy guide rather than
// a direct name search) and is simplified to a single representative boss sharing the
// DATA_WILL_OF_THE_EMPEROR slot. Gara'jal, the four Spirit Kings and Elegon's kits are sourced
// from their wowhead strategy-guide pages (their individual NPC pages carry no ability data).
// This project's own gameobject reference import does carry a real "Golden Doors" template
// (fits this raid's treasure-vault theme; see below) - it has no placed spawn/position data
// in that same import, so the door is wired up in code but will only actually appear once a
// real spawn-placement entry places it.

enum MogushanVaultsGameObjects
{
    // Opens once The Stone Guard are defeated, matching the raid's linear layout.
    GO_GOLDEN_DOORS = 211703
};

uint32_t const MogushanVaultsEncounterCount = 6;

enum MogushanVaultsData
{
    DATA_STONE_GUARD         = 0,
    DATA_FENG_THE_ACCURSED   = 1,
    DATA_GARAJAL             = 2,
    DATA_SPIRIT_KINGS        = 3,
    DATA_ELEGON              = 4,
    DATA_WILL_OF_THE_EMPEROR = 5
};

enum MogushanVaultsCreatures
{
    BOSS_JASPER_GUARDIAN   = 59915,
    BOSS_AMETHYST_GUARDIAN = 60047,
    BOSS_JADE_GUARDIAN     = 60043,
    BOSS_COBALT_GUARDIAN   = 60051,

    BOSS_FENG_THE_ACCURSED = 60016,

    BOSS_GARAJAL           = 60257,

    BOSS_ZIAN              = 60701,
    BOSS_MENG              = 60708,
    BOSS_QIANG             = 60709,
    BOSS_SUBETAI           = 60710,

    BOSS_ELEGON            = 60410,

    // Will of the Emperor is fought as a pair of mogu constructs; Qin-xi is used as the
    // registered representative, running abilities from both constructs' shared kit.
    BOSS_QIN_XI            = 60399,
    BOSS_JAN_XI            = 60400,

    // Trash - this raid has no placed spawn data in either reference database; this is a
    // real, named creature template found by proximity to Elegon's confirmed boss entry, but
    // its actual presence in this specific raid (rather than the surrounding Vale of Eternal
    // Blossoms open world, which shares this id range) is not spawn-confirmed.
    NPC_MOGU_AMBUSHER      = 60057
};

enum MoguAmbusherSpells
{
    SPELL_MOGU_AMBUSHER_CLEAVE = 845,    // stand-in: reuses "Cleave" rank 1
    SPELL_MOGU_AMBUSHER_ENRAGE = 8599    // stand-in: reuses the generic "Enrage" self-buff
};

// The four Stone Guard statues share a "power link" mechanic (damaging one drains a linked
// partner's shield) that isn't replicated; each instead runs its own primary attack plus a
// petrification-style debuff.
enum StoneGuardSpells
{
    SPELL_JASPER_CHAINS          = 130395,
    SPELL_JASPER_PETRIFICATION   = 116036,

    SPELL_AMETHYST_POOL          = 116235,
    SPELL_AMETHYST_PETRIFICATION = 116057,

    SPELL_JADE_SHARDS            = 116223,
    SPELL_JADE_PETRIFICATION     = 116006,

    SPELL_COBALT_MINE            = 129424,
    SPELL_COBALT_PETRIFICATION   = 115852
};

// Feng cycles through the elemental themes of past champion spirits (storm, fire, arcane,
// shadow) in the real fight, each requiring different positioning; simplified to one
// representative ability per theme run continuously.
enum FengSpells
{
    SPELL_FENG_SPIRIT_BOLT    = 118530,
    SPELL_FENG_LIGHTNING_LASH = 131788,
    SPELL_FENG_FLAMING_SPEAR  = 116942,
    SPELL_FENG_ARCANE_SHOCK   = 131790,
    SPELL_FENG_SHADOWBURN     = 17877
};

// Sourced from Gara'jal's wowhead strategy-guide page (his own NPC page carries no ability
// data).
enum GarajalSpells
{
    SPELL_GARAJAL_SHADOW_BOLT     = 32860,
    SPELL_GARAJAL_VOODOO_DOLLS    = 116000,
    SPELL_GARAJAL_SPIRITUAL_GRASP = 118421,
    SPELL_GARAJAL_SOUL_SEVER      = 116278
};

// Sourced from The Spirit Kings' wowhead strategy-guide page. Simplified to one representative
// ability per king (Qiang the warrior, Subetai the archer, Zian the shadow-rogue, Meng the
// demented caster) rather than the real fight's four-body simultaneous encounter.
enum SpiritKingsSpells
{
    SPELL_QIANG_MASSIVE_ATTACKS  = 117920,
    SPELL_SUBETAI_RAIN_OF_ARROWS = 118122,
    SPELL_ZIAN_ROBBED_BLIND      = 118163,
    SPELL_MENG_SHADOW_BLAST      = 117628
};

// Sourced from Elegon's wowhead strategy-guide page (his own NPC page carries no ability
// data).
enum ElegonSpells
{
    SPELL_ELEGON_CELESTIAL_BREATH = 117960,
    SPELL_ELEGON_ARCING_ENERGY    = 117945,
    SPELL_ELEGON_CLOSED_CIRCUIT   = 117949,
    SPELL_ELEGON_STABILITY_FLUX   = 117911
};

// Will of the Emperor - fought as a pair of animated mogu constructs (Qin-xi/Jan-xi), found
// via the encounter's own wowhead strategy guide. Simplified to a single representative boss.
enum WillOfTheEmperorSpells
{
    SPELL_EMPEROR_FOCUSED_ASSAULT  = 116525,
    SPELL_EMPEROR_ENERGIZING_SMASH = 116550,
    SPELL_EMPEROR_DEVASTATING_ARC  = 116835,
    SPELL_EMPEROR_STOMP            = 34716
};
