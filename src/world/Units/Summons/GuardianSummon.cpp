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
#include "Units/Summons/GuardianSummon.h"
#include "../../../scripts/Battlegrounds/EyeOfTheStorm.h"
#include "Spell/Definitions/PowerType.h"

GuardianSummon::GuardianSummon(uint64 GUID) : Summon(GUID)
{}

GuardianSummon::~GuardianSummon()
{}

void GuardianSummon::Load(CreatureProperties const* properties_, Unit* pOwner, LocationVector & position, uint32 spellid, int32 pSummonslot)
{
    Summon::Load(properties_, pOwner, position, spellid, pSummonslot);

    SetPowerType(POWER_TYPE_MANA);
    SetMaxPower(POWER_TYPE_MANA, GetMaxPower(POWER_TYPE_MANA) + 28 + 10 * getLevel());
    SetPower(POWER_TYPE_MANA, GetPower(POWER_TYPE_MANA) + 28 + 10 * getLevel());
    setLevel(pOwner->getLevel());
    SetMaxHealth(GetMaxHealth() + 28 + 30 * getLevel());
    SetHealth(GetMaxHealth());
    SetType(CREATURE_TYPE_GUARDIAN);

    m_aiInterface->Init(this, AI_SCRIPT_PET, Movement::WP_MOVEMENT_SCRIPT_NONE, pOwner);
    m_aiInterface->SetUnitToFollow(pOwner);
    m_aiInterface->SetFollowDistance(3.0f);

    m_noRespawn = true;
}

void GuardianSummon::OnPushToWorld()
{
    Summon::OnPushToWorld();
}

void GuardianSummon::OnPreRemoveFromWorld()
{
    Summon::OnPreRemoveFromWorld();
}
