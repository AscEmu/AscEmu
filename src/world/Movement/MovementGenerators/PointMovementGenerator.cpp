/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "PointMovementGenerator.h"
#include "Objects/Units/Creatures/Creature.h"
#include "Objects/Units/Creatures/AIInterface.h"
#include "Objects/Units/Players/Player.hpp"
#include "Movement/MovementManager.h"
#include "Movement/MovementDefines.h"
#include "Movement/Spline/MoveSpline.h"
#include "Movement/Spline/MoveSplineInit.h"

//////////////////////////////////////////////////////////////////////////////////////////
// PointMovementGenerator
template<class T>
PointMovementGenerator<T>::PointMovementGenerator(uint32_t id, float x, float y, float z, bool generatePath, float speed, Optional<float> finalOrient) : _movementId(id), _x(x), _y(y), _z(z), _speed(speed), _generatePath(generatePath), _finalOrient(finalOrient)
{
    this->Mode = MOTION_MODE_DEFAULT;
    this->Priority = MOTION_PRIORITY_NORMAL;
    this->Flags = MOVEMENTGENERATOR_FLAG_INITIALIZATION_PENDING;
    this->BaseUnitState = UNIT_STATE_ROAMING;
}

template<class T>
MovementGeneratorType PointMovementGenerator<T>::getMovementGeneratorType() const
{
    return POINT_MOTION_TYPE;
}

template<class T>
void PointMovementGenerator<T>::doInitialize(T* owner)
{
    MovementGenerator::removeFlag(MOVEMENTGENERATOR_FLAG_INITIALIZATION_PENDING | MOVEMENTGENERATOR_FLAG_TRANSITORY | MOVEMENTGENERATOR_FLAG_DEACTIVATED);
    MovementGenerator::addFlag(MOVEMENTGENERATOR_FLAG_INITIALIZED);

    if (_movementId == EVENT_CHARGE_PREPATH)
    {
        owner->addUnitStateFlag(UNIT_STATE_ROAMING_MOVE);
        return;
    }

    if (owner->hasUnitStateFlag(UNIT_STATE_NOT_MOVE) || owner->isCastingSpell())
    {
        MovementGenerator::addFlag(MOVEMENTGENERATOR_FLAG_INTERRUPTED);
        owner->stopMoving();
        return;
    }

    owner->addUnitStateFlag(UNIT_STATE_ROAMING_MOVE);

    MovementMgr::MoveSplineInit init(owner);
    init.MoveTo(_x, _y, _z , _generatePath);
    if (_speed > 0.0f)
        init.SetVelocity(_speed);

    if (_finalOrient)
        init.SetFacing(*_finalOrient);

    init.Launch();

    // Call for creature group update
    if (Creature* creature = owner->ToCreature())
        creature->signalFormationMovement();
}

template<class T>
void PointMovementGenerator<T>::doReset(T* owner)
{
    MovementGenerator::removeFlag(MOVEMENTGENERATOR_FLAG_TRANSITORY | MOVEMENTGENERATOR_FLAG_DEACTIVATED);

    doInitialize(owner);
}

template<class T>
bool PointMovementGenerator<T>::doUpdate(T* owner, uint32_t /*diff*/)
{
    if (!owner)
        return false;

    if (_movementId == EVENT_CHARGE_PREPATH)
    {
        if (owner->movespline->Finalized())
        {
            MovementGenerator::addFlag(MOVEMENTGENERATOR_FLAG_INFORM_ENABLED);
            return false;
        }
        return true;
    }

    if (owner->hasUnitStateFlag(UNIT_STATE_NOT_MOVE) || owner->ToUnit()->isCastingSpell())
    {
        MovementGenerator::addFlag(MOVEMENTGENERATOR_FLAG_INTERRUPTED);
        owner->stopMoving();
        return true;
    }

    if ((MovementGenerator::hasFlag(MOVEMENTGENERATOR_FLAG_INTERRUPTED) && owner->movespline->Finalized()) || (MovementGenerator::hasFlag(MOVEMENTGENERATOR_FLAG_SPEED_UPDATE_PENDING) && !owner->movespline->Finalized()))
    {
        MovementGenerator::removeFlag(MOVEMENTGENERATOR_FLAG_INTERRUPTED | MOVEMENTGENERATOR_FLAG_SPEED_UPDATE_PENDING);

        owner->addUnitStateFlag(UNIT_STATE_ROAMING_MOVE);

        MovementMgr::MoveSplineInit init(owner);
        init.MoveTo(_x, _y, _z, _generatePath);
        if (_speed > 0.0f) // Default value for point motion type is 0.0, if 0.0 spline will use GetSpeed on unit
            init.SetVelocity(_speed);
        init.Launch();

        // Call for creature group update
        if (Creature* creature = owner->ToCreature())
            creature->signalFormationMovement();
    }

    if (owner->movespline->Finalized())
    {
        MovementGenerator::removeFlag(MOVEMENTGENERATOR_FLAG_TRANSITORY);
        MovementGenerator::addFlag(MOVEMENTGENERATOR_FLAG_INFORM_ENABLED);
        return false;
    }
    return true;
}

template<class T>
void PointMovementGenerator<T>::doDeactivate(T* owner)
{
    MovementGenerator::addFlag(MOVEMENTGENERATOR_FLAG_DEACTIVATED);
    owner->removeUnitStateFlag(UNIT_STATE_ROAMING_MOVE);
}

template<class T>
void PointMovementGenerator<T>::doFinalize(T* owner, bool active, bool _movementInform)
{
    MovementGenerator::addFlag(MOVEMENTGENERATOR_FLAG_FINALIZED);
    if (active)
        owner->removeUnitStateFlag(UNIT_STATE_ROAMING_MOVE);

    if (_movementInform && MovementGenerator::hasFlag(MOVEMENTGENERATOR_FLAG_INFORM_ENABLED))
        movementInform(owner);
}

template<class T>
void PointMovementGenerator<T>::movementInform(T*) { }

template <>
void PointMovementGenerator<Creature>::movementInform(Creature* owner)
{
    if (owner->getAIInterface())
        owner->getAIInterface()->movementInform(POINT_MOTION_TYPE, _movementId);
}

template PointMovementGenerator<Player>::PointMovementGenerator(uint32_t, float, float, float, bool, float, Optional<float>);
template PointMovementGenerator<Creature>::PointMovementGenerator(uint32_t, float, float, float, bool, float, Optional<float>);
template MovementGeneratorType PointMovementGenerator<Player>::getMovementGeneratorType() const;
template MovementGeneratorType PointMovementGenerator<Creature>::getMovementGeneratorType() const;
template void PointMovementGenerator<Player>::doInitialize(Player*);
template void PointMovementGenerator<Creature>::doInitialize(Creature*);
template void PointMovementGenerator<Player>::doReset(Player*);
template void PointMovementGenerator<Creature>::doReset(Creature*);
template bool PointMovementGenerator<Player>::doUpdate(Player*, uint32_t);
template bool PointMovementGenerator<Creature>::doUpdate(Creature*, uint32_t);
template void PointMovementGenerator<Player>::doDeactivate(Player*);
template void PointMovementGenerator<Creature>::doDeactivate(Creature*);
template void PointMovementGenerator<Player>::doFinalize(Player*, bool, bool);
template void PointMovementGenerator<Creature>::doFinalize(Creature*, bool, bool);

//////////////////////////////////////////////////////////////////////////////////////////
// AssistanceMovementGenerator
void AssistanceMovementGenerator::finalize(Unit* owner, bool active, bool movementInform)
{
    addFlag(MOVEMENTGENERATOR_FLAG_FINALIZED);
    if (active)
        owner->removeUnitStateFlag(UNIT_STATE_ROAMING_MOVE);

    if (movementInform && hasFlag(MOVEMENTGENERATOR_FLAG_INFORM_ENABLED))
    {
        Creature* ownerCreature = owner->ToCreature();
        ownerCreature->getAIInterface()->setNoCallAssistance(false);
        ownerCreature->getAIInterface()->callAssistance();
        if (ownerCreature->isAlive())
            ownerCreature->getMovementManager()->moveSeekAssistanceDistract(1500); // move to config
    }
}

MovementGeneratorType AssistanceMovementGenerator::getMovementGeneratorType() const
{
    return ASSISTANCE_MOTION_TYPE;
}
