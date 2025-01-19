/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "AIEvents.h"
#include "AIInterface.h"

pAIEvent AIEventHandlers[NUM_AI_EVENTS] =
{
    &AIInterface::eventEnterCombat,
    &AIInterface::eventLeaveCombat,
    &AIInterface::eventDamageTaken,
    &AIInterface::eventFear,
    &AIInterface::eventUnfear,
    &AIInterface::eventFollowOwner,
    &AIInterface::eventWander,
    &AIInterface::eventUnwander,
    &AIInterface::eventUnitDied,
    &AIInterface::eventHostileAction,
    &AIInterface::eventForceRedirected,
};
