/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "../../StdAfx.h"
#include "Units/Summons/PossessedSummon.h"

PossessedSummon::PossessedSummon(uint64_t GUID) : Summon(GUID) {}

PossessedSummon::~PossessedSummon() {}

void PossessedSummon::Load(CreatureProperties const* properties_, Unit* pOwner, LocationVector & position, uint32_t spellid, int32_t pSummonslot)
{
    Summon::Load(properties_, pOwner, position, spellid, pSummonslot);

    setLevel(pOwner->getLevel());
    setAItoUse(false);
    m_aiInterface->StopMovement(0);
}

void PossessedSummon::OnPushToWorld()
{
    Summon::OnPushToWorld();
}

void PossessedSummon::OnPreRemoveFromWorld()
{
    Summon::OnPreRemoveFromWorld();
}
