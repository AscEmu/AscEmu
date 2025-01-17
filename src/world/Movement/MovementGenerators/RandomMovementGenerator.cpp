/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "RandomMovementGenerator.h"
#include "Objects/Units/Creatures/Creature.h"
#include "Movement/MovementDefines.h"
#include "Movement/Spline/MoveSpline.h"
#include "Movement/Spline/MoveSplineInit.h"
#include "Movement/PathGenerator.h"
#include "Server/Definitions.h"
#include "Utilities/Random.hpp"
#include "Utilities/TimeTracker.hpp"

template<class T>
RandomMovementGenerator<T>::RandomMovementGenerator(float distance) :
_timer(std::make_unique<Util::SmallTimeTracker>(0)), _reference(), _maxWanderDistance(distance), _wanderSteps(0)
{
    this->Mode = MOTION_MODE_DEFAULT;
    this->Priority = MOTION_PRIORITY_NORMAL;
    this->Flags = MOVEMENTGENERATOR_FLAG_INITIALIZATION_PENDING;
    this->BaseUnitState = UNIT_STATE_ROAMING;
}

template RandomMovementGenerator<Creature>::RandomMovementGenerator(float/* distance*/);

template<class T>
MovementGeneratorType RandomMovementGenerator<T>::getMovementGeneratorType() const
{
    return RANDOM_MOTION_TYPE;
}

template<class T>
void RandomMovementGenerator<T>::pause(uint32_t timer /*= 0*/)
{
    if (timer)
    {
        this->addFlag(MOVEMENTGENERATOR_FLAG_TIMED_PAUSED);
        _timer->resetInterval(timer);
        this->removeFlag(MOVEMENTGENERATOR_FLAG_PAUSED);
    }
    else
    {
        this->addFlag(MOVEMENTGENERATOR_FLAG_PAUSED);
        this->removeFlag(MOVEMENTGENERATOR_FLAG_TIMED_PAUSED);
    }
}

template<class T>
void RandomMovementGenerator<T>::resume(uint32_t overrideTimer /*= 0*/)
{
    if (overrideTimer)
        _timer->resetInterval(overrideTimer);

    this->removeFlag(MOVEMENTGENERATOR_FLAG_PAUSED);
}

template MovementGeneratorType RandomMovementGenerator<Creature>::getMovementGeneratorType() const;

template<class T>
void RandomMovementGenerator<T>::doInitialize(T*) { }

template<>
void RandomMovementGenerator<Creature>::doInitialize(Creature* owner)
{
    removeFlag(MOVEMENTGENERATOR_FLAG_INITIALIZATION_PENDING | MOVEMENTGENERATOR_FLAG_TRANSITORY | MOVEMENTGENERATOR_FLAG_DEACTIVATED | MOVEMENTGENERATOR_FLAG_TIMED_PAUSED);
    addFlag(MOVEMENTGENERATOR_FLAG_INITIALIZED);

    if (!owner || !owner->isAlive())
        return;

    _reference = owner->GetPosition();
    owner->stopMoving();

    if (_maxWanderDistance == 0.f)
        _maxWanderDistance = owner->getMaxWanderDistance();

    // Retail seems to let a creature walk 2 up to 10 splines before triggering a pause
    _wanderSteps = static_cast<uint8_t>(Util::getRandomUInt(2, 10));

    _timer->resetInterval(0);
    _path = nullptr;
}

template<class T>
void RandomMovementGenerator<T>::doReset(T*) { }

template<>
void RandomMovementGenerator<Creature>::doReset(Creature* owner)
{
    removeFlag(MOVEMENTGENERATOR_FLAG_TRANSITORY | MOVEMENTGENERATOR_FLAG_DEACTIVATED);

    doInitialize(owner);
}

template<class T>
void RandomMovementGenerator<T>::setRandomLocation(T*) { }

template<>
void RandomMovementGenerator<Creature>::setRandomLocation(Creature* owner)
{
    if (!owner)
        return;

    if (owner->hasUnitStateFlag(UNIT_STATE_NOT_MOVE | UNIT_STATE_LOST_CONTROL) || owner->isCastingSpell())
    {
        addFlag(MOVEMENTGENERATOR_FLAG_INTERRUPTED);
        owner->stopMoving();
        _path = nullptr;
        return;
    }

    LocationVector position(_reference);
    float distance = Util::getRandomFloat(0.f, _maxWanderDistance);
    float angle = Util::getRandomFloat(0.f, float(M_PI * 2));
    owner->movePositionToFirstCollision(position, distance, angle);

    // Check if the destination is in LOS
    if (!owner->IsWithinLOS(position))
    {
        // Retry later on
        _timer->resetInterval(200);
        return;
    }

    if (!_path)
    {
        _path = std::make_unique<PathGenerator>(owner);
        _path->setPathLengthLimit(30.0f);
    }

    bool result = _path->calculatePath(position.getPositionX(), position.getPositionY(), position.getPositionZ());
    // PATHFIND_FARFROMPOLY shouldn't be checked as creatures in water are most likely far from poly
    if (!result || (_path->getPathType() & PATHFIND_NOPATH)
                || (_path->getPathType() & PATHFIND_SHORTCUT)
                /*|| (_path->GetPathType() & PATHFIND_FARFROMPOLY)*/)
    {
        _timer->resetInterval(100);
        return;
    }

    removeFlag(MOVEMENTGENERATOR_FLAG_TRANSITORY | MOVEMENTGENERATOR_FLAG_TIMED_PAUSED);

    owner->addUnitStateFlag(UNIT_STATE_ROAMING_MOVE);

    bool walk = true;
    switch (owner->getMovementTemplate().getRandom())
    {
        case CreatureRandomMovementType::CanRun:
            walk = owner->isWalking();
            break;
        case CreatureRandomMovementType::AlwaysRun:
            walk = false;
            break;
        default:
            break;
    }

    MovementMgr::MoveSplineInit init(owner);
    init.MovebyPath(_path->getPath());
    init.SetWalk(walk);
    int32_t splineDuration = init.Launch();

    --_wanderSteps;
    if (_wanderSteps) // Creature has yet to do steps before pausing
        _timer->resetInterval(splineDuration);
    else
    {
        // Creature has made all its steps, time for a little break
        _timer->resetInterval(splineDuration + Util::getRandomUInt(4, 10) * IN_MILLISECONDS); // Retails seems to use rounded numbers so we do as well
        _wanderSteps = static_cast<uint8_t>(Util::getRandomUInt(2, 10));
    }

    // Call for creature group update
    owner->signalFormationMovement();
}

template<class T>
bool RandomMovementGenerator<T>::doUpdate(T*, uint32_t)
{
    return false;
}

template<>
bool RandomMovementGenerator<Creature>::doUpdate(Creature* owner, uint32_t diff)
{
    if (!owner || !owner->isAlive())
        return true;

    if (hasFlag(MOVEMENTGENERATOR_FLAG_FINALIZED | MOVEMENTGENERATOR_FLAG_PAUSED))
        return true;

    if (owner->hasUnitStateFlag(UNIT_STATE_NOT_MOVE) || owner->isCastingSpell())
    {
        addFlag(MOVEMENTGENERATOR_FLAG_INTERRUPTED);
        owner->stopMoving();
        _path = nullptr;
        return true;
    }
    else
        removeFlag(MOVEMENTGENERATOR_FLAG_INTERRUPTED);

    _timer->updateTimer(diff);
    if ((hasFlag(MOVEMENTGENERATOR_FLAG_SPEED_UPDATE_PENDING) && !owner->movespline->Finalized()) || (_timer->isTimePassed() && owner->movespline->Finalized()))
        setRandomLocation(owner);

    return true;
}

template<class T>
void RandomMovementGenerator<T>::doDeactivate(T*) { }

template<>
void RandomMovementGenerator<Creature>::doDeactivate(Creature* owner)
{
    addFlag(MOVEMENTGENERATOR_FLAG_DEACTIVATED);
    owner->removeUnitStateFlag(UNIT_STATE_ROAMING_MOVE);
}

template<class T>
void RandomMovementGenerator<T>::doFinalize(T*, bool, bool) { }

template<>
void RandomMovementGenerator<Creature>::doFinalize(Creature* owner, bool active, bool/* movementInform*/)
{
    addFlag(MOVEMENTGENERATOR_FLAG_FINALIZED);
    if (active)
    {
        owner->removeUnitStateFlag(UNIT_STATE_ROAMING_MOVE);
        owner->stopMoving();

        // TODO: Research if this modification is needed, which most likely isnt
        owner->setMoveWalk(false);
    }
}
