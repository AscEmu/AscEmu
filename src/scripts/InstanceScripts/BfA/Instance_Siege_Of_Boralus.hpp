/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <cstdint>

uint32_t const SiegeOfBoralusEncounterCount = 5;

enum SiegeOfBoralusData
{
    // first boss of a Horde run
    DATA_SERGEANT_BAINBRIDGE    = 0, // 2097
    // first boss of an Alliance run
    DATA_CHOPPER_REDHOOK        = 1, // 2098
    DATA_HADAL_DARKFATHOM       = 2, // 2099
    DATA_VIQGOTH                = 3, // 2100
    DATA_DREAD_CAPTAIN_LOCKWOOD = 4  // 2109
};

enum SiegeOfBoralusCreatures
{
    BOSS_SERGEANT_BAINBRIDGE    = 128649,
    BOSS_CHOPPER_REDHOOK        = 128650,

    BOSS_HADAL_DARKFATHOM       = 128651,

    BOSS_VIQGOTH                = 128652,
    NPC_DEMOLISHING_TERROR      = 137614,
    NPC_GRIPPING_TERROR         = 137405,

    BOSS_DREAD_CAPTAIN_LOCKWOOD = 129208,
    NPC_ASHVANE_DECKHAND        = 136483,
    NPC_ASHVANE_CANNONEER       = 136549
};

enum SiegeOfBoralusActions
{
    ACTION_VIQGOTH_RISE = 1,
    ACTION_VIQGOTH_CANNON_SHOT
};

enum DockBossSpells
{
    SPELL_BAINBRIDGE_STEEL_TEMPEST = 260924,
    SPELL_BAINBRIDGE_IRON_GAZE     = 260954,

    SPELL_REDHOOK_GORE_CRASH       = 257326,
    SPELL_REDHOOK_ON_THE_HOOK      = 257459,
    SPELL_REDHOOK_HEAVY_HITTER     = 256867,
    SPELL_REDHOOK_BOILING_RAGE     = 257650,

    // cannon fire both first bosses call in
    SPELL_DOCK_SIGHTED_ARTILLERY   = 272425
};

enum HadalDarkfathomSpells
{
    SPELL_HADAL_BREAK_WATER           = 257882,
    SPELL_HADAL_BREAK_WATER_MISSILE   = 257884,
    SPELL_HADAL_CRASHING_TIDE_MISSILE = 261564
};

enum ViqGothSpells
{
    SPELL_VIQGOTH_PUTRID_WATERS           = 275014,
    SPELL_DEMOLISHING_TERROR_SLAM         = 269266,
    SPELL_DEMOLISHING_TERROR_HULL_CRACKER = 270590
};

enum DreadCaptainLockwoodSpells
{
    SPELL_LOCKWOOD_SHOOT          = 280389,
    SPELL_LOCKWOOD_GUT_SHOT       = 273470,
    SPELL_LOCKWOOD_CLEAR_THE_DECK = 269029,
    SPELL_LOCKWOOD_WITHDRAW       = 268752,
    SPELL_LOCKWOOD_DREAD_VOLLEY   = 268944
};
