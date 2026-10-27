/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <cstdint>

uint32_t const HallsOfValorEncounterCount = 5;

enum HallsOfValorData
{
    DATA_HYMDALL          = 0, // 1805
    DATA_HYRJA            = 1, // 1806
    DATA_FENRYR           = 2, // 1807
    DATA_GOD_KING_SKOVALD = 3, // 1808
    DATA_ODYN             = 4  // 1809
};

enum HallsOfValorCreatures
{
    BOSS_HYMDALL                = 94960,

    BOSS_HYRJA                  = 95833,
    NPC_OLMYR_THE_ENLIGHTENED   = 97202,
    NPC_SOLSTEN                 = 97219,

    BOSS_FENRYR                 = 95674,
    // den form of the wolf, same fight
    BOSS_FENRYR_DEN             = 99868,
    NPC_EBONCLAW_PACKMATE       = 99922,

    BOSS_GOD_KING_SKOVALD       = 95675,
    NPC_KING_HALDOR             = 95843,
    NPC_KING_BJORN              = 97081,
    NPC_KING_RANULF             = 97083,
    NPC_KING_TOR                = 97084,

    BOSS_ODYN                   = 95676,
    NPC_STORMFORGED_OBLITERATOR = 102019
};

enum HallsOfValorGameObjects
{
    GO_HYMDALL_ENTRANCE_GATE = 245700,
    GO_HYMDALL_EXIT_GATE     = 245701,
    GO_HYRJA_GATE            = 245702,
    // The Gateway of Splendor and the bridge behind it
    GO_GATEWAY_OF_SPLENDOR   = 244664,
    GO_BRIDGE_OF_GLORY       = 246144,
    // shared by Skovald and Odyn
    GO_ODYN_HALL_GATE        = 246145
};

enum HallsOfValorHymdallSpells
{
    SPELL_HYMDALL_HORN_OF_VALOR      = 191284,
    SPELL_HYMDALL_DANCING_BLADE      = 193235,
    SPELL_HYMDALL_BLOODLETTING_SWEEP = 193092
};

enum HallsOfValorHyrjaSpells
{
    SPELL_HYRJA_ASCENDANCE          = 191475,
    SPELL_HYRJA_EXPEL_LIGHT         = 192048,
    SPELL_HYRJA_SHIELD_OF_LIGHT     = 192018,
    SPELL_HYRJA_ARCING_BOLT         = 191976,
    SPELL_HYRJA_EYE_OF_THE_STORM    = 192304,
    SPELL_HYRJA_SANCTIFY            = 192158,

    SPELL_OLMYR_SEARING_LIGHT       = 192288
};

enum HallsOfValorFenryrSpells
{
    SPELL_FENRYR_UNNERVING_HOWL     = 196543,
    SPELL_FENRYR_RAVENOUS_LEAP      = 197558,
    SPELL_FENRYR_CLAW_FRENZY        = 196512,
    SPELL_FENRYR_SCENT_OF_BLOOD     = 198800
};

enum HallsOfValorSkovaldSpells
{
    SPELL_SKOVALD_FELBLAZE_RUSH     = 193658,
    SPELL_SKOVALD_SAVAGE_BLADE      = 193668,
    SPELL_SKOVALD_RAGNAROK          = 193826,

    SPELL_KING_CALL_ANCESTOR        = 200969,
    SPELL_KING_WICKED_DAGGER        = 199674,
    SPELL_KING_UNRULY_YELL          = 199726,
    SPELL_KING_SEVER                = 199652
};

enum HallsOfValorOdynSpells
{
    SPELL_ODYN_SPEAR_OF_LIGHT       = 198072,
    SPELL_ODYN_RADIANT_TEMPEST      = 201006,
    SPELL_ODYN_SHATTER_SPEARS       = 198077,
    SPELL_ODYN_RUNIC_BRAND          = 197961,
    SPELL_ODYN_RUNIC_BRAND_PURE     = 197963,
    SPELL_ODYN_RUNIC_BRAND_RED      = 197964,
    SPELL_ODYN_RUNIC_BRAND_YELLOW   = 197965,
    SPELL_ODYN_RUNIC_BRAND_BLUE     = 197966,
    SPELL_ODYN_RUNIC_BRAND_GREEN    = 197967
};
