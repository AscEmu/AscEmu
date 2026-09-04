/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

// Scarlet Halls - three bosses, fought in order per wowhead's dungeon guide: Houndmaster
// Braun -> Armsmaster Harlan -> Flameweaver Koegler. No prior scripting or dialogue data
// exists for this dungeon - all three kits below are built
// from wowhead's MoP Classic data instead.

uint32_t const ScarletHallsEncounterCount = 3;

enum ScarletHallsData
{
    DATA_HOUNDMASTER_BRAUN   = 0,
    DATA_ARMSMASTER_HARLAN   = 1,
    DATA_FLAMEWEAVER_KOEGLER = 2
};

enum ScarletHallsCreatures
{
    BOSS_HOUNDMASTER_BRAUN   = 59303,
    BOSS_ARMSMASTER_HARLAN   = 58632,
    BOSS_FLAMEWEAVER_KOEGLER = 59150,

    // Trash - all three confirmed against an expanded reference database's own
    // placed spawns on this instance's map. None of their NPC pages carried ability data, so
    // their kits are thematically-reasonable rotations built from generic, verified-safe
    // spells rather than confirmed tooltips.
    NPC_MASTER_ARCHER        = 59175,
    NPC_SCARLET_EVANGELIST   = 58685,
    NPC_STARVING_HOUND       = 58876
};

enum MasterArcherSpells
{
    SPELL_MASTER_ARCHER_MULTI_SHOT = 2643,  // stand-in: reuses the real "Multi-Shot" spell
    SPELL_MASTER_ARCHER_CLEAVE     = 845    // stand-in: reuses "Cleave" rank 1
};

enum ScarletEvangelistSpells
{
    SPELL_EVANGELIST_SMITE = 585,    // stand-in: reuses "Smite" rank 1
    SPELL_EVANGELIST_HEAL  = 2050   // stand-in: reuses "Lesser Heal" rank 1
};

enum StarvingHoundSpells
{
    SPELL_STARVING_HOUND_CLEAVE = 845,   // stand-in: reuses "Cleave" rank 1
    SPELL_STARVING_HOUND_ENRAGE = 8599   // stand-in: reuses the generic "Enrage" self-buff
};

// Houndmaster Braun summons an Obedient Hound add at 90/80/70/60% health and enrages
// (Bloody Rage) at 50%; the add summons are approximated as periodic casts rather than real
// health-percent triggers to keep this in line with the rest of the dungeon's timer-based
// rotations, except Bloody Rage which is a genuine one-shot health trigger.
enum HoundmasterBraunSpells
{
    SPELL_BRAUN_PIERCING_THROW = 114004,
    SPELL_BRAUN_DEATH_BLOSSOM  = 114242,
    SPELL_BRAUN_BLOODY_MESS    = 114056,
    SPELL_BRAUN_CALL_DOG       = 114259,
    SPELL_BRAUN_BLOODY_RAGE    = 116140
};

enum ArmsmasterHarlanSpells
{
    SPELL_HARLAN_HEROIC_LEAP     = 111218,
    SPELL_HARLAN_BLADES_OF_LIGHT = 111216,
    SPELL_HARLAN_DRAGONS_REACH   = 111217
};

enum FlameweaverKoeglerSpells
{
    SPELL_KOEGLER_BOOK_BURNER            = 113364,
    SPELL_KOEGLER_GREATER_DRAGONS_BREATH = 113653,
    SPELL_KOEGLER_FIREBALL_VOLLEY        = 113691,
    SPELL_KOEGLER_PYROBLAST              = 11366
};
