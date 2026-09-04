/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

// Dragon Soul - eight encounters. Only Madness of Deathwing has a hand-scripted reference
// implementation in Cata; Morchok, Warlord Zon'ozz, Yor'sahj, Hagara, Ultraxion and Warmaster
// Blackhorn have no AIName/ScriptName and no AI scripting data rows there at all, so their
// entry ids and abilities below come from the Cata reference database and wowhead directly.
// Spine of Deathwing has no fightable "boss" creature in the data (it's an add/tendon-based
// platform-assault phase) and is not scripted here - only its encounter slot exists.

uint32_t const DragonSoulEncounterCount = 8;

enum DragonSoulData
{
    DATA_MORCHOK                = 0,
    DATA_WARLORD_ZONOZZ         = 1,
    DATA_YORSAHJ_THE_UNSLEEPING = 2,
    DATA_HAGARA_THE_STORMBINDER = 3,
    DATA_ULTRAXION              = 4,
    DATA_WARMASTER_BLACKHORN    = 5,
    DATA_SPINE_OF_DEATHWING     = 6,
    DATA_MADNESS_OF_DEATHWING   = 7
};

enum DragonSoulCreatures
{
    BOSS_MORCHOK                = 55265,
    BOSS_WARLORD_ZONOZZ         = 55308,
    BOSS_YORSAHJ_THE_UNSLEEPING = 55312,
    BOSS_HAGARA_THE_STORMBINDER = 55689,
    BOSS_ULTRAXION              = 55293,
    BOSS_WARMASTER_BLACKHORN    = 56427,
    BOSS_MADNESS_OF_DEATHWING   = 56173
};

enum DragonSoulTrashCreatures
{
    NPC_WYRMREST_PROTECTOR     = 27953,
    NPC_EARTHEN_DESTROYER      = 57158,
    NPC_EARTHEN_SOLDIER        = 57159,
    NPC_ANCIENT_WATER_LORD     = 57160,
    NPC_TWILIGHT_SIEGE_CAPTAIN = 57280

    // NPC_TWILIGHT_PORTAL (57231) is a non-combat scenery prop that only triggers a say
    // event, and NPC_SIEGE_BREAKER_STALKER (57261) carries only movement/patrol data in the
    // reference - neither has offensive AI scripting data data, so neither is scripted.
};

enum DragonSoulTrashSpells
{
    SPELL_WYRMREST_PROTECTOR_TAIL_SWEEP = 6533,
    SPELL_WYRMREST_PROTECTOR_CLEAVE     = 16145,
    SPELL_WYRMREST_PROTECTOR_GORE       = 15496,
    SPELL_WYRMREST_PROTECTOR_REND       = 17547,

    SPELL_EARTHEN_DESTROYER_ROCK_BLAST  = 107597,
    SPELL_EARTHEN_DESTROYER_QUAKE       = 107675,

    // Also used by Earthen Soldier below - the reference casts the same spell id on both.
    SPELL_EARTHEN_STONE_SPIKES          = 95440,

    SPELL_EARTHEN_SOLDIER_SHIELD_SLAM   = 107852,
    SPELL_EARTHEN_SOLDIER_ENRAGE        = 107872,

    SPELL_WATER_LORD_FROST_BOLT         = 107791,
    SPELL_WATER_LORD_TIDAL_WAVE         = 107801,

    SPELL_SIEGE_CAPTAIN_CLEAVE          = 108096
};

enum MorchokSpells
{
    SPELL_MORCHOK_STOMP                    = 103414,
    SPELL_MORCHOK_CRUSH_ARMOR              = 103687,
    SPELL_MORCHOK_BLACK_BLOOD_OF_THE_EARTH = 103785
};

enum ZonozzSpells
{
    SPELL_ZONOZZ_PSYCHIC_DRAIN      = 104322,
    SPELL_ZONOZZ_DISRUPTING_SHADOWS = 103434,
    SPELL_ZONOZZ_FOCUSED_ANGER      = 104543
};

enum YorsahjSpells
{
    SPELL_YORSAHJ_VOID_BOLT     = 104849,
    SPELL_YORSAHJ_SEARING_BLOOD = 105033
};

enum HagaraSpells
{
    SPELL_HAGARA_ICE_LANCE       = 105313,
    SPELL_HAGARA_FROZEN_TEMPEST  = 105256,
    SPELL_HAGARA_LIGHTNING_STORM = 77918
};

enum UltraxionSpells
{
    SPELL_ULTRAXION_FADING_LIGHT     = 105925,
    SPELL_ULTRAXION_HOUR_OF_TWILIGHT = 106174
};

enum BlackhornSpells
{
    SPELL_BLACKHORN_DEVASTATE       = 20243,
    SPELL_BLACKHORN_DISRUPTING_ROAR = 108044,
    SPELL_BLACKHORN_SHOCKWAVE       = 46968
};

enum MadnessOfDeathwingSpells
{
    SPELL_DEATHWING_CATACLYSM       = 106523,
    SPELL_DEATHWING_ELEMENTIUM_BOLT = 105651
};
