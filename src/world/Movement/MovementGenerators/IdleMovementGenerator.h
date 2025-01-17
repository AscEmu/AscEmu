/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "Movement/MovementGenerator.h"

enum RotateDirection : uint8_t;

class IdleMovementGenerator : public MovementGenerator
{
public:
    explicit IdleMovementGenerator();

    void initialize(Unit*) override;
    void reset(Unit*) override;
    bool update(Unit*, uint32_t) override { return true; }
    void deactivate(Unit*) override;
    void finalize(Unit*, bool, bool) override;
    MovementGeneratorType getMovementGeneratorType() const override;
};

class RotateMovementGenerator : public MovementGenerator
{
public:
    explicit RotateMovementGenerator(uint32_t id, uint32_t time, RotateDirection direction);

    void initialize(Unit*) override;
    void reset(Unit*) override;
    bool update(Unit*, uint32_t) override;
    void deactivate(Unit*) override;
    void finalize(Unit*, bool, bool) override;
    MovementGeneratorType getMovementGeneratorType() const override;

private:
    uint32_t _id, _duration, _maxDuration;
    RotateDirection _direction;
};

class DistractMovementGenerator : public MovementGenerator
{
public:
    explicit DistractMovementGenerator(uint32_t timer, float orientation);

    void initialize(Unit*) override;
    void reset(Unit*) override;
    bool update(Unit*, uint32_t) override;
    void deactivate(Unit*) override;
    void finalize(Unit*, bool, bool) override;
    MovementGeneratorType getMovementGeneratorType() const override;

private:
    uint32_t _timer;
    float _orientation;
};

class AssistanceDistractMovementGenerator : public DistractMovementGenerator
{
public:
    explicit AssistanceDistractMovementGenerator(uint32_t timer, float orientation);

    void finalize(Unit*, bool, bool) override;
    MovementGeneratorType getMovementGeneratorType() const override;
};
