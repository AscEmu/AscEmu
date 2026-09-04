/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

// Halls of Origination - seven bosses. Identities and mechanics verified against wowhead.
// The "Vault of Lights" optional light-beam puzzle room and its bonus chest are not ported.

uint32_t const HallsOfOriginationEncounterCount = 7;

enum HallsOfOriginationData
{
    DATA_TEMPLE_GUARDIAN_ANHUUR = 0,
    DATA_EARTHRAGER_PTAH        = 1,
    DATA_ANRAPHET               = 2,
    DATA_ISISET                 = 3,
    DATA_AMMUNAE                = 4,
    DATA_SETESH                 = 5,
    DATA_RAJH                   = 6
};

enum HallsOfOriginationCreatures
{
    BOSS_TEMPLE_GUARDIAN_ANHUUR    = 39425,
    BOSS_EARTHRAGER_PTAH           = 39428,
    BOSS_ANRAPHET                  = 39788,
    BOSS_ISISET                    = 39587,
    BOSS_AMMUNAE                   = 39731,
    BOSS_SETESH                    = 39732,
    BOSS_RAJH                      = 39378,

    NPC_SUN_TOUCHED_SERVANT        = 39366,
    NPC_SUN_TOUCHED_SPRITE         = 39369,
    NPC_SUN_TOUCHED_SPRITELING     = 39370,

    NPC_SUN_TOUCHED_SPEAKER        = 39373,
    NPC_PIT_VIPER                  = 39444,
    NPC_DUSTBONE_TORMENTOR         = 40311,
    NPC_DUSTBONE_HORROR_SIMPLE     = 40787,
    NPC_DUSTBONE_HORROR_SUBMERGING = 40808,
    NPC_TEMPLE_SWIFTSTALKER        = 48139,
    NPC_TEMPLE_RUNECASTER          = 48140,
    NPC_TEMPLE_SHADOWLANCER        = 48141,
    NPC_TEMPLE_FIRESHAPER          = 48143
};

enum HallsOfOriginationTrashSpells
{
    SPELL_SPEAKER_OUT_OF_COMBAT         = 74632,
    SPELL_SPEAKER_RANDOM                = 73861,

    SPELL_PIT_VIPER_VICTIM              = 74538,

    SPELL_TORMENTOR_CURSE_OF_EXHAUSTION = 77357,
    SPELL_TORMENTOR_SHADOW_BOLT         = 77570,

    SPELL_DUSTBONE_SMASH                = 75453,
    SPELL_DUSTBONE_SUBMERGE             = 76084,

    SPELL_SWIFTSTALKER_SHOOT            = 83877,
    SPELL_SWIFTSTALKER_CHARGED_SHOT     = 89574,

    SPELL_RUNECASTER_RUNIC_CLEAVE       = 91806,
    SPELL_RUNECASTER_CURSE              = 95181,

    SPELL_SHADOWLANCER_SHADOWLANCE      = 89555,
    SPELL_SHADOWLANCER_PACT_OF_DARKNESS = 89560,

    SPELL_FIRESHAPER_FIREBALL           = 89854,
    SPELL_FIRESHAPER_MOLTEN_BARRIER     = 89542,
    SPELL_FIRESHAPER_METEOR             = 90023
};

enum SunTouchedSpells
{
    SPELL_SUN_TOUCHED_SEARING_FLAMES = 74101
};

enum HallsOfOriginationGameObjects
{
    GO_ANHUUR_DOOR = 202306
};

enum AnhuurSpells
{
    SPELL_ANHUUR_DIVINE_RECKONING = 75592,
    SPELL_ANHUUR_BURNING_LIGHT    = 75115
};

enum PtahSpells
{
    SPELL_PTAH_RAGING_SMASH = 83650,
    SPELL_PTAH_FLAME_BOLT   = 77370
};

enum AnraphetSpells
{
    SPELL_ANRAPHET_NEMESIS_STRIKE = 75604,
    SPELL_ANRAPHET_ALPHA_BEAMS    = 76184,
    SPELL_ANRAPHET_OMEGA_STANCE   = 75622
};

enum IsisetSpells
{
    SPELL_ISISET_SUPERNOVA                  = 74136,
    SPELL_ISISET_ASTRAL_RAIN_CONTROLLER     = 74381,
    SPELL_ISISET_ASTRAL_FAMILIAR_CONTROLLER = 74383,
    SPELL_ISISET_VEIL_OF_SKY_DAMAGE         = 79370
};

enum AmmunaeSpells
{
    SPELL_AMMUNAE_WITHER              = 76043,
    SPELL_AMMUNAE_SUMMON_SEEDLING_POD = 75621,
    SPELL_AMMUNAE_CONSUME_LIFE_ENERGY = 75725
};

enum SeteshSpells
{
    SPELL_SETESH_CHAOS_BOLT     = 77069,
    SPELL_SETESH_SEED_OF_CHAOS  = 76888,
    SPELL_SETESH_REIGN_OF_CHAOS = 77023
};

enum RajhSpells
{
    SPELL_RAJH_SUN_STRIKE      = 73872,
    SPELL_RAJH_SUMMON_SUN_ORB  = 80352,
    SPELL_RAJH_BLAZING_INFERNO = 76195
};
