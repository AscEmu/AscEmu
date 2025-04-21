/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <memory>
#include <vector>

class AreaBoundary;

typedef std::vector<std::unique_ptr<AreaBoundary const>> CreatureBoundary;

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
#define CREATURE_DAZE_TRIGGER_ANGLE M_H_PI

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
#define MAX_POINT_PATH_LENGTH   74
/// -
#define SMOOTH_PATH_STEP_SIZE 4.0f

/// -
#define SMOOTH_PATH_SLOP 0.3f

#define INVALID_POLYREF   0
