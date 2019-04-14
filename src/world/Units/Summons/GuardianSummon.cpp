/*
 * AscEmu Framework based on ArcEmu MMORPG Server
 * Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
 * Copyright (C) 2008-2011 ArcEmu Team <http://www.ArcEmu.org/>
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

void GuardianSummon::Load(CreatureProperties const* properties_, Unit* pOwner, LocationVector & position, uint32_t spellid, int32_t pSummonslot)
{
    Summon::Load(properties_, pOwner, position, spellid, pSummonslot);

    setPowerType(POWER_TYPE_MANA);
    setMaxPower(POWER_TYPE_MANA, getMaxPower(POWER_TYPE_MANA) + 28 + 10 * getLevel());
    setPower(POWER_TYPE_MANA, getPower(POWER_TYPE_MANA) + 28 + 10 * getLevel());
    setLevel(pOwner->getLevel());
    setMaxHealth(getMaxHealth() + 28 + 30 * getLevel());
    setHealth(getMaxHealth());
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
