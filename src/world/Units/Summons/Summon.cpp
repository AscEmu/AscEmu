/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "../../StdAfx.h"
#include "Units/Creatures/Creature.h"
#include "Units/Summons/Summon.h"

Summon::Summon(uint64_t guid) : Creature(guid), m_unitOwner(nullptr), m_summonSlot(-1) {}

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

//////////////////////////////////////////////////////////////////////////////////////////
// Override Object functions

void Summon::OnPushToWorld()
{
    if (m_summonSlot != -1)
        m_unitOwner->summonhandler.AddSummonToSlot(this, static_cast<uint8_t>(m_summonSlot));
    else
        m_unitOwner->summonhandler.AddSummon(this);

    Creature::OnPushToWorld();
}

void Summon::OnPreRemoveFromWorld()
{
    if (m_unitOwner == nullptr)
        return;

    if (getCreatedBySpellId() != 0)
        m_unitOwner->RemoveAura(getCreatedBySpellId());

    if (m_summonSlot != -1)
        m_unitOwner->summonhandler.RemoveSummonFromSlot(static_cast<uint8_t>(m_summonSlot), false);
    else
        m_unitOwner->summonhandler.RemoveSummon(this);

    m_summonSlot = -1;
    m_unitOwner = nullptr;

    SendDestroyObject();
}

bool Summon::isSummon() const { return true; }

void Summon::onRemoveInRangeObject(Object* object)
{
    if (m_unitOwner != nullptr && object->getGuid() == m_unitOwner->getGuid())
        Despawn(1, 0);

    Creature::onRemoveInRangeObject(object);
}

//////////////////////////////////////////////////////////////////////////////////////////
// Override Unit functions
void Summon::Die(Unit* pAttacker, uint32 damage, uint32 spellid)
{
    if (m_unitOwner->isTotem())
        m_unitOwner->Die(pAttacker, damage, spellid);

    Creature::Die(pAttacker, damage, spellid);

    if (m_summonSlot != -1)
        m_unitOwner->summonhandler.RemoveSummonFromSlot(static_cast<uint8_t>(m_summonSlot), false);
    else
        m_unitOwner->summonhandler.RemoveSummon(this);

    m_summonSlot = -1;
    m_unitOwner = nullptr;
}

//////////////////////////////////////////////////////////////////////////////////////////
// Misc
bool Summon::isSummonedToSlot() const
{
    return m_summonSlot != -1;
}

Object* Summon::getPlayerOwner()
{
    if (m_unitOwner && m_unitOwner->isPlayer())
        return m_unitOwner;

    return nullptr;
}
