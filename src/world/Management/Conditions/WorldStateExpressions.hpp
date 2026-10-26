/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <cstdint>
#include <string_view>

class Player;

namespace Conditions
{
    // a realm with its own db2 tables adds the expressions, every other realm reads the dbc store
    void addWorldStateExpression(uint32_t id, std::string_view expression);
    void initializeWorldStateExpressions();

    // the expression of the client data for the map and zone of the player, false when it can not be evaluated
    bool isWorldStateExpressionMet(Player* player, uint32_t expressionId);
}
