/*
 * AscEmu Framework based on ArcEmu MMORPG Server
 * Copyright (c) 2014-2018 AscEmu Team <http://www.ascemu.org>
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

#include "Units/Unit.h"

SERVER_DECL bool isHostile(Object* objA, Object* objB);                                 /// B is hostile for A?
SERVER_DECL bool isAttackable(Object* objA, Object* objB, bool CheckStealth = true);    /// A can attack B?
SERVER_DECL bool isCombatSupport(Object* objA, Object* objB);                           /// B combat supports A?;
SERVER_DECL bool isAlliance(Object* objA);                                              /// A is alliance?

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

inline bool isFriendly(Object* objA, Object* objB)       /// B is friendly to A if its not hostile
{
    if (!(objA->m_phase & objB->m_phase))                       /// We have to return prematurely, because isHostile would return false (phase difference!!!), and it would result in a true return value here.
        return false;                                           /// We must do this, as it affects AoE spell targets, thus required for them to function properly (so you won't heal out of phase friends...).
    return !isHostile(objA, objB);
}

inline bool isSameFaction(Object* objA, Object* objB)
{
    if (!objB->m_faction || !objA->m_faction)
        return true;                                            /// we return true to not give any agro to this object since it might cause other problems later
    return (objB->m_faction->Faction == objA->m_faction->Faction);
}

#endif // FACTION_H
