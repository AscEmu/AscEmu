/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <cstdint>

uint32_t const TheBlindingValeEncounterCount = 4;

enum TheBlindingValeData
{
    // Lekshi, Kezkitt and Meittik
    DATA_LIGHTBLOSSOM_TRIO   = 0,    // 3199
    DATA_IKUZZ                  = 1,    // 3200
    DATA_LIGHTWARDEN_RUIA       = 2,    // 3201
    DATA_ZIEKKET                = 3     // 3202
};

enum TheBlindingValeCreatures
{
    BOSS_TRIO_MEITTIK            = 243028,
    BOSS_TRIO_KEZKITT            = 243029,
    BOSS_TRIO_LEKSHI             = 243030,
    NPC_TRIO_LIGHTBLOSSOM        = 244528,

    BOSS_IKUZZ                      = 244887,
    NPC_IKUZZ_BLOODTHORN_ROOTS      = 244930,
    NPC_IKUZZ_BLOODTHORN_ROOTS_ALT  = 253571,

    // four templates share the name
    BOSS_LIGHTWARDEN_RUIA           = 245912,
    BOSS_LIGHTWARDEN_RUIA_ALT1      = 237836,
    BOSS_LIGHTWARDEN_RUIA_ALT2      = 241160,
    BOSS_LIGHTWARDEN_RUIA_ALT3      = 241386,

    BOSS_ZIEKKET                    = 247676,
    NPC_ZIEKKET_LIGHTSPAWN_LASHER   = 247755,
    NPC_ZIEKKET_LIGHTSPAWN_LASHER_ALT = 254697
};

enum TheBlindingValeActions
{
    ACTION_BLOSSOM_FED = 1,
    ACTION_LASHER_AWAKEN
};

enum TheBlindingValeCreatureData
{
    CREATURE_DATA_LASHER_DORMANT = 1
};

enum LightblossomTrioSpells
{
    SPELL_TRIO_SHARED_THICKET          = 1253028,

    SPELL_MEITTIK_BEDROCK_SLAM              = 1234753,
    SPELL_MEITTIK_FERTILE_LOAM              = 1234782,

    SPELL_LEKSHI_LIGHTSOWER_DASH            = 1234850,
    SPELL_LEKSHI_THORNBLADE                 = 1235640,

    SPELL_KEZKITT_LIGHTBLOSSOM_BEAM         = 1235564,
    SPELL_KEZKITT_LIGHT_BOLT                = 1235616,

    SPELL_BLOSSOM_LIGHT_GORGED              = 1235729,
    SPELL_BLOSSOM_LIGHTBLOOM_OVERGROWTH     = 1235751,
    SPELL_BLOSSOM_LIGHT_SCORCHED_EARTH      = 1235814
};

enum IkuzzSpells
{
    SPELL_IKUZZ_BLOODTHORN_ROOTS            = 1236658,
    SPELL_IKUZZ_BLOODTHIRSTY_GAZE           = 1237090,
    SPELL_IKUZZ_CRUSHING_FOOTFALLS          = 1237093,
    SPELL_IKUZZ_INCISE                      = 1237166,
    SPELL_IKUZZ_VERDANT_STOMP               = 1236746,
    SPELL_IKUZZ_THORNCALLER_ROAR            = 1236709,
    SPELL_IKUZZ_LIGHTCRAZED_FRENZY          = 1237073
};

enum LightwardenRuiaSpells
{
    // Moonkin form
    SPELL_RUIA_LIGHTFIRE                    = 1239824,
    SPELL_RUIA_LIGHTFALL                    = 1240098,
    SPELL_RUIA_WARDENS_WRATH                = 1239821,

    // Bear form
    SPELL_RUIA_PULVERIZING_STRIKES          = 1240210,
    SPELL_RUIA_GRIEVOUS_THRASH              = 1241058,
    SPELL_RUIA_MANGLING_CLAWS               = 1272265
};

enum ZiekketSpells
{
    SPELL_ZIEKKET_AWAKEN_THE_LIGHTBLOOM     = 1246372,
    SPELL_ZIEKKET_CONCENTRATED_LIGHTBEAM    = 1246607,
    SPELL_ZIEKKET_LIGHTSAP                  = 1246660,
    SPELL_ZIEKKET_THORNSPIKE                = 1247685,
    SPELL_ZIEKKET_OOZING_XYLEM              = 1247377,

    SPELL_LASHER_LIGHTSPORE_SHOT            = 1247669,
    SPELL_LASHER_DORMANT                    = 1246379,
    SPELL_LASHER_VICIOUS_REGROWTH           = 1253320
};
