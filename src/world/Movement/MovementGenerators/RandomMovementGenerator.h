/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "Movement/MovementGenerator.h"
#include "LocationVector.h"

namespace Util
{
    struct SmallTimeTracker;
}

class PathGenerator;
struct SmallTimeTracker;

template<class T>
class RandomMovementGenerator : public MovementGeneratorMedium<T, RandomMovementGenerator<T>>
{
public:
    explicit RandomMovementGenerator(float distance = 0.0f);

    MovementGeneratorType getMovementGeneratorType() const override;

    void pause(uint32_t timer = 0) override;
    void resume(uint32_t overrideTimer = 0) override;

    void doInitialize(T*);
    void doReset(T*);
    bool doUpdate(T*, uint32_t);
    void doDeactivate(T*);
    void doFinalize(T*, bool, bool);

    void unitSpeedChanged() override { RandomMovementGenerator<T>::addFlag(MOVEMENTGENERATOR_FLAG_SPEED_UPDATE_PENDING); }

private:
    void setRandomLocation(T*);

    std::unique_ptr<PathGenerator> _path;
    std::unique_ptr<Util::SmallTimeTracker> _timer;
    LocationVector _reference;
    float _maxWanderDistance;
    uint8_t _wanderSteps;
};
