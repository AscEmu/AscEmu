/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

// End Time - players fight one of three "Echo" bosses (Baine, Sylvanas or Tyrande, chosen at
// the door) followed by Murozond. Only Echo of Jaina carries a hand-scripted rotation in the
// reference implementation (used here as the template for all three Echo encounters); the
// other two get encounter tracking only, matching upstream. Entry ids verified against known
// Cata data, since the reference implementation registers these scripts by name rather than a
// hardcoded id.

uint32_t const EndTimeEncounterCount = 5;

enum EndTimeData
{
    DATA_ECHO_OF_BAINE    = 0,
    DATA_ECHO_OF_JAINA    = 1,
    DATA_ECHO_OF_SYLVANAS = 2,
    DATA_ECHO_OF_TYRANDE  = 3,
    DATA_MUROZOND         = 4
};

enum EndTimeCreatures
{
    BOSS_ECHO_OF_BAINE          = 54431,
    BOSS_ECHO_OF_JAINA          = 54445,
    BOSS_ECHO_OF_SYLVANAS       = 54123,
    BOSS_ECHO_OF_TYRANDE        = 54544,
    BOSS_MUROZOND               = 54432,

    // Trash - both confirmed against this instance's own real creature spawn data. Neither
    // NPC page carried ability data, so their kits are thematically-reasonable rotations built
    // from generic, verified-safe spells rather than confirmed tooltips.
    NPC_TIME_TWISTED_NIGHTSABER = 54688,
    NPC_TIME_TWISTED_GEIST      = 54511
};

enum TimeTwistedNightsaberSpells
{
    SPELL_NIGHTSABER_CLAW   = 16793,  // stand-in: reuses a generic cat "Claw"
    SPELL_NIGHTSABER_ENRAGE = 8599    // stand-in: reuses the generic "Enrage" self-buff
};

enum TimeTwistedGeistSpells
{
    SPELL_GEIST_SHADOW_BOLT = 32860,  // stand-in: reuses the real "Shadow Bolt" spell
    SPELL_GEIST_CURSE       = 30910   // stand-in: reuses a generic curse debuff
};

enum EchoOfJainaSpells
{
    SPELL_JAINA_PYROBLAST        = 101809,
    SPELL_JAINA_FROSTBOLT_VOLLEY = 101810,
    SPELL_JAINA_FLARECORE        = 101944
};

enum MurozondSpells
{
    SPELL_MUROZOND_TEMPORAL_BLAST  = 102381,
    SPELL_MUROZOND_INFINITE_BREATH = 102569
};
