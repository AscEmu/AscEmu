/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "../../StdAfx.h"
#include "Units/Summons/CompanionSummon.h"

CompanionSummon::CompanionSummon(uint64_t GUID) : Summon(GUID) {}

CompanionSummon::~CompanionSummon() {}

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

