/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <cstdint>

uint32_t const FreeholdEncounterCount = 4;

enum FreeholdData
{
    DATA_SKYCAPN_KRAGG        = 0, // 2093
    // Captain Raoul, Captain Eudora and Captain Jolly
    DATA_COUNCIL_O_CAPTAINS   = 1, // 2094
    // Ludwig Von Tortollan, then Trothak
    DATA_RING_OF_BOOTY        = 2, // 2095
    DATA_HARLAN_SWEETE        = 3  // 2096
};

enum FreeholdCreatures
{
    BOSS_SKYCAPN_KRAGG        = 126832,
    NPC_SHARKBAIT             = 126841,

    BOSS_CAPTAIN_RAOUL        = 126847,
    BOSS_CAPTAIN_EUDORA       = 126848,
    BOSS_CAPTAIN_JOLLY        = 126845,

    BOSS_LUDWIG_VON_TORTOLLAN = 129699,
    BOSS_TROTHAK              = 126969,
    NPC_HAMMER_SHARK          = 129448,
    NPC_SAWTOOTH_SHARK        = 129359,

    BOSS_HARLAN_SWEETE        = 126983,
    NPC_IRONTIDE_GRENADIER    = 129758,
    NPC_SWIFTWIND_SABER       = 129747
};

enum KraggSpells
{
    SPELL_KRAGG_PISTOL_SHOT          = 255966,
    SPELL_KRAGG_CHARRRRRGE           = 255952,
    SPELL_KRAGG_AZERITE_POWDER_SHOT  = 256106,
    SPELL_KRAGG_REVITALIZING_BREW    = 256060,
    SPELL_SHARKBAIT_VILE_BOMBARDMENT = 256005
};

enum CouncilOCaptainsSpells
{
    SPELL_CAPTAINS_UNDER_ONE_BANNER  = 257821,

    SPELL_RAOUL_BLACKOUT_BARREL      = 258338,
    SPELL_RAOUL_BARREL_SMASH         = 256589,

    SPELL_EUDORA_POWDER_SHOT         = 256979,
    SPELL_EUDORA_GRAPESHOT           = 258352,

    SPELL_JOLLY_CUTTING_SURGE        = 267522,
    SPELL_JOLLY_WHIRLPOOL_OF_BLADES  = 267533
};

enum RingOfBootySpells
{
    SPELL_LUDWIG_SHELL_BOUNCE        = 257904,

    SPELL_TROTHAK_RIPPER_PUNCH       = 256363,
    SPELL_TROTHAK_SHARK_TORNADO      = 256405,
    SPELL_TROTHAK_SHARK_TOSS         = 256424,
    SPELL_TROTHAK_THROW_CHUM         = 272534,
    SPELL_SHARK_FLAILING_SHARK       = 256551
};

enum HarlanSweeteSpells
{
    SPELL_HARLAN_CANNON_BARRAGE      = 257305,
    SPELL_HARLAN_SWIFTWIND_SABER     = 257454,
    SPELL_HARLAN_LOADED_DICE_HANDS   = 257402,
    SPELL_HARLAN_LOADED_DICE_WAR     = 257458,
    SPELL_SABER_SWIFTWIND_SABER      = 257293,
    SPELL_GRENADIER_BOMB_MARK        = 257314,
    SPELL_GRENADIER_BLACK_POWDER     = 257315
};
