/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "../../StdAfx.h"
#include "Units/Summons/GuardianSummon.h"
#include "Spell/Definitions/PowerType.h"

GuardianSummon::GuardianSummon(uint64_t GUID) : Summon(GUID)
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
