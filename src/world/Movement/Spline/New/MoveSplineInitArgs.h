/*
Copyright (c) 2014-2021 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "MoveSplineFlag.h"
#include <vector>

class Unit;

namespace MovementNew {

typedef std::vector<Vector3> PointsArray;

union FacingInfo
{
    struct
    {
        float x, y, z;
    } f;
    uint64_t target;
    float angle;

    FacingInfo(float o) : angle(o) { }
    FacingInfo(uint64_t t) : target(t) { }
    FacingInfo() { }
};

struct MoveSplineInitArgs
{
    MoveSplineInitArgs(size_t path_capacity = 16);
    MoveSplineInitArgs(MoveSplineInitArgs&& args);
    ~MoveSplineInitArgs();

    PointsArray path;
    FacingInfo facing;
    MoveSplineFlag flags;
    int32_t path_Idx_offset;
    float velocity;
    float parabolic_amplitude;
    float time_perc;
    uint32_t splineId;
    float initialOrientation;
    bool walk;
    bool HasVelocity;
    bool TransformForTransport;

    // Returns true to show that the arguments were configured correctly and MoveSpline initialization will succeed.
    bool Validate(Unit* unit) const;

private:
    bool _checkPathBounds() const;
};
} // namespace MovementNew
