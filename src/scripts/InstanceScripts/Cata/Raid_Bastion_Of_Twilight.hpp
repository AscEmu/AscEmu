/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

// Bastion of Twilight - four normal-mode bosses (verified against wowhead); Sinestra
// (heroic-only bonus boss) is not ported.

uint32_t const BastionOfTwilightEncounterCount = 4;

enum BastionOfTwilightData
{
    DATA_HALFUS_WYRMBREAKER    = 0,
    DATA_THERALION_AND_VALIONA = 1,
    DATA_ASCENDANT_COUNCIL     = 2,
    DATA_CHOGALL               = 3
};

enum BastionOfTwilightGameObjects
{
    GO_HALFUS_EXIT             = 205223,
    GO_DRAGON_SIBLINGS_EXIT    = 205225,
    GO_ASCENDANT_COUNCIL_EXIT  = 205227,
    GO_CHOGALL_ENTRANCE        = 205228,
    GO_TWILIGHTS_HAMMER_THRONE = 205901

    // GO_HALFUS_ENTRANCE (205222), GO_DRAGON_SIBLINGS_ENTRANCE (205224) and
    // GO_ASCENDANT_COUNCIL_ENTRANCE (205226) lead into their still-unfought side rooms and
    // are open by default in the reference - not gated, so not tracked here.
};

enum BastionOfTwilightCreatures
{
    BOSS_HALFUS_WYRMBREAKER    = 44600,
    BOSS_THERALION             = 45993,
    BOSS_VALIONA               = 45992,
    BOSS_IGNACIOUS             = 43686,
    BOSS_FELUDIUS              = 43687,
    BOSS_TERRASTRA             = 43689,
    BOSS_ARION                 = 43688,
    BOSS_CHOGALL               = 43324,

    NPC_TWILIGHT_SHADOW_KNIGHT = 45261,
    NPC_TWILIGHT_CROSSFIRE     = 45264,
    NPC_TWILIGHT_SOUL_BLADE    = 45265,
    NPC_TWILIGHT_DARK_MENDER   = 45266,
    NPC_TWILIGHT_PHASE_TWISTER = 45267,
    NPC_BOUND_INFERNO          = 49817,
    NPC_BOUND_ZEPHYR           = 49821,
    NPC_BOUND_DELUGE           = 49825,
    NPC_BOUND_RUMBLER          = 49826
};

enum BastionOfTwilightTrashSpells
{
    SPELL_SHADOW_KNIGHT_DEVASTATE       = 78660,
    SPELL_SHADOW_KNIGHT_DISMANTLE       = 84832,

    SPELL_CROSSFIRE_SHOOT               = 84837,
    SPELL_CROSSFIRE_RAPID_FIRE          = 36828,
    SPELL_CROSSFIRE_WYVERN_STING        = 90488,
    SPELL_CROSSFIRE_MULTI_SHOT          = 84836,

    SPELL_SOUL_BLADE_DARK_POOL          = 84853,

    SPELL_DARK_MENDER_HEAL              = 84855,
    SPELL_DARK_MENDER_HUNGERING_SHADOWS = 84856,

    SPELL_PHASE_TWISTER_TWIST_PHASE     = 84737,

    SPELL_BOUND_INFERNO_FLAMESTRIKE     = 93362,

    SPELL_BOUND_ZEPHYR_LIGHTNING_SHOCK  = 93278,
    SPELL_BOUND_ZEPHYR_RENDING_GALE     = 93277,
    SPELL_BOUND_ZEPHYR_VAPORIZE         = 93306,

    SPELL_BOUND_DELUGE_FROST_WHIRL      = 93340,

    SPELL_BOUND_RUMBLER_ENTOMB          = 93327,
    SPELL_BOUND_RUMBLER_SHOCKWAVE       = 93325
};

enum HalfusSpells
{
    SPELL_HALFUS_FURIOUS_ROAR       = 83710,
    SPELL_HALFUS_MALEVOLENT_STRIKES = 39171,
    SPELL_HALFUS_FIREBALL_BARRAGE   = 83706,
    SPELL_HALFUS_BERSERK            = 47008
};

enum TheralionValionaSpells
{
    SPELL_VALIONA_TWILIGHT_BLAST    = 86369,
    SPELL_VALIONA_DEVOURING_FLAMES  = 86840,
    SPELL_THERALION_ENGULFING_MAGIC = 86631,
    SPELL_THERALION_BLACKOUT        = 86825
};

enum AscendantCouncilSpells
{
    SPELL_FELUDIUS_GLACIATE       = 82746,
    SPELL_FELUDIUS_HYDRO_LANCE    = 82752,

    SPELL_IGNACIOUS_FLAME_TORRENT = 82777,
    SPELL_IGNACIOUS_RISING_FLAMES = 82636,

    SPELL_TERRASTRA_QUAKE         = 83565,
    SPELL_TERRASTRA_SHATTER       = 83760,

    SPELL_ARION_THUNDERSHOCK      = 83067,
    SPELL_ARION_LIGHTNING_ROD     = 83099
};

enum ChogallSpells
{
    SPELL_CHOGALL_CORRUPTED_BLOOD            = 93104,
    SPELL_CHOGALL_FURY_OF_CHOGALL            = 82524,
    SPELL_CHOGALL_SUMMON_CORRUPTING_ADHERENT = 81628
};
