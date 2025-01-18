/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "IdleMovementGenerator.h"
#include "Objects/Units/Creatures/Creature.h"
#include "Objects/Units/Creatures/AIInterface.h"
#include "Management/G3DPosition.hpp"
#include "Movement/MovementDefines.h"
#include "Movement/Spline/MoveSplineInit.h"
#include "Objects/Units/Unit.hpp"

IdleMovementGenerator::IdleMovementGenerator()
{
    Mode = MOTION_MODE_DEFAULT;
    Priority = MOTION_PRIORITY_NORMAL;
    Flags = MOVEMENTGENERATOR_FLAG_INITIALIZED;
    BaseUnitState = 0;
}

///  TODO: "if (!owner->IsStopped())" is useless, each generator cleans their own STATE_MOVE, the result is that StopMoving is almost never called
///  Old comment: "StopMoving is needed to make unit stop if its last movement generator expires but it should not be sent otherwise there are many redundent packets"

void IdleMovementGenerator::initialize(Unit* owner)
{
    owner->stopMoving();
}

void IdleMovementGenerator::reset(Unit* owner)
{
    owner->stopMoving();
}

void IdleMovementGenerator::deactivate(Unit* /*owner*/)
{
}

void IdleMovementGenerator::finalize(Unit* /*owner*/, bool/* active*/, bool/* movementInform*/)
{
    addFlag(MOVEMENTGENERATOR_FLAG_FINALIZED);
}

MovementGeneratorType IdleMovementGenerator::getMovementGeneratorType() const
{
    return IDLE_MOTION_TYPE;
}

//////////////////////////////////////////////////////////////////////////////////////////
// RotateMovementGenerator
RotateMovementGenerator::RotateMovementGenerator(uint32_t id, uint32_t time, RotateDirection direction) : _id(id), _duration(time), _maxDuration(time), _direction(direction)
{
    Mode = MOTION_MODE_DEFAULT;
    Priority = MOTION_PRIORITY_NORMAL;
    Flags = MOVEMENTGENERATOR_FLAG_INITIALIZATION_PENDING;
    BaseUnitState = UNIT_STATE_ROTATING;
}

void RotateMovementGenerator::initialize(Unit* owner)
{
    removeFlag(MOVEMENTGENERATOR_FLAG_INITIALIZATION_PENDING | MOVEMENTGENERATOR_FLAG_DEACTIVATED);
    addFlag(MOVEMENTGENERATOR_FLAG_INITIALIZED);

    owner->stopMoving();
}

void RotateMovementGenerator::reset(Unit* owner)
{
    removeFlag(MOVEMENTGENERATOR_FLAG_DEACTIVATED);

    initialize(owner);
}

#if VERSION_STRING <= WotLK
bool RotateMovementGenerator::update(Unit* owner, uint32_t diff)
{
    if (!owner)
        return false;

    float angle = owner->GetOrientation();
    if (_direction == ROTATE_DIRECTION_LEFT)
    {
        angle += float(diff) * float(M_PI) * 2.f / float(_maxDuration);
        while (angle >= float(M_PI) * 2.f)
            angle -= float(M_PI) * 2.f;
    }
    else
    {
        angle -= float(diff) * float(M_PI) * 2.f / float(_maxDuration);
        while (angle < 0.f)
            angle += float(M_PI) * 2.f;
    }

    MovementMgr::MoveSplineInit init(owner);
    init.MoveTo(positionToVector3(owner->GetPosition()), false);
    if (owner->hasUnitMovementFlag(MOVEFLAG_TRANSPORT) && owner->getTransGuid())
        init.DisableTransportPathTransformations();
    init.SetFacing(angle);
    init.Launch();

    if (_duration > diff)
        _duration -= diff;
    else
    {
        addFlag(MOVEMENTGENERATOR_FLAG_INFORM_ENABLED);
        return false;
    }

    return true;
}
#else
bool RotateMovementGenerator::update(Unit* owner, uint32_t diff)
{
    float angle = owner->GetOrientation();
    angle += (float(diff) * static_cast<float>(M_PI * 2) / _maxDuration) * (_direction == ROTATE_DIRECTION_LEFT ? 1.0f : -1.0f);
    angle = G3D::wrap(angle, 0.0f, float(G3D::twoPi()));

    owner->SetOrientation(angle);   // UpdateSplinePosition does not set orientation with UNIT_STATE_ROTATING
    owner->setFacingTo(angle);      // Send spline movement to clients

    if (_duration > diff)
        _duration -= diff;
    else
        return false;

    return true;
}
#endif

void RotateMovementGenerator::deactivate(Unit*)
{
    addFlag(MOVEMENTGENERATOR_FLAG_DEACTIVATED);
}

void RotateMovementGenerator::finalize(Unit* owner, bool/* active*/, bool movementInform)
{
    addFlag(MOVEMENTGENERATOR_FLAG_FINALIZED);

    if (movementInform && owner->getObjectTypeId() == TYPEID_UNIT)
        owner->ToCreature()->getAIInterface()->movementInform(ROTATE_MOTION_TYPE, _id);
}

MovementGeneratorType RotateMovementGenerator::getMovementGeneratorType() const
{
    return ROTATE_MOTION_TYPE;
}

//////////////////////////////////////////////////////////////////////////////////////////
// DistractMovementGenerator
DistractMovementGenerator::DistractMovementGenerator(uint32_t timer, float orientation) : _timer(timer), _orientation(orientation)
{
    Mode = MOTION_MODE_DEFAULT;
    Priority = MOTION_PRIORITY_HIGHEST;
    Flags = MOVEMENTGENERATOR_FLAG_INITIALIZATION_PENDING;
    BaseUnitState = UNIT_STATE_DISTRACTED;
}

#if VERSION_STRING <= WotLK
void DistractMovementGenerator::initialize(Unit* owner)
{
    removeFlag(MOVEMENTGENERATOR_FLAG_INITIALIZATION_PENDING | MOVEMENTGENERATOR_FLAG_DEACTIVATED);
    addFlag(MOVEMENTGENERATOR_FLAG_INITIALIZED);

    // Distracted creatures stand up if not standing
    if (!owner->getStandState())
        owner->setStandState(STANDSTATE_STAND);

    MovementMgr::MoveSplineInit init(owner);
    init.MoveTo(positionToVector3(owner->GetPosition()), false);
    if (owner->hasUnitMovementFlag(MOVEFLAG_TRANSPORT) && owner->getTransGuid())
        init.DisableTransportPathTransformations();
    init.SetFacing(_orientation);
    init.Launch();
}
#else
void DistractMovementGenerator::initialize(Unit* owner)
{
    // Distracted creatures stand up if not standing
    if (!owner->getStandState())
        owner->setStandState(STANDSTATE_STAND);

    owner->addUnitStateFlag(UNIT_STATE_DISTRACTED);
}
#endif

void DistractMovementGenerator::reset(Unit* owner)
{
    removeFlag(MOVEMENTGENERATOR_FLAG_DEACTIVATED);

    initialize(owner);
}

bool DistractMovementGenerator::update(Unit* owner, uint32_t diff)
{
    if (!owner)
        return false;

    if (diff > _timer)
    {
        addFlag(MOVEMENTGENERATOR_FLAG_INFORM_ENABLED);
        return false;
    }

    _timer -= diff;
    return true;
}

void DistractMovementGenerator::deactivate(Unit*)
{
    addFlag(MOVEMENTGENERATOR_FLAG_DEACTIVATED);
}

void DistractMovementGenerator::finalize(Unit* owner, bool/* active*/, bool movementInform)
{
    addFlag(MOVEMENTGENERATOR_FLAG_FINALIZED);

    // TODO: This code should be handled somewhere else
    // If this is a creature, then return orientation to original position (for idle movement creatures)
    if (movementInform && hasFlag(MOVEMENTGENERATOR_FLAG_INFORM_ENABLED) && owner->getObjectTypeId() == TYPEID_UNIT)
    {
        float angle = owner->ToCreature()->GetSpawnPosition().getOrientation();
        owner->setFacingTo(angle);
    }
}

MovementGeneratorType DistractMovementGenerator::getMovementGeneratorType() const
{
    return DISTRACT_MOTION_TYPE;
}

//////////////////////////////////////////////////////////////////////////////////////////
// AssistanceDistractMovementGenerator
AssistanceDistractMovementGenerator::AssistanceDistractMovementGenerator(uint32_t timer, float orientation) : DistractMovementGenerator(timer, orientation)
{
    Priority = MOTION_PRIORITY_NORMAL;
}

void AssistanceDistractMovementGenerator::finalize(Unit* owner, bool/* active*/, bool movementInform)
{
    addFlag(MOVEMENTGENERATOR_FLAG_FINALIZED);

    if (movementInform && hasFlag(MOVEMENTGENERATOR_FLAG_INFORM_ENABLED) && owner->getObjectTypeId() == TYPEID_UNIT)
        owner->ToCreature()->getAIInterface()->setReactState(REACT_AGGRESSIVE);
}

MovementGeneratorType AssistanceDistractMovementGenerator::getMovementGeneratorType() const
{
    return ASSISTANCE_DISTRACT_MOTION_TYPE;
}
