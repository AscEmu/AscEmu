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


initialiseSingleton(SpellCustomizations);

///\brief: This file includes all setted custom values and/or spell.dbc values (overwrite)
/// Set the values you want based on spell Id (Do not set your values based on some text!)

SpellCustomizations::SpellCustomizations() {}
SpellCustomizations::~SpellCustomizations() {}

void SpellCustomizations::LoadServersideSpells()
{
    uint32 start_time = getMSTime();

    QueryResult* spells_result = WorldDatabase.Query("SELECT * FROM spells");

    if (spells_result == nullptr)
    {
        Log.Notice("SpellCustomizations", "Table `spells` is empty!");
        return;
    }

    Log.Notice("SpellCustomizations", "Table `spells` has %u columns", spells_result->GetFieldCount());

    _serversideSpellContainerStore.rehash(spells_result->GetRowCount());

    uint32 serverside_spell_count = 0;
    do
    {
        Field* fields = spells_result->Fetch();

        uint32 entry = fields[0].GetUInt32();

        OLD_SpellEntry& serversideSpell = _serversideSpellContainerStore[entry];

        serversideSpell.Id = entry;

        serversideSpell.Category = fields[1].GetUInt32();
        serversideSpell.DispelType = fields[2].GetUInt32();
        serversideSpell.MechanicsType = fields[3].GetUInt32();
        serversideSpell.Attributes = fields[4].GetUInt32();
        serversideSpell.AttributesEx = fields[5].GetUInt32();
        serversideSpell.AttributesExB = fields[6].GetUInt32();
        serversideSpell.AttributesExC = fields[7].GetUInt32();
        serversideSpell.AttributesExD = fields[8].GetUInt32();
        serversideSpell.AttributesExE = fields[9].GetUInt32();
        serversideSpell.AttributesExF = fields[10].GetUInt32();
        serversideSpell.AttributesExG = fields[11].GetUInt32();
        serversideSpell.RequiredShapeShift = fields[12].GetUInt32();
        //serversideSpell.Unknown = fields[13].GetUInt32();
        serversideSpell.ShapeshiftExclude = fields[14].GetUInt32();
        //serversideSpell.Unknown = fields[15].GetUInt32();
        serversideSpell.Targets = fields[16].GetUInt32();
        serversideSpell.TargetCreatureType = fields[17].GetUInt32();
        serversideSpell.RequiresSpellFocus = fields[18].GetUInt32();
        serversideSpell.FacingCasterFlags = fields[19].GetUInt32();
        serversideSpell.CasterAuraState = fields[20].GetUInt32();
        serversideSpell.TargetAuraState = fields[21].GetUInt32();
        serversideSpell.CasterAuraStateNot = fields[22].GetUInt32();
        serversideSpell.TargetAuraStateNot = fields[23].GetUInt32();
        serversideSpell.casterAuraSpell = fields[24].GetUInt32();
        serversideSpell.targetAuraSpell = fields[25].GetUInt32();
        serversideSpell.casterAuraSpellNot = fields[26].GetUInt32();
        serversideSpell.targetAuraSpellNot = fields[27].GetUInt32();
        serversideSpell.CastingTimeIndex = fields[28].GetUInt32();
        serversideSpell.RecoveryTime = fields[29].GetUInt32();
        serversideSpell.CategoryRecoveryTime = fields[30].GetUInt32();
        serversideSpell.InterruptFlags = fields[31].GetUInt32();
        serversideSpell.AuraInterruptFlags = fields[32].GetUInt32();
        serversideSpell.ChannelInterruptFlags = fields[33].GetUInt32();
        serversideSpell.procFlags = fields[34].GetUInt32();
        serversideSpell.procChance = fields[35].GetUInt32();
        serversideSpell.procCharges = fields[36].GetUInt32();
        serversideSpell.maxLevel = fields[37].GetUInt32();
        serversideSpell.baseLevel = fields[38].GetUInt32();
        serversideSpell.spellLevel = fields[39].GetUInt32();
        serversideSpell.DurationIndex = fields[40].GetUInt32();
        serversideSpell.powerType = fields[41].GetInt32();
        serversideSpell.manaCost = fields[42].GetUInt32();
        serversideSpell.manaCostPerlevel = fields[43].GetUInt32();
        serversideSpell.manaPerSecond = fields[44].GetUInt32();
        serversideSpell.manaPerSecondPerLevel = fields[45].GetUInt32();
        serversideSpell.rangeIndex = fields[46].GetUInt32();
        serversideSpell.speed = fields[47].GetFloat();
        serversideSpell.modalNextSpell = fields[48].GetUInt32();
        serversideSpell.maxstack = fields[49].GetUInt32();

        for (uint8 i = 0; i < 2; ++i)
            serversideSpell.Totem[i] = fields[50 + i].GetUInt32();

        for (uint8 i = 0; i < 8; ++i)
            serversideSpell.Reagent[i] = fields[52 + i].GetUInt32();

        for (uint8 i = 0; i < 8; ++i)
            serversideSpell.ReagentCount[i] = fields[60 + i].GetUInt32();

        serversideSpell.EquippedItemClass = fields[68].GetInt32();
        serversideSpell.EquippedItemSubClass = fields[69].GetUInt32();
        serversideSpell.RequiredItemFlags = fields[70].GetUInt32();

        for (uint8 i = 0; i < MAX_SPELL_EFFECTS; ++i)
            serversideSpell.Effect[i] = fields[71 + i].GetUInt32();

        for (uint8 i = 0; i < MAX_SPELL_EFFECTS; ++i)
            serversideSpell.EffectDieSides[i] = fields[74 + i].GetUInt32();

        for (uint8 i = 0; i < MAX_SPELL_EFFECTS; ++i)
            serversideSpell.EffectRealPointsPerLevel[i] = fields[77 + i].GetFloat();

        for (uint8 i = 0; i < MAX_SPELL_EFFECTS; ++i)
            serversideSpell.EffectBasePoints[i] = fields[80 + i].GetInt32();

        for (uint8 i = 0; i < MAX_SPELL_EFFECTS; ++i)
            serversideSpell.EffectMechanic[i] = fields[83 + i].GetInt32();

        for (uint8 i = 0; i < MAX_SPELL_EFFECTS; ++i)
            serversideSpell.EffectImplicitTargetA[i] = fields[86 + i].GetUInt32();

        for (uint8 i = 0; i < MAX_SPELL_EFFECTS; ++i)
            serversideSpell.EffectImplicitTargetB[i] = fields[89 + i].GetUInt32();

        for (uint8 i = 0; i < MAX_SPELL_EFFECTS; ++i)
            serversideSpell.EffectRadiusIndex[i] = fields[92 + i].GetUInt32();

        for (uint8 i = 0; i < MAX_SPELL_EFFECTS; ++i)
            serversideSpell.EffectApplyAuraName[i] = fields[95 + i].GetUInt32();

        for (uint8 i = 0; i < MAX_SPELL_EFFECTS; ++i)
            serversideSpell.EffectAmplitude[i] = fields[98 + i].GetUInt32();

        for (uint8 i = 0; i < MAX_SPELL_EFFECTS; ++i)
            serversideSpell.EffectMultipleValue[i] = fields[101 + i].GetFloat();

        for (uint8 i = 0; i < MAX_SPELL_EFFECTS; ++i)
            serversideSpell.EffectChainTarget[i] = fields[104 + i].GetUInt32();

        for (uint8 i = 0; i < MAX_SPELL_EFFECTS; ++i)
            serversideSpell.EffectItemType[i] = fields[107 + i].GetUInt32();

        for (uint8 i = 0; i < MAX_SPELL_EFFECTS; ++i)
            serversideSpell.EffectMiscValue[i] = fields[110 + i].GetUInt32();

        for (uint8 i = 0; i < MAX_SPELL_EFFECTS; ++i)
            serversideSpell.EffectMiscValueB[i] = fields[113 + i].GetUInt32();

        for (uint8 i = 0; i < MAX_SPELL_EFFECTS; ++i)
            serversideSpell.EffectTriggerSpell[i] = fields[116 + i].GetUInt32();

        for (uint8 i = 0; i < MAX_SPELL_EFFECTS; ++i)
            serversideSpell.EffectPointsPerComboPoint[i] = fields[119 + i].GetFloat();

        for (uint8 c = 0; c < MAX_SPELL_EFFECTS; ++c)
        {
            serversideSpell.EffectSpellClassMask[0][c] = fields[122 + c].GetUInt32();
            serversideSpell.EffectSpellClassMask[1][c] = fields[125 + c].GetUInt32();
            serversideSpell.EffectSpellClassMask[2][c] = fields[128 + c].GetUInt32();
        }

        serversideSpell.SpellVisual = fields[131].GetUInt32();
        serversideSpell.field114 = fields[132].GetUInt32();
        serversideSpell.spellIconID = fields[133].GetUInt32();
        serversideSpell.activeIconID = fields[134].GetUInt32();
        serversideSpell.spellPriority = fields[135].GetUInt32();
        serversideSpell.Name = fields[136].GetString();
        serversideSpell.Rank = fields[137].GetString();
        serversideSpell.Description = fields[138].GetString();
        serversideSpell.BuffDescription = fields[139].GetString();
        serversideSpell.ManaCostPercentage = fields[140].GetUInt32();
        serversideSpell.StartRecoveryCategory = fields[141].GetUInt32();
        serversideSpell.StartRecoveryTime = fields[142].GetUInt32();
        serversideSpell.MaxTargetLevel = fields[143].GetUInt32();
        serversideSpell.SpellFamilyName = fields[144].GetUInt32();

        for (uint8 i = 0; i < MAX_SPELL_EFFECTS; ++i)
            serversideSpell.SpellGroupType[i] = fields[145 + i].GetUInt32();

        serversideSpell.MaxTargets = fields[148].GetUInt32();
        serversideSpell.Spell_Dmg_Type = fields[149].GetUInt32();
        serversideSpell.PreventionType = fields[150].GetUInt32();
        serversideSpell.StanceBarOrder = fields[151].GetInt32();

        for (uint8 i = 0; i < MAX_SPELL_EFFECTS; ++i)
            serversideSpell.dmg_multiplier[i] = fields[152 + i].GetUInt32();

        serversideSpell.MinFactionID = fields[155].GetUInt32();
        serversideSpell.MinReputation = fields[156].GetUInt32();
        serversideSpell.RequiredAuraVision = fields[157].GetUInt32();

        for (uint8 i = 0; i < 2; ++i)
            serversideSpell.TotemCategory[i] = fields[158 + i].GetUInt32();

        serversideSpell.RequiresAreaId = fields[160].GetInt32();
        serversideSpell.School = fields[161].GetUInt32();
        serversideSpell.RuneCostID = fields[162].GetUInt32();
        //serversideSpell.SpellMissileID = fields[163].GetUInt32();
        //serversideSpell.PowerDisplayId = fields[164].GetUInt32();
        //for (uint8 i = 0; i < MAX_SPELL_EFFECTS; ++i)
            //serversideSpell.EffectBonusMultiplier[i] = fields[165 + i].GetFloat();

        //serversideSpell.SpellDescriptionVariable = fields[168].GetUInt32();
        serversideSpell.SpellDifficultyID = fields[169].GetUInt32();

        ++serverside_spell_count;
    } while (spells_result->NextRow());

    delete spells_result;

    Log.Success("SpellCustomizations", "Loaded %u serverside spells from `spells` table in %u ms!", serverside_spell_count, getMSTime() - start_time);
}

OLD_SpellEntry* SpellCustomizations::GetServersideSpell(uint32 spell_id)
{
    ServersideSpellContainer::const_iterator itr = _serversideSpellContainerStore.find(spell_id);
    if (itr != _serversideSpellContainerStore.end())
        return const_cast<OLD_SpellEntry*>(&itr->second);

    return nullptr;
}

void SpellCustomizations::StartSpellCustomization()
{
    LoadServersideSpells();

    Log.Debug("SpellCustomizations::StartSpellCustomization", "Successfull started");

    LoadSpellRanks();
    LoadSpellCustomAssign();
    LoadSpellCustomCoefFlags();
    LoadSpellProcs();

    for (auto it = sSpellCustomizations.GetServersideSpellStore()->begin(); it != sSpellCustomizations.GetServersideSpellStore()->end(); ++it)
    {
        auto spellentry = GetServersideSpell(it->first);
        if (spellentry != nullptr)
        {
            //Set spell overwrites (effect based)
            SetEffectAmplitude(spellentry);
            SetAuraFactoryFunc(spellentry);

            // Set custom values (effect based)
            SetMeleeSpellBool(spellentry);
            SetRangedSpellBool(spellentry);

            // Set custom values (spell based)
            SetMissingCIsFlags(spellentry);
            SetCustomFlags(spellentry);
            SetOnShapeshiftChange(spellentry);
            SetAlwaysApply(spellentry);
        }
    }
}

void SpellCustomizations::LoadSpellRanks()
{
    uint32 spell_rank_count = 0;

    if (QueryResult* result = WorldDatabase.Query("SELECT spell_id, rank FROM spell_ranks"))
    {
        do
        {
            uint32 spell_id = result->Fetch()[0].GetUInt32();
            uint32 pRank = result->Fetch()[1].GetUInt32();

            OLD_SpellEntry* spell_entry = GetServersideSpell(spell_id);
            if (spell_entry != nullptr)
            {
                spell_entry->custom_RankNumber = pRank;
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

void SpellCustomizations::LoadSpellCustomAssign()
{
    uint32 spell_custom_assign_count = 0;

    if (QueryResult* result = WorldDatabase.Query("SELECT spell_id, on_target_flag, from_caster_on_self_flag, self_cast_only, c_is_flag FROM spell_custom_assign"))
    {
        do
        {
            uint32 spell_id = result->Fetch()[0].GetUInt32();
            uint32 on_target = result->Fetch()[1].GetUInt32();
            uint32 from_caster_on_self_flag = result->Fetch()[2].GetUInt32();
            bool self_cast_only = result->Fetch()[3].GetBool();
            uint32 c_is_flag = result->Fetch()[4].GetUInt32();

            OLD_SpellEntry* spell_entry = GetServersideSpell(spell_id);
            if (spell_entry != nullptr)
            {
                spell_entry->custom_BGR_one_buff_on_target = on_target;
                spell_entry->custom_BGR_one_buff_from_caster_on_self = from_caster_on_self_flag;
                spell_entry->custom_self_cast_only = self_cast_only;
                spell_entry->custom_c_is_flags = c_is_flag;

                ++spell_custom_assign_count;
            }
            else
            {
                Log.Error("SpellCustomizations::LoadSpellCustomAssign", "your spell_custom_assign table includes an invalid spell %u.", spell_id);
                continue;
            }

        } while (result->NextRow());
        delete result;
    }

    if (spell_custom_assign_count > 0)
    {
        Log.Success("SpellCustomizations::LoadSpellCustomAssign", "Loaded %u attributes from spell_custom_assign table", spell_custom_assign_count);
    }
    else
    {
        Log.Debug("SpellCustomizations::LoadSpellCustomAssign", "Your spell_custom_assign table is empty!");
    }
}

void SpellCustomizations::LoadSpellCustomCoefFlags()
{
    uint32 spell_custom_coef_flags_count = 0;

    if (QueryResult* result = WorldDatabase.Query("SELECT spell_id, spell_coef_flags FROM spell_coef_flags"))
    {
        do
        {
            uint32 spell_id = result->Fetch()[0].GetUInt32();
            uint32 coef_flags = result->Fetch()[1].GetUInt32();

            OLD_SpellEntry* spell_entry = GetServersideSpell(spell_id);
            if (spell_entry != nullptr)
            {
                spell_entry->custom_spell_coef_flags = coef_flags;
                ++spell_custom_coef_flags_count;
            }
            else
            {
                Log.Error("SpellCustomizations::LoadSpellCustomCoefFlags", "your spell_coef_flags table includes an invalid spell %u.", spell_id);
                continue;
            }

        } while (result->NextRow());
        delete result;
    }

    if (spell_custom_coef_flags_count > 0)
    {
        Log.Success("SpellCustomizations::LoadSpellCustomCoefFlags", "Loaded %u attributes from spell_coef_flags table", spell_custom_coef_flags_count);
    }
    else
    {
        Log.Debug("SpellCustomizations::LoadSpellCustomCoefFlags", "Your spell_coef_flags table is empty!");
    }
}

void SpellCustomizations::LoadSpellProcs()
{
    uint32 spell_procs_count = 0;

    if (QueryResult* result = WorldDatabase.Query("SELECT spell_id, proc_on_name_hash, proc_flags, target_self, proc_chance, proc_charges, proc_interval, effect_trigger_spell_0, effect_trigger_spell_1, effect_trigger_spell_2, description FROM spell_proc"))
    {
        do
        {
            Field* f = result->Fetch();
            uint32 spell_id = f[0].GetUInt32();
            uint32 name_hash = f[1].GetUInt32();

            auto spell_entry = GetServersideSpell(spell_id);
            if (spell_entry != nullptr)
            {
                uint8 x;
                for (x = 0; x < 3; ++x)
                    if (spell_entry->custom_ProcOnNameHash[x] == 0)
                        break;

                if (x != 3)
                {
                    spell_entry->custom_ProcOnNameHash[x] = name_hash;
                }
                else
                    Log.Error("SpellCustomizations::LoadSpellProcs", "Wrong ProcOnNameHash for Spell: %u!", spell_id);

                if (f[2].GetInt32() >= 0)
                    spell_entry->procFlags = f[2].GetUInt32();

                if (f[3].GetBool())
                    spell_entry->procFlags |= PROC_TARGET_SELF;
                if (f[4].GetInt32() >= 0)
                    spell_entry->procChance = f[4].GetUInt32();
                if (f[5].GetInt32() >= 0)
                    spell_entry->procCharges = f[5].GetInt32();

                spell_entry->custom_proc_interval = f[6].GetUInt32();

                if (f[7].GetInt32() >= 0)
                    spell_entry->EffectTriggerSpell[0] = f[7].GetUInt32();
                if (f[8].GetInt32() >= 0)
                    spell_entry->EffectTriggerSpell[1] = f[8].GetUInt32();
                if (f[9].GetInt32() >= 0)
                    spell_entry->EffectTriggerSpell[2] = f[9].GetUInt32();

                if (spell_entry->EffectTriggerSpell[0] > 0)
                    spell_entry->EffectApplyAuraName[0] = SPELL_AURA_PROC_TRIGGER_SPELL;
                if (spell_entry->EffectTriggerSpell[1] > 0)
                    spell_entry->EffectApplyAuraName[1] = SPELL_AURA_PROC_TRIGGER_SPELL;
                if (spell_entry->EffectTriggerSpell[2] > 0)
                    spell_entry->EffectApplyAuraName[2] = SPELL_AURA_PROC_TRIGGER_SPELL;

                ++spell_procs_count;
            }
            else
            {
                Log.Error("SpellCustomizations::LoadSpellProcs()", "Invalid spellID %u in table spell_proc", spell_id);
            }
        } while (result->NextRow());
        delete result;
    }

    if (spell_procs_count > 0)
    {
        Log.Success("SpellCustomizations::LoadSpellProcs", "Loaded %u proc definitions from spell_proc table", spell_procs_count);
    }
    else
    {
        Log.Debug("SpellCustomizations::LoadSpellProcs", "Your spell_proc table is empty!");
    }
}

///Fix if it is a periodic trigger with amplitude = 0, to avoid division by zero
void SpellCustomizations::SetEffectAmplitude(OLD_SpellEntry* spell_entry)
{
    for (uint8 y = 0; y < 3; y++)
    {
        if (spell_entry->Effect[y] != SPELL_EFFECT_APPLY_AURA)
        {
            continue;
        }
        else
        {
            if ((spell_entry->EffectApplyAuraName[y] == SPELL_AURA_PERIODIC_TRIGGER_SPELL || spell_entry->EffectApplyAuraName[y] == SPELL_AURA_PERIODIC_TRIGGER_SPELL_WITH_VALUE) && spell_entry->EffectApplyAuraName[y] == 0)
            {
                spell_entry->EffectAmplitude[y] = 1000;

                Log.Debug("SpellCustomizations::SetEffectAmplitude", "EffectAmplitude applied Spell - %s (%u)", spell_entry->Name.c_str(), spell_entry->Id);
            }
        }
    }
}

void SpellCustomizations::SetAuraFactoryFunc(OLD_SpellEntry* spell_entry)
{
    bool spell_aura_factory_functions_loaded = false;

    for (uint8 y = 0; y < 3; y++)
    {
        if (spell_entry->Effect[y] != SPELL_EFFECT_APPLY_AURA)
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
        Log.Debug("SpellCustomizations::SetAuraFactoryFunc", "AuraFactoryFunc definitions applied to Spell - %s (%u)", spell_entry->Name.c_str(), spell_entry->Id);
    }
}

void SpellCustomizations::SetMeleeSpellBool(OLD_SpellEntry* spell_entry)
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
        Log.Debug("SpellCustomizations::SetMeleeSpellBool", "custom_is_melee_spell = true for Spell - %s (%u)", spell_entry->Name.c_str(), spell_entry->Id);
    }
}

void SpellCustomizations::SetRangedSpellBool(OLD_SpellEntry* spell_entry)
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
        Log.Debug("SpellCustomizations::SetRangedSpellBool", "custom_is_ranged_spell = true for Spell - %s (%u)", spell_entry->Name.c_str(), spell_entry->Id);
    }
}

void SpellCustomizations::SetMissingCIsFlags(OLD_SpellEntry* spell_entry)
{
    // Zyres: Special cases, not handled in spell_custom_assign!
    if (IsDamagingSpell(spell_entry))
        spell_entry->custom_c_is_flags |= SPELL_FLAG_IS_DAMAGING;
    if (IsHealingSpell(spell_entry))
        spell_entry->custom_c_is_flags |= SPELL_FLAG_IS_HEALING;
    if (IsTargetingStealthed(spell_entry))
        spell_entry->custom_c_is_flags |= SPELL_FLAG_IS_TARGETINGSTEALTHED;
    if (IsRequireCooldownSpell(spell_entry))
        spell_entry->custom_c_is_flags |= SPELL_FLAG_IS_REQUIRECOOLDOWNUPDATE;
}

void SpellCustomizations::SetCustomFlags(OLD_SpellEntry* spell_entry)
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

void SpellCustomizations::SetOnShapeshiftChange(OLD_SpellEntry* spell_entry)
{
    // Currently only for spell Track Humanoids
    if (spell_entry->Id != 5225 && spell_entry->Id != 19883)
    {
        return;
    }
    else
    {
        spell_entry->custom_apply_on_shapeshift_change = true;
    }
}

void SpellCustomizations::SetAlwaysApply(OLD_SpellEntry* spell_entry)
{
    switch (spell_entry->Id)
    {
        // SPELL_HASH_BLOOD_FURY
        case 20572:
        case 23230:
        case 24571:
        case 33697:
        case 33702:
        // SPELL_HASH_SHADOWSTEP
        case 36554:
        case 36563:
        case 41176:
        case 44373:
        case 45273:
        case 46463:
        case 55965:
        case 55966:
        case 63790:
        case 63793:
        case 66178:
        case 68759:
        case 68760:
        case 68761:
        case 69087:
        case 70431:
        case 72326:
        case 72327:
        // SPELL_HASH_PSYCHIC_HORROR
        case 34984:
        case 65545:
        {
            spell_entry->custom_always_apply = true;
        } break;
        default:
            break;
    }
}
