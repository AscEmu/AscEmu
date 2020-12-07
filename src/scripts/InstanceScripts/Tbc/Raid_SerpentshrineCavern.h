/*
Copyright (c) 2014-2020 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

enum
{
    SP_SCALDING_WATER = 37284, // This "dot" is supposed to tick whenever you get in the water, it doesnt tick anymore after u kill lurker.

    //////////////////////////////////////////////////////////////////////////////////////////
    // Hydross the Unstable
    WATER_TOMB = 38235,
    VILE_SLUDGE = 38246,
    MARK_OF_HYDROSS1 = 38215,
    MARK_OF_HYDROSS2 = 38216,
    MARK_OF_HYDROSS3 = 38217,
    MARK_OF_HYDROSS4 = 38218,
    MARK_OF_HYDROSS5 = 38231,
    MARK_OF_HYDROSS6 = 40584,

    MARK_OF_CORRUPTION1 = 38219,
    MARK_OF_CORRUPTION2 = 38220,
    MARK_OF_CORRUPTION3 = 38221,
    MARK_OF_CORRUPTION4 = 38222,
    MARK_OF_CORRUPTION5 = 38230,
    MARK_OF_CORRUPTION6 = 40583,

    HYDROSS_ENRAGE = 13048,
    PURIFY_ELEMENTAL = 36461,

    //////////////////////////////////////////////////////////////////////////////////////////
    // The Lurker Below

    WHIRL = 37363,
    GEYSER = 37478,
    SPOUT = 37433,
    SUBMERGE = 37433, // Didn't find the spell id
    // WATER_BOLT  = 37138,

    MIND_BLAST = 37531,
    WHIRLWINDLEO = 40653,
    INSIDIOUS_WHISPER = 37676,
    CHAOS_BLAST_ANIMATION = 37674,
    CHAOS_BLAST_EFFECT = 37675,
    LEO_ENRAGE = 41447, // 13048
    LEO_BANISH = 37546,


    CATACLYSMIC_BOLT = 38441,
    BLESSING_OF_THE_TIDES = 38449,
    KARATHRESS_ENRAGE = 39249,


    //////////////////////////////////////////////////////////////////////////////////////////
    // Fathom-Guard Sharkkis (Hunter)
    MULTI_SHOT = 29576,
    LEECHING_THROW = 29436,
    THE_BEAST_WITHIN = 38373,


    //////////////////////////////////////////////////////////////////////////////////////////
    // Fathom-Guard Tidalvess (Shaman)
    SPITFIRE_TOTEM = 38236,
    POISON_CLEANSING_TOTEM = 38306,
    EARTHBIND_TOTEM = 38304,
    FROST_SHOCK = 38234,
    WINDFURY = 38229,


    //////////////////////////////////////////////////////////////////////////////////////////
    // Fathom-Guard Caribdis (Priest)
    HEAL = 33144,
    TIDAL_SURGE = 38358,
    FATHOM_GUARD_CARIBDIS_AI_SUMMON_CYCLONE = 38337,


    //TIDAL_WAVE = 37730,
    //WATERY_GRAVE = 38049,
    //EARTHQUAKE = 37764,
    //SUMMON_WATER_GLOBULE = 37854,


    VASHJ_AI_SHOOT = 37770,
    //ENTANGLE = 38316,
    //SPAWN_ELEMENTAL = 37053,
    EEMULTI_SHOT = 29576,
    //STATIC_CHARGE = 38280,
    //FORKED_LIGHTNING = 40088,
    VASHJ_SHIELD = 38112,
    POISON_SPIT = 40078,
    //TOXIC_SPORES = 38575,
    //SHOCK_BLAST = 38509,


    CA_MULTI_SHOT = 27021,


    COILFANG_GUARDIAN_AI_CLEAVE = 38260,


    COILFANG_PRIESTESS_AI_HOLY_NOVA = 38589,
    SMITE = 25364,
    SPIRIT_OF_REDEMPTION = 35618,


    UNDER_BOG_COLOSSUS_AI_ACID_GEYSER = 37959,
    //ATROPIC_BLOW = 39015,
    SPORE_QUAKE = 38976,
    TOXIC_POOL = 38718,
    UNDER_BOG_COLOSSUS_AI_FRENZY = 37605,
    RAMPANT_INFECTION = 39042,
    PARASITE = 39044,


    TW_CLEAVE = 38260,
    TW_BLOODTHIRST = 30335, // Instant
    TW_FRENZY = 37605,


    CSERP_CLEAVE = 38260,
    CSERP_REFLECTION = 36096,
    CSERP_DEVOTION = 38603,

    CSHATT_ARMOR = 38591,


    CSTRID_SCREAM = 10890,

    COILFANGFATHOM_WITCH_AI_SHADOW_BOLT = 27209,
    WHIRLWIND_KNOCKBACK = 34109,

};

enum CreatureEntry
{
    CN_LADY_VASHJ               = 21212,
    CN_ENCHANTED_ELEMENTAL      = 21958,
    CN_COILFANG_STRIDER         = 22056,
    CN_TAINTED_ELEMENTAL        = 22009,
    CN_COILFANG_ELITE           = 22055,
    CN_TOXIC_SPORE_BAT          = 22140,
    CN_SHIELD_GENERATOR_CHANNEL = 19870,
    CN_MOROGRIM_TIDEWALKER      = 21213,
    CN_TIDEWALKER_LURKER        = 21920, // Murlocks that he spawns after earthquake
    // Fathom-Lord Karathress
    CN_FATHOM_LORD_KARATHRESS   = 21214,
    CN_FATHOM_GUARD_SHARKKIS    = 21966,
    CN_FATHOM_GUARD_TIDALVESS   = 21965,
    CN_FATHOM_LURKER            = 22119,
    CN_FATHOM_SPOREBAT          = 22120,
    CN_FATHOM_GUARD_CARIBDIS    = 21964,
    CN_SEER_OLUM                = 22820,
    // Leotheras the Blind
    CN_LEOTHERAS_THE_BLIND      = 21215,
    CN_INNER_DEMON              = 21857,
    CN_GREYHEART_SPELLBINDER    = 21806,
    CN_SHADOW_OF_LEOTHERAS      = 21875,
    // The Lurker Below
    CN_THE_LURKER_BELOW         = 21217,
    CN_CN_COILFANG_AMBUSHER     = 21865,
    CN_CN_COILFANG_GUARDIAN     = 21873,
    // Hydross the Unstable
    CN_HYDROSS_THE_UNSTABLE     = 21216,
    CN_TAINTED_SPAWN_OF_HYDROSS = 22036,
    CN_PURE_SPAWN_OF_HYDROSS    = 22035,
    // Trash Mobs
    CN_COILFANG_SHATTERER       = 21301,
    CN_COILFANG_SERPENTGUARD    = 21298,
    CN_COILFANG_PRIESTESS       = 21220,
    CN_COILFANG_GUARDIAN        = 21873,
    CN_COILFANG_FATHOM_WITCH    = 21299,
    CN_COILFANG_AMBUSHER        = 21865,
    CN_TIDEWALKER_WARRIOR       = 21225,
    CN_UNDERBOG_COLOSSUS        = 21251
};

enum CreatureSpells
{
};

static Movement::Location ElementalSpawnPoints[] =
{
    { 8.3f, -835.3f, 21.9f, 5 },
    { 53.4f, -835.3f, 21.9f, 4.5f },
    { 96.0f, -861.9f, 21.8f, 4 },
    { 96.0f, -986.4f, 21.4f, 2.5f },
    { 54.4f, -1010.6f, 22.0f, 1.8f },
    { 9.8f, -1012.0f, 21.7f, 1.4f },
    { -35.0f, -987.6f, 21.5f, 0.8f },
    { -58.9f, -901.6f, 21.5f, 6.0f }
};

static Movement::Location ElementalSpawnPoints2[] =
{
    { 16.305f, -867.82f, 41.09f, 0 },
    { 43.853f, -868.338f, 41.097f, 0 },
    { 71.55f, -885.12f, 40.87f, 0 },
    { 70.96f, -962.56f, 41.09f, 0 },
    { 45.227f, -977.987f, 41.09f, 0 },
    { 17.35f, -979.27f, 41.01f, 0 },
    { -9.89f, -963.63f, 41.09f, 0 },
    { -25.37f, -910.266f, 41.09f, 0 }
};

static Movement::Location CoilfangEliteSpawnPoints[] =
{
    { 14.837f, -949.106f, 41.53f, 0 },
    { 14.857f, -897.68f, 41.536f, 0 },
    { 29.79f, -923.35f, 42.9f, 0 },
    { 44.269f, -948.832f, 41.54f, 0 }
};

static float ShieldGeneratorCoords[4][3] =
{
    { 49.256f, -902.354f, 42.9769f },
    { 9.78695f, -902.882f, 42.9f },
    { 10.4122f, -944.613f, 42.8262f },
    { 49.687f, -944.406f, 42.7324f }
};

static Movement::Location fly[] =
{
    { 29.769f, -866.190f, 43 },
    { 1.135f, -874.345f, 43 },
    { -19.719f, -894.950f, 43 },
    { -27.4222f, -923.572f, 43 },
    { -19.739f, -951.907f, 43 },
    { 1.059f, -973.314f, 43 },
    { 30.071f, -980.424f, 43 },
    { 58.665f, -973.410f, 43 },
    { 79.353f, -952.011f, 43 },
    { 87.552f, -923.175f, 43 },
    { 79.068f, -894.570f, 43 },
    { 58.503f, -873.295f, 43 }
};
