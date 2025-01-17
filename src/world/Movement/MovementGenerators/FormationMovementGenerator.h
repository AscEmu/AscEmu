/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "Movement/AbstractFollower.h"
#include "Movement/MovementGenerator.h"
#include "LocationVector.h"

namespace Util
{
    struct SmallTimeTracker;
}

class Creature;

class FormationMovementGenerator : public MovementGeneratorMedium<Creature, FormationMovementGenerator>, public AbstractFollower
{
public:
    explicit FormationMovementGenerator(Unit* leader, float range, float angle, uint32_t point1, uint32_t point2);

    MovementGeneratorType getMovementGeneratorType() const override;

    void doInitialize(Creature*);
    void doReset(Creature*);
    bool doUpdate(Creature*, uint32_t);
    void doDeactivate(Creature*);
    void doFinalize(Creature*, bool, bool);

private:
    void movementInform(Creature*);
    void launchMovement(Creature* owner, Unit* target);

    static constexpr uint32_t FORMATION_MOVEMENT_INTERVAL = 1200; // sniffed (3 batch update cycles)
    float const _range;
    float _angle;
    uint32_t const _point1;
    uint32_t const _point2;
    uint32_t _lastLeaderSplineID;
    bool _hasPredictedDestination;

    LocationVector _lastLeaderPosition;
    std::unique_ptr<Util::SmallTimeTracker> _nextMoveTimer;
};
