/*
Copyright (c) 2014-2021 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "../../StdAfx.h"
#include "Units/Creatures/Creature.h"
#include "Units/Summons/Summon.h"
#include "Units/Summons/TotemSummon.h"

Summon::Summon(uint64_t guid, uint32_t duration) : Creature(guid), m_unitOwner(nullptr), m_summonSlot(-1), m_duration(duration) {}

Summon::~Summon() {}

void Summon::Load(CreatureProperties const* creatureProperties, Unit* unitOwner, LocationVector& position, uint32_t spellId, int32_t summonSlot)
{
    ARCEMU_ASSERT(unitOwner != nullptr);

    Creature::Load(creatureProperties, position.x, position.y, position.z, position.o);

    SetFaction(unitOwner->getFactionTemplate());
    Phase(PHASE_SET, unitOwner->GetPhase());
    SetZoneId(unitOwner->GetZoneId());
    setCreatedBySpellId(spellId);
    this->m_summonSlot = summonSlot;

    if (unitOwner->isPvpFlagSet())
        setPvpFlag();
    else
        removePvpFlag();

    if (unitOwner->isFfaPvpFlagSet())
        setFfaPvpFlag();
    else
        removeFfaPvpFlag();

    if (unitOwner->isSanctuaryFlagSet())
        setSanctuaryFlag();
    else
        removeSanctuaryFlag();

    setCreatedByGuid(unitOwner->getGuid());

    if (unitOwner->getSummonedByGuid() == 0)
        setSummonedByGuid(unitOwner->getGuid());
    else
        setSummonedByGuid(unitOwner->getSummonedByGuid());

    this->m_unitOwner = unitOwner;

    if (unitOwner->hasUnitFlags(UNIT_FLAG_PVP_ATTACKABLE))
        addUnitFlags(UNIT_FLAG_PVP_ATTACKABLE);
}

void Summon::unSummon()
{
    // If this summon is summoned by a totem, unsummon the totem also
    if (m_unitOwner->isTotem())
        static_cast<TotemSummon*>(m_unitOwner)->unSummon();

    Despawn(10, 0);
}

uint32_t Summon::getTimeLeft() const
{
    return m_duration;
}

void Summon::setTimeLeft(uint32_t time)
{
    m_duration = time;
}

//////////////////////////////////////////////////////////////////////////////////////////
// Override Object functions
void Summon::OnPushToWorld()
{
    if (!isTotem())
        m_unitOwner->getSummonInterface()->addGuardian(this);

    Creature::OnPushToWorld();
}

void Summon::OnPreRemoveFromWorld()
{
    if (m_unitOwner == nullptr)
        return;

    if (getCreatedBySpellId() != 0)
        m_unitOwner->RemoveAura(getCreatedBySpellId());

    if (!isTotem())
        m_unitOwner->getSummonInterface()->removeGuardian(this, false);

    if (getPlayerOwner() != nullptr)
        getPlayerOwner()->sendDestroyObjectPacket(getGuid());

    m_summonSlot = -1;
    m_unitOwner = nullptr;
}

bool Summon::isSummon() const { return true; }

void Summon::onRemoveInRangeObject(Object* object)
{
    if (m_unitOwner != nullptr && object->getGuid() == m_unitOwner->getGuid())
        unSummon();

    Creature::onRemoveInRangeObject(object);
}

//////////////////////////////////////////////////////////////////////////////////////////
// Override Unit functions
void Summon::Die(Unit* pAttacker, uint32 damage, uint32 spellid)
{
    // If this summon is summoned by a totem, unsummon the totem on death
    if (m_unitOwner->isTotem())
        static_cast<TotemSummon*>(m_unitOwner)->unSummon();

    Creature::Die(pAttacker, damage, spellid);

    m_unitOwner->getSummonInterface()->removeGuardian(this, false);

    m_summonSlot = -1;
    m_unitOwner = nullptr;
}

//////////////////////////////////////////////////////////////////////////////////////////
// Misc
bool Summon::isSummonedToSlot() const
{
    return m_summonSlot != -1;
}

Player* Summon::getPlayerOwner()
{
    if (m_unitOwner != nullptr && m_unitOwner->isPlayer())
        return dynamic_cast<Player*>(m_unitOwner);

    return nullptr;
}
