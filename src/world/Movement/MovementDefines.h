/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <optional>

#include "CommonTypes.hpp"

//! Optional helper class to wrap optional values within.
template <class T>
using Optional = std::optional<T>;

#define CONTACT_DISTANCE                0.5f
#define MAX_HEIGHT                      100000.0f
#define INVALID_HEIGHT                  -100000.0f
#define MAX_FALL_DISTANCE               250000.0f
#define PET_FOLLOW_DIST                 1.0f
#define PET_FOLLOW_ANGLE                float(M_PI/2)
#define SPEED_CHARGE                    42.0f // assume it is 25 yard per 0.6 second
#define M_PI_4                          0.785398163397448309616  // pi/4

enum EventId
{
    EVENT_CHARGE                    = 1003,
    EVENT_JUMP                      = 1004,

    /// Special charge event which is used for charge spells that have explicit targets
    /// and had a path already generated - using it in PointMovementGenerator will not
    /// create a new spline and launch it
    EVENT_CHARGE_PREPATH            = 1005,

    EVENT_FACE                      = 1006,
    EVENT_VEHICLE_BOARD             = 1007,
    EVENT_VEHICLE_EXIT              = 1008,
    EVENT_ASSIST_MOVE               = 1009,
};

enum MovementGeneratorType : uint8_t
{
    IDLE_MOTION_TYPE                = 0,     // IdleMovementGenerator.h
    RANDOM_MOTION_TYPE              = 1,     // RandomMovementGenerator.h
    WAYPOINT_MOTION_TYPE            = 2,     // WaypointMovementGenerator.h
    MAX_DB_MOTION_TYPE              = 3,     // Below motion types can't be set in DB.
    CONFUSED_MOTION_TYPE            = 4,     // ConfusedMovementGenerator.h
    CHASE_MOTION_TYPE               = 5,     // ChaseMovementGenerator.h
    HOME_MOTION_TYPE                = 6,     // HomeMovementGenerator.h
    FLIGHT_MOTION_TYPE              = 7,     // FlightPathMovementGenerator.h
    POINT_MOTION_TYPE               = 8,     // PointMovementGenerator.h
    FLEEING_MOTION_TYPE             = 9,     // FleeingMovementGenerator.h
    DISTRACT_MOTION_TYPE            = 10,    // IdleMovementGenerator.h
    ASSISTANCE_MOTION_TYPE          = 11,    // PointMovementGenerator.h
    ASSISTANCE_DISTRACT_MOTION_TYPE = 12,    // IdleMovementGenerator.h
    TIMED_FLEEING_MOTION_TYPE       = 13,    // FleeingMovementGenerator.h
    FOLLOW_MOTION_TYPE              = 14,    // FollowMovementGenerator.h
    ROTATE_MOTION_TYPE              = 15,    // IdleMovementGenerator.h
    EFFECT_MOTION_TYPE              = 16,
    SPLINE_CHAIN_MOTION_TYPE        = 17,    // SplineChainMovementGenerator.h
    FORMATION_MOTION_TYPE           = 18,    // FormationMovementGenerator.h
    MAX_MOTION_TYPE                          // SKIP
};

enum MovementGeneratorMode : uint8_t
{
    MOTION_MODE_DEFAULT = 0,
    MOTION_MODE_OVERRIDE
};

enum MovementGeneratorPriority : uint8_t
{
    MOTION_PRIORITY_NONE = 0,
    MOTION_PRIORITY_NORMAL,
    MOTION_PRIORITY_HIGHEST
};

enum MovementSlot : uint8_t
{
    MOTION_SLOT_DEFAULT = 0,
    MOTION_SLOT_ACTIVE,
    MAX_MOTION_SLOT
};

enum RotateDirection : uint8_t
{
    ROTATE_DIRECTION_LEFT = 0,
    ROTATE_DIRECTION_RIGHT
};

struct SERVER_DECL ChaseRange
{
    ChaseRange(float range);
    ChaseRange(float _minRange, float _maxRange);
    ChaseRange(float _minRange, float _minTolerance, float _maxTolerance, float _maxRange);

    // this contains info that informs how we should path!
    float MinRange;     // we have to move if we are within this range...    (min. attack range)
    float MinTolerance; // ...and if we are, we will move this far away
    float MaxRange;     // we have to move if we are outside this range...   (max. attack range)
    float MaxTolerance; // ...and if we are, we will move into this range
};

struct SERVER_DECL ChaseAngle
{
    ChaseAngle(float angle, float _tolerance = M_PI_4);

    float RelativeAngle; // we want to be at this angle relative to the target (0 = front, M_PI = back)
    float Tolerance;     // but we'll tolerate anything within +- this much

    float upperBound() const;
    float lowerBound() const;
    bool isAngleOkay(float relativeAngle) const;
};

inline bool isInvalidMovementGeneratorType(uint8_t const type) { return type == MAX_DB_MOTION_TYPE || type >= MAX_MOTION_TYPE; }
inline bool isInvalidMovementSlot(uint8_t const slot) { return slot >= MAX_MOTION_SLOT; }
