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
#include "Units/Summons/CompanionSummon.h"

CompanionSummon::CompanionSummon(uint64 GUID) : Summon(GUID)
{ }

CompanionSummon::~CompanionSummon()
{ }

void CompanionSummon::Load(CreatureProperties const* properties_, Unit* companionOwner, LocationVector & position, uint32_t spellid, int32_t summonSlot)
{
    Summon::Load(properties_, companionOwner, position, spellid, summonSlot);

    SetFaction(35);
    setLevel(1);
    m_aiInterface->Init(this, AI_SCRIPT_PET, Movement::WP_MOVEMENT_SCRIPT_NONE, companionOwner);
    m_aiInterface->SetUnitToFollow(companionOwner);
    m_aiInterface->SetUnitToFollowAngle(-M_PI_FLOAT / 2);
    m_aiInterface->SetFollowDistance(3.0f);
    m_aiInterface->setMeleeDisabled(true);
    bInvincible = true;

    removePvpFlag();
    removeFfaPvpFlag();
}

void CompanionSummon::OnPushToWorld()
{
    Summon::OnPushToWorld();
}

void CompanionSummon::OnPreRemoveFromWorld()
{
    Summon::OnPreRemoveFromWorld();
}

