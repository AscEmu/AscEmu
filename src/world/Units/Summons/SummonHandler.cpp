/*
Copyright (c) 2014-2020 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "SummonHandler.h"

#include "Units/Summons/Summon.h"
#include "Units/Summons/TotemSummon.h"

SummonHandler::SummonHandler()
{
    for (uint8_t i = 0; i < MAX_TOTEM_SLOT; ++i)
        _totems[i] = nullptr;

    _guardianPets.clear();
}

SummonHandler::~SummonHandler()
{
    removeAllSummons();
}

void SummonHandler::addGuardian(Summon* summon)
{
    _guardianPets.insert(summon);
}

void SummonHandler::removeGuardian(Summon* summon, bool deleteObject)
{
    auto itr = _guardianPets.find(summon);
    if (itr == _guardianPets.end())
        return;

    if (deleteObject)
        (*itr)->Delete();

    _guardianPets.erase(itr);
}

void SummonHandler::addTotem(TotemSummon* totem, TotemSlots slot)
{
    if (slot >= MAX_TOTEM_SLOT)
        return;

    _totems[slot] = totem;
}

void SummonHandler::removeTotem(TotemSummon* totem, bool deleteObject)
{
    for (uint8_t i = 0; i < MAX_TOTEM_SLOT; ++i)
    {
        if (_totems[i] == totem)
        {
            if (deleteObject)
                totem->Delete();

            _totems[i] = nullptr;
            break;
        }
    }
}

void SummonHandler::removeAllSummons(bool totemsOnly/* = false*/)
{
    if (!totemsOnly)
    {
        for (auto itr = _guardianPets.begin(); itr != _guardianPets.end();)
        {
            auto guardian = *itr;
            ++itr;
            guardian->Delete();
        }

        _guardianPets.clear();
    }

    for (uint8_t i = 0; i < MAX_TOTEM_SLOT; ++i)
    {
        if (_totems[i] == nullptr)
            continue;

        _totems[i]->Delete();
        _totems[i] = nullptr;
    }
}

void SummonHandler::setPvPFlags(bool set)
{
    for (const auto& guardian : _guardianPets)
    {
        if (set)
            guardian->setPvpFlag();
        else
            guardian->removePvpFlag();
    }
        
    for (uint8_t i = 0; i < MAX_TOTEM_SLOT; ++i)
    {
        if (_totems[i] == nullptr)
            continue;

        if (set)
            _totems[i]->setPvpFlag();
        else
            _totems[i]->removePvpFlag();
    }
}

void SummonHandler::setFFAPvPFlags(bool set)
{
    for (const auto& guardian : _guardianPets)
    {
        if (set)
            guardian->setFfaPvpFlag();
        else
            guardian->removeFfaPvpFlag();
    }

    for (uint8_t i = 0; i < MAX_TOTEM_SLOT; ++i)
    {
        if (_totems[i] == nullptr)
            continue;

        if (set)
            _totems[i]->setFfaPvpFlag();
        else
            _totems[i]->removeFfaPvpFlag();
    }
}

void SummonHandler::setSanctuaryFlags(bool set)
{
    for (const auto& guardian : _guardianPets)
    {
        if (set)
            guardian->setSanctuaryFlag();
        else
            guardian->removeSanctuaryFlag();
    }

    for (uint8_t i = 0; i < MAX_TOTEM_SLOT; ++i)
    {
        if (_totems[i] == nullptr)
            continue;

        if (set)
            _totems[i]->setSanctuaryFlag();
        else
            _totems[i]->removeSanctuaryFlag();
    }
}

bool SummonHandler::hasTotemInSlot(TotemSlots slot) const
{
    if (slot >= MAX_TOTEM_SLOT)
        return false;

    return _totems[slot] != nullptr ? true : false;
}

TotemSummon* SummonHandler::getTotemInSlot(TotemSlots slot) const
{
    if (slot >= MAX_TOTEM_SLOT)
        return nullptr;

    return _totems[slot];
}

Summon* SummonHandler::getSummonWithEntry(uint32_t entry) const
{
    for (const auto& guardian : _guardianPets)
    {
        if (guardian->getEntry() == entry)
            return guardian;
    }

    for (uint8_t i = 0; i < MAX_TOTEM_SLOT; ++i)
    {
        const auto totem = _totems[i];
        if (totem == nullptr)
            continue;

        if (totem->getEntry() == entry)
            return totem;
    }

    return nullptr;
}

void SummonHandler::getTotemSpellIds(std::vector<uint32_t>& spellIds)
{
    for (uint8_t i = 0; i < MAX_TOTEM_SLOT; ++i)
    {
        const auto totem = _totems[i];
        if (totem == nullptr)
            continue;
        if (totem->getCreatedBySpellId() == 0)
            continue;

        spellIds.push_back(totem->getCreatedBySpellId());
    }
}

void SummonHandler::update(uint16_t diff)
{
    // Update the duration of summons
    for (const auto& guardian : _guardianPets)
    {
        const auto timeLeft = guardian->getTimeLeft();
        if (timeLeft == 0)
            continue;

        if (diff >= timeLeft)
            guardian->unSummon();
        else
            guardian->setTimeLeft(timeLeft - diff);
    }

    for (uint8_t i = 0; i < MAX_TOTEM_SLOT; ++i)
    {
        const auto totem = _totems[i];
        if (totem == nullptr)
            continue;

        const auto timeLeft = totem->getTimeLeft();
        if (timeLeft == 0)
            continue;

        if (diff >= timeLeft)
            totem->unSummon();
        else
            totem->setTimeLeft(timeLeft - diff);
    }
}