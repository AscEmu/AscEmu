/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "Movement/MovementGenerator.h"

namespace Util
{
    struct SmallTimeTracker;
}

class Creature;
class PathGenerator;
class LocationVector;

template<class T>
class FleeingMovementGenerator : public MovementGeneratorMedium<T, FleeingMovementGenerator<T>>
{
public:
    explicit FleeingMovementGenerator(uint64_t fleeTargetGUID);

    MovementGeneratorType getMovementGeneratorType() const override;

    void doInitialize(T*);
    void doReset(T*);
    bool doUpdate(T*, uint32_t);
    void doDeactivate(T*);
    void doFinalize(T*, bool, bool);

    void unitSpeedChanged() override { FleeingMovementGenerator<T>::addFlag(MOVEMENTGENERATOR_FLAG_SPEED_UPDATE_PENDING); }

private:
    void setTargetLocation(T*);
    void getPoint(T*, LocationVector& position);

    std::unique_ptr<PathGenerator> _path;
    uint64_t _fleeTargetGUID;
    std::unique_ptr<Util::SmallTimeTracker> _timer;
};

class TimedFleeingMovementGenerator : public FleeingMovementGenerator<Creature>
{
public:
    explicit TimedFleeingMovementGenerator(uint64_t fleeTargetGUID, uint32_t time);

    bool update(Unit*, uint32_t) override;
    void finalize(Unit*, bool, bool) override;
    MovementGeneratorType getMovementGeneratorType() const override;

private:
    std::unique_ptr<Util::SmallTimeTracker> _totalFleeTime;
};
