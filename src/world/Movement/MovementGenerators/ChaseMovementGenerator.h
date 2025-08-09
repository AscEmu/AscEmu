/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "Movement/AbstractFollower.h"
#include "Movement/MovementDefines.h"
#include "Movement/MovementGenerator.h"
#include "LocationVector.h"

namespace Util
{
    struct SmallTimeTracker;
}

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
    MovementGeneratorType getMovementGeneratorType() const override;

    void unitSpeedChanged() override;

private:
    static constexpr uint32_t RANGE_CHECK_INTERVAL = 100; // time (ms) until we attempt to recalculate

    Optional<ChaseRange> const _range;
    Optional<ChaseAngle> const _angle;

    std::unique_ptr<PathGenerator> _path;
    Optional<LocationVector> _lastTargetPosition;
    std::unique_ptr<Util::SmallTimeTracker> _rangeCheckTimer;
    bool _movingTowards = true;
    bool _mutualChase = true;
};
