/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "../../StdAfx.h"
#include "Units/Summons/WildSummon.h"

WildSummon::WildSummon(uint64_t GUID) : Summon(GUID) {}

WildSummon::~WildSummon() {}

void WildSummon::Load(CreatureProperties const* properties_, Unit* pOwner, LocationVector & position, uint32_t spellid, int32_t pSummonslot)
{
    Summon::Load(properties_, pOwner, position, spellid, pSummonslot);

    setLevel(pOwner->getLevel());
}

void WildSummon::OnPushToWorld()
{
    Summon::OnPushToWorld();
}

void WildSummon::OnPreRemoveFromWorld()
{
    Summon::OnPreRemoveFromWorld();
}
