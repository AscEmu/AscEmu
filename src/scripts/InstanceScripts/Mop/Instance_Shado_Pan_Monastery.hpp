/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

// Shado-Pan Monastery - three bosses (Master Snowdrift, Sha of Violence, Gu Cloudstrike).
// No prior scripting or dialogue data exists for this
// dungeon - all three kits below are built from wowhead's MoP Classic data instead. This
// project's own gameobject reference import does carry a real "Outer Doors" template (see
// below) - it has no placed spawn/position data in that same import, so the door is wired up
// in code but will only actually appear once a real spawn-placement entry places it.

enum ShadoPanMonasteryGameObjects
{
    // Opens once Master Snowdrift is defeated, matching the dungeon's linear layout.
    GO_OUTER_DOORS = 210868
};

uint32_t const ShadoPanMonasteryEncounterCount = 3;

enum ShadoPanMonasteryData
{
    DATA_MASTER_SNOWDRIFT = 0,
    DATA_SHA_OF_VIOLENCE  = 1,
    DATA_GU_CLOUDSTRIKE   = 2
};

enum ShadoPanMonasteryCreatures
{
    BOSS_MASTER_SNOWDRIFT = 56541,
    BOSS_SHA_OF_VIOLENCE  = 56719,
    BOSS_GU_CLOUDSTRIKE   = 56747,

    // Trash - both confirmed against this instance's own real creature spawn data.
    NPC_SHADOPAN_AMBUSHER = 59752,
    NPC_SHADOPAN_DISCIPLE = 63717
};

// Sourced directly from the Ambusher's own wowhead NPC page.
enum ShadopanAmbusherSpells
{
    SPELL_AMBUSHER_FLIP_OUT   = 128248,
    SPELL_AMBUSHER_ICE_TRAP   = 135382,
    SPELL_AMBUSHER_SHADOWSTEP = 128766
};

// The Disciple's NPC page carried no ability data - a thematically-reasonable rotation built
// from generic, verified-safe spells rather than confirmed tooltips.
enum ShadopanDiscipleSpells
{
    SPELL_DISCIPLE_CLEAVE = 845,   // stand-in: reuses "Cleave" rank 1
    SPELL_DISCIPLE_ENRAGE = 8599  // stand-in: reuses the generic "Enrage" self-buff
};

enum MasterSnowdriftSpells
{
    SPELL_SNOWDRIFT_FISTS_OF_FURY  = 106853,
    SPELL_SNOWDRIFT_TORNADO_KICK   = 106434,
    SPELL_SNOWDRIFT_QUIVERING_PALM = 106422,
    SPELL_SNOWDRIFT_FLYING_KICK    = 106439
};

enum ShaOfViolenceSpells
{
    SPELL_VIOLENCE_DISORIENTING_SMASH = 106872,
    SPELL_VIOLENCE_SMOKE_BLADES       = 106827,
    SPELL_VIOLENCE_SHA_SPIKE          = 106871
};

enum GuCloudstrikeSpells
{
    SPELL_CLOUDSTRIKE_INVOKE_LIGHTNING = 106984,
    SPELL_CLOUDSTRIKE_STATIC_FIELD     = 106941,
    SPELL_CLOUDSTRIKE_LIGHTNING_BREATH = 102573
};
