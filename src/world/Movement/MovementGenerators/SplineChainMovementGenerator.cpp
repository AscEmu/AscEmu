/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "SplineChainMovementGenerator.h"
#include "Objects/Units/Creatures/Creature.h"
#include "Objects/Units/Creatures/AIInterface.h"
#include "Debugging/Errors.h"
#include "Movement/MovementManager.h"
#include "Movement/MovementDefines.h"
#include "Movement/Spline/MoveSpline.h"
#include "Movement/Spline/MoveSplineInit.h"
#include "Objects/Units/Unit.hpp"

SplineChainMovementGenerator::SplineChainMovementGenerator(uint32_t id, std::vector<SplineChainLink> const& chain, bool walk) : _id(id), _chain(chain), _chainSize(static_cast<uint8_t>(chain.size())), _walk(walk), _nextIndex(0), _nextFirstWP(0), _msToNext(0)
{
    Mode = MOTION_MODE_DEFAULT;
    Priority = MOTION_PRIORITY_NORMAL;
    Flags = MOVEMENTGENERATOR_FLAG_INITIALIZATION_PENDING;
    BaseUnitState = UNIT_STATE_ROAMING;
}

SplineChainMovementGenerator::SplineChainMovementGenerator(SplineChainResumeInfo const& info) : _id(info.PointID), _chain(*info.Chain), _chainSize(static_cast<uint8_t>(info.Chain->size())), _walk(info.IsWalkMode), _nextIndex(info.SplineIndex), _nextFirstWP(info.PointIndex), _msToNext(info.TimeToNext)
{
    Mode = MOTION_MODE_DEFAULT;
    Priority = MOTION_PRIORITY_NORMAL;

    Flags = MOVEMENTGENERATOR_FLAG_INITIALIZATION_PENDING;
    if (info.SplineIndex >= info.Chain->size())
        addFlag(MOVEMENTGENERATOR_FLAG_FINALIZED);

    BaseUnitState = UNIT_STATE_ROAMING;
}

uint32_t SplineChainMovementGenerator::sendPathSpline(Unit* owner, float velocity, MovementMgr::PointsArray const& path) const
{
    const auto nodeCount = path.size();
    ASSERT(nodeCount > 1 && "SplineChainMovementGenerator::SendPathSpline: Every path must have source & destination (size > 1)!");

    MovementMgr::MoveSplineInit init(owner);
    if (nodeCount > 2)
        init.MovebyPath(path);
    else
        init.MoveTo(path[1], false, true);

    if (velocity > 0.f)
        init.SetVelocity(velocity);
    init.SetWalk(_walk);
    return init.Launch();
}

void SplineChainMovementGenerator::sendSplineFor(Unit* owner, uint32_t index, uint32_t& duration)
{
    ASSERT(index < _chainSize && "SplineChainMovementGenerator::SendSplineFor: referenced index higher than path size!");

    SplineChainLink const& thisLink = _chain[index];
    uint32_t actualDuration = sendPathSpline(owner, thisLink.Velocity, thisLink.Points);
    if (actualDuration != thisLink.ExpectedDuration)
    {
        duration = uint32_t(double(actualDuration) / double(thisLink.ExpectedDuration) * duration);
    }
}

void SplineChainMovementGenerator::initialize(Unit* owner)
{
    removeFlag(MOVEMENTGENERATOR_FLAG_INITIALIZATION_PENDING | MOVEMENTGENERATOR_FLAG_DEACTIVATED);
    addFlag(MOVEMENTGENERATOR_FLAG_INITIALIZED);

    if (!_chainSize)
    {
        return;
    }

    if (_nextIndex >= _chainSize)
    {
        _msToNext = 0;
        return;
    }

    if (_nextFirstWP) // this is a resumed movegen that has to start with a partial spline
    {
        if (hasFlag(MOVEMENTGENERATOR_FLAG_FINALIZED))
            return;

        SplineChainLink const& thisLink = _chain[_nextIndex];
        if (_nextFirstWP >= thisLink.Points.size())
        {
            _nextFirstWP = static_cast<uint8_t>(thisLink.Points.size() - 1);
        }

        owner->addUnitStateFlag(UNIT_STATE_ROAMING_MOVE);
        MovementMgr::PointsArray partial(thisLink.Points.begin() + (_nextFirstWP-1), thisLink.Points.end());
        sendPathSpline(owner, thisLink.Velocity, partial);

        ++_nextIndex;
        if (_nextIndex >= _chainSize)
            _msToNext = 0;
        else if (!_msToNext)
            _msToNext = 1;
        _nextFirstWP = 0;
    }
    else
    {
        _msToNext = std::max(_chain[_nextIndex].TimeToNext, 1u);
        sendSplineFor(owner, _nextIndex, _msToNext);

        ++_nextIndex;
        if (_nextIndex >= _chainSize)
            _msToNext = 0;
    }
}

void SplineChainMovementGenerator::reset(Unit* owner)
{
    removeFlag(MOVEMENTGENERATOR_FLAG_DEACTIVATED);

    owner->stopMoving();
    initialize(owner);
}

bool SplineChainMovementGenerator::update(Unit* owner, uint32_t diff)
{
    if (!owner || hasFlag(MOVEMENTGENERATOR_FLAG_FINALIZED))
        return false;

    // _msToNext being zero here means we're on the final spline
    if (!_msToNext)
    {
        if (owner->movespline->Finalized())
        {
            addFlag(MOVEMENTGENERATOR_FLAG_INFORM_ENABLED);
            return false;
        }
        return true;
    }

    if (_msToNext <= diff)
    {
        // Send next spline
        _msToNext = std::max(_chain[_nextIndex].TimeToNext, 1u);
        sendSplineFor(owner, _nextIndex, _msToNext);
        ++_nextIndex;
        if (_nextIndex >= _chainSize)
        {
            // We have reached the final spline, once it finalizes we should also finalize the movegen (start checking on next update)
            _msToNext = 0;
            return true;
        }
    }
    else
        _msToNext -= diff;

    return true;
}

void SplineChainMovementGenerator::deactivate(Unit* owner)
{
    addFlag(MOVEMENTGENERATOR_FLAG_DEACTIVATED);
    owner->removeUnitStateFlag(UNIT_STATE_ROAMING_MOVE);
}

void SplineChainMovementGenerator::finalize(Unit* owner, bool active, bool movementInform)
{
    addFlag(MOVEMENTGENERATOR_FLAG_FINALIZED);

    if (active)
        owner->removeUnitStateFlag(UNIT_STATE_ROAMING_MOVE);

    if (movementInform && hasFlag(MOVEMENTGENERATOR_FLAG_INFORM_ENABLED))
    {
        Creature* ownerCreature = owner->ToCreature();
        if (AIInterface* AI = ownerCreature ? ownerCreature->getAIInterface() : nullptr)
            AI->movementInform(SPLINE_CHAIN_MOTION_TYPE, _id);
    }
}

MovementGeneratorType SplineChainMovementGenerator::getMovementGeneratorType() const
{
    return SPLINE_CHAIN_MOTION_TYPE;
}

SplineChainResumeInfo SplineChainMovementGenerator::getResumeInfo(Unit const* owner) const
{
    if (!_nextIndex)
        return SplineChainResumeInfo(_id, &_chain, _walk, 0, 0, _msToNext);

    if (owner->movespline->Finalized())
    {
        if (_nextIndex < _chainSize)
            return SplineChainResumeInfo(_id, &_chain, _walk, _nextIndex, 0, 1u);
        else
            return SplineChainResumeInfo();
    }

    return SplineChainResumeInfo(_id, &_chain, _walk, uint8_t(_nextIndex - 1), uint8_t(owner->movespline->_currentSplineIdx()), _msToNext);
}

/* static */ void SplineChainMovementGenerator::getResumeInfo(SplineChainResumeInfo& info, Unit const* owner, Optional<uint32_t> id)
{
    std::function<bool(MovementGenerator const*)> criteria = [id](MovementGenerator const* movement) -> bool
    {
        if (movement->getMovementGeneratorType() == SPLINE_CHAIN_MOTION_TYPE)
            return (!id || static_cast<SplineChainMovementGenerator const*>(movement)->getId() == *id);

        return false;
    };

    if (MovementGenerator const* activeGenerator = owner->getMovementManager()->getMovementGenerator(criteria))
        info = static_cast<SplineChainMovementGenerator const*>(activeGenerator)->getResumeInfo(owner);
    else
        info.Clear();
}
