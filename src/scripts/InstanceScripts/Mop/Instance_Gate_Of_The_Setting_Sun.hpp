/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

// Gate of the Setting Sun - four bosses, fought in a fixed order per wowhead's dungeon guide:
// Saboteur Kip'tilak -> Striker Ga'dok -> Commander Ri'mok -> Raigonn. No prior scripting or
// dialogue data exists for this dungeon - all four
// kits below are built from wowhead's MoP Classic data instead. This project's own
// gameobject reference import does carry a real "Gate of the Setting Sun" template (the
// dungeon's own namesake door, see below) - it has no placed spawn/position data in that same
// import, so the door is wired up in code but will only actually appear once a real
// spawn-placement entry places it.

enum GateOfTheSettingSunGameObjects
{
    // Opens once Kip'tilak is defeated, matching the dungeon's linear layout.
    GO_GATE_OF_THE_SETTING_SUN = 214360
};

uint32_t const GateOfTheSettingSunEncounterCount = 4;

enum GateOfTheSettingSunData
{
    DATA_KIPTILAK = 0,
    DATA_GADOK    = 1,
    DATA_RIMOK    = 2,
    DATA_RAIGONN  = 3
};

enum GateOfTheSettingSunCreatures
{
    BOSS_KIPTILAK          = 56906,
    BOSS_GADOK             = 56589,
    BOSS_RIMOK             = 56636,
    BOSS_RAIGONN           = 56877,

    // Trash - confirmed against this instance's own real creature spawn data. Its NPC page
    // carried no ability data, so its kit is a thematically-reasonable rotation built from
    // generic, verified-safe spells rather than confirmed tooltips.
    NPC_COURTYARD_DEFENDER = 58824
};

enum CourtyardDefenderSpells
{
    SPELL_COURTYARD_DEFENDER_CLEAVE = 845,   // stand-in: reuses "Cleave" rank 1
    SPELL_COURTYARD_DEFENDER_ENRAGE = 8599  // stand-in: reuses the generic "Enrage" self-buff
};

// Real fight: Kip'tilak seeds the room with Stable Munitions that chain-explode in cardinal
// directions, and periodically wires a random player with Sabotage; at 70%/30% health he
// detonates everything at once with World in Flames. The chain-reaction/room-seeding part
// isn't replicated - only the direct-damage casts are.
enum KiptilakSpells
{
    SPELL_KIPTILAK_THROW_EXPLOSIVES          = 107130,
    SPELL_KIPTILAK_MANTID_MUNITION_EXPLOSION = 107153,
    SPELL_KIPTILAK_SABOTAGE                  = 113645
};

enum GadokSpells
{
    SPELL_GADOK_PREY_TIME       = 106934,
    SPELL_GADOK_IMPALING_STRIKE = 107047,
    SPELL_GADOK_STRAFING_RUN    = 116297,
    SPELL_GADOK_ACID_BOMB       = 115458
};

enum RimokSpells
{
    SPELL_RIMOK_VISCOUS_FLUID    = 107091,
    SPELL_RIMOK_FRENZIED_ASSAULT = 107120,
    SPELL_RIMOK_BOMBARD          = 120559
};

// Raigonn's real fight has a two-phase carapace/weak-spot mechanic with Mantid adds spawning
// throughout phase one; sourced directly from his own wowhead NPC page.
enum RaigonnSpells
{
    SPELL_RAIGONN_IMPERVIOUS_CARAPACE = 107118,
    SPELL_RAIGONN_BATTERING_HEADBUTT  = 111668,
    SPELL_RAIGONN_ENGULFING_WINDS     = 107275,
    SPELL_RAIGONN_STOMP               = 34716
};
