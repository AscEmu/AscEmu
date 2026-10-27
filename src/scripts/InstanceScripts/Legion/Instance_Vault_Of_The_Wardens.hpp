/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <cstdint>

uint32_t const VaultOfTheWardensEncounterCount = 5;

enum VaultOfTheWardensData
{
    DATA_TIRATHON_SALTHERIL     = 0,  // 1815
    DATA_ASHGOLM                = 1,  // 1816
    DATA_GLAZER                 = 2,  // 1817
    DATA_CORDANA_FELSONG        = 3,  // 1818
    DATA_INQUISITOR_TORMENTORUM = 4   // 1850
};

enum VaultOfTheWardensCreatures
{
    BOSS_TIRATHON_SALTHERIL         = 95885,

    BOSS_ASHGOLM                    = 95886,
    NPC_EMBER                       = 99233,

    BOSS_GLAZER                     = 95887,

    BOSS_CORDANA_FELSONG            = 95888,
    NPC_AVATAR_OF_VENGEANCE         = 100351,
    // the one at Cordana's door holds it shut
    NPC_SPIRIT_OF_VENGEANCE         = 100364,

    BOSS_INQUISITOR_TORMENTORUM     = 96015,
    NPC_SHADOWMOON_WARLOCK          = 99704,
    NPC_FELGUARD_ANNIHILATOR        = 99644,
    NPC_SHADOWMOON_TECHNICIAN       = 99645,
    NPC_ENORMOUS_STONE_QUILEN       = 99675,
    NPC_MOGUSHAN_SECRET_KEEPER      = 99676,
    NPC_VOID_TOUCHED_JUGGERNAUT     = 99655,
    NPC_DERANGED_MINDFLAYER         = 99657,
    NPC_FACELESS_VOIDCASTER         = 99651,
    NPC_LINGERING_CORRUPTION        = 99678
};

enum VaultOfTheWardensGameObjects
{
    GO_TIRATHON_DOOR                = 246055,
    GO_TIRATHON_EXIT_1              = 246054,
    GO_TIRATHON_EXIT_2              = 246053,
    GO_TIRATHON_FIREWALL_1          = 247081,
    GO_TIRATHON_FIREWALL_2          = 247082,

    GO_TORMENTORUM_EXIT             = 246112,
    // prison cells in the inquisitor's hall
    GO_TORMENT_CELL_FIRST           = 246506,
    GO_TORMENT_CELL_LAST            = 246517,

    GO_ASHGOLM_VOLCANO              = 247010,

    GO_CORDANA_DOOR                 = 246049
};

enum VaultOfTheWardensTirathonSpells
{
    SPELL_TIRATHON_DARKSTRIKES          = 191941,
    SPELL_TIRATHON_SWOOP                = 191765,
    SPELL_TIRATHON_FURIOUS_BLAST        = 191823,
    SPELL_TIRATHON_FEL_MORTAR           = 202913,
    SPELL_TIRATHON_METAMORPHOSIS_HAVOC  = 192504,
    SPELL_TIRATHON_METAMORPHOSIS_VENGEANCE = 202740
};

enum VaultOfTheWardensAshgolmSpells
{
    SPELL_ASHGOLM_HEAT_UP           = 200981,
    SPELL_ASHGOLM_VOLCANO           = 192622,
    SPELL_ASHGOLM_LAVA_WREATH       = 192631,
    SPELL_ASHGOLM_FISSURE           = 192522,

    SPELL_EMBER_SMOLDERING          = 195188,
    SPELL_EMBER_SEAR                = 195332
};

enum VaultOfTheWardensGlazerSpells
{
    SPELL_GLAZER_PULSE              = 194849,
    SPELL_GLAZER_LINGERING_GAZE     = 194942,
    SPELL_GLAZER_TELEPORT           = 194149,
    SPELL_GLAZER_ENERGIZE           = 193443,
    SPELL_GLAZER_FOCUSED            = 194289
};

enum VaultOfTheWardensCordanaSpells
{
    SPELL_CORDANA_KNOCKDOWN_KICK            = 197251,
    SPELL_CORDANA_DEEPENING_SHADOWS         = 213583,
    SPELL_CORDANA_DEEPENING_SHADOWS_HEROIC  = 197551,
    SPELL_CORDANA_DETONATING_MOONGLAIVE     = 197513,
    SPELL_CORDANA_VENGEANCE                 = 205004
};

enum VaultOfTheWardensTormentorumSpells
{
    SPELL_TORMENTORUM_SAP_SOUL      = 200905,
    SPELL_TORMENTORUM_OPEN_PRISONS  = 196821,
    SPELL_TORMENTORUM_TORMENT       = 206470
};
