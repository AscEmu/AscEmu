/*
 * AscEmu Framework based on ArcEmu MMORPG Server
 * Copyright (C) 2014-2016 AscEmu Team <http://www.ascemu.org>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */


#include "StdAfx.h"
#include "SpellManagement/SpellCustomizations.hpp"

initialiseSingleton(SpellCustomizations);


///\brief: This file includes all setted custom values and/or spell.dbc values (overwrite)
/// Set the values you want based on spell Id (Do not set your values based on some text!)

SpellCustomizations::SpellCustomizations() {}
SpellCustomizations::~SpellCustomizations() {}

void SpellCustomizations::StartSpellCustomization()
{
    Log.Debug("SpellCustomizations::StartSpellCustomization", "Successfull started");

    LoadSpellRanks();

    uint32 spellCount = dbcSpell.GetNumRows();

    for (uint32 spell_row = 0; spell_row < spellCount; spell_row++)
    {
        auto spellentry = dbcSpell.LookupEntry(spell_row);
        if (spellentry != nullptr)
        {
            //Load spell overwrite functions
            //Load spell specific custom functions

            LoadCustomFlags(spellentry);
        }
    }
}

void SpellCustomizations::LoadSpellRanks()
{
    uint32 spell_rank_count = 0;

    QueryResult* result = WorldDatabase.Query("SELECT spell_id, rank FROM spell_ranks");
    if (result != NULL)
    {
        do
        {
            uint32 spell_id = result->Fetch()[0].GetUInt32();
            uint32 rank = result->Fetch()[1].GetUInt32();

            SpellEntry* spell_entry = dbcSpell.LookupEntry(spell_id);
            if (spell_entry != nullptr)
            {
                spell_entry->custom_RankNumber = rank;
                ++spell_rank_count;
            }
            else
            {
                Log.Error("SpellCustomizations::LoadSpellRanks", "your spell_ranks table includes an invalid spell %u.", spell_id);
                continue;
            }

        } while (result->NextRow());
        delete result;
    }

    if (spell_rank_count > 0)
    {
        Log.Success("SpellCustomizations::LoadSpellRanks", "Loaded %u custom_RankNumbers from spell_ranks table", spell_rank_count);
    }
    else
    {
        Log.Debug("SpellCustomizations::LoadSpellRanks", "Your spell_ranks table is empty!");
    }

}

void SpellCustomizations::LoadCustomFlags(SpellEntry* spell_entry)
{
    // Currently only set for 781 Disengage
    if (spell_entry->Id != 781)
    {
        return;
    }
    else
    {
        spell_entry->CustomFlags = CUSTOM_FLAG_SPELL_REQUIRES_COMBAT;
    }
}
