/*
Copyright (c) 2014-2023 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "SummonHandler.hpp"
#include "Map/Maps/WorldMap.hpp"
#include "Objects/Units/Creatures/Summons/Summon.hpp"

SummonHandler::SummonHandler()
{
    for (uint8_t i = 0; i < MAX_SUMMON_SLOT; ++i)
        m_SummonSlot[i] = 0;
}

SummonHandler::~SummonHandler()
{
    removeAllSummons();
}

void SummonHandler::removeAllSummons(bool totemsOnly/* = false*/)
{
    if (!m_Owner || !m_Owner->getWorldMap())
        return;

    for (uint8 i = 0; i < MAX_SUMMON_SLOT; ++i)
    {
        if (m_SummonSlot[i])
        {
            if (auto* const summon = m_Owner->getWorldMap()->getCreature(m_SummonSlot[i]))
            {
                if (summon->isTotem())
                    dynamic_cast<TotemSummon*>(summon)->unSummon();
                else if (!totemsOnly)
                    dynamic_cast<Summon*>(summon)->unSummon();
            }
        }
    }
}

void SummonHandler::setPvPFlags(bool set)
{
    if (!m_Owner || !m_Owner->getWorldMap())
        return;

    for (uint8_t i = 0; i < MAX_SUMMON_SLOT; ++i)
    {
        if (Creature* summon = m_Owner->getWorldMap()->getCreature(m_SummonSlot[i]))
        {
            if (set)
                summon->setPvpFlag();
            else
                summon->removePvpFlag();
        }
    }
}

void SummonHandler::setFFAPvPFlags(bool set)
{
    if (!m_Owner || !m_Owner->getWorldMap())
        return;

    for (uint8_t i = 0; i < MAX_SUMMON_SLOT; ++i)
    {
        if (m_SummonSlot[i])
        {
            if (Creature* summon = m_Owner->getWorldMap()->getCreature(m_SummonSlot[i]))
            {
                if (set)
                    summon->setFfaPvpFlag();
                else
                    summon->removeFfaPvpFlag();
            }
        }
    }
}

void SummonHandler::setSanctuaryFlags(bool set)
{
    if (!m_Owner || !m_Owner->getWorldMap())
        return;

    for (uint8_t i = 0; i < MAX_SUMMON_SLOT; ++i)
    {
        if (m_SummonSlot[i])
        {
            if (Creature* summon = m_Owner->getWorldMap()->getCreature(m_SummonSlot[i]))
            {
                if (set)
                    summon->setSanctuaryFlag();
                else
                    summon->removeSanctuaryFlag();
            }
        }
    }
}

bool SummonHandler::hasTotemInSlot(SummonSlot slot) const
{
    if (slot < SUMMON_SLOT_TOTEM_FIRE || slot > SUMMON_SLOT_TOTEM_AIR)
        return false;

    return m_SummonSlot[slot] != 0 ? true : false;
}

TotemSummon* SummonHandler::getTotemInSlot(SummonSlot slot) const
{
    if (!m_Owner || !m_Owner->getWorldMap())
        return nullptr;

    if (slot < SUMMON_SLOT_TOTEM_FIRE || slot > SUMMON_SLOT_TOTEM_AIR)
        return nullptr;

    return static_cast<TotemSummon*>(m_Owner->getWorldMap()->getCreature(m_SummonSlot[slot]));
}

Summon* SummonHandler::getSummonWithEntry(uint32_t entry) const
{
    if (!m_Owner || !m_Owner->getWorldMap())
        return nullptr;

    for (uint8_t i = 0; i < MAX_SUMMON_SLOT; ++i)
    {
        if (Creature* summon = m_Owner->getWorldMap()->getCreature(m_SummonSlot[i]))
        {
            if (entry == summon->getEntry())
                return static_cast<Summon*>(summon);
        }
    }

    return nullptr;
}

void SummonHandler::getTotemSpellIds(std::vector<uint32_t>& spellIds)
{
    if (!m_Owner || !m_Owner->getWorldMap())
        return;

    for (uint8_t i = 0; i < MAX_SUMMON_SLOT; ++i)
    {
        if (Creature* summon = m_Owner->getWorldMap()->getCreature(m_SummonSlot[i]))
        {
            if (summon->isTotem() && summon->getCreatedBySpellId())
            {
                spellIds.push_back(summon->getCreatedBySpellId());
            }
        }
    }
}

void SummonHandler::setUnitOwner(Unit* owner)
{
    m_Owner = owner;
}
