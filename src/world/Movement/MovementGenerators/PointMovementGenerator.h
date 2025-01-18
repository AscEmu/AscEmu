/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "Movement/MovementDefines.h"
#include "Movement/MovementGenerator.h"

class Creature;

template<class T>
class PointMovementGenerator : public MovementGeneratorMedium<T, PointMovementGenerator<T>>
{
public:
    explicit PointMovementGenerator(uint32_t id, float x, float y, float z, bool generatePath, float speed = 0.0f, Optional<float> finalOrient = {});

    MovementGeneratorType getMovementGeneratorType() const override;

    void doInitialize(T*);
    void doReset(T*);
    bool doUpdate(T*, uint32_t);
    void doDeactivate(T*);
    void doFinalize(T*, bool, bool);

    void unitSpeedChanged() override { PointMovementGenerator<T>::addFlag(MOVEMENTGENERATOR_FLAG_SPEED_UPDATE_PENDING); }

    uint32_t getId() const { return _movementId; }

private:
    void movementInform(T*);

    uint32_t _movementId;
    float _x, _y, _z;
    float _speed;
    bool _generatePath;
    //! if set then unit will turn to specified _orient in provided _pos
    Optional<float> _finalOrient;
};

class AssistanceMovementGenerator : public PointMovementGenerator<Creature>
{
public:
    explicit AssistanceMovementGenerator(uint32_t id, float x, float y, float z) : PointMovementGenerator<Creature>(id, x, y, z, true) { }

    void finalize(Unit*, bool, bool) override;
    MovementGeneratorType getMovementGeneratorType() const override;
};
