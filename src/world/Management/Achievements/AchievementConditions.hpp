/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "AchievementEvents.hpp"

#include <cstdint>

class Player;

namespace Achievements
{
    struct ConditionNode;
    struct Criteria;

    // modifier types that name other rows of the client data
    constexpr uint32_t ConditionTypePlayerCondition = 2;
    constexpr uint32_t ConditionTypeTargetPlayerCondition = 55;
    constexpr uint32_t ConditionTypeModifierTree = 73;

    // a modifier type the core evaluates, every other type keeps its criteria from progressing
    bool isConditionTypeHandled(uint32_t type);

    // the modifier tree of a criteria against the player and the subject of the event
    bool isConditionTreeMet(ConditionNode const* tree, Player* player, EventSubject const& subject);

    // a requirement of the world database the core evaluates, every other kind keeps its criteria from progressing
    bool isRequirementKindHandled(uint8_t kind);

    // every requirement of the world database for the criteria holds
    bool areRequirementsMet(Criteria const& criteria, Player* player, EventSubject const& subject);
}
