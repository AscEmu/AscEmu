/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "HomeMovementGenerator.h"
#include "Objects/Units/Creatures/Creature.h"
#include "Objects/Units/Creatures/AIInterface.h"
#include "Server/Script/CreatureAIScript.hpp"
#include "Management/G3DPosition.hpp"
#include "Movement/MovementManager.h"
#include "Movement/MovementDefines.h"
#include "Movement/Spline/MoveSpline.h"
#include "Movement/Spline/MoveSplineInit.h"

template<class T>
HomeMovementGenerator<T>::HomeMovementGenerator()
{
    this->Mode = MOTION_MODE_DEFAULT;
    this->Priority = MOTION_PRIORITY_NORMAL;
    this->Flags = MOVEMENTGENERATOR_FLAG_INITIALIZATION_PENDING;
    this->BaseUnitState = UNIT_STATE_ROAMING;
}

template HomeMovementGenerator<Creature>::HomeMovementGenerator();

template<class T>
MovementGeneratorType HomeMovementGenerator<T>::getMovementGeneratorType() const
{
    return HOME_MOTION_TYPE;
}

template MovementGeneratorType HomeMovementGenerator<Creature>::getMovementGeneratorType() const;

template<class T>
void HomeMovementGenerator<T>::setTargetLocation(T*) { }

template<>
void HomeMovementGenerator<Creature>::setTargetLocation(Creature* owner)
{
    // if we are ROOT/STUNNED/DISTRACTED even after aura clear, finalize on next update - otherwise we would get stuck in evade
    if (owner->hasUnitStateFlag(UNIT_STATE_ROOTED | UNIT_STATE_STUNNED | UNIT_STATE_DISTRACTED))
    {
        addFlag(MOVEMENTGENERATOR_FLAG_INTERRUPTED);
        return;
    }

    owner->removeUnitStateFlag(UNIT_STATE_ALL_ERASABLE & ~UNIT_STATE_EVADING);
    owner->addUnitStateFlag(UNIT_STATE_ROAMING_MOVE);

    LocationVector destination = owner->GetSpawnPosition();
    MovementMgr::MoveSplineInit init(owner);

    owner->updateAllowedPositionZ(destination.x, destination.y, destination.z);
    init.MoveTo(positionToVector3(destination));
    init.SetFacing(destination.getOrientation());
    init.SetWalk(false);
    init.Launch();
}

template<class T>
void HomeMovementGenerator<T>::doInitialize(T*) { }

template<>
void HomeMovementGenerator<Creature>::doInitialize(Creature* owner)
{
    removeFlag(MOVEMENTGENERATOR_FLAG_INITIALIZATION_PENDING | MOVEMENTGENERATOR_FLAG_DEACTIVATED);
    addFlag(MOVEMENTGENERATOR_FLAG_INITIALIZED);

    owner->getAIInterface()->setNoSearchAssistance(false);

    setTargetLocation(owner);
}

template<class T>
void HomeMovementGenerator<T>::doReset(T*) { }

template<>
void HomeMovementGenerator<Creature>::doReset(Creature* owner)
{
    removeFlag(MOVEMENTGENERATOR_FLAG_DEACTIVATED);

    doInitialize(owner);
}

template<class T>
bool HomeMovementGenerator<T>::doUpdate(T*, uint32_t)
{
    return false;
}

template<>
bool HomeMovementGenerator<Creature>::doUpdate(Creature* owner, uint32_t /*diff*/)
{
    if (hasFlag(MOVEMENTGENERATOR_FLAG_INTERRUPTED) || owner->movespline->Finalized())
    {
        addFlag(MOVEMENTGENERATOR_FLAG_INFORM_ENABLED);
        return false;
    }
    return true;
}

template<class T>
void HomeMovementGenerator<T>::doDeactivate(T*) { }

template<>
void HomeMovementGenerator<Creature>::doDeactivate(Creature* owner)
{
    addFlag(MOVEMENTGENERATOR_FLAG_DEACTIVATED);
    owner->removeUnitStateFlag(UNIT_STATE_ROAMING_MOVE);
}

template<class T>
void HomeMovementGenerator<T>::doFinalize(T*, bool, bool) { }

template<>
void HomeMovementGenerator<Creature>::doFinalize(Creature* owner, bool active, bool movementInform)
{
    addFlag(MOVEMENTGENERATOR_FLAG_FINALIZED);
    if (active)
        owner->removeUnitStateFlag(UNIT_STATE_ROAMING_MOVE | UNIT_STATE_EVADING);

    if (movementInform && hasFlag(MOVEMENTGENERATOR_FLAG_INFORM_ENABLED))
    {
        // todo handle healt regen and other stuff here ( basically combat reset )?
        if (owner->GetScript())
            owner->GetScript()->justReachedSpawn();
    }
}
