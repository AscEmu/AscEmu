/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "GenericMovementGenerator.h"
#include "Objects/Units/Creatures/Creature.h"
#include "Objects/Units/Creatures/AIInterface.h"
#include "Movement/MovementDefines.h"
#include "Movement/Spline/MoveSpline.h"
#include "Objects/Units/Unit.hpp"
#include "Utilities/TimeTracker.hpp"

GenericMovementGenerator::GenericMovementGenerator(MovementMgr::MoveSplineInit&& splineInit, MovementGeneratorType type, uint32_t id) :
_splineInit(std::move(splineInit)), _type(type), _pointId(id), _duration(std::make_unique<Util::SmallTimeTracker>(0))
{
    Mode = MOTION_MODE_DEFAULT;
    Priority = MOTION_PRIORITY_NORMAL;
    Flags = MOVEMENTGENERATOR_FLAG_INITIALIZATION_PENDING;
    BaseUnitState = UNIT_STATE_ROAMING;
}

void GenericMovementGenerator::initialize(Unit* /*owner*/)
{
    if (hasFlag(MOVEMENTGENERATOR_FLAG_DEACTIVATED) && !hasFlag(MOVEMENTGENERATOR_FLAG_INITIALIZATION_PENDING)) // Resume spline is not supported
    {
        removeFlag(MOVEMENTGENERATOR_FLAG_DEACTIVATED);
        addFlag(MOVEMENTGENERATOR_FLAG_FINALIZED);
        return;
    }

    removeFlag(MOVEMENTGENERATOR_FLAG_INITIALIZATION_PENDING | MOVEMENTGENERATOR_FLAG_DEACTIVATED);
    addFlag(MOVEMENTGENERATOR_FLAG_INITIALIZED);

    _duration->resetInterval(_splineInit.Launch());
}

void GenericMovementGenerator::reset(Unit* owner)
{
    initialize(owner);
}

bool GenericMovementGenerator::update(Unit* owner, uint32_t diff)
{
    if (!owner || hasFlag(MOVEMENTGENERATOR_FLAG_FINALIZED))
        return false;

    // Cyclic splines never expire, so update the duration only if it's not cyclic
    if (!owner->movespline->isCyclic())
        _duration->updateTimer(diff);

    if (_duration->isTimePassed() || owner->movespline->Finalized())
    {
        addFlag(MOVEMENTGENERATOR_FLAG_INFORM_ENABLED);
        return false;
    }
    return true;
}

void GenericMovementGenerator::deactivate(Unit*)
{
    addFlag(MOVEMENTGENERATOR_FLAG_DEACTIVATED);
}

void GenericMovementGenerator::finalize(Unit* owner, bool/* active*/, bool _movementInform)
{
    addFlag(MOVEMENTGENERATOR_FLAG_FINALIZED);

    if (_movementInform && hasFlag(MOVEMENTGENERATOR_FLAG_INFORM_ENABLED))
        movementInform(owner);
}

void GenericMovementGenerator::movementInform(Unit* owner)
{
    if (Creature* creature = owner->ToCreature())
    {
        if (creature->getAIInterface())
            creature->getAIInterface()->movementInform(_type, _pointId);
    }
}
