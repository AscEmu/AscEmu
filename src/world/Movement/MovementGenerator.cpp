/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "MovementGenerator.h"

#include <sstream>

#include "Objects/Units/Creatures/Creature.h"
#include "MovementGenerators/IdleMovementGenerator.h"
#include "Movement/MovementDefines.h"
#include "PathGenerator.h"
#include "MovementGenerators/RandomMovementGenerator.h"
#include "Objects/Units/Creatures/AIInterface.h"
#include "MovementGenerators/WaypointMovementGenerator.h"

MovementGenerator::~MovementGenerator() { }

std::string MovementGenerator::getDebugInfo() const
{
    std::stringstream sstr;
    sstr << std::boolalpha
        << "Mode: " << std::to_string(Mode)
        << " Priority: " << std::to_string(Priority)
        << " Flags: " << Flags
        << " BaseUniteState: " << BaseUnitState;
    return sstr.str();
}

IdleMovementFactory::IdleMovementFactory() : MovementGeneratorCreator(IDLE_MOTION_TYPE) { }

MovementGenerator* IdleMovementFactory::create(Unit* /*object*/) const
{
    static IdleMovementGenerator mInstance;
    return &mInstance;
}

RandomMovementFactory::RandomMovementFactory() : MovementGeneratorCreator(RANDOM_MOTION_TYPE) { }

MovementGenerator* RandomMovementFactory::create(Unit* /*object*/) const
{
    return new RandomMovementGenerator<Creature>();
}

WaypointMovementFactory::WaypointMovementFactory() : MovementGeneratorCreator(WAYPOINT_MOTION_TYPE) { }

MovementGenerator* WaypointMovementFactory::create(Unit* /*object*/) const
{
    return new WaypointMovementGenerator<Creature>();
}

void MovementGenerator::notifyAIOnFinalize(Unit* object)
{
    if (auto ai = object->getAIInterface())
        ai->onMovementGeneratorFinalized(getMovementGeneratorType());
}
