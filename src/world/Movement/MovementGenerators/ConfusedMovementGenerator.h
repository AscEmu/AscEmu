/*
Copyright (c) 2014-2021 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "Movement/MovementGenerator.h"

class PathGenerator;

template<class T>
class ConfusedMovementGenerator : public MovementGeneratorMedium<T, ConfusedMovementGenerator<T>>
{
public:
    explicit ConfusedMovementGenerator();

    MovementGeneratorType getMovementGeneratorType() const override;

    void doInitialize(T*);
    void doReset(T*);
    bool doUpdate(T*, uint32);
    void doDeactivate(T*);
    void doFinalize(T*, bool, bool);

    void unitSpeedChanged() override { ConfusedMovementGenerator<T>::addFlag(MOVEMENTGENERATOR_FLAG_SPEED_UPDATE_PENDING); }

private:
    std::unique_ptr<PathGenerator> _path;
    SmallTimeTracker _timer;
    float _x, _y, _z;
};
