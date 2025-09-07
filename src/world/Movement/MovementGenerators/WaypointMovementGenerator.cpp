/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "WaypointMovementGenerator.h"
#include "Objects/Units/Creatures/Creature.h"
#include "Debugging/Errors.h"
#include "Logging/Logger.hpp"
#include "Map/Management/MapMgr.hpp"
#include "Movement/MovementDefines.h"
#include "Movement/Spline/MoveSpline.h"
#include "Movement/Spline/MoveSplineInit.h"
#include "Management/ObjectMgr.hpp"
#include "Objects/Transporter.hpp"
#include "Movement/WaypointManager.h"
#include "Objects/Units/Creatures/AIInterface.h"
#include "Utilities/Random.hpp"

WaypointMovementGenerator<Creature>::WaypointMovementGenerator(uint32_t pathId, bool repeating) : _nextMoveTime(std::make_unique<Util::SmallTimeTracker>(0)), _pathId(pathId), _repeating(repeating), _loadedFromDB(true)
{
    Mode = MOTION_MODE_DEFAULT;
    Priority = MOTION_PRIORITY_NORMAL;
    Flags = MOVEMENTGENERATOR_FLAG_INITIALIZATION_PENDING;
    BaseUnitState = UNIT_STATE_ROAMING;
}

WaypointMovementGenerator<Creature>::WaypointMovementGenerator(WaypointPath& path, bool repeating) : _nextMoveTime(std::make_unique<Util::SmallTimeTracker>(0)), _pathId(0), _repeating(repeating), _loadedFromDB(false)
{
    _path = &path;

    Mode = MOTION_MODE_DEFAULT;
    Priority = MOTION_PRIORITY_NORMAL;
    Flags = MOVEMENTGENERATOR_FLAG_INITIALIZATION_PENDING;
    BaseUnitState = UNIT_STATE_ROAMING;
}

MovementGeneratorType WaypointMovementGenerator<Creature>::getMovementGeneratorType() const
{
    return WAYPOINT_MOTION_TYPE;
}

void WaypointMovementGenerator<Creature>::pause(uint32_t timer/* = 0*/)
{
    if (timer)
    {
        // Don't try to paused an already paused generator
        if (hasFlag(MOVEMENTGENERATOR_FLAG_PAUSED))
            return;

        addFlag(MOVEMENTGENERATOR_FLAG_TIMED_PAUSED);
        _nextMoveTime->resetInterval(timer);
        removeFlag(MOVEMENTGENERATOR_FLAG_PAUSED);
    }
    else
    {
        addFlag(MOVEMENTGENERATOR_FLAG_PAUSED);
        _nextMoveTime->resetInterval(1); // Needed so that Update does not behave as if node was reached
        removeFlag(MOVEMENTGENERATOR_FLAG_TIMED_PAUSED);
    }
}

void WaypointMovementGenerator<Creature>::resume(uint32_t overrideTimer/* = 0*/)
{
    if (overrideTimer)
        _nextMoveTime->resetInterval(overrideTimer);

    if (_nextMoveTime->isTimePassed())
        _nextMoveTime->resetInterval(1); // Needed so that Update does not behave as if node was reached

    removeFlag(MOVEMENTGENERATOR_FLAG_PAUSED);
}

bool WaypointMovementGenerator<Creature>::getResetPosition(Unit* /*owner*/, float& x, float& y, float& z)
{
    // prevent a crash at empty waypoint path.
    if (!_path || _path->nodes.empty())
        return false;

    ASSERT(_currentNode < _path->nodes.size() && "WaypointMovementGenerator::GetResetPosition: tried to reference a node id which is not included in path");
    WaypointNode const &waypoint = _path->nodes.at(_currentNode);

    x = waypoint.x;
    y = waypoint.y;
    z = waypoint.z;
    return true;
}

void WaypointMovementGenerator<Creature>::doInitialize(Creature* owner)
{
    removeFlag(MOVEMENTGENERATOR_FLAG_INITIALIZATION_PENDING | MOVEMENTGENERATOR_FLAG_TRANSITORY | MOVEMENTGENERATOR_FLAG_DEACTIVATED);

    if (_loadedFromDB)
    {
        if (!_pathId)
            _pathId = owner->getWaypointPath();

        _path = sWaypointMgr->getPath(_pathId);
    }

    if (!_path)
    {
        sLogger.failure("WaypointMovementGenerator::DoInitialize: couldn't load path for creature ({}) (_pathId: {})", owner->getGuid(), _pathId);
        return;
    }

    owner->stopMoving();

    _nextMoveTime->resetInterval(1000);
}

void WaypointMovementGenerator<Creature>::doReset(Creature* owner)
{
    removeFlag(MOVEMENTGENERATOR_FLAG_TRANSITORY | MOVEMENTGENERATOR_FLAG_DEACTIVATED);

    owner->stopMoving();

    if (!hasFlag(MOVEMENTGENERATOR_FLAG_FINALIZED) && _nextMoveTime->isTimePassed())
        _nextMoveTime->resetInterval(1); // Needed so that Update does not behave as if node was reached
}

bool WaypointMovementGenerator<Creature>::doUpdate(Creature* owner, uint32_t diff)
{
    if (!owner || !owner->isAlive())
        return true;

    if (hasFlag(MOVEMENTGENERATOR_FLAG_FINALIZED | MOVEMENTGENERATOR_FLAG_PAUSED) || !_path || _path->nodes.empty())
        return true;

    if (owner->hasUnitStateFlag(UNIT_STATE_NOT_MOVE | UNIT_STATE_LOST_CONTROL) || owner->isCastingSpell())
    {
        addFlag(MOVEMENTGENERATOR_FLAG_INTERRUPTED);
        owner->stopMoving();
        return true;
    }

    if (hasFlag(MOVEMENTGENERATOR_FLAG_INTERRUPTED))
    {
        if (hasFlag(MOVEMENTGENERATOR_FLAG_INITIALIZED) && (_nextMoveTime->isTimePassed() || !hasFlag(MOVEMENTGENERATOR_FLAG_INFORM_ENABLED)))
        {
            startMove(owner, true);
            return true;
        }

        removeFlag(MOVEMENTGENERATOR_FLAG_INTERRUPTED);
    }

    // if it's moving
    if (!owner->movespline->Finalized())
    {
        // set home position at place (every MotionMaster::UpdateMotion)
#if VERSION_STRING <= WotLK
        if (!owner->hasUnitMovementFlag(MOVEFLAG_TRANSPORT) || owner->getTransGuid())
            owner->SetSpawnLocation(owner->GetPosition());
#else
        if (owner->getTransGuid())
            owner->SetSpawnLocation(owner->GetPosition());
#endif

        // relaunch movement if its speed has changed
        if (hasFlag(MOVEMENTGENERATOR_FLAG_SPEED_UPDATE_PENDING))
            startMove(owner, true);
    }
    else if (!_nextMoveTime->isTimePassed()) // it's not moving, is there a timer?
    {
        if (updateTimer(diff))
        {
            if (!hasFlag(MOVEMENTGENERATOR_FLAG_INITIALIZED)) // initial movement call
            {
                startMove(owner);
                return true;
            }
            else if (!hasFlag(MOVEMENTGENERATOR_FLAG_INFORM_ENABLED)) // timer set before node was reached, resume now
            {
                startMove(owner, true);
                return true;
            }
        }
        else
            return true; // keep waiting
    }
    else // not moving, no timer
    {
        if (hasFlag(MOVEMENTGENERATOR_FLAG_INITIALIZED) && !hasFlag(MOVEMENTGENERATOR_FLAG_INFORM_ENABLED))
        {
            onArrived(owner); // hooks and wait timer reset (if necessary)
            addFlag(MOVEMENTGENERATOR_FLAG_INFORM_ENABLED); // signals to future startMove that it reached a node
        }

        if (_nextMoveTime->isTimePassed()) // OnArrived might have set a timer
            startMove(owner); // check path status, get next point and move if necessary & can
    }

    return true;
}

void WaypointMovementGenerator<Creature>::doDeactivate(Creature* owner)
{
    addFlag(MOVEMENTGENERATOR_FLAG_DEACTIVATED);
    owner->removeUnitStateFlag(UNIT_STATE_ROAMING_MOVE);
}

void WaypointMovementGenerator<Creature>::doFinalize(Creature* owner, bool active, bool/* movementInform*/)
{
    addFlag(MOVEMENTGENERATOR_FLAG_FINALIZED);
    if (active)
    {
        owner->removeUnitStateFlag(UNIT_STATE_ROAMING_MOVE);

        // TODO: Research if this modification is needed, which most likely isnt
        owner->setMoveWalk(false);
    }
}

void WaypointMovementGenerator<Creature>::movementInform(Creature* owner)
{
    if (owner->getAIInterface())
        owner->getAIInterface()->movementInform(WAYPOINT_MOTION_TYPE, _currentNode);
}

void WaypointMovementGenerator<Creature>::onArrived(Creature* owner)
{
    if (!_path || _path->nodes.empty())
        return;

    ASSERT(_currentNode < _path->nodes.size() && "WaypointMovementGenerator::OnArrived: tried to reference a node id which is not included in path");
    WaypointNode const &waypoint = _path->nodes.at(_currentNode);
    if (waypoint.delay)
    {
        owner->removeUnitStateFlag(UNIT_STATE_ROAMING_MOVE);
        _nextMoveTime->resetInterval(waypoint.delay);
    }

    if (waypoint.eventId && Util::getRandomUInt(0, 99) < waypoint.eventChance)
    {
        sLogger.debug("Creature movement start script {} at point {} for {}.", waypoint.eventId, _currentNode, owner->getGuid());
        owner->removeUnitStateFlag(UNIT_STATE_ROAMING_MOVE);
        // add waypoint scripts
    }

    // inform AI
    if (AIInterface* AI = owner->getAIInterface())
    {
        AI->movementInform(WAYPOINT_MOTION_TYPE, _currentNode);
        AI->waypointReached(waypoint.id, _path->id);
    }

    owner->updateCurrentWaypointInfo(waypoint.id, _path->id);
}

void WaypointMovementGenerator<Creature>::startMove(Creature* owner, bool relaunch/* = false*/)
{
    // sanity checks
    if (!owner || !owner->isAlive() || hasFlag(MOVEMENTGENERATOR_FLAG_FINALIZED) || !_path || _path->nodes.empty() || (relaunch && (hasFlag(MOVEMENTGENERATOR_FLAG_INFORM_ENABLED) || !hasFlag(MOVEMENTGENERATOR_FLAG_INITIALIZED))))
        return;

    if (owner->hasUnitStateFlag(UNIT_STATE_NOT_MOVE) || owner->isCastingSpell() || (owner->isFormationLeader() && !owner->isFormationLeaderMoveAllowed())) // if cannot move OR cannot move because of formation
    {
        _nextMoveTime->resetInterval(1000); // delay 1s
        return;
    }

#if VERSION_STRING <= WotLK
    bool const transportPath = owner->hasUnitMovementFlag(MOVEFLAG_TRANSPORT) && owner->getTransGuid();
#else
    bool const transportPath = owner->GetTransport() != nullptr;
#endif

    if (hasFlag(MOVEMENTGENERATOR_FLAG_INFORM_ENABLED) && hasFlag(MOVEMENTGENERATOR_FLAG_INITIALIZED))
    {
        if (computeNextNode())
        {
            ASSERT(_currentNode < _path->nodes.size() && "WaypointMovementGenerator::startMove: tried to reference a node id which is not included in path");

            // inform AI
            if (AIInterface* AI = owner->getAIInterface())
                AI->waypointStarted(_path->nodes[_currentNode].id, _path->id);
        }
        else
        {
            WaypointNode const &waypoint = _path->nodes[_currentNode];
            float x = waypoint.x;
            float y = waypoint.y;
            float z = waypoint.z;
            float o = owner->GetOrientation();

            if (!transportPath)
                owner->SetSpawnLocation(x, y, z, o);
            else
            {
                if (Transporter* trans = owner->GetTransport())
                {
                    o -= trans->GetOrientation();
                    owner->SetTransportHomePosition(x, y, z, o);
                    trans->calculatePassengerPosition(x, y, z, &o);
                    owner->SetSpawnLocation(x, y, z, o);
                }
                // else if (vehicle) - this should never happen, vehicle offsets are const
            }
            addFlag(MOVEMENTGENERATOR_FLAG_FINALIZED);
            owner->updateCurrentWaypointInfo(0, 0);

            // inform AI
            if (AIInterface* AI = owner->getAIInterface())
                AI->waypointPathEnded(waypoint.id, _path->id);
            return;
        }
    }
    else if (!hasFlag(MOVEMENTGENERATOR_FLAG_INITIALIZED))
    {
        addFlag(MOVEMENTGENERATOR_FLAG_INITIALIZED);

        // inform AI
        if (AIInterface* AI = owner->getAIInterface())
            AI->waypointStarted(_path->nodes[_currentNode].id, _path->id);
    }

    ASSERT(_currentNode < _path->nodes.size() && "WaypointMovementGenerator::startMove: tried to reference a node id which is not included in path");
    WaypointNode const &waypoint = _path->nodes[_currentNode];

    removeFlag(MOVEMENTGENERATOR_FLAG_TRANSITORY | MOVEMENTGENERATOR_FLAG_INFORM_ENABLED | MOVEMENTGENERATOR_FLAG_TIMED_PAUSED);

    owner->addUnitStateFlag(UNIT_STATE_ROAMING_MOVE);

    MovementMgr::MoveSplineInit init(owner);

    //! If creature is on transport, we assume waypoints set in DB are already transport offsets
    if (transportPath)
        init.DisableTransportPathTransformations();

    //! Do not use formationDest here, MoveTo requires transport offsets due to DisableTransportPathTransformations() call
    //! but formationDest contains global coordinates
    init.MoveTo(waypoint.x, waypoint.y, waypoint.z);

    //! Accepts angles such as 0.00001 and -0.00001, 0 must be ignored, default value in waypoint table
    if (waypoint.orientation && waypoint.delay)
        init.SetFacing(waypoint.orientation);

    switch (waypoint.moveType)
    {
#if VERSION_STRING >= WotLK
        case WAYPOINT_MOVE_TYPE_LAND:
            init.SetAnimation(ANIMATION_FLAG_GROUND);
            break;
        case WAYPOINT_MOVE_TYPE_TAKEOFF:
            init.SetAnimation(ANIMATION_FLAG_HOVER);
            break;
#endif
        case WAYPOINT_MOVE_TYPE_RUN:
            init.SetWalk(false);
            break;
        case WAYPOINT_MOVE_TYPE_WALK:
            init.SetWalk(true);
            break;
        default:
            break;
    }

    init.Launch();

    // inform formation
    owner->signalFormationMovement();
}

bool WaypointMovementGenerator<Creature>::computeNextNode()
{
    if ((_currentNode == _path->nodes.size() - 1) && !_repeating)
        return false;

    _currentNode = (_currentNode + 1) % _path->nodes.size();
    return true;
}

bool WaypointMovementGenerator<Creature>::updateTimer(uint32_t diff)
{
    _nextMoveTime->updateTimer(diff);
    if (_nextMoveTime->isTimePassed())
    {
        _nextMoveTime->resetInterval(0);
        return true;
    }
    return false;
}

std::string WaypointMovementGenerator<Creature>::getDebugInfo() const
{
    std::stringstream sstr;
    sstr << PathMovementBase::getDebugInfo() << "\n"
        << MovementGeneratorMedium::getDebugInfo();
    return sstr.str();
}
