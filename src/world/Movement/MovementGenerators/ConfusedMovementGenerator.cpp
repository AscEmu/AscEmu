/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "ConfusedMovementGenerator.h"
#include "Objects/Units/Creatures/Creature.h"
#include "Movement/MovementDefines.h"
#include "Movement/Spline/MoveSpline.h"
#include "Movement/Spline/MoveSplineInit.h"
#include "Movement/PathGenerator.h"
#include "Objects/Units/Creatures/AIInterface.h"
#include "Objects/Units/Players/Player.hpp"
#include "Utilities/Random.hpp"
#include "Utilities/TimeTracker.hpp"

template<class T>
ConfusedMovementGenerator<T>::ConfusedMovementGenerator() : _timer(std::make_unique<Util::SmallTimeTracker>(0)), _x(0.f), _y(0.f), _z(0.f)
{
    this->Mode = MOTION_MODE_DEFAULT;
    this->Priority = MOTION_PRIORITY_HIGHEST;
    this->Flags = MOVEMENTGENERATOR_FLAG_INITIALIZATION_PENDING;
    this->BaseUnitState = UNIT_STATE_CONFUSED;
}

template<class T>
MovementGeneratorType ConfusedMovementGenerator<T>::getMovementGeneratorType() const
{
    return CONFUSED_MOTION_TYPE;
}

template<class T>
void ConfusedMovementGenerator<T>::doInitialize(T* owner)
{
    MovementGenerator::removeFlag(MOVEMENTGENERATOR_FLAG_INITIALIZATION_PENDING | MOVEMENTGENERATOR_FLAG_TRANSITORY | MOVEMENTGENERATOR_FLAG_DEACTIVATED);
    MovementGenerator::addFlag(MOVEMENTGENERATOR_FLAG_INITIALIZED);

    if (!owner || !owner->isAlive())
        return;

    // TODO: UNIT_FIELD_FLAGS should not be handled by generators
    owner->addUnitFlags(UNIT_FLAG_CONFUSED);
    owner->stopMoving();

    _timer->resetInterval(0);
    owner->getPosition(_x, _y, _z);
    _path = nullptr;
}

template<class T>
void ConfusedMovementGenerator<T>::doReset(T* owner)
{
    MovementGenerator::removeFlag(MOVEMENTGENERATOR_FLAG_TRANSITORY | MOVEMENTGENERATOR_FLAG_DEACTIVATED);

    doInitialize(owner);
}

template<class T>
bool ConfusedMovementGenerator<T>::doUpdate(T* owner, uint32_t diff)
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

    // waiting for next move
    _timer->updateTimer(diff);
    if ((MovementGenerator::hasFlag(MOVEMENTGENERATOR_FLAG_SPEED_UPDATE_PENDING) && !owner->movespline->Finalized()) || (_timer->isTimePassed() && owner->movespline->Finalized()))
    {
        MovementGenerator::removeFlag(MOVEMENTGENERATOR_FLAG_TRANSITORY);

        LocationVector destination(_x, _y, _z);
        float distance = 4.0f * Util::getRandomFloat(0.0f, 1.0f) - 2.0f;
        float angle = Util::getRandomFloat(0.0f, 1.0f) * float(M_PI) * 2.0f;
        owner->movePositionToFirstCollision(destination, distance, angle);

        // Check if the destination is in LOS
        if (!owner->IsWithinLOS(destination))
        {
            // Retry later on
            _timer->resetInterval(200);
            return true;
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
            return true;
        }

        owner->addUnitStateFlag(UNIT_STATE_CONFUSED_MOVE);

        MovementMgr::MoveSplineInit init(owner);
        init.MovebyPath(_path->getPath());
        init.SetWalk(true);
        int32_t traveltime = init.Launch();
        _timer->resetInterval(std::max(300u, Util::getRandomUInt(traveltime / 2, traveltime * 2)));
    }

    return true;
}

template<class T>
void ConfusedMovementGenerator<T>::doDeactivate(T* owner)
{
    MovementGenerator::addFlag(MOVEMENTGENERATOR_FLAG_DEACTIVATED);
    owner->removeUnitStateFlag(UNIT_STATE_CONFUSED_MOVE);
}

template<class T>
void ConfusedMovementGenerator<T>::doFinalize(T*, bool, bool) { }

template<>
void ConfusedMovementGenerator<Player>::doFinalize(Player* owner, bool active, bool/* movementInform*/)
{
    addFlag(MOVEMENTGENERATOR_FLAG_FINALIZED);

    if (active)
    {
        owner->removeUnitFlags(UNIT_FLAG_CONFUSED);
        owner->stopMoving();
    }
}

template<>
void ConfusedMovementGenerator<Creature>::doFinalize(Creature* owner, bool active, bool/* movementInform*/)
{
    addFlag(MOVEMENTGENERATOR_FLAG_FINALIZED);

    if (active)
    {
        owner->removeUnitFlags(UNIT_FLAG_CONFUSED);
        owner->removeUnitStateFlag(UNIT_STATE_CONFUSED_MOVE);
        if (owner->getAIInterface()->getCurrentTarget())
            owner->setTargetGuid(owner->getGuid());
    }
}

template ConfusedMovementGenerator<Player>::ConfusedMovementGenerator();
template ConfusedMovementGenerator<Creature>::ConfusedMovementGenerator();
template MovementGeneratorType ConfusedMovementGenerator<Player>::getMovementGeneratorType() const;
template MovementGeneratorType ConfusedMovementGenerator<Creature>::getMovementGeneratorType() const;
template void ConfusedMovementGenerator<Player>::doInitialize(Player*);
template void ConfusedMovementGenerator<Creature>::doInitialize(Creature*);
template void ConfusedMovementGenerator<Player>::doReset(Player*);
template void ConfusedMovementGenerator<Creature>::doReset(Creature*);
template bool ConfusedMovementGenerator<Player>::doUpdate(Player*, uint32_t);
template bool ConfusedMovementGenerator<Creature>::doUpdate(Creature*, uint32_t);
template void ConfusedMovementGenerator<Player>::doDeactivate(Player*);
template void ConfusedMovementGenerator<Creature>::doDeactivate(Creature*);
