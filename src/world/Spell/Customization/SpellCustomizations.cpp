/*
 * AscEmu Framework based on ArcEmu MMORPG Server
 * Copyright (C) 2014-2017 AscEmu Team <http://www.ascemu.org>
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
#include "SpellCustomizations.hpp"
#include "Server/MainServerDefines.h"
#include "Spell/SpellAuras.h"
#include "Singleton.h"
#include <unordered_map>
#include "Spell/Definitions/ProcFlags.h"
#include "Spell/Definitions/SpellDamageType.h"
#include "Spell/Definitions/SpellCustomFlags.h"
#include "Spell/Definitions/SpellIsFlags.h"

initialiseSingleton(SpellCustomizations);

///\brief: This file includes all setted custom values and/or spell.dbc values (overwrite)
/// Set the values you want based on spell Id (Do not set your values based on some text!)

SpellCustomizations::SpellCustomizations() {}
SpellCustomizations::~SpellCustomizations() {}

void SpellCustomizations::LoadSpellInfoData()
{
#if VERSION_STRING != Cata
    for (uint32 i = 0; i < MAX_SPELL_ID; ++i)
    {

        DBC::Structures::SpellEntry_New const* dbc_spell_entry = sSpellStore.LookupEntry(i);
        if (dbc_spell_entry == nullptr)
        {
            continue;
        }
        else
        {
            uint32 spell_id = dbc_spell_entry->Id;
            SpellInfo& spellInfo = _spellInfoContainerStore[spell_id];
            spellInfo.Id = spell_id;
            spellInfo.Attributes = dbc_spell_entry->Attributes;
            spellInfo.AttributesEx = dbc_spell_entry->AttributesEx;
            spellInfo.AttributesExB = dbc_spell_entry->AttributesExB;
            spellInfo.AttributesExC = dbc_spell_entry->AttributesExC;
            spellInfo.AttributesExD = dbc_spell_entry->AttributesExD;
            spellInfo.AttributesExE = dbc_spell_entry->AttributesExE;
            spellInfo.AttributesExF = dbc_spell_entry->AttributesExF;
#if VERSION_STRING > TBC
            spellInfo.AttributesExG = dbc_spell_entry->AttributesExG;
#endif
            spellInfo.RequiredShapeShift = dbc_spell_entry->RequiredShapeShift;
            spellInfo.ShapeshiftExclude = dbc_spell_entry->ShapeshiftExclude;
            spellInfo.Targets = dbc_spell_entry->Targets;
            spellInfo.TargetCreatureType = dbc_spell_entry->TargetCreatureType;
            spellInfo.RequiresSpellFocus = dbc_spell_entry->RequiresSpellFocus;
            spellInfo.FacingCasterFlags = dbc_spell_entry->FacingCasterFlags;
            spellInfo.CasterAuraState = dbc_spell_entry->CasterAuraState;
            spellInfo.TargetAuraState = dbc_spell_entry->TargetAuraState;
            spellInfo.CasterAuraStateNot = dbc_spell_entry->CasterAuraStateNot;
            spellInfo.TargetAuraStateNot = dbc_spell_entry->TargetAuraStateNot;
#if VERSION_STRING > TBC
            spellInfo.casterAuraSpell = dbc_spell_entry->casterAuraSpell;
            spellInfo.targetAuraSpell = dbc_spell_entry->targetAuraSpell;
            spellInfo.casterAuraSpellNot = dbc_spell_entry->casterAuraSpellNot;
            spellInfo.targetAuraSpellNot = dbc_spell_entry->targetAuraSpellNot;
#endif
            spellInfo.CastingTimeIndex = dbc_spell_entry->CastingTimeIndex;
            spellInfo.RecoveryTime = dbc_spell_entry->RecoveryTime;
            spellInfo.CategoryRecoveryTime = dbc_spell_entry->CategoryRecoveryTime;
            spellInfo.InterruptFlags = dbc_spell_entry->InterruptFlags;
            spellInfo.AuraInterruptFlags = dbc_spell_entry->AuraInterruptFlags;
            spellInfo.ChannelInterruptFlags = dbc_spell_entry->ChannelInterruptFlags;
            spellInfo.procFlags = dbc_spell_entry->procFlags;
            spellInfo.procChance = dbc_spell_entry->procChance;
            spellInfo.procCharges = dbc_spell_entry->procCharges;
            spellInfo.maxLevel = dbc_spell_entry->maxLevel;
            spellInfo.baseLevel = dbc_spell_entry->baseLevel;
            spellInfo.spellLevel = dbc_spell_entry->spellLevel;
            spellInfo.DurationIndex = dbc_spell_entry->DurationIndex;
            spellInfo.powerType = dbc_spell_entry->powerType;
            spellInfo.manaCost = dbc_spell_entry->manaCost;
            spellInfo.manaCostPerlevel = dbc_spell_entry->manaCostPerlevel;
            spellInfo.manaPerSecond = dbc_spell_entry->manaPerSecond;
            spellInfo.manaPerSecondPerLevel = dbc_spell_entry->manaPerSecondPerLevel;
            spellInfo.rangeIndex = dbc_spell_entry->rangeIndex;
            spellInfo.speed = dbc_spell_entry->speed;
            spellInfo.modalNextSpell = dbc_spell_entry->modalNextSpell;
            spellInfo.maxstack = dbc_spell_entry->maxstack;

            for (uint8 i = 0; i < MAX_SPELL_TOTEMS; ++i)
                spellInfo.Totem[i] = dbc_spell_entry->Totem[i];

            for (uint8 i = 0; i < MAX_SPELL_REAGENTS; ++i)
                spellInfo.Reagent[i] = dbc_spell_entry->Reagent[i];

            for (uint8 i = 0; i < MAX_SPELL_REAGENTS; ++i)
                spellInfo.ReagentCount[i] = dbc_spell_entry->ReagentCount[i];

            spellInfo.EquippedItemClass = dbc_spell_entry->EquippedItemClass;
            spellInfo.EquippedItemSubClass = dbc_spell_entry->EquippedItemSubClass;
            spellInfo.RequiredItemFlags = dbc_spell_entry->RequiredItemFlags;

            for (uint8 i = 0; i < MAX_SPELL_EFFECTS; ++i)
                spellInfo.Effect[i] = dbc_spell_entry->Effect[i];

            for (uint8 i = 0; i < MAX_SPELL_EFFECTS; ++i)
                spellInfo.EffectDieSides[i] = dbc_spell_entry->EffectDieSides[i];

            for (uint8 i = 0; i < MAX_SPELL_EFFECTS; ++i)
                spellInfo.EffectRealPointsPerLevel[i] = dbc_spell_entry->EffectRealPointsPerLevel[i];

            for (uint8 i = 0; i < MAX_SPELL_EFFECTS; ++i)
                spellInfo.EffectBasePoints[i] = dbc_spell_entry->EffectBasePoints[i];

            for (uint8 i = 0; i < MAX_SPELL_EFFECTS; ++i)
                spellInfo.EffectMechanic[i] = dbc_spell_entry->EffectMechanic[i];

            for (uint8 i = 0; i < MAX_SPELL_EFFECTS; ++i)
                spellInfo.EffectImplicitTargetA[i] = dbc_spell_entry->EffectImplicitTargetA[i];

            for (uint8 i = 0; i < MAX_SPELL_EFFECTS; ++i)
                spellInfo.EffectImplicitTargetB[i] = dbc_spell_entry->EffectImplicitTargetB[i];

            for (uint8 i = 0; i < MAX_SPELL_EFFECTS; ++i)
                spellInfo.EffectRadiusIndex[i] = dbc_spell_entry->EffectRadiusIndex[i];

            for (uint8 i = 0; i < MAX_SPELL_EFFECTS; ++i)
                spellInfo.EffectApplyAuraName[i] = dbc_spell_entry->EffectApplyAuraName[i];

            for (uint8 i = 0; i < MAX_SPELL_EFFECTS; ++i)
                spellInfo.EffectAmplitude[i] = dbc_spell_entry->EffectAmplitude[i];

            for (uint8 i = 0; i < MAX_SPELL_EFFECTS; ++i)
                spellInfo.EffectMultipleValue[i] = dbc_spell_entry->EffectMultipleValue[i];

            for (uint8 i = 0; i < MAX_SPELL_EFFECTS; ++i)
                spellInfo.EffectChainTarget[i] = dbc_spell_entry->EffectChainTarget[i];

            for (uint8 i = 0; i < MAX_SPELL_EFFECTS; ++i)
                spellInfo.EffectItemType[i] = dbc_spell_entry->EffectItemType[i];

            for (uint8 i = 0; i < MAX_SPELL_EFFECTS; ++i)
                spellInfo.EffectMiscValue[i] = dbc_spell_entry->EffectMiscValue[i];

            for (uint8 i = 0; i < MAX_SPELL_EFFECTS; ++i)
                spellInfo.EffectMiscValueB[i] = dbc_spell_entry->EffectMiscValueB[i];

            for (uint8 i = 0; i < MAX_SPELL_EFFECTS; ++i)
                spellInfo.EffectTriggerSpell[i] = dbc_spell_entry->EffectTriggerSpell[i];

            for (uint8 i = 0; i < MAX_SPELL_EFFECTS; ++i)
                spellInfo.EffectPointsPerComboPoint[i] = dbc_spell_entry->EffectPointsPerComboPoint[i];

#if VERSION_STRING > TBC
            for (uint8 i = 0; i < 3; ++i)
                for (uint8 j = 0; j < 3; ++j)
                    spellInfo.EffectSpellClassMask[i][j] = dbc_spell_entry->EffectSpellClassMask[i][j];
#endif

            spellInfo.SpellVisual = dbc_spell_entry->SpellVisual;
            spellInfo.field114 = dbc_spell_entry->field114;
            spellInfo.spellIconID = dbc_spell_entry->spellIconID;
            spellInfo.activeIconID = dbc_spell_entry->activeIconID;
            spellInfo.spellPriority = dbc_spell_entry->spellPriority;
            spellInfo.Name = dbc_spell_entry->Name;
            spellInfo.Rank = dbc_spell_entry->Rank;
            spellInfo.Description = dbc_spell_entry->Description;
            spellInfo.BuffDescription = dbc_spell_entry->BuffDescription;
            spellInfo.ManaCostPercentage = dbc_spell_entry->ManaCostPercentage;
            spellInfo.StartRecoveryCategory = dbc_spell_entry->StartRecoveryCategory;
            spellInfo.StartRecoveryTime = dbc_spell_entry->StartRecoveryTime;
#if VERSION_STRING > TBC
            spellInfo.MaxTargetLevel = dbc_spell_entry->MaxTargetLevel;
#endif
            spellInfo.SpellFamilyName = dbc_spell_entry->SpellFamilyName;

#if VERSION_STRING > TBC
            for (uint8 i = 0; i < MAX_SPELL_EFFECTS; ++i)
                spellInfo.SpellGroupType[i] = dbc_spell_entry->SpellGroupType[i];
#endif

            spellInfo.MaxTargets = dbc_spell_entry->MaxTargets;
            spellInfo.Spell_Dmg_Type = dbc_spell_entry->Spell_Dmg_Type;
            spellInfo.PreventionType = dbc_spell_entry->PreventionType;
            spellInfo.StanceBarOrder = dbc_spell_entry->StanceBarOrder;

            for (uint8 i = 0; i < MAX_SPELL_EFFECTS; ++i)
                spellInfo.dmg_multiplier[i] = dbc_spell_entry->dmg_multiplier[i];

            spellInfo.MinFactionID = dbc_spell_entry->MinFactionID;
            spellInfo.MinReputation = dbc_spell_entry->MinReputation;
            spellInfo.RequiredAuraVision = dbc_spell_entry->RequiredAuraVision;

            for (uint8 i = 0; i < MAX_SPELL_TOTEM_CATEGORIES; ++i)
                spellInfo.TotemCategory[i] = dbc_spell_entry->TotemCategory[i];

            spellInfo.RequiresAreaId = dbc_spell_entry->RequiresAreaId;
            spellInfo.School = dbc_spell_entry->School;
#if VERSION_STRING > TBC
            spellInfo.RuneCostID = dbc_spell_entry->RuneCostID;

            spellInfo.SpellDifficultyID = dbc_spell_entry->SpellDifficultyID;
#endif
        }
    }
#else
    for (uint32 i = 0; i < MAX_SPELL_ID; ++i)
    {
        DBC::Structures::SpellEntry const* dbc_spell_entry = sSpellStore.LookupEntry(i);
        if (dbc_spell_entry == nullptr)
        {
            continue;
        }
        else
        {
            uint32 spell_id = dbc_spell_entry->Id;
            SpellInfo& spellInfo = _spellInfoContainerStore[spell_id];
            spellInfo.Id = spell_id;
            spellInfo.Attributes = dbc_spell_entry->Attributes;
            spellInfo.AttributesEx = dbc_spell_entry->AttributesEx;
            spellInfo.AttributesExB = dbc_spell_entry->AttributesExB;
            spellInfo.AttributesExC = dbc_spell_entry->AttributesExC;
            spellInfo.AttributesExD = dbc_spell_entry->AttributesExD;
            spellInfo.AttributesExE = dbc_spell_entry->AttributesExE;
            spellInfo.AttributesExF = dbc_spell_entry->AttributesExF;
            spellInfo.AttributesExG = dbc_spell_entry->AttributesExG;
            spellInfo.AttributesExH = dbc_spell_entry->AttributesExH;
            spellInfo.AttributesExI = dbc_spell_entry->AttributesExI;
            spellInfo.AttributesExJ = dbc_spell_entry->AttributesExJ;
            spellInfo.CastingTimeIndex = dbc_spell_entry->CastingTimeIndex;
            spellInfo.DurationIndex = dbc_spell_entry->DurationIndex;
            spellInfo.powerType = dbc_spell_entry->powerType;
            spellInfo.rangeIndex = dbc_spell_entry->rangeIndex;
            spellInfo.speed = dbc_spell_entry->speed;
            spellInfo.SpellVisual = dbc_spell_entry->SpellVisual[0];
            spellInfo.field114 = dbc_spell_entry->SpellVisual[1];
            spellInfo.spellIconID = dbc_spell_entry->spellIconID;
            spellInfo.activeIconID = dbc_spell_entry->activeIconID;
            spellInfo.Name = dbc_spell_entry->Name;
            spellInfo.Rank = dbc_spell_entry->Rank;
            spellInfo.Description = dbc_spell_entry->Description;
            spellInfo.BuffDescription = dbc_spell_entry->BuffDescription;
            spellInfo.School = dbc_spell_entry->School;
            spellInfo.RuneCostID = dbc_spell_entry->RuneCostID;
            spellInfo.SpellDifficultyID = dbc_spell_entry->SpellDifficultyId;

            // dbc links
            spellInfo.SpellScalingId = dbc_spell_entry->SpellScalingId;
            spellInfo.SpellAuraOptionsId = dbc_spell_entry->SpellAuraOptionsId;
            spellInfo.SpellAuraRestrictionsId = dbc_spell_entry->SpellAuraRestrictionsId;
            spellInfo.SpellCastingRequirementsId = dbc_spell_entry->SpellCastingRequirementsId;
            spellInfo.SpellCategoriesId = dbc_spell_entry->SpellCategoriesId;
            spellInfo.SpellClassOptionsId = dbc_spell_entry->SpellClassOptionsId;
            spellInfo.SpellCooldownsId = dbc_spell_entry->SpellCooldownsId;
            spellInfo.SpellEquippedItemsId = dbc_spell_entry->SpellEquippedItemsId;
            spellInfo.SpellInterruptsId = dbc_spell_entry->SpellInterruptsId;
            spellInfo.SpellLevelsId = dbc_spell_entry->SpellLevelsId;
            spellInfo.SpellPowerId = dbc_spell_entry->SpellPowerId;
            spellInfo.SpellReagentsId = dbc_spell_entry->SpellReagentsId;
            spellInfo.SpellShapeshiftId = dbc_spell_entry->SpellShapeshiftId;
            spellInfo.SpellTargetRestrictionsId = dbc_spell_entry->SpellTargetRestrictionsId;
            spellInfo.SpellTotemsId = dbc_spell_entry->SpellTotemsId;

            // data from SpellScaling.dbc
            // data from SpellAuraOptions.dbc
            if (dbc_spell_entry->SpellAuraOptionsId && dbc_spell_entry->GetSpellAuraOptions() != nullptr)
            {
                spellInfo.maxstack = dbc_spell_entry->GetSpellAuraOptions()->StackAmount;
                spellInfo.procChance = dbc_spell_entry->GetSpellAuraOptions()->procChance;
                spellInfo.procCharges = dbc_spell_entry->GetSpellAuraOptions()->procCharges;
                spellInfo.procFlags = dbc_spell_entry->GetSpellAuraOptions()->procFlags;
            }

            // data from SpellAuraRestrictions.dbc
            if (dbc_spell_entry->SpellAuraRestrictionsId && dbc_spell_entry->GetSpellAuraRestrictions() != nullptr)
            {
                spellInfo.CasterAuraState = dbc_spell_entry->GetSpellAuraRestrictions()->CasterAuraState;
                spellInfo.TargetAuraState = dbc_spell_entry->GetSpellAuraRestrictions()->TargetAuraState;
                spellInfo.CasterAuraStateNot = dbc_spell_entry->GetSpellAuraRestrictions()->CasterAuraStateNot;
                spellInfo.TargetAuraStateNot = dbc_spell_entry->GetSpellAuraRestrictions()->TargetAuraStateNot;
                spellInfo.casterAuraSpell = dbc_spell_entry->GetSpellAuraRestrictions()->casterAuraSpell;
                spellInfo.targetAuraSpell = dbc_spell_entry->GetSpellAuraRestrictions()->targetAuraSpell;
                spellInfo.casterAuraSpellNot = dbc_spell_entry->GetSpellAuraRestrictions()->excludeCasterAuraSpell;
                spellInfo.targetAuraSpellNot = dbc_spell_entry->GetSpellAuraRestrictions()->excludeTargetAuraSpell;
            }

            // data from SpellCastingRequirements.dbc
            if (dbc_spell_entry->SpellCastingRequirementsId && dbc_spell_entry->GetSpellCastingRequirements() != nullptr)
            {
                spellInfo.FacingCasterFlags = dbc_spell_entry->GetSpellCastingRequirements()->FacingCasterFlags;
                spellInfo.RequiresAreaId = dbc_spell_entry->GetSpellCastingRequirements()->AreaGroupId;
                spellInfo.RequiresSpellFocus = dbc_spell_entry->GetSpellCastingRequirements()->RequiresSpellFocus;
            }

            // data from SpellCategories.dbc
            if (dbc_spell_entry->SpellCategoriesId && dbc_spell_entry->GetSpellCategories() != nullptr)
            {
                spellInfo.Category = dbc_spell_entry->GetSpellCategories()->Category;
                spellInfo.DispelType = dbc_spell_entry->GetSpellCategories()->Dispel;
                spellInfo.Spell_Dmg_Type = dbc_spell_entry->GetSpellCategories()->DmgClass;
                spellInfo.MechanicsType = dbc_spell_entry->GetSpellCategories()->Mechanic;
                spellInfo.PreventionType = dbc_spell_entry->GetSpellCategories()->PreventionType;
                spellInfo.StartRecoveryCategory = dbc_spell_entry->GetSpellCategories()->StartRecoveryCategory;
            }

            // data from SpellClassOptions.dbc
            if (dbc_spell_entry->SpellClassOptionsId && dbc_spell_entry->GetSpellClassOptions() != nullptr)
            {
                spellInfo.SpellFamilyName = dbc_spell_entry->GetSpellClassOptions()->SpellFamilyName;

                for (uint8 i = 0; i < 3; ++i)
                {
                    spellInfo.SpellGroupType[i] = dbc_spell_entry->GetSpellClassOptions()->SpellFamilyFlags[i];
                }

            }

            // data from SpellCooldowns.dbc
            if (dbc_spell_entry->SpellCooldownsId && dbc_spell_entry->GetSpellCooldowns() != nullptr)
            {
                spellInfo.CategoryRecoveryTime = dbc_spell_entry->GetSpellCooldowns()->CategoryRecoveryTime;
                spellInfo.RecoveryTime = dbc_spell_entry->GetSpellCooldowns()->RecoveryTime;
                spellInfo.StartRecoveryTime = dbc_spell_entry->GetSpellCooldowns()->StartRecoveryTime;
            }

            // data from SpellEquippedItems.dbc
            if (dbc_spell_entry->SpellEquippedItemsId && dbc_spell_entry->GetSpellEquippedItems() != nullptr)
            {
                spellInfo.EquippedItemClass = dbc_spell_entry->GetSpellEquippedItems()->EquippedItemClass;
                spellInfo.EquippedItemInventoryTypeMask = dbc_spell_entry->GetSpellEquippedItems()->EquippedItemInventoryTypeMask;
                spellInfo.EquippedItemSubClass = dbc_spell_entry->GetSpellEquippedItems()->EquippedItemSubClassMask;
            }

            // data from SpellInterrupts.dbc
            if (dbc_spell_entry->SpellInterruptsId && dbc_spell_entry->GetSpellInterrupts() != nullptr)
            {
                spellInfo.AuraInterruptFlags = dbc_spell_entry->GetSpellInterrupts()->AuraInterruptFlags;
                spellInfo.ChannelInterruptFlags = dbc_spell_entry->GetSpellInterrupts()->ChannelInterruptFlags;
                spellInfo.InterruptFlags = dbc_spell_entry->GetSpellInterrupts()->InterruptFlags;
            }

            // data from SpellLevels.dbc
            if (dbc_spell_entry->SpellLevelsId && dbc_spell_entry->GetSpellLevels() != nullptr)
            {
                spellInfo.baseLevel = dbc_spell_entry->GetSpellLevels()->baseLevel;
                spellInfo.maxLevel = dbc_spell_entry->GetSpellLevels()->maxLevel;
                spellInfo.spellLevel = dbc_spell_entry->GetSpellLevels()->spellLevel;
            }

            // data from SpellPower.dbc
            if (dbc_spell_entry->SpellPowerId && dbc_spell_entry->GetSpellPower() != nullptr)
            {
                spellInfo.manaCost = dbc_spell_entry->GetSpellPower()->manaCost;
                spellInfo.manaCostPerlevel = dbc_spell_entry->GetSpellPower()->manaCostPerlevel;
                spellInfo.ManaCostPercentage = dbc_spell_entry->GetSpellPower()->ManaCostPercentage;
                spellInfo.manaPerSecond = dbc_spell_entry->GetSpellPower()->manaPerSecond;
                spellInfo.manaPerSecondPerLevel = dbc_spell_entry->GetSpellPower()->manaPerSecondPerLevel;
            }

            // data from SpellReagents.dbc
            if (dbc_spell_entry->SpellReagentsId && dbc_spell_entry->GetSpellReagents() != nullptr)
            {
                for (uint8 i = 0; i < MAX_SPELL_REAGENTS; ++i)
                {
                    spellInfo.Reagent[i] = dbc_spell_entry->GetSpellReagents()->Reagent[i];
                    spellInfo.ReagentCount[i] = dbc_spell_entry->GetSpellReagents()->ReagentCount[i];
                }
            }

            // data from SpellShapeshift.dbc
            if (dbc_spell_entry->SpellShapeshiftId && dbc_spell_entry->GetSpellShapeshift() != nullptr)
            {
                spellInfo.RequiredShapeShift = dbc_spell_entry->GetSpellShapeshift()->Stances;
                spellInfo.ShapeshiftExclude = dbc_spell_entry->GetSpellShapeshift()->StancesNot;
            }

            // data from SpellTargetRestrictions.dbc
            if (dbc_spell_entry->SpellTargetRestrictionsId && dbc_spell_entry->GetSpellTargetRestrictions() != nullptr)
            {
                spellInfo.MaxTargets = dbc_spell_entry->GetSpellTargetRestrictions()->MaxAffectedTargets;
                spellInfo.MaxTargetLevel = dbc_spell_entry->GetSpellTargetRestrictions()->MaxTargetLevel;
                spellInfo.TargetCreatureType = dbc_spell_entry->GetSpellTargetRestrictions()->TargetCreatureType;
                spellInfo.Targets = dbc_spell_entry->GetSpellTargetRestrictions()->Targets;
            }

            // data from SpellTotems.dbc
            if (dbc_spell_entry->SpellTotemsId && dbc_spell_entry->GetSpellTotems() != nullptr)
            {
                for (uint8 i = 0; i < MAX_SPELL_TOTEMS; ++i)
                {
                    spellInfo.TotemCategory[i] = dbc_spell_entry->GetSpellTotems()->TotemCategory[i];
                    spellInfo.Totem[i] = dbc_spell_entry->GetSpellTotems()->Totem[i];
                }
            }

            // data from SpellEffect.dbc
            for (uint8 i = 0; i < MAX_SPELL_EFFECTS; ++i)
            {
                DBC::Structures::SpellEffectEntry const* spell_effect_entry = GetSpellEffectEntry(spell_id, SpellEffectIndex(i));
                if (spell_effect_entry == nullptr)
                {
                    continue;
                }
                else
                {
                    spellInfo.Effect[i] = spell_effect_entry->Effect;
                    spellInfo.EffectMultipleValue[i] = spell_effect_entry->EffectMultipleValue;
                    spellInfo.EffectApplyAuraName[i] = spell_effect_entry->EffectApplyAuraName;
                    spellInfo.EffectAmplitude[i] = spell_effect_entry->EffectAmplitude;
                    spellInfo.EffectBasePoints[i] = spell_effect_entry->EffectBasePoints;
                    spellInfo.EffectBonusMultiplier[i] = spell_effect_entry->EffectBonusMultiplier;
                    spellInfo.dmg_multiplier[i] = spell_effect_entry->EffectDamageMultiplier;
                    spellInfo.EffectChainTarget[i] = spell_effect_entry->EffectChainTarget;
                    spellInfo.EffectDieSides[i] = spell_effect_entry->EffectDieSides;
                    spellInfo.EffectItemType[i] = spell_effect_entry->EffectItemType;
                    spellInfo.EffectMechanic[i] = spell_effect_entry->EffectMechanic;
                    spellInfo.EffectMiscValue[i] = spell_effect_entry->EffectMiscValue;
                    spellInfo.EffectMiscValueB[i] = spell_effect_entry->EffectMiscValueB;
                    spellInfo.EffectPointsPerComboPoint[i] = spell_effect_entry->EffectPointsPerComboPoint;
                    spellInfo.EffectRadiusIndex[i] = spell_effect_entry->EffectRadiusIndex;
                    spellInfo.EffectRadiusMaxIndex[i] = spell_effect_entry->EffectRadiusMaxIndex;
                    spellInfo.EffectRealPointsPerLevel[i] = spell_effect_entry->EffectRealPointsPerLevel;
                    spellInfo.EffectSpellClassMask[i] = spell_effect_entry->EffectSpellClassMask[i];
                    spellInfo.EffectTriggerSpell[i] = spell_effect_entry->EffectTriggerSpell;
                    spellInfo.EffectImplicitTargetA[i] = spell_effect_entry->EffectImplicitTargetA;
                    spellInfo.EffectImplicitTargetB[i] = spell_effect_entry->EffectImplicitTargetB;
                    spellInfo.EffectSpellId[i] = spell_effect_entry->EffectSpellId;
                    spellInfo.EffectIndex[i] = spell_effect_entry->EffectIndex;
                }
            }

        }
    }
#endif
}

SpellInfo* SpellCustomizations::GetSpellInfo(uint32 spell_id)
{
    SpellInfoContainer::const_iterator itr = _spellInfoContainerStore.find(spell_id);
    if (itr != _spellInfoContainerStore.end())
        return const_cast<SpellInfo*>(&itr->second);

    return nullptr;
}

SpellCustomizations::SpellInfoContainer* SpellCustomizations::GetSpellInfoStore()
{
    return &_spellInfoContainerStore;
}

void SpellCustomizations::StartSpellCustomization()
{
    LoadSpellInfoData();

    LOG_DEBUG("Successfull started");

    LoadSpellRanks();
    LoadSpellCustomAssign();
    LoadSpellCustomCoefFlags();
    LoadSpellProcs();

    for (auto it = sSpellCustomizations.GetSpellInfoStore()->begin(); it != sSpellCustomizations.GetSpellInfoStore()->end(); ++it)
    {
        auto spellentry = GetSpellInfo(it->first);
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

            SpellInfo* spell_entry = GetSpellInfo(spell_id);
            if (spell_entry != nullptr)
            {
                spell_entry->custom_RankNumber = pRank;
                ++spell_rank_count;
            }
            else
            {
                LogDebugFlag(LF_DB_TABLES, "SpellCustomizations::LoadSpellRanks : your spell_ranks table includes an invalid spell %u.", spell_id);
                continue;
            }

        } while (result->NextRow());
        delete result;
    }

    if (spell_rank_count > 0)
    {
        LOG_DETAIL("Loaded %u custom_RankNumbers from spell_ranks table", spell_rank_count);
    }
    else
    {
        LogDebugFlag(LF_DB_TABLES, "SpellCustomizations::LoadSpellRanks : Your spell_ranks table is empty!");
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

            SpellInfo* spell_entry = GetSpellInfo(spell_id);
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
                LogDebugFlag(LF_DB_TABLES, "SpellCustomizations::LoadSpellCustomAssign", "your spell_custom_assign table includes an invalid spell %u.", spell_id);
                continue;
            }

        } while (result->NextRow());
        delete result;
    }

    if (spell_custom_assign_count > 0)
    {
        LOG_DETAIL("Loaded %u attributes from spell_custom_assign table", spell_custom_assign_count);
    }
    else
    {
        LogDebugFlag(LF_DB_TABLES, "SpellCustomizations::LoadSpellCustomAssign", "Your spell_custom_assign table is empty!");
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

            SpellInfo* spell_entry = GetSpellInfo(spell_id);
            if (spell_entry != nullptr)
            {
                spell_entry->custom_spell_coef_flags = coef_flags;
                ++spell_custom_coef_flags_count;
            }
            else
            {
                LogDebugFlag(LF_DB_TABLES, "SpellCustomizations::LoadSpellCustomCoefFlags : your spell_coef_flags table includes an invalid spell %u.", spell_id);
                continue;
            }

        } while (result->NextRow());
        delete result;
    }

    if (spell_custom_coef_flags_count > 0)
    {
        LogDebugFlag(LF_DB_TABLES, "SpellCustomizations::LoadSpellCustomCoefFlags : Loaded %u attributes from spell_coef_flags table", spell_custom_coef_flags_count);
    }
    else
    {
        LogDebugFlag(LF_DB_TABLES, "SpellCustomizations::LoadSpellCustomCoefFlags : Your spell_coef_flags table is empty!");
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

            auto spell_entry = GetSpellInfo(spell_id);
            if (spell_entry != nullptr)
            {
                for (uint8 x = 0; x < 3; ++x)
                {
                    if (spell_entry->custom_ProcOnNameHash[x] == 0)
                    {
                        spell_entry->custom_ProcOnNameHash[x] = name_hash;
                    }
                }


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
                LOG_ERROR("Invalid spellID %u in table spell_proc", spell_id);
            }
        } while (result->NextRow());
        delete result;
    }

    if (spell_procs_count > 0)
    {
        LogDebugFlag(LF_DB_TABLES, "SpellCustomizations::LoadSpellProcs : Loaded %u proc definitions from spell_proc table", spell_procs_count);
    }
    else
    {
        LogDebugFlag(LF_DB_TABLES, "SpellCustomizations::LoadSpellProcs : Your spell_proc table is empty!");
    }
}

///Fix if it is a periodic trigger with amplitude = 0, to avoid division by zero
void SpellCustomizations::SetEffectAmplitude(SpellInfo* spell_entry)
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

                LogDebugFlag(LF_DB_TABLES, "SpellCustomizations::SetEffectAmplitude : EffectAmplitude applied Spell - %s (%u)", spell_entry->Name.c_str(), spell_entry->Id);
            }
        }
    }
}

void SpellCustomizations::SetAuraFactoryFunc(SpellInfo* spell_entry)
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
        LogDebugFlag(LF_DB_TABLES, "SpellCustomizations::SetAuraFactoryFunc : AuraFactoryFunc definitions applied to Spell - %s (%u)", spell_entry->Name.c_str(), spell_entry->Id);
    }
}

void SpellCustomizations::SetMeleeSpellBool(SpellInfo* spell_entry)
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
        LogDebugFlag(LF_DB_TABLES, "SpellCustomizations::SetMeleeSpellBool : custom_is_melee_spell = true for Spell - %s (%u)", spell_entry->Name.c_str(), spell_entry->Id);
    }
}

void SpellCustomizations::SetRangedSpellBool(SpellInfo* spell_entry)
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
        LogDebugFlag(LF_DB_TABLES, "SpellCustomizations::SetRangedSpellBool : custom_is_ranged_spell = true for Spell - %s (%u)", spell_entry->Name.c_str(), spell_entry->Id);
    }
}

void SpellCustomizations::SetMissingCIsFlags(SpellInfo* spell_entry)
{
    // Zyres: Special cases, not handled in spell_custom_assign!
    if (spell_entry->isDamagingSpell())
        spell_entry->custom_c_is_flags |= SPELL_FLAG_IS_DAMAGING;
    if (spell_entry->isHealingSpell())
        spell_entry->custom_c_is_flags |= SPELL_FLAG_IS_HEALING;
    if (spell_entry->isTargetingStealthed())
        spell_entry->custom_c_is_flags |= SPELL_FLAG_IS_TARGETINGSTEALTHED;
    if (spell_entry->isRequireCooldownSpell())
        spell_entry->custom_c_is_flags |= SPELL_FLAG_IS_REQUIRECOOLDOWNUPDATE;
}

void SpellCustomizations::SetCustomFlags(SpellInfo* spell_entry)
{
    // Currently only set for 781 Disengage
    if (spell_entry->Id != 781)
    {
        return;
    }

    spell_entry->CustomFlags = CUSTOM_FLAG_SPELL_REQUIRES_COMBAT;
}

void SpellCustomizations::SetOnShapeshiftChange(SpellInfo* spell_entry)
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

void SpellCustomizations::SetAlwaysApply(SpellInfo* spell_entry)
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
