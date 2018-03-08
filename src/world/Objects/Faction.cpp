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

#include "StdAfx.h"
#include "Object.h"
#include "Units/Players/PlayerDefines.hpp"
#include "Units/Unit.h"
#include "Units/Creatures/AIInterface.h"
#include "Units/Players/Player.h"

bool isNeutral(Object* a, Object* b)
{
    if ((a->m_faction->HostileMask & b->m_faction->Mask) == 0 && (a->m_faction->FriendlyMask & b->m_faction->Mask) == 0)
        return true;

    return false;
}

SERVER_DECL bool isHostile(Object* objA, Object* objB)
{
    if ((objA == NULL) || (objB == NULL))
        return false;

    bool hostile = false;

    if (!objA->IsInWorld() || !objB->IsInWorld())
        return false;

    if (objA == objB)
        return false;   // can't attack self.. this causes problems with buffs if we don't have it :p

    if (objA->IsCorpse() || objB->IsCorpse())
        return false;

    if ((objA->m_phase & objB->m_phase) == 0)     //What you can't see, can't be hostile!
        return false;

    if (objA->IsPlayer() && objA->HasFlag(PLAYER_FLAGS, PLAYER_FLAG_CONT_PVP) && objB->IsCreature() && reinterpret_cast<Unit*>(objB)->GetAIInterface()->m_isNeutralGuard)
        return true;
    if (objB->IsPlayer() && objB->HasFlag(PLAYER_FLAGS, PLAYER_FLAG_CONT_PVP) && objA->IsCreature() && reinterpret_cast<Unit*>(objA)->GetAIInterface()->m_isNeutralGuard)
        return true;

    if (objB->IsUnit() && static_cast<Unit*>(objB)->hasUnitFlags(UNIT_FLAG_NOT_ATTACKABLE_2 | UNIT_FLAG_IGNORE_CREATURE_COMBAT | UNIT_FLAG_IGNORE_PLAYER_COMBAT | UNIT_FLAG_ALIVE))
        return false;

    if (!objB->m_faction || !objA->m_faction)
        return false;

    uint32 faction = objB->m_faction->Mask;
    uint32 host = objA->m_faction->HostileMask;

    if ((faction & host) != 0)
        hostile = true;

    faction = objA->m_faction->Mask;
    host = objB->m_faction->HostileMask;

    if ((faction & host) != 0)
        hostile = true;

    // check friend/enemy list
    for (uint8 i = 0; i < 4; i++)
    {
        if (objA->m_faction->EnemyFactions[i] == objB->m_faction->Faction)
        {
            hostile = true;
            break;
        }

        if (objA->m_faction->FriendlyFactions[i] == objB->m_faction->Faction)
        {
            hostile = false;
            break;
        }
    }

    // Reputation System Checks
    if (objA->IsPlayer() && !objB->IsPlayer())
        if (objB->m_factionDBC->RepListId >= 0)
            hostile = reinterpret_cast< Player* >(objA)->IsHostileBasedOnReputation(objB->m_factionDBC);

    if (objB->IsPlayer() && !objA->IsPlayer())
        if (objA->m_factionDBC->RepListId >= 0)
            hostile = reinterpret_cast< Player* >(objB)->IsHostileBasedOnReputation(objA->m_factionDBC);

    // PvP Flag System Checks
    // We check this after the normal isHostile test, that way if we're
    // on the opposite team we'll already know :p
    if ((objA->GetPlayerOwner() != NULL) && (objB->GetPlayerOwner() != NULL))
    {
        Player* a = reinterpret_cast< Player* >(objA->GetPlayerOwner());
        Player* b = reinterpret_cast< Player* >(objB->GetPlayerOwner());

        auto atA = a->GetArea();
        auto atB = b->GetArea();

        if (((atA && atA->flags & 0x800) != 0) || ((atB && atB->flags & 0x800) != 0))
            return false;

        if (hostile)
        {
            if (!b->IsSanctuaryFlagged() && (b->IsPvPFlagged() || b->IsFFAPvPFlagged()))
                return true;
            else
                return false;
        }

    }

    return hostile;
}

/// Where we check if we object A can attack object B. This is used in many feature's
/// Including the spell class and the player class.
SERVER_DECL bool isAttackable(Object* objA, Object* objB, bool CheckStealth)
{
    if ((objA == NULL) || (objB == NULL))
        return false;

    if (!objA->IsInWorld() || !objB->IsInWorld())
        return false;

    if (objA == objB)
        return false;   // can't attack self.. this causes problems with buffs if we don't have it :p

    if ((objA->m_phase & objB->m_phase) == 0)     //What you can't see, you can't attack either...
        return false;

    if (objA->IsCorpse() || objB->IsCorpse())
        return false;

    // Checks for untouchable, unattackable
    if (objA->IsUnit() && static_cast<Unit*>(objA)->hasUnitFlags(UNIT_FLAG_MOUNTED_TAXI | UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_DEAD))
        return false;

    if (objB->IsUnit())
    {
        if (static_cast<Unit*>(objB)->hasUnitFlags(UNIT_FLAG_MOUNTED_TAXI | UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_DEAD))
            return false;

        /// added by Zack :
        /// we cannot attack shealthed units. Maybe checked in other places too ?
        /// !! warning, this presumes that objA is attacking ObjB
        /// Capt: Added the possibility to disregard this (regarding the spell class)
        if (static_cast< Unit* >(objB)->IsStealth() && CheckStealth)
            return false;
    }

    if ((objA->GetPlayerOwner() != NULL) && (objB->GetPlayerOwner() != NULL))
    {
        Player* a = static_cast< Player* >(objA->GetPlayerOwner());
        Player* b = static_cast< Player* >(objB->GetPlayerOwner());

        if ((a->DuelingWith == b) && (a->GetDuelState() == DUEL_STATE_STARTED))
            return true;

        if (b->IsSanctuaryFlagged())
            return false;

        //players in same group should not attack each other. Required for arenas with mixed groups
        if ((a->GetGroup() != NULL) && (a->GetGroup() == b->GetGroup()))
            return false;

        if (a->IsFFAPvPFlagged() && b->IsFFAPvPFlagged())
            return true;
    }

    if (objA->m_faction == nullptr || objB->m_faction == nullptr)     // no faction, no kill (added because spell_caster gos should summon a trap instead of casting the spell directly.)
        return false;

    if (objA->m_faction == objB->m_faction)    // same faction can't kill each other unless in ffa pvp/duel
        return false;

    // Neutral Creature Check
    if (objA->IsPlayer() || objA->IsPet())
    {

        if ((objB->m_factionDBC->RepListId == -1) && (objB->m_faction->HostileMask == 0) && (objB->m_faction->FriendlyMask == 0))
            return true;
    }
    else if (objB->IsPlayer() || objB->IsPet())
    {
        if ((objA->m_factionDBC->RepListId == -1) && (objA->m_faction->HostileMask == 0) && (objA->m_faction->FriendlyMask == 0))
            return true;
    }

    bool attackable = isHostile(objA, objB);   // B is attackable if its hostile for A

    return attackable;
}

bool isCombatSupport(Object* objA, Object* objB)// B combat supports A?
{
    if (!objA || !objB)
        return false;

    if (objA->IsCorpse())
        return false;

    if (objB->IsCorpse())
        return false;

    if (!objA->IsCreature() || !objB->IsCreature()) return false;    // cebernic: lowchance crashfix.
    // also if it's not a unit, it shouldn't support combat anyways.

    if (objA->IsPet() || objB->IsPet())   // fixes an issue where horde pets would chain aggro horde guards and vice versa for alliance.
        return false;

    if (!(objA->m_phase & objB->m_phase))   //What you can't see, you can't support either...
        return false;

    bool combatSupport = false;

    uint32 fSupport = objB->m_faction->FriendlyMask;
    uint32 myFaction = objA->m_faction->Mask;

    if (myFaction & fSupport)
    {
        combatSupport = true;
    }
    // check friend/enemy list
    for (uint8 i = 0; i < 4; i++)
    {
        if (objB->m_faction->EnemyFactions[i] == objA->m_faction->Faction)
        {
            combatSupport = false;
            break;
        }
        if (objB->m_faction->FriendlyFactions[i] == objA->m_faction->Faction)
        {
            combatSupport = true;
            break;
        }
    }

    return combatSupport;
}

bool isAlliance(Object* objA)// A is alliance?
{
    DBC::Structures::FactionTemplateEntry const* m_sw_faction = sFactionTemplateStore.LookupEntry(11);
    DBC::Structures::FactionEntry const* m_sw_factionDBC = sFactionStore.LookupEntry(72);
    if (!objA)          // || objA->m_factionDBC == NULL || objA->m_faction == NULL
        return true;

    if (m_sw_faction == objA->m_faction || m_sw_factionDBC == objA->m_factionDBC)
        return true;

    //bool hostile = false;
    uint32 faction = m_sw_faction->Faction;
    uint32 host = objA->m_faction->HostileMask;

    if (faction & host)
        return false;

    // check friend/enemy list
    for (uint8 i = 0; i < 4; i++)
    {
        if (objA->m_faction->EnemyFactions[i] == faction)
            return false;
    }

    faction = objA->m_faction->Faction;
    host = m_sw_faction->HostileMask;

    if (faction & host)
        return false;

    // check friend/enemy list
    for (uint8 i = 0; i < 4; i++)
    {
        if (objA->m_faction->EnemyFactions[i] == faction)
            return false;
    }

    return true;
}
