/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "Movement/MovementGenerator.h"
#include "PathMovementBase.h"

namespace Util
{
    struct SmallTimeTracker;
}

class Creature;
class Unit;
struct WaypointPath;

template<class T>
class WaypointMovementGenerator;

template<>
class WaypointMovementGenerator<Creature> : public MovementGeneratorMedium<Creature, WaypointMovementGenerator<Creature>>, public PathMovementBase<Creature, WaypointPath const*>
{
public:
    explicit WaypointMovementGenerator(uint32_t pathId = 0, bool repeating = true);
    explicit WaypointMovementGenerator(WaypointPath& path, bool repeating = true);
    ~WaypointMovementGenerator() { _path = nullptr; }

    MovementGeneratorType getMovementGeneratorType() const override;

    void unitSpeedChanged() override { addFlag(MOVEMENTGENERATOR_FLAG_SPEED_UPDATE_PENDING); }
    void pause(uint32_t timer = 0) override;
    void resume(uint32_t overrideTimer = 0) override;
    bool getResetPosition(Unit*, float& x, float& y, float& z) override;

    void doInitialize(Creature*);
    void doReset(Creature*);
    bool doUpdate(Creature*, uint32_t);
    void doDeactivate(Creature*);
    void doFinalize(Creature*, bool, bool);

    std::string getDebugInfo() const override;

private:
    void movementInform(Creature*);
    void onArrived(Creature*);
    void startMove(Creature*, bool relaunch = false);
    bool computeNextNode();
    bool updateTimer(uint32_t diff);

    std::unique_ptr<Util::SmallTimeTracker> _nextMoveTime;
    uint32_t _pathId;
    bool _repeating;
    bool _loadedFromDB;
};
