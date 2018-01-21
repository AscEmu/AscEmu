/*
Copyright (c) 2014-2018 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Units/Players/Player.h"
#include "Objects/ObjectMgr.h"

bool Player::isSpellFitByClassAndRace(uint32_t spell_id)
{
    auto racemask = getRaceMask();
    auto classmask = getClassMask();

    auto bounds = objmgr.GetSkillLineAbilityMapBounds(spell_id);
    if (bounds.first == bounds.second)
    {
        return true;
    }

    for (auto _spell_idx = bounds.first; _spell_idx != bounds.second; ++_spell_idx)
    {
        // skip wrong race skills
        if (_spell_idx->second->race_mask && (_spell_idx->second->race_mask & racemask) == 0)
        {
            continue;
        }

        // skip wrong class skills
        if (_spell_idx->second->class_mask && (_spell_idx->second->class_mask & classmask) == 0)
        {
            continue;
        }

        return true;
    }

    return false;
}
