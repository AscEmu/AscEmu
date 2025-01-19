/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "SpellClickInfo.hpp"
#include "Management/Group.h"
#include "Objects/Units/Players/Player.hpp"
#include "Objects/Units/Unit.hpp"

bool SpellClickInfo::isFitToRequirements(Unit* clicker, Unit* clickee) const
{
    Player* playerClicker = clicker->ToPlayer();
    if (!playerClicker)
        return true;

    Unit* summoner = nullptr;
    // Check summoners for party
    if (clickee->isSummon())
        summoner = clickee->getWorldMapUnit(clickee->getSummonedByGuid());

    if (!summoner)
        summoner = clickee;

    // This only applies to players
    switch (userType)
    {
        case SPELL_CLICK_USER_FRIEND:
            if (!playerClicker->isFriendlyTo(summoner))
                return false;
            break;
        case SPELL_CLICK_USER_RAID:
        case SPELL_CLICK_USER_PARTY:
            if (playerClicker->getGroup()->GetID() != summoner->ToPlayer()->getGroup()->GetID())
                return false;
            break;
        default:
            break;
    }

    return true;
}
