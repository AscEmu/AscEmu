/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "ChaseMovementGenerator.h"
#include "Objects/Units/Creatures/Creature.h"
#include "Objects/Units/Creatures/AIInterface.h"
#include "Management/G3DPosition.hpp"
#include "Movement/MovementManager.h"
#include "Movement/Spline/MoveSpline.h"
#include "Movement/Spline/MoveSplineInit.h"
#include "Movement/PathGenerator.h"
#include "Objects/Units/Unit.hpp"
#include "Server/World.h"
#include "Utilities/Random.hpp"
#include "Utilities/TimeTracker.hpp"
#include "Utilities/Util.hpp"

static bool hasLostTarget(Unit* owner, Unit* target)
{
    return owner->getAIInterface()->getCurrentTarget() != target;
}

static bool isMutualChase(Unit* owner, Unit* target)
{
    if (target->getMovementManager()->getCurrentMovementGeneratorType() != CHASE_MOTION_TYPE)
        return false;

    if (ChaseMovementGenerator* movement = dynamic_cast<ChaseMovementGenerator*>(target->getMovementManager()->getCurrentMovementGenerator()))
        return movement->getTarget() == owner;

    return false;
}

static bool positionOkay(Unit* owner, Unit* target, Optional<float> minDistance, Optional<float> maxDistance, Optional<ChaseAngle> angle)
{
    float const distSq = owner->GetDistance2dSq(target);
    if (minDistance && distSq < Util::square(*minDistance))
        return false;
    if (maxDistance && distSq > Util::square(*maxDistance))
        return false;
    if (angle && !angle->isAngleOkay(target->getRelativeAngle(owner)))
        return false;
    // Objects outside of Line of Sight cannot be detected
    if (worldConfig.terrainCollision.isCollisionEnabled)
    {
        if (!owner->IsWithinLOSInMap(target))
            return false;
    }
    return true;
}

static void doMovementInform(Unit* owner, Unit* target)
{
    if (owner->getObjectTypeId() != TYPEID_UNIT)
        return;

    if (AIInterface* AI = owner->getAIInterface())
        AI->movementInform(CHASE_MOTION_TYPE, target->getGuidLow());
}

ChaseMovementGenerator::ChaseMovementGenerator(Unit *target, Optional<ChaseRange> range, Optional<ChaseAngle> angle) : AbstractFollower(target), _range(range),
    _angle(angle), _rangeCheckTimer(std::make_unique<Util::SmallTimeTracker>(RANGE_CHECK_INTERVAL))
{
    Mode = MOTION_MODE_DEFAULT;
    Priority = MOTION_PRIORITY_NORMAL;
    Flags = MOVEMENTGENERATOR_FLAG_INITIALIZATION_PENDING;
    BaseUnitState = UNIT_STATE_CHASING;
}

ChaseMovementGenerator::~ChaseMovementGenerator() = default;

void ChaseMovementGenerator::initialize(Unit* /*owner*/)
{
    removeFlag(MOVEMENTGENERATOR_FLAG_INITIALIZATION_PENDING | MOVEMENTGENERATOR_FLAG_DEACTIVATED);
    addFlag(MOVEMENTGENERATOR_FLAG_INITIALIZED | MOVEMENTGENERATOR_FLAG_INFORM_ENABLED);

    _path = nullptr;
    _lastTargetPosition.reset();
}

void ChaseMovementGenerator::reset(Unit* owner)
{
    removeFlag(MOVEMENTGENERATOR_FLAG_DEACTIVATED);

    initialize(owner);
}

bool ChaseMovementGenerator::update(Unit* owner, uint32_t diff)
{
    // owner might be dead or gone (can we even get nullptr here?)
    if (!owner || !owner->isAlive())
        return false;

    // our target might have gone away
    Unit* const target = getTarget();
    if (!target || !target->IsInWorld())
        return false;

    // the owner might be unable to move (rooted or casting), or we have lost the target, pause movement
    if (owner->hasUnitStateFlag(UNIT_STATE_NOT_MOVE) || owner->isCastingSpell() || hasLostTarget(owner, target))
    {
        owner->stopMoving();
        _lastTargetPosition.reset();
        if (Creature* cOwner = owner->ToCreature())
            cOwner->getAIInterface()->setCannotReachTarget(false);
        return true;
    }

    bool const mutualChase = isMutualChase(owner, target);
    float const hitboxSum = owner->getCombatReach() + target->getCombatReach();
    float const minRange = _range ? _range->MinRange + hitboxSum : CONTACT_DISTANCE;
    float const minTarget = (_range ? _range->MinTolerance : 0.0f) + hitboxSum;
    float const maxRange = _range ? _range->MaxRange + hitboxSum : owner->getMeleeRange(target); // melee range already includes hitboxes
    float const maxTarget = _range ? _range->MaxTolerance + hitboxSum : CONTACT_DISTANCE + hitboxSum;
    Optional<ChaseAngle> angle = mutualChase ? Optional<ChaseAngle>() : _angle;

    // periodically check if we're already in the expected range...
    _rangeCheckTimer->updateTimer(diff);
    if (_rangeCheckTimer->isTimePassed())
    {
        _rangeCheckTimer->resetInterval(RANGE_CHECK_INTERVAL);
        if (hasFlag(MOVEMENTGENERATOR_FLAG_INFORM_ENABLED) && positionOkay(owner, target, _movingTowards ? Optional<float>() : minTarget, _movingTowards ? maxTarget : Optional<float>(), angle))
        {
            removeFlag(MOVEMENTGENERATOR_FLAG_INFORM_ENABLED);
            _path = nullptr;
            if (Creature* cOwner = owner->ToCreature())
                cOwner->getAIInterface()->setCannotReachTarget(false);
            owner->stopMoving();
            owner->setInFront(target);
            doMovementInform(owner, target);
            return true;
        }
    }

    // if we're done moving, we want to clean up
    if (owner->hasUnitStateFlag(UNIT_STATE_CHASE_MOVE) && owner->movespline->Finalized())
    {
        removeFlag(MOVEMENTGENERATOR_FLAG_INFORM_ENABLED);
        _path = nullptr;
        if (Creature* cOwner = owner->ToCreature())
            cOwner->getAIInterface()->setCannotReachTarget(false);
        owner->removeUnitStateFlag(UNIT_STATE_CHASE_MOVE);
        owner->setInFront(target);
        doMovementInform(owner, target);
    }

    // if the target moved, we have to consider whether to adjust
    if (!_lastTargetPosition || target->GetPosition() != _lastTargetPosition.value() || mutualChase != _mutualChase)
    {
        _lastTargetPosition = target->GetPosition();
        _mutualChase = mutualChase;
        if (owner->hasUnitStateFlag(UNIT_STATE_CHASE_MOVE) || !positionOkay(owner, target, minRange, maxRange, angle))
        {
            Creature* const cOwner = owner->ToCreature();
            // can we get to the target?
            if (cOwner && !target->isInAccessiblePlaceFor(cOwner))
            {
                cOwner->getAIInterface()->setCannotReachTarget(true);
                cOwner->stopMoving();
                _path = nullptr;
                return true;
            }

            // figure out which way we want to move
            bool const moveToward = !owner->isInDist(target, maxRange);

            // make a new path if we have to...
            if (!_path || moveToward != _movingTowards)
                _path = std::make_unique<PathGenerator>(owner);

            float x, y, z;
            bool shortenPath;
            // if we want to move toward the target and there's no fixed angle...
            if (moveToward && !angle)
            {
                // ...we'll pathfind to the center, then shorten the path
                target->getPosition(x, y, z);
                shortenPath = true;
            }
            else
            {
                // otherwise, we fall back to nearpoint finding
                target->getNearPoint(owner, x, y, z, (moveToward ? maxTarget : minTarget) - hitboxSum, angle ? target->toAbsoluteAngle(angle->RelativeAngle) : target->getAbsoluteAngle(owner));
                shortenPath = false;
            }

            if (owner->isHovering())
                owner->updateAllowedPositionZ(x, y, z);

            bool forcedest = owner->canFly() || owner->isInWater();

            bool success = _path->calculatePath(x, y, z, forcedest);
            if (!success || (_path->getPathType() & (PATHFIND_NOPATH /* | PATHFIND_INCOMPLETE*/)))
            {
                if (cOwner)
                    cOwner->getAIInterface()->setCannotReachTarget(true);
                owner->stopMoving();
                return true;
            }

            if (shortenPath)
                _path->shortenPathUntilDist(positionToVector3(target->GetPosition()), maxTarget);

            if (cOwner)
                cOwner->getAIInterface()->setCannotReachTarget(false);

            bool walk = false;
            if (cOwner && !cOwner->isPet())
            {
                switch (cOwner->getMovementTemplate().getChase())
                {
                    case CreatureChaseMovementType::CanWalk:
                        walk = owner->isWalking();
                        break;
                    case CreatureChaseMovementType::AlwaysWalk:
                        walk = true;
                        break;
                    default:
                        break;
                }
            }

            owner->addUnitStateFlag(UNIT_STATE_CHASE_MOVE);
            addFlag(MOVEMENTGENERATOR_FLAG_INFORM_ENABLED);

            MovementMgr::MoveSplineInit init(owner);
            init.MovebyPath(_path->getPath());
            init.SetWalk(walk);
            init.SetFacing(target);
            init.Launch();
        }
    }

    // and then, finally, we're done for the tick
    return true;
}

void ChaseMovementGenerator::deactivate(Unit* owner)
{
    addFlag(MOVEMENTGENERATOR_FLAG_DEACTIVATED);
    removeFlag(MOVEMENTGENERATOR_FLAG_TRANSITORY | MOVEMENTGENERATOR_FLAG_INFORM_ENABLED);
    owner->removeUnitStateFlag(UNIT_STATE_CHASE_MOVE);
    if (Creature* cOwner = owner->ToCreature())
        cOwner->getAIInterface()->setCannotReachTarget(false);
}

void ChaseMovementGenerator::finalize(Unit* owner, bool active, bool/* movementInform*/)
{
    addFlag(MOVEMENTGENERATOR_FLAG_FINALIZED);
    if (active)
    {
        owner->removeUnitStateFlag(UNIT_STATE_CHASE_MOVE);
        if (Creature* cOwner = owner->ToCreature())
            cOwner->getAIInterface()->setCannotReachTarget(false);
    }
}

MovementGeneratorType ChaseMovementGenerator::getMovementGeneratorType() const { return CHASE_MOTION_TYPE; }

void ChaseMovementGenerator::unitSpeedChanged() { _lastTargetPosition.reset(); }
