/*
 * AscEmu Framework based on ArcEmu MMORPG Server
 * Copyright (C) 2005-2007 Ascent Team
 * Copyright (C) 2008-2011 <http://www.ArcEmu.org/>
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
 *
 */

#include "../../StdAfx.h"
#include "Units/Creatures/Creature.h"
#include "Units/Summons/Summon.h"

Summon::Summon(uint64 GUID) : Creature(GUID)
{
    summonslot = -1;
    owner = NULL;
}

Summon::~Summon()
{}

void Summon::Load(CreatureProperties const* properties_, Unit* pOwner, LocationVector & position, uint32 spellid, int32 pSummonslot)
{
    ARCEMU_ASSERT(pOwner != nullptr);

    Creature::Load(properties_, position.x, position.y, position.z, position.o);

    SetFaction(pOwner->GetFaction());
    Phase(PHASE_SET, pOwner->GetPhase());
    SetZoneId(pOwner->GetZoneId());
    SetCreatedBySpell(spellid);
    this->summonslot = pSummonslot;

    if (pOwner->IsPvPFlagged())
        SetPvPFlag();
    else
        RemovePvPFlag();

    if (pOwner->IsFFAPvPFlagged())
        SetFFAPvPFlag();
    else
        RemoveFFAPvPFlag();

    if (pOwner->IsSanctuaryFlagged())
        SetSanctuaryFlag();
    else
        RemoveSanctuaryFlag();

    SetCreatedByGUID(pOwner->GetGUID());

    if (pOwner->GetSummonedByGUID() == 0)
        SetSummonedByGUID(pOwner->GetGUID());
    else
        SetSummonedByGUID(pOwner->GetSummonedByGUID());

    this->owner = pOwner;

    if (pOwner->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PVP_ATTACKABLE))
        SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PVP_ATTACKABLE);

}

void Summon::OnPushToWorld()
{
    if (summonslot != -1)
        owner->summonhandler.AddSummonToSlot(this, summonslot);
    else
        owner->summonhandler.AddSummon(this);

    Creature::OnPushToWorld();

}

void Summon::OnPreRemoveFromWorld()
{
    if (owner == NULL)
        return;

    if (GetCreatedBySpell() != 0)
        owner->RemoveAura(GetCreatedBySpell());

    if (summonslot != -1)
        owner->summonhandler.RemoveSummonFromSlot(summonslot, false);
    else
        owner->summonhandler.RemoveSummon(this);

    summonslot = -1;
    owner = NULL;

    SendDestroyObject();

}

Object* Summon::GetPlayerOwner()
{
    // pOwner is nulled on death
    if (owner == NULL)
        return NULL;

    if (owner->IsPlayer())
        return owner;
    else
        return NULL;
}

void Summon::Die(Unit* pAttacker, uint32 damage, uint32 spellid)
{
    if (owner->IsTotem())
        owner->Die(pAttacker, damage, spellid);

    Creature::Die(pAttacker, damage, spellid);

    if (summonslot != -1)
        owner->summonhandler.RemoveSummonFromSlot(summonslot, false);
    else
        owner->summonhandler.RemoveSummon(this);

    summonslot = -1;
    owner = NULL;
}

void Summon::OnRemoveInRangeObject(Object* object)
{

    if ((owner != NULL) && (object->GetGUID() == owner->GetGUID()))
        Despawn(1, 0);

    Creature::OnRemoveInRangeObject(object);
}
