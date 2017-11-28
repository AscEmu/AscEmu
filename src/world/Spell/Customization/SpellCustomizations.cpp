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
#include "Spell/Definitions/DiminishingGroup.h"

initialiseSingleton(SpellCustomizations);

//\brief: This file includes all setted custom values and/or spell.dbc values (overwrite)
// Set the values you want based on spell Id (Do not set your values based on some text!)

SpellCustomizations::SpellCustomizations() {}
SpellCustomizations::~SpellCustomizations() {}

void SpellCustomizations::LoadSpellInfoData()
{
#if VERSION_STRING != Cata
    for (uint32 i = 0; i < MAX_SPELL_ID; ++i)
    {

        DBC::Structures::SpellEntry const* dbc_spell_entry = sSpellStore.LookupEntry(i);
        if (dbc_spell_entry != nullptr)
        {
            uint32 spell_id = dbc_spell_entry->Id;
            SpellInfo& spellInfo = _spellInfoContainerStore[spell_id];
            spellInfo.setId(spell_id);
            spellInfo.setAttributes(dbc_spell_entry->Attributes);
            spellInfo.setAttributesEx(dbc_spell_entry->AttributesEx);
            spellInfo.setAttributesExB(dbc_spell_entry->AttributesExB);
            spellInfo.setAttributesExC(dbc_spell_entry->AttributesExC);
            spellInfo.setAttributesExD(dbc_spell_entry->AttributesExD);
            spellInfo.setAttributesExE(dbc_spell_entry->AttributesExE);
            spellInfo.setAttributesExF(dbc_spell_entry->AttributesExF);
#if VERSION_STRING > TBC
            spellInfo.setAttributesExG(dbc_spell_entry->AttributesExG);
#endif
            spellInfo.setRequiredShapeShift(dbc_spell_entry->RequiredShapeShift);
            spellInfo.setShapeshiftExclude(dbc_spell_entry->ShapeshiftExclude);
            spellInfo.setTargets(dbc_spell_entry->Targets);
            spellInfo.setTargetCreatureType(dbc_spell_entry->TargetCreatureType);
            spellInfo.setRequiresSpellFocus(dbc_spell_entry->RequiresSpellFocus);
            spellInfo.setFacingCasterFlags(dbc_spell_entry->FacingCasterFlags);
            spellInfo.setCasterAuraState(dbc_spell_entry->CasterAuraState);
            spellInfo.setTargetAuraState(dbc_spell_entry->TargetAuraState);
            spellInfo.setCasterAuraStateNot(dbc_spell_entry->CasterAuraStateNot);
            spellInfo.setTargetAuraStateNot(dbc_spell_entry->TargetAuraStateNot);
#if VERSION_STRING > TBC
            spellInfo.setCasterAuraSpell(dbc_spell_entry->casterAuraSpell);
            spellInfo.setTargetAuraSpell(dbc_spell_entry->targetAuraSpell);
            spellInfo.setCasterAuraSpellNot(dbc_spell_entry->casterAuraSpellNot);
            spellInfo.setTargetAuraSpellNot(dbc_spell_entry->targetAuraSpellNot);
#endif
            spellInfo.setCastingTimeIndex(dbc_spell_entry->CastingTimeIndex);
            spellInfo.setRecoveryTime(dbc_spell_entry->RecoveryTime);
            spellInfo.setCategoryRecoveryTime(dbc_spell_entry->CategoryRecoveryTime);
            spellInfo.setInterruptFlags(dbc_spell_entry->InterruptFlags);
            spellInfo.setAuraInterruptFlags(dbc_spell_entry->AuraInterruptFlags);
            spellInfo.setChannelInterruptFlags(dbc_spell_entry->ChannelInterruptFlags);
            spellInfo.setProcFlags(dbc_spell_entry->procFlags);
            spellInfo.setProcChance(dbc_spell_entry->procChance);
            spellInfo.setProcCharges(dbc_spell_entry->procCharges);
            spellInfo.setMaxLevel(dbc_spell_entry->maxLevel);
            spellInfo.setBaseLevel(dbc_spell_entry->baseLevel);
            spellInfo.setSpellLevel(dbc_spell_entry->spellLevel);
            spellInfo.setDurationIndex(dbc_spell_entry->DurationIndex);
            spellInfo.setPowerType(dbc_spell_entry->powerType);
            spellInfo.setManaCost(dbc_spell_entry->manaCost);
            spellInfo.setManaCostPerlevel(dbc_spell_entry->manaCostPerlevel);
            spellInfo.setManaPerSecond(dbc_spell_entry->manaPerSecond);
            spellInfo.setManaPerSecondPerLevel(dbc_spell_entry->manaPerSecondPerLevel);
            spellInfo.setRangeIndex(dbc_spell_entry->rangeIndex);
            spellInfo.setSpeed(dbc_spell_entry->speed);
            spellInfo.setModalNextSpell(dbc_spell_entry->modalNextSpell);
            spellInfo.setMaxstack(dbc_spell_entry->maxstack);

            for (uint8 j = 0; j < MAX_SPELL_TOTEMS; ++j)
                spellInfo.setTotem(dbc_spell_entry->Totem[j], j);

            for (uint8 j = 0; j < MAX_SPELL_REAGENTS; ++j)
                spellInfo.setReagent(dbc_spell_entry->Reagent[j], j);

            for (uint8 j = 0; j < MAX_SPELL_REAGENTS; ++j)
                spellInfo.setReagentCount(dbc_spell_entry->ReagentCount[j], j);

            spellInfo.setEquippedItemClass(dbc_spell_entry->EquippedItemClass);
            spellInfo.setEquippedItemSubClass(dbc_spell_entry->EquippedItemSubClass);
            spellInfo.setRequiredItemFlags(dbc_spell_entry->RequiredItemFlags);

            for (uint8 j = 0; j < MAX_SPELL_EFFECTS; ++j)
                spellInfo.setEffect(dbc_spell_entry->Effect[j], j);

            for (uint8 j = 0; j < MAX_SPELL_EFFECTS; ++j)
                spellInfo.setEffectDieSides(dbc_spell_entry->EffectDieSides[j], j);

            for (uint8 j = 0; j < MAX_SPELL_EFFECTS; ++j)
                spellInfo.setEffectRealPointsPerLevel(dbc_spell_entry->EffectRealPointsPerLevel[j], j);

            for (uint8 j = 0; j < MAX_SPELL_EFFECTS; ++j)
                spellInfo.setEffectBasePoints(dbc_spell_entry->EffectBasePoints[j], j);

            for (uint8 j = 0; j < MAX_SPELL_EFFECTS; ++j)
                spellInfo.setEffectMechanic(dbc_spell_entry->EffectMechanic[j], j);

            for (uint8 j = 0; j < MAX_SPELL_EFFECTS; ++j)
                spellInfo.setEffectImplicitTargetA(dbc_spell_entry->EffectImplicitTargetA[j], j);

            for (uint8 j = 0; j < MAX_SPELL_EFFECTS; ++j)
                spellInfo.setEffectImplicitTargetB(dbc_spell_entry->EffectImplicitTargetB[j], j);

            for (uint8 j = 0; j < MAX_SPELL_EFFECTS; ++j)
                spellInfo.setEffectRadiusIndex(dbc_spell_entry->EffectRadiusIndex[j], j);

            for (uint8 j = 0; j < MAX_SPELL_EFFECTS; ++j)
                spellInfo.setEffectApplyAuraName(dbc_spell_entry->EffectApplyAuraName[j], j);

            for (uint8 j = 0; j < MAX_SPELL_EFFECTS; ++j)
                spellInfo.setEffectAmplitude(dbc_spell_entry->EffectAmplitude[j], j);

            for (uint8 j = 0; j < MAX_SPELL_EFFECTS; ++j)
                spellInfo.setEffectMultipleValue(dbc_spell_entry->EffectMultipleValue[j], j);

            for (uint8 j = 0; j < MAX_SPELL_EFFECTS; ++j)
                spellInfo.setEffectChainTarget(dbc_spell_entry->EffectChainTarget[j], j);

            for (uint8 j = 0; j < MAX_SPELL_EFFECTS; ++j)
                spellInfo.setEffectItemType(dbc_spell_entry->EffectItemType[j], j);

            for (uint8 j = 0; j < MAX_SPELL_EFFECTS; ++j)
                spellInfo.setEffectMiscValue(dbc_spell_entry->EffectMiscValue[j], j);

            for (uint8 j = 0; j < MAX_SPELL_EFFECTS; ++j)
                spellInfo.setEffectMiscValueB(dbc_spell_entry->EffectMiscValueB[j], j);

            for (uint8 j = 0; j < MAX_SPELL_EFFECTS; ++j)
                spellInfo.setEffectTriggerSpell(dbc_spell_entry->EffectTriggerSpell[j], j);

            for (uint8 j = 0; j < MAX_SPELL_EFFECTS; ++j)
                spellInfo.setEffectPointsPerComboPoint(dbc_spell_entry->EffectPointsPerComboPoint[j], j);

#if VERSION_STRING > TBC
            for (uint8 x = 0; x < 3; ++x)
                for (uint8 j = 0; j < 3; ++j)
                    spellInfo.setEffectSpellClassMask(dbc_spell_entry->EffectSpellClassMask[x][j], x, j);
#endif

            spellInfo.setSpellVisual(dbc_spell_entry->SpellVisual);
            spellInfo.setField114(dbc_spell_entry->field114);
            spellInfo.setSpellIconID(dbc_spell_entry->spellIconID);
            spellInfo.setActiveIconID(dbc_spell_entry->activeIconID);
            spellInfo.setSpellPriority(dbc_spell_entry->spellPriority);
            spellInfo.setName(dbc_spell_entry->Name);
            spellInfo.setRank(dbc_spell_entry->Rank);
            spellInfo.setDescription(dbc_spell_entry->Description);
            spellInfo.setBuffDescription(dbc_spell_entry->BuffDescription);
            spellInfo.setManaCostPercentage(dbc_spell_entry->ManaCostPercentage);
            spellInfo.setStartRecoveryCategory(dbc_spell_entry->StartRecoveryCategory);
            spellInfo.setStartRecoveryTime(dbc_spell_entry->StartRecoveryTime);
#if VERSION_STRING > TBC
            spellInfo.setMaxTargetLevel(dbc_spell_entry->MaxTargetLevel);
#endif
            spellInfo.setSpellFamilyName(dbc_spell_entry->SpellFamilyName);

#if VERSION_STRING > TBC
            for (uint8 j = 0; j < MAX_SPELL_EFFECTS; ++j)
                spellInfo.setSpellGroupType(dbc_spell_entry->SpellGroupType[j], j);
#endif

            spellInfo.setMaxTargets(dbc_spell_entry->MaxTargets);
            spellInfo.setSpell_Dmg_Type(dbc_spell_entry->Spell_Dmg_Type);
            spellInfo.setPreventionType(dbc_spell_entry->PreventionType);
            spellInfo.setStanceBarOrder(dbc_spell_entry->StanceBarOrder);

            for (uint8 j = 0; j < MAX_SPELL_EFFECTS; ++j)
                spellInfo.setDmg_multiplier(dbc_spell_entry->dmg_multiplier[j], j);

            spellInfo.setMinFactionID(dbc_spell_entry->MinFactionID);
            spellInfo.setMinReputation(dbc_spell_entry->MinReputation);
            spellInfo.setRequiredAuraVision(dbc_spell_entry->RequiredAuraVision);

            for (uint8 j = 0; j < MAX_SPELL_TOTEM_CATEGORIES; ++j)
                spellInfo.setTotemCategory(dbc_spell_entry->TotemCategory[j], j);

            spellInfo.setRequiresAreaId(dbc_spell_entry->RequiresAreaId);
            spellInfo.setSchool(dbc_spell_entry->School);
#if VERSION_STRING > TBC
            spellInfo.setRuneCostID(dbc_spell_entry->RuneCostID);

            spellInfo.setSpellDifficultyID(dbc_spell_entry->SpellDifficultyID);
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
            spellInfo.setId(spell_id);
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

                for (uint8_t j = 0; j < 3; ++j)
                {
                    spellInfo.SpellGroupType[j] = dbc_spell_entry->GetSpellClassOptions()->SpellFamilyFlags[j];
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
                for (uint8_t j = 0; j < MAX_SPELL_REAGENTS; ++j)
                {
                    spellInfo.Reagent[j] = dbc_spell_entry->GetSpellReagents()->Reagent[j];
                    spellInfo.ReagentCount[j] = dbc_spell_entry->GetSpellReagents()->ReagentCount[j];
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
                for (uint8_t j = 0; j < MAX_SPELL_TOTEMS; ++j)
                {
                    spellInfo.TotemCategory[j] = dbc_spell_entry->GetSpellTotems()->TotemCategory[j];
                    spellInfo.Totem[j] = dbc_spell_entry->GetSpellTotems()->Totem[j];
                }
            }

            // data from SpellEffect.dbc
            for (uint8_t j = 0; j < MAX_SPELL_EFFECTS; ++j)
            {
                DBC::Structures::SpellEffectEntry const* spell_effect_entry = GetSpellEffectEntry(spell_id, SpellEffectIndex(j));
                if (spell_effect_entry == nullptr)
                {
                    continue;
                }
                else
                {
                    spellInfo.Effect[j] = spell_effect_entry->Effect;
                    spellInfo.EffectMultipleValue[j] = spell_effect_entry->EffectMultipleValue;
                    spellInfo.EffectApplyAuraName[j] = spell_effect_entry->EffectApplyAuraName;
                    spellInfo.EffectAmplitude[j] = spell_effect_entry->EffectAmplitude;
                    spellInfo.EffectBasePoints[j] = spell_effect_entry->EffectBasePoints;
                    spellInfo.EffectBonusMultiplier[j] = spell_effect_entry->EffectBonusMultiplier;
                    spellInfo.dmg_multiplier[j] = spell_effect_entry->EffectDamageMultiplier;
                    spellInfo.EffectChainTarget[j] = spell_effect_entry->EffectChainTarget;
                    spellInfo.EffectDieSides[j] = spell_effect_entry->EffectDieSides;
                    spellInfo.EffectItemType[j] = spell_effect_entry->EffectItemType;
                    spellInfo.EffectMechanic[j] = spell_effect_entry->EffectMechanic;
                    spellInfo.EffectMiscValue[j] = spell_effect_entry->EffectMiscValue;
                    spellInfo.EffectMiscValueB[j] = spell_effect_entry->EffectMiscValueB;
                    spellInfo.EffectPointsPerComboPoint[j] = spell_effect_entry->EffectPointsPerComboPoint;
                    spellInfo.EffectRadiusIndex[j] = spell_effect_entry->EffectRadiusIndex;
                    spellInfo.EffectRadiusMaxIndex[j] = spell_effect_entry->EffectRadiusMaxIndex;
                    spellInfo.EffectRealPointsPerLevel[j] = spell_effect_entry->EffectRealPointsPerLevel;
                    spellInfo.EffectSpellClassMask[j] = spell_effect_entry->EffectSpellClassMask[j];
                    spellInfo.EffectTriggerSpell[j] = spell_effect_entry->EffectTriggerSpell;
                    spellInfo.EffectImplicitTargetA[j] = spell_effect_entry->EffectImplicitTargetA;
                    spellInfo.EffectImplicitTargetB[j] = spell_effect_entry->EffectImplicitTargetB;
                    spellInfo.EffectSpellId[j] = spell_effect_entry->EffectSpellId;
                    spellInfo.EffectIndex[j] = spell_effect_entry->EffectIndex;
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

            // loaded but not used!
            // uint32 from_caster_on_self_flag = result->Fetch()[2].GetUInt32();

            bool self_cast_only = result->Fetch()[3].GetBool();
            uint32 c_is_flag = result->Fetch()[4].GetUInt32();

            SpellInfo* spell_entry = GetSpellInfo(spell_id);
            if (spell_entry != nullptr)
            {
                spell_entry->custom_BGR_one_buff_on_target = on_target;
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
            // uint32 name_hash = f[1].GetUInt32();

            auto spell_entry = GetSpellInfo(spell_id);
            if (spell_entry != nullptr)
            {
                if (f[2].GetInt32() >= 0)
                    spell_entry->setProcFlags(f[2].GetUInt32());

                if (f[3].GetBool())
                    spell_entry->addProcFlags(PROC_TARGET_SELF);
                if (f[4].GetInt32() >= 0)
                    spell_entry->setProcChance(f[4].GetUInt32());
                if (f[5].GetInt32() >= 0)
                    spell_entry->setProcCharges(f[5].GetInt32());

                spell_entry->custom_proc_interval = f[6].GetUInt32();

                if (f[7].GetInt32() >= 0)
                    spell_entry->setEffectTriggerSpell(f[7].GetUInt32(), 0);
                if (f[8].GetInt32() >= 0)
                    spell_entry->setEffectTriggerSpell(f[8].GetUInt32(), 1);
                if (f[9].GetInt32() >= 0)
                    spell_entry->setEffectTriggerSpell(f[9].GetUInt32(), 2);

                if (spell_entry->getEffectTriggerSpell(0) > 0)
                    spell_entry->setEffectApplyAuraName(SPELL_AURA_PROC_TRIGGER_SPELL, 0);
                if (spell_entry->getEffectTriggerSpell(1) > 0)
                    spell_entry->setEffectApplyAuraName(SPELL_AURA_PROC_TRIGGER_SPELL, 1);
                if (spell_entry->getEffectTriggerSpell(2) > 0)
                    spell_entry->setEffectApplyAuraName(SPELL_AURA_PROC_TRIGGER_SPELL, 2);

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

// Fix if it is a periodic trigger with amplitude = 0, to avoid division by zero
void SpellCustomizations::SetEffectAmplitude(SpellInfo* spell_entry)
{
    for (uint8 y = 0; y < 3; y++)
    {
        if (spell_entry->getEffect(y) != SPELL_EFFECT_APPLY_AURA)
        {
            continue;
        }
        else
        {
            if ((spell_entry->getEffectApplyAuraName(y) == SPELL_AURA_PERIODIC_TRIGGER_SPELL || spell_entry->getEffectApplyAuraName(y) == SPELL_AURA_PERIODIC_TRIGGER_SPELL_WITH_VALUE) && spell_entry->getEffectApplyAuraName(y) == 0)
            {
                spell_entry->setEffectAmplitude(1000, y);

                LogDebugFlag(LF_DB_TABLES, "SpellCustomizations::SetEffectAmplitude : EffectAmplitude applied Spell - %s (%u)", spell_entry->getName().c_str(), spell_entry->getId());
            }
        }
    }
}

void SpellCustomizations::SetAuraFactoryFunc(SpellInfo* spell_entry)
{
    bool spell_aura_factory_functions_loaded = false;

    for (uint8 y = 0; y < 3; y++)
    {
        if (spell_entry->getEffect(y) != SPELL_EFFECT_APPLY_AURA)
        {
            continue;
        }
        else
        {
            if (spell_entry->getEffectApplyAuraName(y) == SPELL_AURA_SCHOOL_ABSORB && spell_entry->AuraFactoryFunc == nullptr)
            {
                spell_entry->AuraFactoryFunc = (void * (*)) &AbsorbAura::Create;

                spell_aura_factory_functions_loaded = true;
            }
        }
    }

    if (spell_aura_factory_functions_loaded)
    {
        LogDebugFlag(LF_DB_TABLES, "SpellCustomizations::SetAuraFactoryFunc : AuraFactoryFunc definitions applied to Spell - %s (%u)", spell_entry->getName().c_str(), spell_entry->getId());
    }
}

void SpellCustomizations::SetMeleeSpellBool(SpellInfo* spell_entry)
{
    for (uint8 z = 0; z < 3; z++)
    {
        if (spell_entry->getEffect(z) == SPELL_EFFECT_SCHOOL_DAMAGE && spell_entry->getSpell_Dmg_Type() == SPELL_DMG_TYPE_MELEE)
        {
            spell_entry->custom_is_melee_spell = true;
            continue;
        }

        switch (spell_entry->getEffect(z))
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
        LogDebugFlag(LF_DB_TABLES, "SpellCustomizations::SetMeleeSpellBool : custom_is_melee_spell = true for Spell - %s (%u)", spell_entry->getName().c_str(), spell_entry->getId());
    }
}

void SpellCustomizations::SetRangedSpellBool(SpellInfo* spell_entry)
{
    for (uint8 z = 0; z < 3; z++)
    {
        if (spell_entry->getEffect(z) == SPELL_EFFECT_SCHOOL_DAMAGE && spell_entry->getSpell_Dmg_Type() == SPELL_DMG_TYPE_RANGED)
        {
            spell_entry->custom_is_ranged_spell = true;
        }
    }

    if (spell_entry->custom_is_ranged_spell)
    {
        LogDebugFlag(LF_DB_TABLES, "SpellCustomizations::SetRangedSpellBool : custom_is_ranged_spell = true for Spell - %s (%u)", spell_entry->getName().c_str(), spell_entry->getId());
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
    if (spell_entry->getId() == 781)
    {
        spell_entry->CustomFlags = CUSTOM_FLAG_SPELL_REQUIRES_COMBAT;
    }
}

void SpellCustomizations::SetOnShapeshiftChange(SpellInfo* spell_entry)
{
    // Currently only for spell Track Humanoids
    if (spell_entry->getId() == 5225 || spell_entry->getId() == 19883)
    {
        spell_entry->custom_apply_on_shapeshift_change = true;
    }
}

bool SpellCustomizations::isAlwaysApply(SpellInfo* spell_entry)
{
    switch (spell_entry->getId())
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
            return true;
        } break;
        default:
            return false;
    }
}

// Calculate the Diminishing Group. This is based on id.
// this off course is very hacky, but as its made done in a proper way
// I leave it here.
uint32_t SpellCustomizations::getDiminishingGroup(uint32_t id)
{
    int32_t grp = -1;
    bool pve = false;

    switch (id)
    {
        // SPELL_HASH_WYVERN_STING:
        case 19386:
        case 24131:
        case 24132:
        case 24133:
        case 24134:
        case 24135:
        case 24335:
        case 24336:
        case 26180:
        case 26233:
        case 26748:
        case 27068:
        case 27069:
        case 41186:
        case 49009:
        case 49010:
        case 49011:
        case 49012:
        case 65877:
        case 65878:
        // SPELL_HASH_SLEEP:
        case 700:
        case 1090:
        case 8399:
        case 9159:
        case 9160:
        case 12098:
        case 15970:
        case 20663:
        case 20669:
        case 20989:
        case 24004:
        case 24664:
        case 24778:
        case 31292:
        case 31298:
        case 31541:
        case 31548:
        case 34801:
        case 36402:
        case 41396:
        case 52721:
        case 52742:
        case 53045:
        case 58849:
        case 59165:
        case 66290:
        // SPELL_HASH_RECKLESS_CHARGE: //Gobling Rocket Helmet
        case 13327:
        case 22641:
        case 22646:
        {
            grp = DIMINISHING_GROUP_SLEEP;
        } break;

        // SPELL_HASH_CYCLONE:
        case 29538:
        case 32334:
        case 33786:
        case 38516:
        case 38517:
        case 39594:
        case 40578:
        case 43120:
        case 43121:
        case 43528:
        case 60236:
        case 61662:
        case 62632:
        case 62633:
        case 65859:
        case 69699:
        // SPELL_HASH_BLIND:
        case 2094:
        case 21060:
        case 34654:
        case 34694:
        case 42972:
        case 43433:
        case 65960:
        {
            grp = DIMINISHING_GROUP_BLIND_CYCLONE;
            pve = true;
        } break;

        // SPELL_HASH_GOUGE:
        case 1776:
        case 1777:
        case 8629:
        case 11285:
        case 11286:
        case 12540:
        case 13579:
        case 24698:
        case 28456:
        case 29425:
        case 34940:
        case 36862:
        case 38764:
        case 38863:
        // SPELL_HASH_REPENTANCE: // Repentance
        case 20066:
        case 29511:
        case 32779:
        case 66008:
        // SPELL_HASH_SAP:
        case 2070:
        case 6770:
        case 11297:
        case 30980:
        case 51724:
        // SPELL_HASH_POLYMORPH: // Polymorph
        case 118:
        case 12824:
        case 12825:
        case 12826:
        case 13323:
        case 14621:
        case 15534:
        case 27760:
        case 28271:
        case 28272:
        case 28285:
        case 29124:
        case 29848:
        case 30838:
        case 34639:
        case 36840:
        case 38245:
        case 38896:
        case 41334:
        case 43309:
        case 46280:
        case 58537:
        case 61025:
        case 61305:
        case 61721:
        case 61780:
        case 65801:
        case 66043:
        case 68311:
        case 71319:
        // SPELL_HASH_POLYMORPH__CHICKEN: // Chicken
        case 228:
        // SPELL_HASH_POLYMORPH__CRAFTY_WOBBLESPROCKET: // Errr right?
        case 45683:
        case 45684:
        // SPELL_HASH_POLYMORPH__SHEEP: // Good ol' sheep
        case 851:
        case 61816:
        case 61839:
        // SPELL_HASH_POLYMORPH___PENGUIN: // Penguiiiin!!! :D
        case 59634:
        // SPELL_HASH_MAIM: // Maybe here?
        case 22570:
        case 49802:
        // SPELL_HASH_HEX: // Should share diminish group with polymorph, but not interruptflags.
        case 11641:
        case 16097:
        case 16707:
        case 16708:
        case 16709:
        case 17172:
        case 18503:
        case 22566:
        case 24053:
        case 29044:
        case 36700:
        case 40400:
        case 46295:
        case 51514:
        case 53439:
        case 66054:
        {
            grp = DIMINISHING_GROUP_GOUGE_POLY_SAP;
        } break;

        // SPELL_HASH_FEAR:
        case 5782:
        case 6213:
        case 6215:
        case 12096:
        case 12542:
        case 22678:
        case 26070:
        case 26580:
        case 26661:
        case 27641:
        case 27990:
        case 29168:
        case 29321:
        case 30002:
        case 30530:
        case 30584:
        case 30615:
        case 31358:
        case 31970:
        case 32241:
        case 33547:
        case 33924:
        case 34259:
        case 38154:
        case 38595:
        case 38660:
        case 39119:
        case 39176:
        case 39210:
        case 39415:
        case 41150:
        case 46561:
        case 51240:
        case 59669:
        case 65809:
        case 68950:
        // SPELL_HASH_PSYCHIC_SCREAM:
        case 8122:
        case 8124:
        case 10888:
        case 10890:
        case 13704:
        case 15398:
        case 22884:
        case 26042:
        case 27610:
        case 34322:
        case 43432:
        case 65543:
        // SPELL_HASH_SEDUCTION:
        case 6358:
        case 6359:
        case 20407:
        case 29490:
        case 30850:
        case 31865:
        // SPELL_HASH_HOWL_OF_TERROR:
        case 5484:
        case 17928:
        case 39048:
        case 50577:
        // SPELL_HASH_SCARE_BEAST:
        case 1513:
        case 14326:
        case 14327:
        {
            grp = DIMINISHING_GROUP_FEAR;
        } break;

        // SPELL_HASH_DEATH_COIL:
        case 6789:
        case 17925:
        case 17926:
        case 27223:
        case 28412:
        case 30500:
        case 30741:
        case 32709:
        case 33130:
        case 34437:
        case 35954:
        case 38065:
        case 39661:
        case 41070:
        case 44142:
        case 46283:
        case 47541:
        case 47632:
        case 47633:
        case 47859:
        case 47860:
        case 49892:
        case 49893:
        case 49894:
        case 49895:
        case 50668:
        case 52375:
        case 52376:
        case 53769:
        case 55209:
        case 55210:
        case 55320:
        case 56362:
        case 59134:
        case 60949:
        case 62900:
        case 62901:
        case 62902:
        case 62903:
        case 62904:
        case 65820:
        case 66019:
        case 67929:
        case 67930:
        case 67931:
        case 68139:
        case 68140:
        case 68141:
        case 71490:
        {
            grp = DIMINISHING_GROUP_HORROR;
        } break;

        // SPELL_HASH_ENSLAVE_DEMON: // Enslave Demon
        case 1098:
        case 11725:
        case 11726:
        case 20882:
        case 61191:
        // SPELL_HASH_MIND_CONTROL:
        case 605:
        case 11446:
        case 15690:
        case 36797:
        case 36798:
        case 43550:
        case 43871:
        case 43875:
        case 45112:
        case 67229:
        // SPELL_HASH_TURN_EVIL:
        case 10326:
        {
            grp = DIMINISHING_GROUP_CHARM; //Charm???
        } break;

        // SPELL_HASH_KIDNEY_SHOT:
        case 408:
        case 8643:
        case 27615:
        case 30621:
        case 30832:
        case 32864:
        case 41389:
        case 49616:
        case 72335:
        {
            grp = DIMINISHING_GROUP_KIDNEY_SHOT;
            pve = true;
        }
        break;

        // SPELL_HASH_BANISH: // Banish
        case 710:
        case 8994:
        case 18647:
        case 24466:
        case 27565:
        case 30231:
        case 35182:
        case 37527:
        case 37546:
        case 37833:
        case 38009:
        case 38376:
        case 38791:
        case 39622:
        case 39674:
        case 40370:
        case 44765:
        case 44836:
        case 71298:
        {
            grp = DIMINISHING_GROUP_BANISH;
        } break;


        // group where only 10s limit in pvp is applied, not DR

        // SPELL_HASH_FREEZING_TRAP_EFFECT: // Freezing Trap Effect
        case 3355:
        case 14308:
        case 14309:
        case 31932:
        case 55041:
        // SPELL_HASH_HAMSTRING: // Hamstring
        case 1715:
        case 7372:
        case 7373:
        case 9080:
        case 25212:
        case 26141:
        case 26211:
        case 27584:
        case 29667:
        case 30989:
        case 31553:
        case 38262:
        case 38995:
        case 48639:
        case 62845:
        // SPELL_HASH_CURSE_OF_TONGUES:
        case 1714:
        case 11719:
        case 12889:
        case 13338:
        case 15470:
        case 25195:
        {
            grp = DIMINISHING_GROUP_NOT_DIMINISHED;
        } break;

        // SPELL_HASH_RIPOSTE: // Riposte
        case 14251:
        case 34097:
        case 34099:
        case 41392:
        case 41393:
        // SPELL_HASH_DISARM: // Disarm
        case 676:
        case 1843:
        case 6713:
        case 8379:
        case 11879:
        case 13534:
        case 15752:
        case 22691:
        case 27581:
        case 30013:
        case 31955:
        case 36139:
        case 41062:
        case 48883:
        case 65935:
        {
            grp = DIMINISHING_GROUP_DISARM;
        } break;

        // SPELL_HASH_BASH
        case 5211:
        case 6798:
        case 8983:
        case 25515:
        case 43612:
        case 57094:
        case 58861:
        //SPELL_HASH_IMPACT
        case 11103:
        case 12355:
        case 12357:
        case 12358:
        case 64343:
        //SPELL_HASH_CHEAP_SHOT
        case 1833:
        case 6409:
        case 14902:
        case 30986:
        case 31819:
        case 31843:
        case 34243:
        //SPELL_HASH_SHADOWFURY
        case 30283:
        case 30413:
        case 30414:
        case 35373:
        case 39082:
        case 45270:
        case 47846:
        case 47847:
        case 56733:
        case 61463:
        //SPELL_HASH_CHARGE_STUN
        case 7922:
        case 65929:
        //SPELL_HASH_INTERCEPT
        case 20252:
        case 20253:
        case 20614:
        case 20615:
        case 20616:
        case 20617:
        case 25272:
        case 25273:
        case 25274:
        case 25275:
        case 27577:
        case 27826:
        case 30151:
        case 30153:
        case 30154:
        case 30194:
        case 30195:
        case 30197:
        case 30198:
        case 30199:
        case 30200:
        case 47995:
        case 47996:
        case 50823:
        case 58743:
        case 58747:
        case 58769:
        case 61490:
        case 61491:
        case 67540:
        case 67573:
        //SPELL_HASH_CONCUSSION_BLOW
        case 12809:
        case 22427:
        case 32588:
        case 52719:
        case 54132:
        //SPELL_HASH_INTIMIDATION
        case 7093:
        case 19577:
        case 24394:
        case 70495:
        //SPELL_HASH_WAR_STOMP
        case 45:
        case 11876:
        case 15593:
        case 16727:
        case 16740:
        case 19482:
        case 20549:
        case 24375:
        case 25188:
        case 27758:
        case 28125:
        case 28725:
        case 31408:
        case 31480:
        case 31755:
        case 33707:
        case 35238:
        case 36835:
        case 38682:
        case 38750:
        case 38911:
        case 39313:
        case 40936:
        case 41534:
        case 46026:
        case 56427:
        case 59705:
        case 60960:
        case 61065:
        //SPELL_HASH_POUNCE
        case 9005:
        case 9823:
        case 9827:
        case 27006:
        case 39449:
        case 43356:
        case 49803:
        case 54272:
        case 55077:
        case 61184:
        case 64399:
        //SPELL_HASH_HAMMER_OF_JUSTICE
        case 853:
        case 5588:
        case 5589:
        case 10308:
        case 13005:
        case 32416:
        case 37369:
        case 39077:
        case 41468:
        case 66007:
        case 66613:
        case 66863:
        case 66940:
        case 66941:
        {
            grp = DIMINISHING_GROUP_STUN;
            pve = true;
        } break;

        // SPELL_HASH_STARFIRE_STUN
        case 16922:
        // SPELL_HASH_STONECLAW_STUN
        case 39796:
        // SPELL_HASH_STUN              // Generic ones
        case 25:
        case 56:
        case 2880:
        case 9179:
        case 17308:
        case 20170:
        case 20310:
        case 23454:
        case 24647:
        case 27880:
        case 34510:
        case 35856:
        case 39568:
        case 46441:
        case 52093:
        case 52847:
        {
            grp = DIMINISHING_GROUP_STUN_PROC;
            pve = true;
        } break;

        //SPELL_HASH_ENTANGLING_ROOTS
        case 339:
        case 1062:
        case 5195:
        case 5196:
        case 9852:
        case 9853:
        case 11922:
        case 12747:
        case 19970:
        case 19971:
        case 19972:
        case 19973:
        case 19974:
        case 19975:
        case 20654:
        case 20699:
        case 21331:
        case 22127:
        case 22415:
        case 22800:
        case 24648:
        case 26071:
        case 26989:
        case 27010:
        case 28858:
        case 31287:
        case 32173:
        case 33844:
        case 37823:
        case 40363:
        case 53308:
        case 53313:
        case 57095:
        case 65857:
        case 66070:
        //SPELL_HASH_FROST_NOVA
        case 122:
        case 865:
        case 1194:
        case 1225:
        case 6131:
        case 6132:
        case 9915:
        case 10230:
        case 10231:
        case 11831:
        case 12674:
        case 12748:
        case 14907:
        case 15063:
        case 15531:
        case 15532:
        case 22645:
        case 27088:
        case 27387:
        case 29849:
        case 30094:
        case 31250:
        case 32192:
        case 32365:
        case 34326:
        case 36989:
        case 38033:
        case 39035:
        case 39063:
        case 42917:
        case 43426:
        case 44177:
        case 45905:
        case 46555:
        case 57629:
        case 57668:
        case 58458:
        case 59253:
        case 59995:
        case 61376:
        case 61462:
        case 62597:
        case 62605:
        case 63912:
        case 65792:
        case 68198:
        case 69060:
        case 69571:
        case 70209:
        case 71320:
        case 71929:
        {
            grp = DIMINISHING_GROUP_ROOT;
        } break;

        //SPELL_HASH_IMPROVED_WING_CLIP
        case 35963:
        case 47168:
        //SPELL_HASH_FROSTBITE
        case 11071:
        case 12494:
        case 12496:
        case 12497:
        case 57455:
        case 57456:
        case 61572:
        case 72004:
        case 72098:
        case 72120:
        case 72121:
        //SPELL_HASH_IMPROVED_HAMSTRING
        case 12289:
        case 12668:
        case 23694:
        case 23695:
        case 24428:
        //SPELL_HASH_ENTRAPMENT
        case 19184:
        case 19185:
        case 19387:
        case 19388:
        case 64803:
        case 64804:
        {
            grp = DIMINISHING_GROUP_ROOT_PROC;
        } break;

        //SPELL_HASH_HIBERNATE
        case 2637:
        case 18657:
        case 18658:
        {
            grp = DIMINISHING_GROUP_SLEEP;
        } break;

        //SPELL_HASH_SILENCE:
        case 6726:
        case 8988:
        case 12528:
        case 15487:
        case 18278:
        case 18327:
        case 22666:
        case 23207:
        case 26069:
        case 27559:
        case 29943:
        case 30225:
        case 37160:
        case 38491:
        case 38913:
        case 54093:
        case 56777:
        case 65542:
        //SPELL_HASH_GARROTE___SILENCE:
        case 1330:
        //SPELL_HASH_SILENCED___IMPROVED_COUNTERSPELL:
        case 18469:
        case 55021:
        //SPELL_HASH_SILENCED___IMPROVED_KICK:
        case 18425:
        //SPELL_HASH_SILENCED___GAG_ORDER:
        case 18498:
        case 74347:
        {
            grp = DIMINISHING_GROUP_SILENCE;
        } break;
    }

    uint32_t ret;
    if (pve)
        ret = grp | (1 << 16);
    else
        ret = grp;

    return ret;
}
