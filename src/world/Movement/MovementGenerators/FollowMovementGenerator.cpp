/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "FollowMovementGenerator.h"
#include "Objects/Units/Creatures/Creature.h"
#include "Objects/Units/Creatures/AIInterface.h"
#include "Movement/Spline/MoveSpline.h"
#include "Movement/Spline/MoveSplineInit.h"
#include "Movement/PathGenerator.h"
#include "Objects/Units/Unit.hpp"
#include "Utilities/Random.hpp"
#include "Utilities/TimeTracker.hpp"

static void doMovementInform(Unit* owner, Unit* target)
{
    if (owner->getObjectTypeId() != TYPEID_UNIT)
        return;

    if (AIInterface* AI = owner->getAIInterface())
        AI->movementInform(FOLLOW_MOTION_TYPE, target->getGuidLow());
}

FollowMovementGenerator::FollowMovementGenerator(Unit* target, float range, ChaseAngle angle) : AbstractFollower(target), _range(range), _angle(angle), _checkTimer(std::make_unique<Util::SmallTimeTracker>(CHECK_INTERVAL))
{
    Mode = MOTION_MODE_DEFAULT;
    Priority = MOTION_PRIORITY_NORMAL;
    Flags = MOVEMENTGENERATOR_FLAG_INITIALIZATION_PENDING;
    BaseUnitState = UNIT_STATE_FOLLOWING;
}

FollowMovementGenerator::~FollowMovementGenerator() = default;

static bool positionOkay(Unit* owner, Unit* target, float range, Optional<ChaseAngle> angle = {})
{
    if (owner->GetDistance2dSq(target) > Util::square(owner->getCombatReach() + target->getCombatReach() + range))
        return false;

    return !angle || angle->isAngleOkay(target->getRelativeAngle(owner));
}

void FollowMovementGenerator::initialize(Unit* owner)
{
    removeFlag(MOVEMENTGENERATOR_FLAG_INITIALIZATION_PENDING | MOVEMENTGENERATOR_FLAG_DEACTIVATED);
    addFlag(MOVEMENTGENERATOR_FLAG_INITIALIZED | MOVEMENTGENERATOR_FLAG_INFORM_ENABLED);

    owner->stopMoving();
    updatePetSpeed(owner);
    _path = nullptr;
    _lastTargetPosition.reset();
}

void FollowMovementGenerator::reset(Unit* owner)
{
    removeFlag(MOVEMENTGENERATOR_FLAG_DEACTIVATED);

    initialize(owner);
}

bool FollowMovementGenerator::update(Unit* owner, uint32_t diff)
{
    // owner might be dead or gone
    if (!owner || !owner->isAlive())
        return false;

    // our target might have gone away
    Unit* const target = getTarget();
    if (!target || !target->IsInWorld())
        return false;

    if (owner->hasUnitStateFlag(UNIT_STATE_NOT_MOVE) || owner->isCastingSpell())
    {
        _path = nullptr;
        owner->stopMoving();
        _lastTargetPosition.reset();
        return true;
    }

    _checkTimer->updateTimer(diff);
    if (_checkTimer->isTimePassed())
    {
        _checkTimer->resetInterval(CHECK_INTERVAL);
        if (hasFlag(MOVEMENTGENERATOR_FLAG_INFORM_ENABLED) && positionOkay(owner, target, _range, _angle))
        {
            removeFlag(MOVEMENTGENERATOR_FLAG_INFORM_ENABLED);
            _path = nullptr;
            owner->stopMoving();
            _lastTargetPosition.reset();
            doMovementInform(owner, target);
            return true;
        }
    }

    if (owner->hasUnitStateFlag(UNIT_STATE_FOLLOW_MOVE) && owner->movespline->Finalized())
    {
        removeFlag(MOVEMENTGENERATOR_FLAG_INFORM_ENABLED);
        _path = nullptr;
        owner->removeUnitStateFlag(UNIT_STATE_FOLLOW_MOVE);
        doMovementInform(owner, target);
    }

    if (!_lastTargetPosition || _lastTargetPosition->distanceSquare(target->GetPosition()) > 0.0f)
    {
        _lastTargetPosition = target->GetPosition();
        if (owner->hasUnitStateFlag(UNIT_STATE_FOLLOW_MOVE) || !positionOkay(owner, target, _range + FOLLOW_RANGE_TOLERANCE))
        {
            if (!_path)
                _path = std::make_unique<PathGenerator>(owner);

            float x, y, z;

            // select angle
            float tAngle;
            float const curAngle = target->getRelativeAngle(owner);
            if (_angle.isAngleOkay(curAngle))
                tAngle = curAngle;
            else
            {
                float const diffUpper = normalizeOrientation(curAngle - _angle.upperBound());
                float const diffLower = normalizeOrientation(_angle.lowerBound() - curAngle);
                if (diffUpper < diffLower)
                    tAngle = _angle.upperBound();
                else
                    tAngle = _angle.lowerBound();
            }

            target->getNearPoint(owner, x, y, z, _range, target->toAbsoluteAngle(tAngle));

            if (owner->isHovering())
                owner->updateAllowedPositionZ(x, y, z);

            // pets are allowed to "cheat" on pathfinding when following their master
            bool allowShortcut = false;
            if (Unit* oPet = owner)
            {
                auto* const petOwner = oPet->getUnitOwner();
                if (petOwner && target->getGuid() == petOwner->getGuid())
                    allowShortcut = true;
            }

            bool success = _path->calculatePath(x, y, z, allowShortcut);
            if (!success || (_path->getPathType() & PATHFIND_NOPATH))
            {
                owner->stopMoving();
                return true;
            }

            owner->addUnitStateFlag(UNIT_STATE_FOLLOW_MOVE);
            addFlag(MOVEMENTGENERATOR_FLAG_INFORM_ENABLED);

            MovementMgr::MoveSplineInit init(owner);
            init.MovebyPath(_path->getPath());
            init.SetWalk(target->isWalking());
            init.SetFacing(target->GetOrientation());
            init.Launch();
        }
    }
    return true;
}

void FollowMovementGenerator::deactivate(Unit* owner)
{
    addFlag(MOVEMENTGENERATOR_FLAG_DEACTIVATED);
    removeFlag(MOVEMENTGENERATOR_FLAG_TRANSITORY | MOVEMENTGENERATOR_FLAG_INFORM_ENABLED);
    owner->removeUnitStateFlag(UNIT_STATE_FOLLOW_MOVE);
}

void FollowMovementGenerator::finalize(Unit* owner, bool active, bool/* movementInform*/)
{
    addFlag(MOVEMENTGENERATOR_FLAG_FINALIZED);
    if (active)
    {
        owner->removeUnitStateFlag(UNIT_STATE_FOLLOW_MOVE);
        updatePetSpeed(owner);
    }
}

MovementGeneratorType FollowMovementGenerator::getMovementGeneratorType() const { return FOLLOW_MOTION_TYPE; }

void FollowMovementGenerator::unitSpeedChanged() { _lastTargetPosition.reset(); }

void FollowMovementGenerator::updatePetSpeed(Unit* owner)
{
    if (Unit* oPet = owner)
    {
        auto* const petOwner = owner->getUnitOwner();
        if (!getTarget() || (petOwner && getTarget()->getGuid() == petOwner->getGuid()))
        {
            oPet->updateSpeed();
        }
    }
}
