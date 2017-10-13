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

//\brief: This file includes all setted custom values and/or spell.dbc values (overwrite)
// Set the values you want based on spell Id (Do not set your values based on some text!)

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

            for (uint8 i = 0; i < MAX_SPELL_TOTEMS; ++i)
                spellInfo.setTotem(dbc_spell_entry->Totem[i], i);

            for (uint8 i = 0; i < MAX_SPELL_REAGENTS; ++i)
                spellInfo.setReagent(dbc_spell_entry->Reagent[i], i);

            for (uint8 i = 0; i < MAX_SPELL_REAGENTS; ++i)
                spellInfo.setReagentCount(dbc_spell_entry->ReagentCount[i], i);

            spellInfo.setEquippedItemClass(dbc_spell_entry->EquippedItemClass);
            spellInfo.setEquippedItemSubClass(dbc_spell_entry->EquippedItemSubClass);
            spellInfo.setRequiredItemFlags(dbc_spell_entry->RequiredItemFlags);

            for (int i = 0; i < MAX_SPELL_EFFECTS; ++i)
                spellInfo.setEffect(dbc_spell_entry->Effect[i], i);

            for (uint8 i = 0; i < MAX_SPELL_EFFECTS; ++i)
                spellInfo.setEffectDieSides(dbc_spell_entry->EffectDieSides[i], i);

            for (uint8 i = 0; i < MAX_SPELL_EFFECTS; ++i)
                spellInfo.setEffectRealPointsPerLevel(dbc_spell_entry->EffectRealPointsPerLevel[i], i);

            for (uint8 i = 0; i < MAX_SPELL_EFFECTS; ++i)
                spellInfo.setEffectBasePoints(dbc_spell_entry->EffectBasePoints[i], i);

            for (uint8 i = 0; i < MAX_SPELL_EFFECTS; ++i)
                spellInfo.setEffectMechanic(dbc_spell_entry->EffectMechanic[i], i);

            for (uint8 i = 0; i < MAX_SPELL_EFFECTS; ++i)
                spellInfo.setEffectImplicitTargetA(dbc_spell_entry->EffectImplicitTargetA[i], i);

            for (uint8 i = 0; i < MAX_SPELL_EFFECTS; ++i)
                spellInfo.setEffectImplicitTargetB(dbc_spell_entry->EffectImplicitTargetB[i], i);

            for (uint8 i = 0; i < MAX_SPELL_EFFECTS; ++i)
                spellInfo.setEffectRadiusIndex(dbc_spell_entry->EffectRadiusIndex[i], i);

            for (uint8 i = 0; i < MAX_SPELL_EFFECTS; ++i)
                spellInfo.setEffectApplyAuraName(dbc_spell_entry->EffectApplyAuraName[i], i);

            for (uint8 i = 0; i < MAX_SPELL_EFFECTS; ++i)
                spellInfo.setEffectAmplitude(dbc_spell_entry->EffectAmplitude[i], i);

            for (uint8 i = 0; i < MAX_SPELL_EFFECTS; ++i)
                spellInfo.setEffectMultipleValue(dbc_spell_entry->EffectMultipleValue[i], i);

            for (uint8 i = 0; i < MAX_SPELL_EFFECTS; ++i)
                spellInfo.setEffectChainTarget(dbc_spell_entry->EffectChainTarget[i], i);

            for (uint8 i = 0; i < MAX_SPELL_EFFECTS; ++i)
                spellInfo.setEffectItemType(dbc_spell_entry->EffectItemType[i], i);

            for (uint8 i = 0; i < MAX_SPELL_EFFECTS; ++i)
                spellInfo.setEffectMiscValue(dbc_spell_entry->EffectMiscValue[i], i);

            for (uint8 i = 0; i < MAX_SPELL_EFFECTS; ++i)
                spellInfo.setEffectMiscValueB(dbc_spell_entry->EffectMiscValueB[i], i);

            for (uint8 i = 0; i < MAX_SPELL_EFFECTS; ++i)
                spellInfo.setEffectTriggerSpell(dbc_spell_entry->EffectTriggerSpell[i], i);

            for (uint8 i = 0; i < MAX_SPELL_EFFECTS; ++i)
                spellInfo.setEffectPointsPerComboPoint(dbc_spell_entry->EffectPointsPerComboPoint[i], i);

#if VERSION_STRING > TBC
            for (uint8 i = 0; i < 3; ++i)
                for (uint8 j = 0; j < 3; ++j)
                    spellInfo.setEffectSpellClassMask(dbc_spell_entry->EffectSpellClassMask[i][j], i, j);
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
            for (uint8 i = 0; i < MAX_SPELL_EFFECTS; ++i)
                spellInfo.setSpellGroupType(dbc_spell_entry->SpellGroupType[i], i);
#endif

            spellInfo.setMaxTargets(dbc_spell_entry->MaxTargets);
            spellInfo.setSpell_Dmg_Type(dbc_spell_entry->Spell_Dmg_Type);
            spellInfo.setPreventionType(dbc_spell_entry->PreventionType);
            spellInfo.setStanceBarOrder(dbc_spell_entry->StanceBarOrder);

            for (uint8 i = 0; i < MAX_SPELL_EFFECTS; ++i)
                spellInfo.setDmg_multiplier(dbc_spell_entry->dmg_multiplier[i], i);

            spellInfo.setMinFactionID(dbc_spell_entry->MinFactionID);
            spellInfo.setMinReputation(dbc_spell_entry->MinReputation);
            spellInfo.setRequiredAuraVision(dbc_spell_entry->RequiredAuraVision);

            for (uint8 i = 0; i < MAX_SPELL_TOTEM_CATEGORIES; ++i)
                spellInfo.setTotemCategory(dbc_spell_entry->TotemCategory[i], i);

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

///Fix if it is a periodic trigger with amplitude = 0, to avoid division by zero
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
            if (spell_entry->getEffectApplyAuraName(y) == SPELL_AURA_SCHOOL_ABSORB && spell_entry->AuraFactoryFunc == NULL)
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
    if (spell_entry->getId() != 5225 && spell_entry->getId() != 19883)
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
            spell_entry->custom_always_apply = true;
        } break;
        default:
            break;
    }
}
