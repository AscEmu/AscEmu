/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

// Hour of Twilight - three bosses (Council of Twilight, Asira Dawnslayer, Archbishop
// Benedictus). The reference implementation has no hand-scripted bosses for this instance at
// all (empty AIName/ScriptName, no scripting and no dialogue rows on any of the boss
// entries), so identities, abilities and combat texts below come from wowhead rather than a
// ported reference script - same as Beth'tilac/Shannox in Firelands and several Dragon Soul
// bosses. Benedictus' Faceless Corruptor transformation phase is dropped in favor of a single
// continuous rotation, matching this session's simplification convention for every other
// multi-phase Cata boss.

uint32_t const HourOfTwilightEncounterCount = 3;

enum HourOfTwilightData
{
    DATA_COUNCIL_OF_TWILIGHT   = 0,
    DATA_ASIRA_DAWNSLAYER      = 1,
    DATA_ARCHBISHOP_BENEDICTUS = 2
};

enum HourOfTwilightBosses
{
    BOSS_EARTHCALLER_TORUNSCAR = 54663,
    BOSS_TAWN_WINTERBLUFF      = 54664,
    BOSS_HARGOTH_DIMBLAZE      = 54665,
    BOSS_STORMCALLER_JALARA    = 54666,
    BOSS_ASIRA_DAWNSLAYER      = 54968,
    BOSS_ARCHBISHOP_BENEDICTUS = 54938
};

enum CouncilOfTwilightSpells
{
    SPELL_TORUNSCAR_EARTHEN_VORTEX = 106310,
    SPELL_WINTERBLUFF_FROST_BEACON = 106312,
    SPELL_DIMBLAZE_WILDFIRE        = 106314,
    SPELL_JALARA_STATIC_CLING      = 106316
};

enum AsiraDawnslayerSpells
{
    SPELL_ASIRA_SHADOW_BLADES = 106320,
    SPELL_ASIRA_VOID_BLAST    = 32326,
    SPELL_ASIRA_DARK_MENDING  = 106324
};

enum ArchbishopBenedictusSpells
{
    SPELL_BENEDICTUS_TWILIGHT_BARRAGE = 107352,
    SPELL_BENEDICTUS_DOMINATE_MIND    = 71282,
    SPELL_BENEDICTUS_HOLY_FIRE        = 106330
};

enum HourOfTwilightCreatures
{
    NPC_EMACIATED_MAMMOTH_BULL = 26271,
    NPC_HULKING_JORMUNGAR      = 26293,
    NPC_MAGNATAUR_PATRIARCH    = 26295,
    NPC_DRAGONBONE_CONDOR      = 26483,
    NPC_WASTES_DIGGER          = 26492,
    NPC_SNOWPLAIN_DISCIPLE     = 26705,
    NPC_SNOWPLAIN_SHAMAN       = 27279

    // NPC_SIEGE_BREAKER_STALKER (57261) carries only movement/patrol AI scripting data data in
    // the reference, no offensive ability - not scripted.
};

enum HourOfTwilightTrashSpells
{
    SPELL_MAMMOTH_BULL_TRAMPLE       = 51944,

    SPELL_JORMUNGAR_ACID_SPIT        = 50293,

    // Reference drives Magnataur Patriarch's charge/knockback choreography through
    // movement-flag AI scripting data actions we have no direct equivalent for; only this one
    // ability cast is ported.
    SPELL_MAGNATAUR_PATRIARCH_SMASH  = 38556,

    SPELL_DRAGONBONE_CONDOR_SCREECH  = 51946,

    SPELL_WASTES_DIGGER_BURROW       = 26047,
    SPELL_WASTES_DIGGER_CLAW         = 30639,

    // Reference drives Snowplain Disciple's phase choreography through movement-flag
    // AI scripting data actions we have no direct equivalent for; only the two clear ability
    // casts below are ported.
    SPELL_SNOWPLAIN_DISCIPLE_STRIKE  = 61730,
    SPELL_SNOWPLAIN_DISCIPLE_HEAL    = 52011,

    SPELL_SNOWPLAIN_SHAMAN_LIGHTNING = 39591
};
