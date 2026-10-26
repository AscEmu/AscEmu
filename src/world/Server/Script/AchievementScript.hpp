/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "AEVersion.hpp"
#include "Platform/SymbolVisibility.hpp"

#include <cstdint>


class Object;
class Player;

class SERVER_DECL AchievementCriteriaScript
{
public:
    AchievementCriteriaScript() = default;
    virtual ~AchievementCriteriaScript() {}

    // Note; target can be nullptr!
    virtual bool canCompleteCriteria(uint32_t criteriaId, Player* player, Object* target);
};
