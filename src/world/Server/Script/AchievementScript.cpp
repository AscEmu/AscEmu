/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "AchievementScript.hpp"

#ifdef FT_ACHIEVEMENTS

bool AchievementCriteriaScript::canCompleteCriteria(uint32_t /*criteriaId*/, Player* /*player*/, Object* /*target*/)
{
    return true;
}
#endif
