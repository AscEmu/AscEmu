/*
Copyright (c) 2014-2021 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "Movement/AbstractFollower.h"
#include "Movement/MovementDefines.h"
#include "Movement/MovementGenerator.h"
#include "LocationVector.h"

class PathGenerator;
class Unit;

class ChaseMovementGenerator : public MovementGenerator, public AbstractFollower
{
    public:
        explicit ChaseMovementGenerator(Unit* target, Optional<ChaseRange> range = {}, Optional<ChaseAngle> angle = {});
        ~ChaseMovementGenerator();

        void initialize(Unit*) override;
        void reset(Unit*) override;
        bool update(Unit*, uint32_t) override;
        void deactivate(Unit*) override;
        void finalize(Unit*, bool, bool) override;
        MovementGeneratorType getMovementGeneratorType() const override { return CHASE_MOTION_TYPE; }

        void unitSpeedChanged() override { _lastTargetPosition.reset(); }

    private:
        static constexpr uint32 RANGE_CHECK_INTERVAL = 100; // time (ms) until we attempt to recalculate

        Optional<ChaseRange> const _range;
        Optional<ChaseAngle> const _angle;

        std::unique_ptr<PathGenerator> _path;
        Optional<LocationVector> _lastTargetPosition;
        SmallTimeTracker _rangeCheckTimer;
        bool _movingTowards = true;
        bool _mutualChase = true;
};
