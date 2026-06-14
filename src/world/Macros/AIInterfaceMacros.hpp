/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <vector>

/// All criteria must be completed for the achievement to be complete.
#define ACHIEVEMENT_CRITERIA_COMPLETE_FLAG_ALL 2

/// You must not be in a group to complete the achievement.
#define ACHIEVEMENT_CRITERIA_GROUP_NOT_IN_GROUP 2

/// Alliance-only achievement
#define ACHIEVEMENT_FACTION_FLAG_ALLIANCE 0

/// Horde-only achievement
#define ACHIEVEMENT_FACTION_FLAG_HORDE 1

/// minimal level of the target player to daze, from 3.3.0
#define CREATURE_DAZE_MIN_LEVEL 6

/// not try to reposition creature to obtain perfect combat range
const float MIN_WALK_DISTANCE = 2.0f;

/// Pathfinding stuff
#define VERTEX_SIZE 3

/// -
#define MAX_PATH_LENGTH 512 // 1024
#define MAX_POINT_PATH_LENGTH 74
/// -
#define SMOOTH_PATH_STEP_SIZE 4.0f

/// -
#define SMOOTH_PATH_SLOP 0.3f

#define INVALID_POLYREF   0
