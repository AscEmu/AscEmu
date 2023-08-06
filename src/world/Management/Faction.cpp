/*
 * AscEmu Framework based on ArcEmu MMORPG Server
 * Copyright (c) 2014-2023 AscEmu Team <http://www.ascemu.org>
 * Copyright (C) 2008-2012 ArcEmu Team <http://www.ArcEmu.org/>
 * Copyright (C) 2005-2007 Ascent Team
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "Storage/WDB/WDBStores.hpp"
#include "Objects/Object.hpp"
#include "Objects/Units/Players/PlayerDefines.hpp"
#include "Objects/Units/Unit.hpp"
#include "Objects/Units/Creatures/AIInterface.h"
#include "Objects/Units/Players/Player.hpp"

bool canBeginCombat(Unit* objA, Unit* objB)
{
    if (objA == objB)
        return false;
    // ...the two units need to be in the world
    if (!objA->IsInWorld() || !objB->IsInWorld())
        return false;
    // ...the two units need to both be alive
    if (!objA->isAlive() || !objB->isAlive())
        return false;
    // ...the two units need to be on the same map
    if (objA->getWorldMap() != objB->getWorldMap())
        return false;
    // ...the two units need to be in the same phase
    if (objA->GetPhase() != objB->GetPhase())
        return false;
    if (objA->hasUnitStateFlag(UNIT_STATE_EVADING) || objB->hasUnitStateFlag(UNIT_STATE_EVADING))
        return false;
    if (objA->hasUnitStateFlag(UNIT_STATE_IN_FLIGHT) || objB->hasUnitStateFlag(UNIT_STATE_IN_FLIGHT))
        return false;
    // ... both units must not be ignoring combat
    if (objA->getAIInterface()->isCombatDisabled() || objB->getAIInterface()->isCombatDisabled())
        return false;
    if (objA->isFriendlyTo(objB) || objB->isFriendlyTo(objA))
        return false;
    Player* playerA = objA->getUnitOwnerOrSelf() ? objA->getUnitOwnerOrSelf()->ToPlayer() : nullptr;
    Player* playerB = objB->getUnitOwnerOrSelf() ? objB->getUnitOwnerOrSelf()->ToPlayer() : nullptr;
    // ...neither of the two units must be (owned by) a player with .gm on
    if ((playerA && playerA->isGMFlagSet()) || (playerB && playerB->isGMFlagSet()))
        return false;

    return true;
}

bool isNeutral(Object* a, Object* b)
{
    if ((a->m_factionTemplate->HostileMask & b->m_factionTemplate->Mask) == 0 && (a->m_factionTemplate->FriendlyMask & b->m_factionTemplate->Mask) == 0)
        return true;

    return false;
}

bool isAlliance(Object* objA)// A is alliance?
{
    WDB::Structures::FactionTemplateEntry const* m_sw_faction = sFactionTemplateStore.lookupEntry(11);
    WDB::Structures::FactionEntry const* m_sw_factionDBC = sFactionStore.lookupEntry(72);
    if (!objA)          // || objA->m_factionEntry == NULL || objA->m_factionTemplate == NULL
        return true;

    if (m_sw_faction == objA->m_factionTemplate || m_sw_factionDBC == objA->m_factionEntry)
        return true;

    //bool hostile = false;
    uint32 faction = m_sw_faction->Faction;
    uint32 host = objA->m_factionTemplate->HostileMask;

    if (faction & host)
        return false;

    // check friend/enemy list
    for (uint8 i = 0; i < 4; i++)
    {
        if (objA->m_factionTemplate->EnemyFactions[i] == faction)
            return false;
    }

    faction = objA->m_factionTemplate->Faction;
    host = m_sw_faction->HostileMask;

    if (faction & host)
        return false;

    // check friend/enemy list
    for (uint8 i = 0; i < 4; i++)
    {
        if (objA->m_factionTemplate->EnemyFactions[i] == faction)
            return false;
    }

    return true;
}
