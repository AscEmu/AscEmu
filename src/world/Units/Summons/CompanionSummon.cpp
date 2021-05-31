/*
Copyright (c) 2014-2021 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "../../StdAfx.h"
#include "Units/Summons/CompanionSummon.h"

CompanionSummon::CompanionSummon(uint64_t GUID, uint32_t duration) : Summon(GUID, duration) {}

CompanionSummon::~CompanionSummon() {}

void CompanionSummon::Load(CreatureProperties const* properties_, Unit* companionOwner, LocationVector & position, uint32_t spellid, int32_t summonSlot)
{
    Summon::Load(properties_, companionOwner, position, spellid, summonSlot);

    SetFaction(35);
    setLevel(1);

    m_aiInterface->Init(this, AI_SCRIPT_PET, companionOwner);
    m_aiInterface->setPetOwner(companionOwner);
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

