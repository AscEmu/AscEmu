/*
Copyright (c) 2014-2021 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Units/Summons/GuardianSummon.h"
#include "Spell/Definitions/PowerType.hpp"

GuardianSummon::GuardianSummon(uint64_t GUID, uint32_t duration) : Summon(GUID, duration)
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

    m_aiInterface->Init(this, AI_SCRIPT_PET, pOwner);
    m_aiInterface->setPetOwner(pOwner);

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
