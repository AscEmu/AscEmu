/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <vector>

/// All criteria must be completed for the achievement to be complete.
#define ACHIEVEMENT_CRITERIA_COMPLETE_FLAG_ALL 2

/// Some of the criteria must be completed for the achievement to be complete.
#define ACHIEVEMENT_CRITERIA_COMPLETE_FLAG_SOME 4

/// You must not be in a group to complete the achievement.
#define ACHIEVEMENT_CRITERIA_GROUP_NOT_IN_GROUP 2

/// Alliance-only achievement
#define ACHIEVEMENT_FACTION_FLAG_ALLIANCE 0

/// Horde-only achievement
#define ACHIEVEMENT_FACTION_FLAG_HORDE 1

/// ms smoother server/client side moving vs less cpu/ less b/w
// #define UNIT_MOVEMENT_INTERPOLATE_INTERVAL 400/*750*/

/// we most likely will have to kill players and only then check mobs
#define TARGET_UPDATE_INTERVAL_ON_PLAYER 1000

/// -
// #define PLAYER_SIZE 1.5f

/// -
#define CREATURE_SPELL_TO_DAZE 1604

/// for the beginners this means 45 degrees
#define CREATURE_DAZE_TRIGGER_ANGLE AscEmu::Math::HalfPiF

/// minimal level of the target player to daze, from 3.3.0
#define CREATURE_DAZE_MIN_LEVEL 6

/// not try to reposition creature to obtain perfect combat range
const float MIN_WALK_DISTANCE = 2.0f;

/// it is in seconds and not Milliseconds
#define MOB_SPELLCAST_GLOBAL_COOLDOWN 2 // there are individual cooldown and global ones. Global cooldown stops mob from casting 1 instant spell on you per second

/// -
#define MOB_SPELLCAST_REFRESH_COOLDOWN_INTERVAL 2

/// -
// #define INHERIT_FOLLOWED_UNIT_SPEED 1

/// Pathfinding stuff
#define VERTEX_SIZE 3

/// -
#define MAX_PATH_LENGTH 512 // 1024
// Was 74 (74 * SMOOTH_PATH_STEP_SIZE = ~296 world units of path length
// before PathGenerator::buildPointPath() discards the whole, otherwise
// valid, path and falls back to a straight-line shortcut instead - see
// PATHFIND_SHORT in PathGenerator.cpp). That ceiling is easily reached by
// an ordinary chase distance, especially once a slope needs several extra
// steering points to follow properly, so a creature that pathed correctly
// at close range would revert to cutting straight through terrain once the
// target moved far enough away. 256 matches AIInterface's own more
// generous MAX_PATH_LENGTH-based budget used elsewhere; the extra stack
// usage (a few KB at most, in a handful of fixed-size arrays sized off
// this constant) is negligible.
#define MAX_POINT_PATH_LENGTH   256
/// -
#define SMOOTH_PATH_STEP_SIZE 4.0f

/// -
#define SMOOTH_PATH_SLOP 0.3f

#define INVALID_POLYREF   0
