/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "FleeingMovementGenerator.h"
#include "Objects/Units/Creatures/Creature.h"
#include "Objects/Units/Creatures/AIInterface.h"
#include "Movement/MovementDefines.h"
#include "Movement/Spline/MoveSpline.h"
#include "Movement/Spline/MoveSplineInit.h"
#include "Movement/PathGenerator.h"
#include "Objects/Units/Players/Player.hpp"
#include "Objects/Units/Unit.hpp"
#include "Utilities/Random.hpp"
#include "Utilities/TimeTracker.hpp"

#define MIN_QUIET_DISTANCE 28.0f
#define MAX_QUIET_DISTANCE 43.0f

template<class T>
FleeingMovementGenerator<T>::FleeingMovementGenerator(uint64_t fleeTargetGUID) : _fleeTargetGUID(fleeTargetGUID), _timer(std::make_unique<Util::SmallTimeTracker>(0))
{
    this->Mode = MOTION_MODE_DEFAULT;
    this->Priority = MOTION_PRIORITY_HIGHEST;
    this->Flags = MOVEMENTGENERATOR_FLAG_INITIALIZATION_PENDING;
    this->BaseUnitState = UNIT_STATE_FLEEING;
}

template<class T>
MovementGeneratorType FleeingMovementGenerator<T>::getMovementGeneratorType() const
{
    return FLEEING_MOTION_TYPE;
}

template<class T>
void FleeingMovementGenerator<T>::doInitialize(T* owner)
{
    MovementGenerator::removeFlag(MOVEMENTGENERATOR_FLAG_INITIALIZATION_PENDING | MOVEMENTGENERATOR_FLAG_TRANSITORY | MOVEMENTGENERATOR_FLAG_DEACTIVATED);
    MovementGenerator::addFlag(MOVEMENTGENERATOR_FLAG_INITIALIZED);

    if (!owner || !owner->isAlive())
        return;

    // TODO: UNIT_FIELD_FLAGS should not be handled by generators
    owner->addUnitFlags(UNIT_FLAG_FLEEING);

    _path = nullptr;
    setTargetLocation(owner);
}

template<class T>
void FleeingMovementGenerator<T>::doReset(T* owner)
{
    MovementGenerator::removeFlag(MOVEMENTGENERATOR_FLAG_TRANSITORY | MOVEMENTGENERATOR_FLAG_DEACTIVATED);

    doInitialize(owner);
}

template<class T>
bool FleeingMovementGenerator<T>::doUpdate(T* owner, uint32_t diff)
{
    if (!owner || !owner->isAlive())
        return false;

    if (owner->hasUnitStateFlag(UNIT_STATE_NOT_MOVE) || owner->isCastingSpell())
    {
        MovementGenerator::addFlag(MOVEMENTGENERATOR_FLAG_INTERRUPTED);
        owner->stopMoving();
        _path = nullptr;
        return true;
    }
    else
        MovementGenerator::removeFlag(MOVEMENTGENERATOR_FLAG_INTERRUPTED);

    _timer->updateTimer(diff);
    if ((MovementGenerator::hasFlag(MOVEMENTGENERATOR_FLAG_SPEED_UPDATE_PENDING) && !owner->movespline->Finalized()) || (_timer->isTimePassed() && owner->movespline->Finalized()))
    {
        MovementGenerator::removeFlag(MOVEMENTGENERATOR_FLAG_TRANSITORY);
        setTargetLocation(owner);
    }

    return true;
}

template<class T>
void FleeingMovementGenerator<T>::doDeactivate(T* owner)
{
    MovementGenerator::addFlag(MOVEMENTGENERATOR_FLAG_DEACTIVATED);
    owner->removeUnitStateFlag(UNIT_STATE_FLEEING_MOVE);
}

template<class T>
void FleeingMovementGenerator<T>::doFinalize(T*, bool, bool)
{
}

template<>
void FleeingMovementGenerator<Player>::doFinalize(Player* owner, bool active, bool/* movementInform*/)
{
    addFlag(MOVEMENTGENERATOR_FLAG_FINALIZED);

    if (active)
    {
        owner->removeUnitFlags(UNIT_FLAG_FLEEING);
        owner->removeUnitStateFlag(UNIT_STATE_FLEEING_MOVE);
        owner->stopMoving();
    }
}

template<>
void FleeingMovementGenerator<Creature>::doFinalize(Creature* owner, bool active, bool/* movementInform*/)
{
    addFlag(MOVEMENTGENERATOR_FLAG_FINALIZED);

    if (active)
    {
        owner->removeUnitFlags(UNIT_FLAG_FLEEING);
        owner->removeUnitStateFlag(UNIT_STATE_FLEEING_MOVE);
        if (owner->getAIInterface()->getCurrentTarget())
            owner->setTargetGuid(owner->getGuid());
    }
}

template<class T>
void FleeingMovementGenerator<T>::setTargetLocation(T* owner)
{
    if (!owner || !owner->isAlive())
        return;

    if (owner->hasUnitStateFlag(UNIT_STATE_NOT_MOVE) || owner->isCastingSpell())
    {
        MovementGenerator::addFlag(MOVEMENTGENERATOR_FLAG_INTERRUPTED);
        owner->stopMoving();
        _path = nullptr;
        return;
    }

    LocationVector destination = owner->GetPosition();
    getPoint(owner, destination);

    // Add LOS check for target point
    if (!owner->IsWithinLOS(destination))
    {
        _timer->resetInterval(200);
        return;
    }

    if (!_path)
    {
        _path = std::make_unique<PathGenerator>(owner);
        _path->setPathLengthLimit(30.0f);
    }

    bool result = _path->calculatePath(destination.getPositionX(), destination.getPositionY(), destination.getPositionZ());
    if (!result || (_path->getPathType() & PATHFIND_NOPATH)
                || (_path->getPathType() & PATHFIND_SHORTCUT)
                || (_path->getPathType() & PATHFIND_FARFROMPOLY))
    {
        _timer->resetInterval(100);
        return;
    }

    owner->addUnitStateFlag(UNIT_STATE_FLEEING_MOVE);

    MovementMgr::MoveSplineInit init(owner);
    init.MovebyPath(_path->getPath());
    init.SetWalk(false);
    int32_t traveltime = init.Launch();
    _timer->resetInterval(traveltime + Util::getRandomUInt(800, 1500));
}

template<class T>
void FleeingMovementGenerator<T>::getPoint(T* owner, LocationVector &position)
{
    float casterDistance, casterAngle;
    if (Unit* fleeTarget = (*owner).getWorldMapUnit(_fleeTargetGUID))
    {
        casterDistance = fleeTarget->getDistance(owner);
        if (casterDistance > 0.2f)
            casterAngle = fleeTarget->getAbsoluteAngle(owner);
        else
            casterAngle = Util::getRandomFloat(0.0f, 2.0f * float(M_PI));
    }
    else
    {
        casterDistance = 0.0f;
        casterAngle = Util::getRandomFloat(0.0f, 2.0f * float(M_PI));
    }

    float distance, angle;
    if (casterDistance < MIN_QUIET_DISTANCE)
    {
        distance = Util::getRandomFloat(0.4f, 1.3f) * (MIN_QUIET_DISTANCE - casterDistance);
        angle = casterAngle + Util::getRandomFloat(-float(M_PI) / 8.0f, float(M_PI) / 8.0f);
    }
    else if (casterDistance > MAX_QUIET_DISTANCE)
    {
        distance = Util::getRandomFloat(0.4f, 1.0f) * (MAX_QUIET_DISTANCE - MIN_QUIET_DISTANCE);
        angle = -casterAngle + Util::getRandomFloat(-float(M_PI) / 4.0f, float(M_PI) / 4.0f);
    }
    else // we are inside quiet range
    {
        distance = Util::getRandomFloat(0.6f, 1.2f) * (MAX_QUIET_DISTANCE - MIN_QUIET_DISTANCE);
        angle = Util::getRandomFloat(0.0f, 2.0f * float(M_PI));
    }

    owner->movePositionToFirstCollision(position, distance, angle);
}

template FleeingMovementGenerator<Player>::FleeingMovementGenerator(uint64_t);
template FleeingMovementGenerator<Creature>::FleeingMovementGenerator(uint64_t);
template MovementGeneratorType FleeingMovementGenerator<Player>::getMovementGeneratorType() const;
template MovementGeneratorType FleeingMovementGenerator<Creature>::getMovementGeneratorType() const;
template void FleeingMovementGenerator<Player>::doInitialize(Player*);
template void FleeingMovementGenerator<Creature>::doInitialize(Creature*);
template void FleeingMovementGenerator<Player>::doReset(Player*);
template void FleeingMovementGenerator<Creature>::doReset(Creature*);
template bool FleeingMovementGenerator<Player>::doUpdate(Player*, uint32_t);
template bool FleeingMovementGenerator<Creature>::doUpdate(Creature*, uint32_t);
template void FleeingMovementGenerator<Player>::doDeactivate(Player*);
template void FleeingMovementGenerator<Creature>::doDeactivate(Creature*);
template void FleeingMovementGenerator<Player>::setTargetLocation(Player*);
template void FleeingMovementGenerator<Creature>::setTargetLocation(Creature*);
template void FleeingMovementGenerator<Player>::getPoint(Player*, LocationVector &);
template void FleeingMovementGenerator<Creature>::getPoint(Creature*, LocationVector &);

//////////////////////////////////////////////////////////////////////////////////////////
// TimedFleeingMovementGenerator
TimedFleeingMovementGenerator::TimedFleeingMovementGenerator(uint64_t fleeTargetGUID, uint32_t time) : FleeingMovementGenerator<Creature>(fleeTargetGUID), _totalFleeTime(std::make_unique<Util::SmallTimeTracker>(time)) { }
bool TimedFleeingMovementGenerator::update(Unit* owner, uint32_t diff)
{
    if (!owner || !owner->isAlive())
        return false;

    _totalFleeTime->updateTimer(diff);
    if (_totalFleeTime->isTimePassed())
        return false;

    return FleeingMovementGenerator<Creature>::doUpdate(owner->ToCreature(), diff);
}

void TimedFleeingMovementGenerator::finalize(Unit* owner, bool active, bool/* movementInform*/)
{
    addFlag(MOVEMENTGENERATOR_FLAG_FINALIZED);
    if (!active)
        return;

    owner->removeUnitFlags(UNIT_FLAG_FLEEING);
    owner->stopMoving();
    if (Unit* victim = owner->getAIInterface()->getCurrentTarget())
    {
        if (owner->isAlive())
        {
            //owner->AttackStop(); //todo
            owner->ToCreature()->getAIInterface()->onHostileAction(victim);
        }
    }
}

MovementGeneratorType TimedFleeingMovementGenerator::getMovementGeneratorType() const
{
    return TIMED_FLEEING_MOTION_TYPE;
}
