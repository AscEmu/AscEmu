/*
 * AscEmu Framework based on ArcEmu MMORPG Server
 * Copyright (c) 2014-2017 AscEmu Team <http://www.ascemu.org/>
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
    for (auto itr : summonslots)
    {
        Unit* u = itr;
        if (u != nullptr)
            u->Delete();
    }
    std::fill(summonslots.begin(), summonslots.end(), reinterpret_cast<Unit*>(NULL));
}

void SummonHandler::RemoveAllSummons()
{
    for (auto itr : guardians)
    {
        Unit* g = itr; 
        g->Delete();
    }
    guardians.clear();

    ExpireSummonsInSlot();
}

void SummonHandler::GetSummonSlotSpellIDs(std::vector< uint32 > &spellids)
{
    for (auto itr : summonslots)
    {
        Unit* u = itr;
        if (u != nullptr)
            if (u->GetCreatedBySpell() != 0)
                spellids.push_back(u->GetCreatedBySpell());
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
    for (auto itr : guardians)
    {
        if (itr != nullptr && itr->GetEntry() == entry)
            return itr;
    }
    for (auto itr : summonslots)
    {
        if (itr != nullptr && itr->GetEntry() == entry)
            return itr;
    }
    return NULL;
}

void SummonHandler::SetPvPFlags()
{
    for (auto itr : guardians)
        itr->SetPvPFlag();

    for (auto itr : summonslots)
    {
        Unit* u = itr;
        if (u != nullptr)
            u->SetPvPFlag();
    }
}

void SummonHandler::SetFFAPvPFlags()
{
    for (auto itr : guardians)
        itr->SetFFAPvPFlag();

    for (auto itr : summonslots)
    {
        Unit* u = itr;
        if (u != nullptr)
            u->SetFFAPvPFlag();
    }
}

void SummonHandler::SetSanctuaryFlags()
{
    for (auto itr : guardians)
        itr->SetSanctuaryFlag();

    for (auto itr : summonslots)
    {
        Unit* u = itr;
        if (u != nullptr)
            u->SetSanctuaryFlag();
    }
}

void SummonHandler::RemovePvPFlags()
{
    for (auto itr : guardians)
        itr->RemovePvPFlag();

    for (auto itr : summonslots)
    {
        Unit* u = itr;
        if (u != nullptr)
            u->RemovePvPFlag();
    }
}

void SummonHandler::RemoveFFAPvPFlags()
{
    for (auto itr : guardians)
        itr->RemoveFFAPvPFlag();

    for (auto itr : summonslots)
    {
        Unit* u = itr;
        if (u != nullptr)
            u->RemoveFFAPvPFlag();
    }
}

void SummonHandler::RemoveSanctuaryFlags()
{
    for (auto itr : guardians)
        itr->RemoveSanctuaryFlag();

    for (auto itr : summonslots)
    {
        Unit* u = itr;
        if (u != nullptr)
            u->RemoveSanctuaryFlag();
    }
}
