/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

// Temple of Kotmogu (map 998). Gameobject entries are real ("Orb of Power"/"Great
// Door"). Graveyard entries (3552/3553) independently cross-checked
// against our own WorldSafeLocs.dbc parse - exact match. Tick scaling (interval/points indexed
// 0-4 by orbs currently held) is real data, tracked here as orbs-held-per-team so the tick rate
// and points actually scale with how many orbs a team holds simultaneously.
enum KotmoguObjects
{
    KOTMOGU_DOOR_ENTRY = 213172,
    KOTMOGU_ORB_1_ENTRY = 212091,
    KOTMOGU_ORB_2_ENTRY = 212092,
    KOTMOGU_ORB_3_ENTRY = 212093,
    KOTMOGU_ORB_4_ENTRY = 212094,
    KOTMOGU_NUM_ORBS = 4
};

enum KotmoguSpells
{
    KOTMOGU_SPELL_ORB_PICKED_UP_1 = 121164,
    KOTMOGU_SPELL_ORB_PICKED_UP_2 = 121175,
    KOTMOGU_SPELL_ORB_PICKED_UP_3 = 121176,
    KOTMOGU_SPELL_ORB_PICKED_UP_4 = 121177
};

enum
{
    KOTMOGU_MAX_TEAM_SCORE = 1600,
    KOTMOGU_PLAYER_KILL_POINTS = 10,
    KOTMOGU_SOUND_ORB = 8174,

    KOTMOGU_GRAVEYARD_ALLIANCE = 3552,
    KOTMOGU_GRAVEYARD_HORDE = 3553
};

// World state ids.
enum KotmoguWorldStates
{
    WORLDSTATE_KOTMOGU_MAX_SCORE = 1780,
    WORLDSTATE_KOTMOGU_ALLIANCE_SCORE = 6303,
    WORLDSTATE_KOTMOGU_HORDE_SCORE = 6304
};

// Indexed by number of orbs a team currently holds (0-4). Real data.
static const uint32_t KotmoguTickIntervals[5] = { 15000, 12000, 9000, 6000, 3000 };
static const uint32_t KotmoguTickPoints[5] = { 0, 10, 10, 10, 10 };
