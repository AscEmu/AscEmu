/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

// Siege of Orgrimmar - fourteen bosses: Immerseus, The Fallen Protectors, Norushen, Sha of
// Pride, Galakras, Iron Juggernaut, Kor'kron Dark Shaman, General Nazgrim, Malkorok, Spoils
// of Pandaria, Thok the Bloodthirsty, Siegecrafter Blackfuse, Paragons of the Klaxxi, Garrosh
// Hellscream. No prior scripting or dialogue data exists for
// this raid - all kits below are built from wowhead's MoP Classic data instead. Several
// encounters that field multiple independent NPCs in the real fight (The Fallen Protectors,
// Kor'kron Dark Shaman, Paragons of the Klaxxi) are simplified to a single representative
// boss, since only one member's NPC entry could be confirmed for each. Rook Stonetoe (Fallen
// Protectors) and Skeer the Bloodseeker (Paragons of the Klaxxi) use entry ids 58466/63071 -
// this project's own creature data reference import (with directly-supplied real stat
// data), which was cross-checked as correct against wowhead's higher-numbered duplicates
// (71475/71152) that turned out to be inert Journal-only ids sharing the same name rather than
// the actual spawned raid boss. Spoils of Pandaria is
// a non-combat "defend the caravan from loot goblins" gimmick in the real fight rather than a
// stand-up boss fight; it's ported as a plain encounter-tracking slot with a light generic
// rotation rather than an attempt at the real minigame. This project's own gameobject
// reference import carries real, clearly-named door templates for this raid (see below) - they
// have no placed spawn/position data in that same import, so the doors are wired up in code
// but will only actually appear once real spawn-placement entries place
// them.

enum SiegeOfOrgrimmarGameObjects
{
    // Gates entry to General Nazgrim's room; opens once the Kor'kron Dark Shaman are defeated.
    GO_NAZGRIM_ENTRY_DOOR         = 223276,
    // Gates entry to Garrosh Hellscream's room; opens once the Paragons of the Klaxxi are
    // defeated.
    GO_GARROSH_ENCOUNTER_SHA_DOOR = 221442
};

uint32_t const SiegeOfOrgrimmarEncounterCount = 14;

enum SiegeOfOrgrimmarData
{
    DATA_IMMERSEUS              = 0,
    DATA_FALLEN_PROTECTORS      = 1,
    DATA_NORUSHEN               = 2,
    DATA_SHA_OF_PRIDE           = 3,
    DATA_GALAKRAS               = 4,
    DATA_IRON_JUGGERNAUT        = 5,
    DATA_KORKRON_DARK_SHAMAN    = 6,
    DATA_GENERAL_NAZGRIM        = 7,
    DATA_MALKOROK               = 8,
    DATA_SPOILS_OF_PANDARIA     = 9,
    DATA_THOK                   = 10,
    DATA_SIEGECRAFTER_BLACKFUSE = 11,
    DATA_PARAGONS_OF_THE_KLAXXI = 12,
    DATA_GARROSH_HELLSCREAM     = 13
};

enum SiegeOfOrgrimmarCreatures
{
    BOSS_IMMERSEUS              = 71543,
    BOSS_ROOK_STONETOE          = 58466,
    BOSS_NORUSHEN               = 71965,
    BOSS_SHA_OF_PRIDE           = 71734,
    BOSS_GALAKRAS               = 72249,
    BOSS_IRON_JUGGERNAUT        = 71466,
    BOSS_EARTHBREAKER_HAROMM    = 71859,
    BOSS_GENERAL_NAZGRIM        = 71515,
    BOSS_MALKOROK               = 71454,
    BOSS_SPOILS_OF_PANDARIA     = 71889,
    BOSS_THOK                   = 71529,
    BOSS_SIEGECRAFTER_BLACKFUSE = 71504,
    BOSS_SKEER_THE_BLOODSEEKER  = 63071,
    BOSS_GARROSH_HELLSCREAM     = 71865,

    // Trash - both confirmed against this instance's own real creature spawn data. Neither
    // NPC page carried ability data, so their kits are thematically-reasonable rotations built
    // from generic, verified-safe spells rather than confirmed tooltips.
    NPC_KORKRON_BLOOD_AXE       = 72728,
    NPC_KORKRON_GRUNT           = 72374
};

enum KorkronBloodAxeSpells
{
    SPELL_BLOOD_AXE_CLEAVE        = 845,    // stand-in: reuses "Cleave" rank 1
    SPELL_BLOOD_AXE_MORTAL_STRIKE = 12294  // stand-in: reuses "Mortal Strike" rank 1
};

enum KorkronGruntSpells
{
    SPELL_GRUNT_WHIRLWIND = 1680,   // stand-in: reuses "Whirlwind" rank 1
    SPELL_GRUNT_ENRAGE    = 8599   // stand-in: reuses the generic "Enrage" self-buff
};

enum ImmerseusSpells
{
    SPELL_IMMERSEUS_SHA_BOLT        = 143295,
    SPELL_IMMERSEUS_SWIRL           = 143309,
    SPELL_IMMERSEUS_CORROSIVE_BLAST = 143436,
    SPELL_IMMERSEUS_ERUPTING_SHA    = 143498
};

// Simplified to Rook Stonetoe's own kit plus one representative ability each from the other
// two Fallen Protectors (a rogue and a monk in the real fight).
enum FallenProtectorsSpells
{
    SPELL_PROTECTORS_INFERNO_STRIKE   = 143962,
    SPELL_PROTECTORS_CORRUPTION_SHOCK = 144018,
    SPELL_PROTECTORS_CORRUPTED_BREW   = 143019,
    SPELL_PROTECTORS_CORRUPTION_KICK  = 143007
};

enum NorushenSpells
{
    SPELL_NORUSHEN_CORRUPTION           = 114920,
    SPELL_NORUSHEN_UNLEASHED_ANGER      = 145214,
    SPELL_NORUSHEN_LINGERING_CORRUPTION = 144514
};

enum ShaOfPrideSpells
{
    SPELL_PRIDE_MOCKING_BLAST  = 144379,
    SPELL_PRIDE_LAST_WORD      = 144370,
    SPELL_PRIDE_WOUNDED_PRIDE  = 144358,
    SPELL_PRIDE_BURSTING_PRIDE = 144911
};

enum GalakrasSpells
{
    SPELL_GALAKRAS_THUNDER_CLAP = 6343,
    SPELL_GALAKRAS_ARCING_SMASH = 147688,
    SPELL_GALAKRAS_FLAMESTRIKE  = 146787,
    SPELL_GALAKRAS_POISON_CLOUD = 147705
};

enum IronJuggernautSpells
{
    SPELL_JUGGERNAUT_LASER_BURN     = 144459,
    SPELL_JUGGERNAUT_FLAME_VENTS    = 144464,
    SPELL_JUGGERNAUT_SHOCK_PULSE    = 144485,
    SPELL_JUGGERNAUT_MORTAR_BARRAGE = 144553
};

// Simplified to Earthbreaker Haromm's own kit plus one representative ability from his
// partner Wraith of the Elements (frost/toxic themed) in the real fight.
enum KorkronDarkShamanSpells
{
    SPELL_SHAMAN_FROSTSTORM_BOLT = 144214,
    SPELL_SHAMAN_TOXIC_STORM     = 144005,
    SPELL_SHAMAN_IRON_TOMB       = 144328,
    SPELL_SHAMAN_FOUL_GEYSER     = 143990
};

enum GeneralNazgrimSpells
{
    SPELL_NAZGRIM_SUNDERING_BLOW   = 143494,
    SPELL_NAZGRIM_BONECRACKER      = 143638,
    SPELL_NAZGRIM_HEROIC_SHOCKWAVE = 143716,
    SPELL_NAZGRIM_RAVAGER          = 143872
};

enum MalkorokSpells
{
    SPELL_MALKOROK_BREATH_OF_YSHAARJ = 142842,
    SPELL_MALKOROK_ARCING_SMASH      = 142826,
    SPELL_MALKOROK_SEISMIC_SLAM      = 142851,
    SPELL_MALKOROK_BLOOD_RAGE        = 142879
};

// Spoils of Pandaria is a non-combat "defend the caravan" minigame in the real fight; this is
// a light generic placeholder rotation, not an attempt at the real mechanic.
enum SpoilsOfPandariaSpells
{
    SPELL_SPOILS_CRUSH        = 845,    // stand-in: reuses "Cleave" rank 1
    SPELL_SPOILS_GUARD_STANCE = 8599    // stand-in: reuses the generic "Enrage" self-buff
};

enum ThokSpells
{
    SPELL_THOK_FEARSOME_ROAR     = 143426,
    SPELL_THOK_DEAFENING_SCREECH = 143343,
    SPELL_THOK_ACID_BREATH       = 143780,
    SPELL_THOK_WRECKING_BALL     = 147906
};

enum SiegecrafterBlackfuseSpells
{
    SPELL_BLACKFUSE_LAUNCH_SAWBLADE      = 143291,
    SPELL_BLACKFUSE_SERRATED_SLASH       = 143327,
    SPELL_BLACKFUSE_ELECTROSTATIC_CHARGE = 143385,
    SPELL_BLACKFUSE_DEATH_FROM_ABOVE     = 142232
};

// Simplified to Skeer the Bloodseeker's own kit representing the wider Paragons of the
// Klaxxi encounter (six independent Klaxxi paragons in the real fight).
enum ParagonsOfTheKlaxxiSpells
{
    SPELL_KLAXXI_CLAW         = 143338,
    SPELL_KLAXXI_SWIPE        = 143378,
    SPELL_KLAXXI_STING        = 143376,
    SPELL_KLAXXI_BLOODLETTING = 138693
};

enum GarroshHellscreamSpells
{
    SPELL_GARROSH_HAMSTRING        = 1715,
    SPELL_GARROSH_IRON_STAR_IMPACT = 144653,
    SPELL_GARROSH_ANNIHILATE       = 144969,
    SPELL_GARROSH_CRUSHING_FEAR    = 147324
};
