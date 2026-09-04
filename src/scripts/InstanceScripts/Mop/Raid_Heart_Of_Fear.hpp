/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

// Heart of Fear - six bosses: Imperial Vizier Zor'lok, Blade Lord Ta'yak, Garalon, Wind Lord
// Mel'jarak, Amber-Shaper Un'sok, Grand Empress Shek'zeer. No prior scripting or dialogue data
// exists for this raid - all kits below are built from
// wowhead's MoP Classic data instead. This project's own gameobject reference import does
// carry a real "Doodad_Mantid_Amber_Door001" template (amber/mantid theme matches this raid;
// see below) - it has no placed spawn/position data in that same import, so the door is
// wired up in code but will only actually appear once a real spawn-placement entry places it.

enum HeartOfFearGameObjects
{
    // Opens once Imperial Vizier Zor'lok is defeated, matching the raid's linear layout.
    GO_MANTID_AMBER_DOOR = 215459
};

uint32_t const HeartOfFearEncounterCount = 6;

enum HeartOfFearData
{
    DATA_ZORLOK   = 0,
    DATA_TAYAK    = 1,
    DATA_GARALON  = 2,
    DATA_MELJARAK = 3,
    DATA_UNSOK    = 4,
    DATA_SHEKZEER = 5
};

enum HeartOfFearCreatures
{
    BOSS_ZORLOK            = 62980,
    BOSS_TAYAK             = 62543,
    BOSS_GARALON           = 62164,
    BOSS_MELJARAK          = 62397,
    BOSS_UNSOK             = 62511,
    BOSS_SHEKZEER          = 62837,

    // Trash - this raid has no placed spawn data in either reference database; these are
    // real, named creature templates found by proximity to Zor'lok's confirmed boss entry and
    // matching the raid's mantid-hive theme (one is even named directly after Shek'zeer), but
    // their actual presence in this specific raid is not spawn-confirmed.
    NPC_ZANTHIK_GENERAL    = 62966,
    NPC_SHEKZEER_SWARMBORN = 62582
};

enum ZanthikGeneralSpells
{
    SPELL_ZANTHIK_GENERAL_CLEAVE = 845,    // stand-in: reuses "Cleave" rank 1
    SPELL_ZANTHIK_GENERAL_ENRAGE = 8599    // stand-in: reuses the generic "Enrage" self-buff
};

enum ShekzeerSwarmbornSpells
{
    SPELL_SWARMBORN_SHADOW_BOLT = 32860,  // stand-in: reuses the real "Shadow Bolt" spell
    SPELL_SWARMBORN_POISON      = 3391    // stand-in: reuses the classic "Poison" spell
};

enum ZorlokSpells
{
    SPELL_ZORLOK_INHALE          = 122852,
    SPELL_ZORLOK_EXHALE          = 122761,
    SPELL_ZORLOK_ATTENUATION     = 122440,
    SPELL_ZORLOK_FORCE_AND_VERVE = 122713
};

enum TayakSpells
{
    SPELL_TAYAK_TEMPEST_SLASH   = 122842,
    SPELL_TAYAK_UNSEEN_STRIKE   = 122994,
    SPELL_TAYAK_WIND_STEP       = 123175,
    SPELL_TAYAK_STORM_UNLEASHED = 123815
};

// Garalon's real fight (four crushing legs, a "Pheromones" position-swap debuff) is a heavily
// positional encounter that isn't replicated; sourced directly from his own wowhead NPC page.
enum GaralonSpells
{
    SPELL_GARALON_FURIOUS_SWIPE = 122735,
    SPELL_GARALON_FURY          = 122754,
    SPELL_GARALON_CRUSH         = 122774,
    SPELL_GARALON_PHEROMONES    = 123092
};

// Sourced directly from Wind Lord Mel'jarak's own wowhead NPC page.
enum MeljarakSpells
{
    SPELL_MELJARAK_WHIRLING_BLADE = 121896,
    SPELL_MELJARAK_WIND_BOMB      = 131813,
    SPELL_MELJARAK_IMPALING_SPEAR = 122224,
    SPELL_MELJARAK_AMBER_PRISON   = 121885
};

// Sourced directly from Amber-Shaper Un'sok's own wowhead NPC page.
enum UnsokSpells
{
    SPELL_UNSOK_AMBER_SCALPEL   = 121994,
    SPELL_UNSOK_BURNING_AMBER   = 123020,
    SPELL_UNSOK_AMBER_EXPLOSION = 122398,
    SPELL_UNSOK_MASSIVE_STOMP   = 122408
};

// Sourced directly from Grand Empress Shek'zeer's own wowhead NPC page.
enum ShekzeerSpells
{
    SPELL_SHEKZEER_DISSONANCE_FIELD = 123184,
    SPELL_SHEKZEER_SONIC_DISCHARGE  = 123504,
    SPELL_SHEKZEER_DREAD_SCREECH    = 123735,
    SPELL_SHEKZEER_CRY_OF_TERROR    = 123792
};
