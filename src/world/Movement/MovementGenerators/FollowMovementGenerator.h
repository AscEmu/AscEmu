/*
Copyright (c) 2014-2024 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "Movement/AbstractFollower.h"
#include "Movement/MovementDefines.h"
#include "Movement/MovementGenerator.h"
#include "LocationVector.h"
#include "Utilities/Util.hpp"

class PathGenerator;
class Unit;

#define FOLLOW_RANGE_TOLERANCE 1.0f

class FollowMovementGenerator : public MovementGenerator, public AbstractFollower
{
public:
    explicit FollowMovementGenerator(Unit* target, float range, ChaseAngle angle);
    ~FollowMovementGenerator();

    void initialize(Unit*) override;
    void reset(Unit*) override;
    bool update(Unit*, uint32_t) override;
    void deactivate(Unit*) override;
    void finalize(Unit*, bool, bool) override;
    MovementGeneratorType getMovementGeneratorType() const override { return FOLLOW_MOTION_TYPE; }

    void unitSpeedChanged() override { _lastTargetPosition.reset(); }

private:
    static constexpr uint32_t CHECK_INTERVAL = 100;

    void updatePetSpeed(Unit* owner);

    float const _range;
    ChaseAngle const _angle;

    SmallTimeTracker _checkTimer;
    std::unique_ptr<PathGenerator> _path;
    Optional<LocationVector> _lastTargetPosition;
};
