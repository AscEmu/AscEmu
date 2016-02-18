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
    LoadSpellProcAuto();

    uint32 spellCount = dbcSpell.GetNumRows();

    for (uint32 spell_row = 0; spell_row < spellCount; spell_row++)
    {
        auto spellentry = dbcSpell.LookupEntry(spell_row);
        if (spellentry != nullptr)
        {
            //Set spell overwrites (effect based)
            SetEffectAmplitude(spellentry);
            SetAuraFactoryFunc(spellentry);

            // Set custom values (effect based)
            SetMeleeSpellBool(spellentry);
            SetRangedSpellBool(spellentry);

            // Set custom values (spell based)
            SetCustomFlags(spellentry);
            SetBuffGrouRelation(spellentry);
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

void SpellCustomizations::LoadSpellProcAuto()
{
    uint32 spell_proc_count = 0;

    QueryResult* result = WorldDatabase.Query("SELECT spellID, ProcFlag, TargetSelf FROM spell_proc_auto");
    if (result != NULL)
    {
        do
        {
            uint32 spell_id = result->Fetch()[0].GetUInt32();
            uint32 proc_flags = result->Fetch()[1].GetUInt32();
            bool target_self = result->Fetch()[2].GetBool();

            SpellEntry* spell_entry = dbcSpell.LookupEntry(spell_id);
            if (spell_entry != nullptr)
            {
                if (target_self)
                    proc_flags |= static_cast<uint32>(PROC_TARGET_SELF);

                spell_entry->procFlags = proc_flags;

                ++spell_proc_count;
            }
            else
            {
                Log.Error("SpellCustomizations::LoadSpellProcAuto", "your spell_proc_auto table includes an invalid spell %u.", spell_id);
                continue;
            }

        } while (result->NextRow());
        delete result;
    }

    if (spell_proc_count > 0)
    {
        Log.Success("SpellCustomizations::LoadSpellProcAuto", "Loaded %u procFlags from spell_proc_auto table", spell_proc_count);
    }
    else
    {
        Log.Debug("SpellCustomizations::LoadSpellProcAuto", "Your spell_proc_auto table is empty!");
    }

}

///Fix if it is a periodic trigger with amplitude = 0, to avoid division by zero
void SpellCustomizations::SetEffectAmplitude(SpellEntry* spell_entry)
{
    bool spell_effect_amplitude_loaded = false;

    for (uint8 y = 0; y < 3; y++)
    {
        if (!spell_entry->Effect[y] == SPELL_EFFECT_APPLY_AURA)
        {
            continue;
        }
        else
        {
            if (!spell_entry->EffectApplyAuraName[y] == SPELL_AURA_PERIODIC_TRIGGER_SPELL && !spell_entry->EffectApplyAuraName[y] == SPELL_AURA_PERIODIC_TRIGGER_SPELL_WITH_VALUE)
            {
                continue;
            }
            else
            {
                if (spell_entry->EffectAmplitude[y] == 0)
                {
                    spell_entry->EffectAmplitude[y] = 1000;

                    spell_effect_amplitude_loaded = true;
                }
            }
        }
    }

    if (spell_effect_amplitude_loaded)
    {
        Log.Debug("SpellCustomizations::SetEffectAmplitude", "EffectAmplitude applied Spell - %s (%u)", spell_entry->Name, spell_entry->Id);
    }
}

void SpellCustomizations::SetAuraFactoryFunc(SpellEntry* spell_entry)
{
    bool spell_aura_factory_functions_loaded = false;

    for (uint8 y = 0; y < 3; y++)
    {
        if (!spell_entry->Effect[y] == SPELL_EFFECT_APPLY_AURA)
        {
            continue;
        }
        else
        {
            if (spell_entry->EffectApplyAuraName[y] == SPELL_AURA_SCHOOL_ABSORB && spell_entry->AuraFactoryFunc == NULL)
            {
                spell_entry->AuraFactoryFunc = (void * (*)) &AbsorbAura::Create;

                spell_aura_factory_functions_loaded = true;
            }
        }
    }

    if (spell_aura_factory_functions_loaded)
    {
        Log.Debug("SpellCustomizations::SetAuraFactoryFunc", "AuraFactoryFunc definitions applied to Spell - %s (%u)", spell_entry->Name, spell_entry->Id);
    }
}

void SpellCustomizations::SetMeleeSpellBool(SpellEntry* spell_entry)
{
    for (uint8 z = 0; z < 3; z++)
    {
        if (spell_entry->Effect[z] == SPELL_EFFECT_SCHOOL_DAMAGE && spell_entry->Spell_Dmg_Type == SPELL_DMG_TYPE_MELEE)
        {
            spell_entry->custom_is_melee_spell = true;
            continue;
        }

        switch (spell_entry->Effect[z])
        {
            case SPELL_EFFECT_WEAPON_DAMAGE_NOSCHOOL:
            case SPELL_EFFECT_WEAPON_DAMAGE:
            case SPELL_EFFECT_WEAPON_PERCENT_DAMAGE:
            case SPELL_EFFECT_DUMMYMELEE:
            {
                spell_entry->custom_is_melee_spell = true;
            } break;
            default:
                continue;
        }
    }

    if (spell_entry->custom_is_melee_spell)
    {
        Log.Debug("SpellCustomizations::SetMeleeSpellBool", "custom_is_melee_spell = true for Spell - %s (%u)", spell_entry->Name, spell_entry->Id);
    }
}

void SpellCustomizations::SetRangedSpellBool(SpellEntry* spell_entry)
{
    for (uint8 z = 0; z < 3; z++)
    {
        if (spell_entry->Effect[z] == SPELL_EFFECT_SCHOOL_DAMAGE && spell_entry->Spell_Dmg_Type == SPELL_DMG_TYPE_RANGED)
        {
            spell_entry->custom_is_ranged_spell = true;
        }
    }

    if (spell_entry->custom_is_ranged_spell)
    {
        Log.Debug("SpellCustomizations::SetRangedSpellBool", "custom_is_ranged_spell = true for Spell - %s (%u)", spell_entry->Name, spell_entry->Id);
    }
}

void SpellCustomizations::SetCustomFlags(SpellEntry* spell_entry)
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

void SpellCustomizations::SetBuffGrouRelation(SpellEntry* spell_entry)
{
    switch (spell_entry->Id)
    {
        // SPELL_HASH_CRUSADER_AURA
        case 32223:

        // SPELL_HASH_FROST_RESISTANCE_AURA
        case 19888:     // Frost Resistance Aura Rank 1
        case 19897:     // Frost Resistance Aura Rank 2
        case 19898:     // Frost Resistance Aura Rank 3
        case 27152:     // Frost Resistance Aura Rank 4
        case 48945:     // Frost Resistance Aura Rank 5

        // SPELL_HASH_FIRE_RESISTANCE_AURA
        case 19891:     // Fire Resistance Aura Rank 1
        case 19899:     // Fire Resistance Aura Rank 2
        case 19900:     // Fire Resistance Aura Rank 3
        case 27153:     // Fire Resistance Aura Rank 4
        case 48947:     // Fire Resistance Aura Rank 5

        // SPELL_HASH_SHADOW_RESISTANCE_AURA
        case 19876:     // Shadow Resistance Aura Rank 1
        case 19895:     // Shadow Resistance Aura Rank 2
        case 19896:     // Shadow Resistance Aura Rank 3
        case 27151:     // Shadow Resistance Aura Rank 4
        case 48943:     // Shadow Resistance Aura Rank 5

        // SPELL_HASH_CONCENTRATION_AURA
        case 19746:

        // SPELL_HASH_RETRIBUTION_AURA
        case 7294:      // Retribution Aura Rank 1
        case 8990:      // Retribution Aura Rank 1
        case 10298:     // Retribution Aura Rank 2
        case 10299:     // Retribution Aura Rank 3
        case 10300:     // Retribution Aura Rank 4
        case 10301:     // Retribution Aura Rank 5
        case 13008:
        case 27150:     // Retribution Aura Rank 6
        case 54043:     // Retribution Aura Rank 7

        // SPELL_HASH_DEVOTION_AURA
        case 465:       // Devotion Aura Rank 1
        case 643:       // Devotion Aura Rank 3
        case 1032:      // Devotion Aura Rank 5
        case 8258:
        case 10290:     // Devotion Aura Rank 2
        case 10291:     // Devotion Aura Rank 4
        case 10292:     // Devotion Aura Rank 6
        case 10293:     // Devotion Aura Rank 7
        case 17232:
        case 27149:     // Devotion Aura Rank 8
        case 41452:
        case 48941:     // Devotion Aura Rank 9
        case 48942:     // Devotion Aura Rank 10
        case 52442:
        case 57740:
        case 58944:
        {
            spell_entry->custom_BGR_one_buff_from_caster_on_self = SPELL_TYPE2_PALADIN_AURA;
        } break;

        // SPELL_HASH_BLOOD_PRESENCE
        case 48266:
        case 50475:
        case 50689:
        case 54476:
        case 55212:

        // SPELL_HASH_FROST_PRESENCE
        case 48263:
        case 61261:

        // SPELL_HASH_UNHOLY_PRESENCE
        case 48265:
        case 49772:
        case 55222:
        {
            spell_entry->custom_BGR_one_buff_from_caster_on_self = SPELL_TYPE3_DEATH_KNIGHT_AURA;
        } break;

        // SPELL_HASH_BEACON_OF_LIGHT
        case 53563:
        case 53652:
        case 53653:
        case 53654:
        {
            spell_entry->custom_BGR_one_buff_on_target = SPELL_TYPE2_PALADIN_AURA;
        } break;
        default:
            break;
    }
}
