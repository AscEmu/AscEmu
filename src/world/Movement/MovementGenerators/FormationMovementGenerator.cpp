/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "FormationMovementGenerator.h"
#include "Objects/Units/Creatures/Creature.h"
#include "Objects/Units/Creatures/AIInterface.h"
#include "Objects/Units/Creatures/CreatureGroups.h"
#include "Management/G3DPosition.hpp"
#include "Movement/MovementDefines.h"
#include "Movement/Spline/MoveSpline.h"
#include "Movement/Spline/MoveSplineInit.h"
#include "Utilities/TimeTracker.hpp"

FormationMovementGenerator::FormationMovementGenerator(Unit* leader, float range, float angle, uint32_t point1, uint32_t point2) :
AbstractFollower(leader), _range(range), _angle(angle), _point1(point1), _point2(point2), _lastLeaderSplineID(0), _hasPredictedDestination(false)
{
    Mode = MOTION_MODE_DEFAULT;
    Priority = MOTION_PRIORITY_NORMAL;
    Flags = MOVEMENTGENERATOR_FLAG_INITIALIZATION_PENDING;
    BaseUnitState = UNIT_STATE_FOLLOW_FORMATION;
    _nextMoveTimer = std::make_unique<Util::SmallTimeTracker>(0);
}

MovementGeneratorType FormationMovementGenerator::getMovementGeneratorType() const
{
    return FORMATION_MOTION_TYPE;
}

void FormationMovementGenerator::doInitialize(Creature* owner)
{
    removeFlag(MOVEMENTGENERATOR_FLAG_INITIALIZATION_PENDING | MOVEMENTGENERATOR_FLAG_TRANSITORY | MOVEMENTGENERATOR_FLAG_DEACTIVATED);
    addFlag(MOVEMENTGENERATOR_FLAG_INITIALIZED);

    if (owner->hasUnitStateFlag(UNIT_STATE_NOT_MOVE) || owner->isCastingSpell())
    {
        addFlag(MOVEMENTGENERATOR_FLAG_INTERRUPTED);
        owner->stopMoving();
        return;
    }

    _nextMoveTimer->resetInterval(0);
}

void FormationMovementGenerator::doReset(Creature* owner)
{
    removeFlag(MOVEMENTGENERATOR_FLAG_TRANSITORY | MOVEMENTGENERATOR_FLAG_DEACTIVATED);

    doInitialize(owner);
}

bool FormationMovementGenerator::doUpdate(Creature* owner, uint32_t diff)
{
    Unit* target = getTarget();

    if (!owner || !target)
        return false;

    // Owner cannot move. Reset all fields and wait for next action
    if (owner->hasUnitStateFlag(UNIT_STATE_NOT_MOVE) || owner->isCastingSpell())
    {
        addFlag(MOVEMENTGENERATOR_FLAG_INTERRUPTED);
        owner->stopMoving();
        _nextMoveTimer->resetInterval(0);
        _hasPredictedDestination = false;
        return true;
    }

    // If target is not moving and destination has been predicted and if we are on the same spline, we stop as well
    if (target->movespline->Finalized() && target->movespline->GetId() == _lastLeaderSplineID && _hasPredictedDestination)
    {
        addFlag(MOVEMENTGENERATOR_FLAG_INTERRUPTED);
        owner->stopMoving();
        _nextMoveTimer->resetInterval(0);
        _hasPredictedDestination = false;
        return true;
    }

    if (!owner->movespline->Finalized())
        owner->SetSpawnLocation(owner->GetPosition());

    // Formation leader has launched a new spline, launch a new one for our member as well
    // This action does not reset the regular movement launch cycle interval
    if (!target->movespline->Finalized() && target->movespline->GetId() != _lastLeaderSplineID)
    {
        // Update formation angle
        if (_point1 && target->getObjectTypeId() == TYPEID_UNIT)
        {
            if (CreatureGroup* formation = target->ToCreature()->getFormation())
            {
                if (Creature* leader = formation->getLeader())
                {
                    const auto currentWaypoint = leader->getCurrentWaypointInfo().first;
                    if (currentWaypoint == _point1 || currentWaypoint == _point2)
                        _angle = float(M_PI) * 2 - _angle;
                }
            }
        }

        launchMovement(owner, target);
        _lastLeaderSplineID = target->movespline->GetId();
        return true;
    }

    _nextMoveTimer->updateTimer(diff);
    if (_nextMoveTimer->isTimePassed())
    {
        _nextMoveTimer->resetInterval(FORMATION_MOVEMENT_INTERVAL);

        // Our leader has a different position than on our last check, launch movement.
        if (_lastLeaderPosition != target->GetPosition())
        {
            launchMovement(owner, target);
            return true;
        }
    }

    // We have reached our destination before launching a new movement. Alling facing with leader
    if (owner->hasUnitStateFlag(UNIT_STATE_FOLLOW_FORMATION_MOVE) && owner->movespline->Finalized())
    {
        owner->removeUnitStateFlag(UNIT_STATE_FOLLOW_FORMATION_MOVE);
        owner->setFacingTo(target->GetOrientation());
        movementInform(owner);
    }

    return true;
}

void FormationMovementGenerator::launchMovement(Creature* owner, Unit* target)
{
    float relativeAngle = 0.f;

    // Determine our relative angle to our current spline destination point
    if (!target->movespline->Finalized())
        relativeAngle = target->getRelativeAngle(vector3ToPosition(target->movespline->CurrentDestination()));

    //////////////////////////////////////////////////////////////////////////////////////////
    // Destination calculation

    /// According to sniff data, formation members have a periodic move interal of 1,2s.
    /// Each of these splines has a exact duration of 1650ms +- 1ms when no pathfinding is involved.
    /// To get a representative result like that we have to predict our formation leader's path
    /// and apply our formation shape based on that destination.

    LocationVector dest = target->GetPosition();
    float velocity = 0.f;

    // Formation leader is moving. Predict our destination
    if (!target->movespline->Finalized())
    {
        // Pick up leader's spline velocity
        velocity = target->movespline->Velocity();

        // Calculate travel distance to get a 1650ms result
        float travelDist = velocity * 1.65f;

        // Move destination ahead...
        target->movePositionToFirstCollision(dest, travelDist, relativeAngle);
        // ... and apply formation shape
        target->movePositionToFirstCollision(dest, _range, _angle + relativeAngle);

        float distance = owner->GetPosition().getExactDist(dest);

        // Calculate catchup speed mod (Limit to a maximum of 50% of our original velocity
        float velocityMod = std::min<float>(distance / travelDist, 1.5f);

        // Now we will always stay synch with our leader
        velocity *= velocityMod;
        _hasPredictedDestination = true;
    }
    else
    {
        // Formation leader is not moving. Just apply the base formation shape on his position.
        target->movePositionToFirstCollision(dest, _range, _angle + relativeAngle);
        _hasPredictedDestination = false;
    }

    // Leader is not moving, so just pick up his default walk speed
    if (velocity == 0.f)
        velocity = target->getSpeedRate(TYPE_WALK, true);

    MovementMgr::MoveSplineInit init(owner);
    init.MoveTo(positionToVector3(dest));
    init.SetVelocity(velocity);
    init.Launch();

    _lastLeaderPosition = target->GetPosition();
    owner->addUnitStateFlag(UNIT_STATE_FOLLOW_FORMATION_MOVE);
    removeFlag(MOVEMENTGENERATOR_FLAG_INTERRUPTED);
}

void FormationMovementGenerator::doDeactivate(Creature* owner)
{
    addFlag(MOVEMENTGENERATOR_FLAG_DEACTIVATED);
    owner->removeUnitStateFlag(UNIT_STATE_FOLLOW_FORMATION_MOVE);
}

void FormationMovementGenerator::doFinalize(Creature* owner, bool active, bool _movementInform)
{
    addFlag(MOVEMENTGENERATOR_FLAG_FINALIZED);
    if (active)
        owner->removeUnitStateFlag(UNIT_STATE_FOLLOW_FORMATION_MOVE);

    if (_movementInform && hasFlag(MOVEMENTGENERATOR_FLAG_INFORM_ENABLED))
        movementInform(owner);
}

void FormationMovementGenerator::movementInform(Creature* owner)
{
    if (owner->getAIInterface())
        owner->getAIInterface()->movementInform(FORMATION_MOTION_TYPE, 0);
}
