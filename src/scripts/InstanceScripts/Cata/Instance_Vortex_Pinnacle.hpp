/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

// The Vortex Pinnacle - three independent boss encounters (no doors between them), identities
// and mechanics verified against wowhead.

uint32_t const VortexPinnacleEncounterCount = 3;

enum VortexPinnacleData
{
    DATA_GRAND_VIZIER_ERTAN = 0,
    DATA_ALTAIRUS           = 1,
    DATA_ASAAD              = 2
};

enum VortexPinnacleCreatures
{
    BOSS_GRAND_VIZIER_ERTAN    = 43878,
    BOSS_ALTAIRUS              = 43873,
    BOSS_ASAAD                 = 43875,

    NPC_VP_SKYFALL_STAR        = 52019,

    NPC_WILD_VORTEX            = 45912,
    NPC_GUST_SOLDIER           = 45477,
    NPC_ARMORED_MISTRAL        = 45915,
    NPC_CLOUD_PRINCE           = 45917,
    NPC_TURBULENT_SQUALL       = 45924,
    NPC_EMPYREAN_ASSASSIN      = 45922,
    NPC_SERVANT_OF_ASAAD       = 45926,
    NPC_EXECUTOR_OF_THE_CALIPH = 45928,
    NPC_TEMPLE_ADEPT           = 45935,
    NPC_MINISTER_OF_AIR        = 45930
};

enum TrashSpells
{
    SPELL_GUST_SOLDIER_CHARGE              = 87930,
    SPELL_GUST_SOLDIER_WIND_BLAST          = 87923,
    SPELL_GUST_SOLDIER_AIR_NOVA            = 87933,

    SPELL_WILD_VORTEX_LIGHTNING_BOLT       = 88032,
    SPELL_WILD_VORTEX_WIND_SHOCK           = 88029,

    SPELL_ARMORED_MISTRAL_RISING_WINDS     = 88057,
    SPELL_ARMORED_MISTRAL_GALE_STRIKE      = 88061,
    SPELL_ARMORED_MISTRAL_STORM_SURGE      = 88055,

    SPELL_CLOUD_PRINCE_WHIPPING_WINDS      = 88081,
    SPELL_CLOUD_PRINCE_STARFALL            = 88073,
    SPELL_CLOUD_PRINCE_TYPHOON             = 88075,

    SPELL_EMPYREAN_ASSASSIN_VAPOR_FORM     = 88182,

    SPELL_TURBULENT_SQUALL_ASPHYXIATE      = 88175,
    SPELL_TURBULENT_SQUALL_CLOUDBURST      = 88170,
    SPELL_TURBULENT_SQUALL_HURRICANE       = 88171,

    SPELL_SERVANT_OF_ASAAD_CRUSADER_STRIKE = 87771,
    SPELL_SERVANT_OF_ASAAD_DIVINE_STORM    = 58127,

    SPELL_EXECUTOR_DEVASTATE               = 78660,
    SPELL_EXECUTOR_SHOCKWAVE               = 87759,

    SPELL_MINISTER_OF_AIR_LIGHTNING_LASH   = 87762,
    SPELL_MINISTER_OF_AIR_LIGHTNING_NOVA   = 87768,

    SPELL_TEMPLE_ADEPT_HOLY_SMITE          = 88959,
    SPELL_TEMPLE_ADEPT_GREATER_HEAL        = 87779
};

enum ErtanSpells
{
    SPELL_ERTAN_STORMS_EDGE_PERIODIC   = 86295,
    SPELL_ERTAN_STORMS_EDGE_VISUAL     = 86329,
    SPELL_ERTAN_STORMS_EDGE_PERIODIC_2 = 86310,
    SPELL_ERTAN_LIGHTNING_BOLT         = 86331,
    SPELL_ERTAN_SUMMON_TEMPEST         = 86340
};

// Cross-checked against wowhead's own Altairus ability list: her signature positioning
// mechanic - Upwind/Downwind of Altairus and the Twisting Winds punishment - was missing
// entirely from the rotation and is added below.
enum AltairusSpells
{
    SPELL_ALTAIRUS_CALL_THE_WIND   = 88276,
    SPELL_ALTAIRUS_CHILLING_BREATH = 88322,
    SPELL_ALTAIRUS_LIGHTNING_BLAST = 88357,
    SPELL_ALTAIRUS_UPWIND          = 88282,
    SPELL_ALTAIRUS_DOWNWIND        = 88286,
    SPELL_ALTAIRUS_TWISTING_WINDS  = 88314
};

enum AsaadSpells
{
    SPELL_ASAAD_CHAIN_LIGHTNING     = 87622,
    SPELL_ASAAD_STATIC_CLING        = 87618,
    SPELL_ASAAD_SUMMON_SKYFALL_STAR = 96260,
    SPELL_ASAAD_ARCANE_BARRAGE      = 87845
};
