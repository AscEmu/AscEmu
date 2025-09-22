/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "SummonHandler.hpp"
#include "Logging/Logger.hpp"
#include "Objects/Units/Creatures/AIInterface.h"
#include "Objects/Units/Creatures/Pet.h"
#include "Objects/Units/Creatures/Summons/Summon.hpp"
#include "Objects/Units/Players/Player.hpp"
#include "Storage/WDB/WDBStructures.hpp"

SummonHandler::SummonHandler(Unit* owner) : m_owner(owner)
{ }

SummonHandler::~SummonHandler()
{
    removeAllSummons();
}

// Needed to avoid implicit operator generation in header without knowing object size
SummonHandler& SummonHandler::operator=(SummonHandler const&) = default;

void SummonHandler::addSummonToHandler(Summon* summon)
{
    if (summon == nullptr || summon->getUnitOwner() != m_owner)
        return;

    if (summon->isPet())
    {
        m_summons.push_back(summon);

        // Check if current pet needs to be updated
        if (m_pet == nullptr || (m_pet->isPermanentSummon() || summon->isPermanentSummon() || m_pet->getEntry() != summon->getEntry()))
        {
            m_pet = dynamic_cast<Pet*>(summon);
            if (m_pet != nullptr)
            {
                m_owner->setSummonGuid(m_pet->getGuid());
                m_pet->sendSpellsToController(m_owner, m_pet->getTimeLeft());
            }
        }
        return;
    }

    // Do not add scripted summons to handler
    if (summon->getSummonProperties() == nullptr)
        return;

    switch (summon->getSummonProperties()->Slot)
    {
        // Not handled
        case SUMMON_SLOT_NONE:
            break;
        case SUMMON_SLOT_TOTEM_FIRE:
        case SUMMON_SLOT_TOTEM_EARTH:
        case SUMMON_SLOT_TOTEM_WATER:
        case SUMMON_SLOT_TOTEM_AIR:
        case SUMMON_SLOT_MINIPET:
        case SUMMON_SLOT_QUEST:
        {
            const uint8_t slot = static_cast<uint8_t>(summon->getSummonProperties()->Slot) - 1;

            // Send summon to totem bar
            if (summon->getSummonProperties()->Slot != SUMMON_SLOT_MINIPET)
            {
                if (auto* const playerUnit = m_owner->isPlayer() ? dynamic_cast<Player*>(m_owner) : nullptr)
                    playerUnit->sendTotemCreatedPacket(slot, summon->getGuid(), summon->getTimeLeft(), summon->getCreatedBySpellId());
            }

            // Remove old summon if exists
            if (auto* const oldSummon = m_summonSlots[slot])
                oldSummon->unSummon();

            m_summonSlots[slot] = summon;
        } break;
        default:
        {
            sLogger.failure("SummonHandler::addSummonToHandler : Unknown pet slot {}", summon->getSummonProperties()->Slot);
        } break;
    }

    m_summons.push_back(summon);
}

void SummonHandler::removeSummonFromHandler(Summon* summon)
{
    if (summon == nullptr || summon->getUnitOwner() != m_owner)
        return;

    if (summon->isPet())
    {
        std::erase(m_summons, summon);

        if (m_pet != nullptr && m_pet->getGuid() == summon->getGuid())
        {
            // Current pet was removed, try find another controllable pet
            auto foundControllablePet = false;
            if (!m_summons.empty())
            {
                Summon* newCurrentPet = nullptr;
                for (const auto& existingSummon : m_summons)
                {
                    if (!existingSummon->isPet() || !existingSummon->isAlive())
                        continue;

                    if (existingSummon->isPermanentSummon())
                    {
                        newCurrentPet = existingSummon;
                        break;
                    }
                    else if (newCurrentPet == nullptr)
                    {
                        // Use possibly this pet but try find permanent pet
                        newCurrentPet = existingSummon;
                    }
                }

                if (newCurrentPet != nullptr)
                {
                    foundControllablePet = true;
                    m_pet = dynamic_cast<Pet*>(newCurrentPet);
                    if (m_pet != nullptr)
                    {
                        m_owner->setSummonGuid(m_pet->getGuid());
                        m_pet->sendSpellsToController(m_owner, m_pet->getTimeLeft());
                    }
                }
            }

            if (!foundControllablePet)
            {
                m_pet = nullptr;
                m_owner->setSummonGuid(0);
                if (const auto plrOwner = m_owner->isPlayer() ? dynamic_cast<Player*>(m_owner) : nullptr)
                    plrOwner->sendEmptyPetSpellList();
            }
        }
        return;
    }

    if (summon->getSummonProperties() == nullptr)
        return;

    switch (summon->getSummonProperties()->Slot)
    {
        // Not handled
        case SUMMON_SLOT_NONE:
            break;
        case SUMMON_SLOT_TOTEM_FIRE:
        case SUMMON_SLOT_TOTEM_EARTH:
        case SUMMON_SLOT_TOTEM_WATER:
        case SUMMON_SLOT_TOTEM_AIR:
        case SUMMON_SLOT_MINIPET:
        case SUMMON_SLOT_QUEST:
        {
            const uint8_t slot = static_cast<uint8_t>(summon->getSummonProperties()->Slot) - 1;
            m_summonSlots[slot] = nullptr;
        } break;
        default:
        {
            sLogger.failure("SummonHandler::removeSummonFromHandler : Unknown pet slot {}", summon->getSummonProperties()->Slot);
        } break;
    }

    std::erase(m_summons, summon);
}

Pet* SummonHandler::getPet() const
{
    return m_pet;
}

std::vector<Summon*> const& SummonHandler::getSummons() const
{
    return m_summons;
}

void SummonHandler::removeAllSummons()
{
    // Also removes m_pet and summons in m_summonSlots
    for (auto itr = m_summons.cbegin(); itr != m_summons.cend();)
    {
        (*itr)->unSummon();
        itr = m_summons.cbegin();
    }
}

void SummonHandler::setPvPFlags(bool set)
{
    for (const auto& summon : m_summons)
    {
        if (set)
            summon->setPvpFlag();
        else
            summon->removePvpFlag();
    }
}

void SummonHandler::setFFAPvPFlags(bool set)
{
    for (const auto& summon : m_summons)
    {
        if (set)
            summon->setFfaPvpFlag();
        else
            summon->removeFfaPvpFlag();
    }
}

void SummonHandler::setSanctuaryFlags(bool set)
{
    for (const auto& summon : m_summons)
    {
        if (set)
            summon->setSanctuaryFlag();
        else
            summon->removeSanctuaryFlag();
    }
}

void SummonHandler::setPhase(uint8_t command, uint32_t newPhase)
{
    for (const auto& summon : m_summons)
        summon->setPhase(command, newPhase);
}

void SummonHandler::notifyOnOwnerSpeedChange(UnitSpeedType type, float_t rate, bool current)
{
    for (const auto& summon : m_summons)
        summon->setSpeedRate(type, rate, current);
}

void SummonHandler::notifyOnOwnerAttacked(Unit* attacker)
{
    for (const auto& summon : m_summons)
    {
        if (summon->getAIInterface()->getReactState() != REACT_PASSIVE)
        {
            summon->getAIInterface()->onHostileAction(attacker);
            // todo: handle this in pet system
            if (auto* const pet = summon->isPet() ? dynamic_cast<Pet*>(summon) : nullptr)
                pet->HandleAutoCastEvent(AUTOCAST_EVENT_OWNER_ATTACKED);
        }
    }
}

void SummonHandler::notifyOnPetDeath(Summon* pet)
{
    if (pet == nullptr || pet->getUnitOwner() != m_owner)
        return;

    if (m_pet != nullptr && m_pet != pet)
        return;

    // Some spells summon multiple pets like druid's Force of Nature
    // If one of those pets die, summon guid should be updated to one that is alive
    for (const auto& summon : m_summons)
    {
        if (!summon->isPet())
            continue;

        if (summon->getEntry() == pet->getEntry() && summon->isAlive())
        {
            m_pet = dynamic_cast<Pet*>(summon);
            if (m_pet != nullptr)
            {
                m_owner->setSummonGuid(m_pet->getGuid());
                m_pet->sendSpellsToController(m_owner, m_pet->getTimeLeft());
            }
            break;
        }
    }
}

Summon* SummonHandler::getSummonWithEntry(uint32_t entry) const
{
    for (const auto& summon : m_summons)
    {
        if (entry == summon->getEntry())
            return summon;
    }

    return nullptr;
}

Summon* SummonHandler::getSummonInSlot(SummonSlot slot) const
{
    if (slot == SUMMON_SLOT_NONE || slot >= MAX_SUMMON_SLOT)
        return nullptr;

    const uint8_t summonSlot = slot - 1;
    return m_summonSlots[summonSlot];
}

bool SummonHandler::hasTotemInSlot(SummonSlot slot) const
{
    if (slot < SUMMON_SLOT_TOTEM_FIRE || slot > SUMMON_SLOT_TOTEM_AIR)
        return false;

    const auto* totem = getSummonInSlot(slot);
    return totem != nullptr && totem->isTotem();
}

void SummonHandler::killAllTotems() const
{
    for (uint8_t i = SUMMON_SLOT_TOTEM_FIRE; i <= SUMMON_SLOT_TOTEM_AIR; ++i)
    {
        auto* const totem = getSummonInSlot(static_cast<SummonSlot>(i));
        if (totem == nullptr || !totem->isTotem())
            continue;

        totem->dieAndDisappearOnExpire();
    }
}
