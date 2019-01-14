/*
 * AscEmu Framework based on ArcEmu MMORPG Server
 * Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
 * Copyright (C) 2008-2012 ArcEmu Team <http://www.ArcEmu.org/>
 * Copyright (C) 2005-2007 Ascent Team
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "StdAfx.h"
#include "SummonHandler.h"
#include "Units/Unit.h"


SummonHandler::SummonHandler()
{
    std::fill(summonslots.begin(), summonslots.end(), reinterpret_cast<Unit*>(NULL));
}

SummonHandler::~SummonHandler()
{
    RemoveAllSummons();
}

void SummonHandler::AddSummon(Unit* summon)
{
    guardians.insert(summon);
}

void SummonHandler::AddSummonToSlot(Unit* summon, uint8 slot)
{
    if (summonslots[slot] != NULL)
        summonslots[slot]->Delete();

    summonslots[slot] = summon;
}

void SummonHandler::RemoveSummon(Unit* summon)
{
    std::set< Unit* >::iterator itr = guardians.find(summon);
    if (itr != guardians.end())
        guardians.erase(itr);
}

void SummonHandler::RemoveSummonFromSlot(uint8 slot, bool del)
{
    if (summonslots[slot] != NULL)
    {
        if (del)
            summonslots[slot]->Delete();

        summonslots[slot] = NULL;
    }
}

void SummonHandler::ExpireSummonsInSlot()
{
    for (std::array< Unit*, SUMMON_SLOTS >::iterator itr = summonslots.begin(); itr != summonslots.end(); ++itr)
    {
        Unit* u = *itr;

        if (u != NULL)
            u->Delete();
    }
    std::fill(summonslots.begin(), summonslots.end(), reinterpret_cast<Unit*>(NULL));
}

void SummonHandler::RemoveAllSummons()
{
    for (std::set< Unit* >::iterator itr = guardians.begin(); itr != guardians.end();)
    {
        Unit* g = *itr;
        ++itr;
        g->Delete();
    }
    guardians.clear();

    ExpireSummonsInSlot();
}

void SummonHandler::GetSummonSlotSpellIDs(std::vector< uint32 > &spellids)
{
    for (std::array< Unit*, SUMMON_SLOTS >::iterator itr = summonslots.begin(); itr != summonslots.end(); ++itr)
    {
        Unit* u = (*itr);

        if (u != NULL)
            if (u->getCreatedBySpellId() != 0)
                spellids.push_back(u->getCreatedBySpellId());
    }
}

bool SummonHandler::HasSummonInSlot(uint8 slot)
{
    if (summonslots[slot] != 0)
        return true;
    else
        return false;
}

Unit* SummonHandler::GetSummonInSlot(uint8 slot)
{
    return summonslots[slot];
}

Unit* SummonHandler::GetSummonWithEntry(uint32 entry)
{
    for (std::set< Unit* >::iterator itr = guardians.begin(); itr != guardians.end(); ++itr)
        if ((*itr) != NULL && (*itr)->getEntry() == entry)
            return (*itr);

    for (std::array< Unit*, SUMMON_SLOTS >::iterator itr = summonslots.begin(); itr != summonslots.end(); ++itr)
    {
        if ((*itr) != NULL && (*itr)->getEntry() == entry)
            return (*itr);
    }
    return NULL;
}

void SummonHandler::SetPvPFlags()
{
    for (std::set< Unit* >::iterator itr = guardians.begin(); itr != guardians.end(); ++itr)
        (*itr)->setPvpFlag();

    for (std::array< Unit*, SUMMON_SLOTS >::iterator itr = summonslots.begin(); itr != summonslots.end(); ++itr)
    {
        Unit* u = (*itr);
        if (u != NULL)
            u->setPvpFlag();
    }
}

void SummonHandler::SetFFAPvPFlags()
{
    for (std::set< Unit* >::iterator itr = guardians.begin(); itr != guardians.end(); ++itr)
        (*itr)->setFfaPvpFlag();

    for (std::array< Unit*, SUMMON_SLOTS >::iterator itr = summonslots.begin(); itr != summonslots.end(); ++itr)
    {
        Unit* u = (*itr);
        if (u != NULL)
            u->setFfaPvpFlag();
    }
}

void SummonHandler::SetSanctuaryFlags()
{
    for (std::set< Unit* >::iterator itr = guardians.begin(); itr != guardians.end(); ++itr)
        (*itr)->setSanctuaryFlag();

    for (std::array< Unit*, SUMMON_SLOTS >::iterator itr = summonslots.begin(); itr != summonslots.end(); ++itr)
    {
        Unit* u = (*itr);
        if (u != NULL)
            u->setSanctuaryFlag();
    }
}

void SummonHandler::RemovePvPFlags()
{
    for (std::set< Unit* >::iterator itr = guardians.begin(); itr != guardians.end(); ++itr)
        (*itr)->removePvpFlag();

    for (std::array< Unit*, SUMMON_SLOTS >::iterator itr = summonslots.begin(); itr != summonslots.end(); ++itr)
    {
        Unit* u = (*itr);
        if (u != NULL)
            u->removePvpFlag();
    }
}

void SummonHandler::RemoveFFAPvPFlags()
{
    for (std::set< Unit* >::iterator itr = guardians.begin(); itr != guardians.end(); ++itr)
        (*itr)->removeFfaPvpFlag();

    for (std::array< Unit*, SUMMON_SLOTS >::iterator itr = summonslots.begin(); itr != summonslots.end(); ++itr)
    {
        Unit* u = (*itr);
        if (u != NULL)
            u->removeFfaPvpFlag();
    }
}

void SummonHandler::RemoveSanctuaryFlags()
{
    for (std::set< Unit* >::iterator itr = guardians.begin(); itr != guardians.end(); ++itr)
        (*itr)->removeSanctuaryFlag();

    for (std::array< Unit*, SUMMON_SLOTS >::iterator itr = summonslots.begin(); itr != summonslots.end(); ++itr)
    {
        Unit* u = (*itr);
        if (u != NULL)
            u->removeSanctuaryFlag();
    }
}
