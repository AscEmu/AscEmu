/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

// Temple of the Jade Serpent - four bosses (Lorewalker Stonestep, Wise Mari, Liu Flameheart,
// Sha of Doubt). The reference only has an instance script shell for this
// dungeon (encounter tracking, no door GameObjects) - none of the four bosses have a
// reference C++ script or prior scripting/dialogue data, so all four kits below are built
// from wowhead's MoP Classic data instead. This project's own gameobject reference import
// does carry a real "Temple Door" template pair (see below) - it has no placed spawn/
// position data in that same import, so the door is wired up in code but will only actually
// appear once a real spawn-placement entry places it.

enum TempleOfTheJadeSerpentGameObjects
{
    // Two leaves of the same door (matching displayid/name) - opens once Wise Mari is
    // defeated, matching the dungeon's layout.
    GO_TEMPLE_DOOR_1 = 209971,
    GO_TEMPLE_DOOR_2 = 209972
};

uint32_t const TempleOfTheJadeSerpentEncounterCount = 4;

enum TempleOfTheJadeSerpentData
{
    DATA_WISE_MARI    = 0,
    DATA_STONESTEP    = 1,
    DATA_FLAMEHEART   = 2,
    DATA_SHA_OF_DOUBT = 3
};

enum TempleOfTheJadeSerpentCreatures
{
    BOSS_WISE_MARI    = 56448,
    BOSS_STONESTEP    = 56843,
    BOSS_FLAMEHEART   = 56732,
    BOSS_SHA_OF_DOUBT = 56439,

    // Trash - both confirmed against this instance's own real creature spawn data. Neither
    // NPC page carried ability data, so their kits are thematically-reasonable rotations built
    // from generic, verified-safe spells rather than confirmed tooltips.
    NPC_LESSER_SHA    = 58319,
    NPC_YULON_PRIEST  = 62231
};

enum LesserShaSpells
{
    SPELL_LESSER_SHA_MIND_FLAY   = 15407,  // stand-in: reuses the real "Mind Flay" spell
    SPELL_LESSER_SHA_SHADOW_BOLT = 32860   // stand-in: reuses the real "Shadow Bolt" spell
};

enum YulonPriestSpells
{
    SPELL_YULON_PRIEST_SMITE = 585,    // stand-in: reuses "Smite" rank 1
    SPELL_YULON_PRIEST_HEAL  = 2050    // stand-in: reuses "Lesser Heal" rank 1
};

enum WiseMariSpells
{
    SPELL_MARI_HYDROLANCE   = 106055,
    SPELL_MARI_CALL_WATER   = 106462,
    SPELL_MARI_BUBBLE_BURST = 106612,
    SPELL_MARI_WASH_AWAY    = 106334
};

// Stonestep alternates between a "focused" caster stance (Intensity/Sunfire Rays) and a
// defensive one (Dissipation) in the real fight; simplified to a single continuous rotation.
enum StonestepSpells
{
    SPELL_STONESTEP_AGONY           = 114571,
    SPELL_STONESTEP_SUNFIRE_RAYS    = 107223,
    SPELL_STONESTEP_HAUNTING_GAZE   = 114646,
    SPELL_STONESTEP_HELLFIRE_ARROWS = 113017
};

// Liu Flameheart channels the temple's Jade Serpent statues, copying their Serpent Strike/
// Kick/Wave into an empowered "Jade" version; only the empowered version is ported.
enum FlameheartSpells
{
    SPELL_FLAMEHEART_JADE_SERPENT_STRIKE = 106841,
    SPELL_FLAMEHEART_JADE_SERPENT_KICK   = 106864,
    SPELL_FLAMEHEART_JADE_SERPENT_WAVE   = 107053,
    SPELL_FLAMEHEART_JADE_FIRE           = 107045
};

enum ShaOfDoubtSpells
{
    SPELL_SHA_WITHER_WILL          = 106736,
    SPELL_SHA_TOUCH_OF_NOTHINGNESS = 106113,
    SPELL_SHA_GATHERING_DOUBT      = 117570
};
