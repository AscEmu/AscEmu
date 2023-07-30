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

#ifndef FACTION_H
#define FACTION_H

#include "Objects/Object.hpp"
#include "Objects/Units/Unit.hpp"

inline bool isHostile(Object* objA, Object* objB) { return objA->isHostileTo(objB); }
inline bool isFriendly(Object* objA, Object* objB) { return objA->isFriendlyTo(objB); }
inline bool isAttackable(Object* objA, Object* objB, SpellInfo const* bySpell = nullptr) { return objA->isValidTarget(objB, bySpell); }
SERVER_DECL bool canBeginCombat(Unit* objA, Unit* objB);

//////////////////////////////////////////////////////////////////////////////////////////
/// \note bool isNeutral    - Tells if 2 Objects are neutral to each others based on their faction.
///
/// \param Object* a  -  Pointer to an Object
/// \param Object* b  -  Pointer to an Object
///
/// \returns true if they are neutral, false otherwise.
///
//////////////////////////////////////////////////////////////////////////////////////////
SERVER_DECL bool isNeutral(Object* a, Object* b);
SERVER_DECL bool isAlliance(Object* objA);

inline bool isSameFaction(Object* objA, Object* objB)
{
    if (!objB->m_factionTemplate || !objA->m_factionTemplate)
        return true;                                            /// we return true to not give any agro to this object since it might cause other problems later
    return (objB->m_factionTemplate->Faction == objA->m_factionTemplate->Faction);
}

#endif // FACTION_H
