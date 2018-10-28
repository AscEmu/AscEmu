/*
Copyright (c) 2014-2018 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "StdAfx.h"
#include "SpellCustomizations.hpp"
#include "Server/MainServerDefines.h"
#include "Spell/SpellAuras.h"
#include "Singleton.h"
#include <unordered_map>
#include "Spell/Definitions/ProcFlags.h"
#include "Spell/Definitions/SpellDamageType.h"
#include "Spell/Definitions/SpellFamily.h"
#include "Spell/Definitions/SpellIsFlags.h"
#include "Spell/Definitions/DiminishingGroup.h"

initialiseSingleton(SpellCustomizations);

//\brief: This file includes all setted custom values and/or spell.dbc values (overwrite)
// Set the values you want based on spell Id (Do not set your values based on some text!)

SpellCustomizations::SpellCustomizations() {}
SpellCustomizations::~SpellCustomizations() {}

// Helper function for spell coefficient calculation
bool hasAdditionalEffects(SpellInfo const* sp)
{
    // TODO: is there more? -Appled
    if (sp->hasEffectApplyAuraName(SPELL_AURA_MOD_DECREASE_SPEED) ||
        sp->hasEffectApplyAuraName(SPELL_AURA_MOD_ROOT) ||
        sp->hasEffectApplyAuraName(SPELL_AURA_MOD_HIT_CHANCE) ||
        sp->hasEffectApplyAuraName(SPELL_AURA_MOD_HASTE))
        return true;
    return false;
}

// Helper function for spell coefficient calculation
bool isAoESpell(SpellInfo const* sp)
{
    for (uint8_t i = 0; i < MAX_SPELL_EFFECTS; ++i)
    {
        if (sp->getEffect(i) == 0)
            continue;
        // TODO: is there more? -Appled
        if (sp->getEffectImplicitTargetA(i) == EFF_TARGET_ALL_ENEMY_IN_AREA ||
            sp->getEffectImplicitTargetA(i) == EFF_TARGET_ALL_ENEMY_IN_AREA_INSTANT ||
            sp->getEffectImplicitTargetA(i) == EFF_TARGET_ALL_PARTY_AROUND_CASTER ||
            sp->getEffectImplicitTargetA(i) == EFF_TARGET_ALL_ENEMIES_AROUND_CASTER ||
            sp->getEffectImplicitTargetA(i) == EFF_TARGET_IN_FRONT_OF_CASTER ||
            sp->getEffectImplicitTargetA(i) == EFF_TARGET_ALL_ENEMY_IN_AREA_CHANNELED ||
            sp->getEffectImplicitTargetA(i) == EFF_TARGET_ALL_PARTY_IN_AREA_CHANNELED ||
            sp->getEffectImplicitTargetA(i) == EFF_TARGET_ALL_FRIENDLY_IN_AREA ||
            sp->getEffectImplicitTargetA(i) == EFF_TARGET_ALL_TARGETABLE_AROUND_LOCATION_IN_RADIUS_OVER_TIME ||
            sp->getEffectImplicitTargetA(i) == EFF_TARGET_ALL_PARTY ||
            sp->getEffectImplicitTargetA(i) == EFF_TARGET_ENEMIES_IN_AREA_CHANNELED_WITH_EXCEPTIONS ||
            sp->getEffectImplicitTargetA(i) == EFF_TARGET_SELECTED_ENEMY_DEADLY_POISON ||
            // Dunno if these are needed (custom parameters)
            sp->getEffectImplicitTargetA(i) == EFF_TARGET_CUSTOM_PARTY_INJURED_MULTI ||
            sp->getEffectImplicitTargetA(i) == EFF_TARGET_CONE_IN_FRONT)
            return true;

        if (sp->getEffectImplicitTargetB(i) == EFF_TARGET_ALL_ENEMY_IN_AREA ||
            sp->getEffectImplicitTargetB(i) == EFF_TARGET_ALL_ENEMY_IN_AREA_INSTANT ||
            sp->getEffectImplicitTargetB(i) == EFF_TARGET_ALL_PARTY_AROUND_CASTER ||
            sp->getEffectImplicitTargetB(i) == EFF_TARGET_ALL_ENEMIES_AROUND_CASTER ||
            sp->getEffectImplicitTargetB(i) == EFF_TARGET_IN_FRONT_OF_CASTER ||
            sp->getEffectImplicitTargetB(i) == EFF_TARGET_ALL_ENEMY_IN_AREA_CHANNELED ||
            sp->getEffectImplicitTargetB(i) == EFF_TARGET_ALL_PARTY_IN_AREA_CHANNELED ||
            sp->getEffectImplicitTargetB(i) == EFF_TARGET_ALL_FRIENDLY_IN_AREA ||
            sp->getEffectImplicitTargetB(i) == EFF_TARGET_ALL_TARGETABLE_AROUND_LOCATION_IN_RADIUS_OVER_TIME ||
            sp->getEffectImplicitTargetB(i) == EFF_TARGET_ALL_PARTY ||
            sp->getEffectImplicitTargetB(i) == EFF_TARGET_ENEMIES_IN_AREA_CHANNELED_WITH_EXCEPTIONS ||
            sp->getEffectImplicitTargetB(i) == EFF_TARGET_SELECTED_ENEMY_DEADLY_POISON ||
            // Dunno if these are needed (custom parameters)
            sp->getEffectImplicitTargetB(i) == EFF_TARGET_CUSTOM_PARTY_INJURED_MULTI ||
            sp->getEffectImplicitTargetB(i) == EFF_TARGET_CONE_IN_FRONT)
            return true;
    }
    return false;
}

// Helper function for spell coefficient calculation
bool hasChanneledEffect(SpellInfo const* sp)
{
    if (!sp->isChanneled())
        return false;

    if (sp->hasEffectApplyAuraName(SPELL_AURA_PERIODIC_TRIGGER_SPELL) ||
        sp->hasEffectApplyAuraName(SPELL_AURA_PERIODIC_TRIGGER_SPELL_WITH_VALUE) ||
        sp->hasEffectApplyAuraName(SPELL_AURA_PERIODIC_DAMAGE) ||
        sp->hasEffectApplyAuraName(SPELL_AURA_PERIODIC_HEAL) ||
        sp->hasEffectApplyAuraName(SPELL_AURA_PERIODIC_LEECH))
        return true;

    return false;
}

void SpellCustomizations::LoadSpellInfoData()
{
    for (auto i = 0; i < MAX_SPELL_ID; ++i)
    {
        auto dbc_spell_entry = sSpellStore.LookupEntry(i);
        if (dbc_spell_entry != nullptr)
        {
            uint32_t spell_id = dbc_spell_entry->Id;
            SpellInfo& spellInfo = _spellInfoContainerStore[spell_id];
            spellInfo.setId(spell_id);
            spellInfo.setAttributes(dbc_spell_entry->Attributes);
            spellInfo.setAttributesEx(dbc_spell_entry->AttributesEx);
            spellInfo.setAttributesExB(dbc_spell_entry->AttributesExB);
            spellInfo.setAttributesExC(dbc_spell_entry->AttributesExC);
            spellInfo.setAttributesExD(dbc_spell_entry->AttributesExD);
#if VERSION_STRING >= TBC
            spellInfo.setAttributesExE(dbc_spell_entry->AttributesExE);
            spellInfo.setAttributesExF(dbc_spell_entry->AttributesExF);
#endif
#if VERSION_STRING >= WotLK
            spellInfo.setAttributesExG(dbc_spell_entry->AttributesExG);
#endif
            spellInfo.setCastingTimeIndex(dbc_spell_entry->CastingTimeIndex);
            spellInfo.setDurationIndex(dbc_spell_entry->DurationIndex);
            spellInfo.setPowerType(dbc_spell_entry->powerType);
            spellInfo.setRangeIndex(dbc_spell_entry->rangeIndex);
            spellInfo.setSpeed(dbc_spell_entry->speed);
            spellInfo.setSpellVisual(dbc_spell_entry->SpellVisual);
            spellInfo.setSpellIconID(dbc_spell_entry->spellIconID);
            spellInfo.setActiveIconID(dbc_spell_entry->activeIconID);
            spellInfo.setSchool(dbc_spell_entry->School);
#if VERSION_STRING >= WotLK
            spellInfo.setRuneCostID(dbc_spell_entry->RuneCostID);
            spellInfo.setSpellDifficultyID(dbc_spell_entry->SpellDifficultyId);
#endif

#if VERSION_STRING < Cata
            spellInfo.setRequiredShapeShift(dbc_spell_entry->Shapeshifts);
            spellInfo.setShapeshiftExclude(dbc_spell_entry->ShapeshiftsExcluded);
            spellInfo.setTargets(dbc_spell_entry->Targets);
            spellInfo.setTargetCreatureType(dbc_spell_entry->TargetCreatureType);
            spellInfo.setRequiresSpellFocus(dbc_spell_entry->RequiresSpellFocus);
#if VERSION_STRING >= TBC
            spellInfo.setFacingCasterFlags(dbc_spell_entry->FacingCasterFlags);
#endif
            spellInfo.setCasterAuraState(dbc_spell_entry->CasterAuraState);
            spellInfo.setTargetAuraState(dbc_spell_entry->TargetAuraState);
#if VERSION_STRING >= TBC
            spellInfo.setCasterAuraStateNot(dbc_spell_entry->CasterAuraStateNot);
            spellInfo.setTargetAuraStateNot(dbc_spell_entry->TargetAuraStateNot);
#endif
#if VERSION_STRING == WotLK
            spellInfo.setCasterAuraSpell(dbc_spell_entry->casterAuraSpell);
            spellInfo.setTargetAuraSpell(dbc_spell_entry->targetAuraSpell);
            spellInfo.setCasterAuraSpellNot(dbc_spell_entry->casterAuraSpellNot);
            spellInfo.setTargetAuraSpellNot(dbc_spell_entry->targetAuraSpellNot);
#endif
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
            spellInfo.setManaCost(dbc_spell_entry->manaCost);
            spellInfo.setManaCostPerlevel(dbc_spell_entry->manaCostPerlevel);
            spellInfo.setManaPerSecond(dbc_spell_entry->manaPerSecond);
            spellInfo.setManaPerSecondPerLevel(dbc_spell_entry->manaPerSecondPerLevel);
            spellInfo.setMaxstack(dbc_spell_entry->MaxStackAmount);
            for (uint8_t j = 0; j < MAX_SPELL_TOTEMS; ++j)
                spellInfo.setTotem(dbc_spell_entry->Totem[j], j);
            for (uint8_t j = 0; j < MAX_SPELL_REAGENTS; ++j)
            {
                spellInfo.setReagent(dbc_spell_entry->Reagent[j], j);
                spellInfo.setReagentCount(dbc_spell_entry->ReagentCount[j], j);
            }
            spellInfo.setEquippedItemClass(dbc_spell_entry->EquippedItemClass);
            spellInfo.setEquippedItemSubClass(dbc_spell_entry->EquippedItemSubClass);
            spellInfo.setEquippedItemInventoryTypeMask(dbc_spell_entry->EquippedItemInventoryTypeMask);
            for (uint8_t j = 0; j < MAX_SPELL_EFFECTS; ++j)
            {
                spellInfo.setEffect(dbc_spell_entry->Effect[j], j);
                spellInfo.setEffectDieSides(dbc_spell_entry->EffectDieSides[j], j);
                spellInfo.setEffectRealPointsPerLevel(dbc_spell_entry->EffectRealPointsPerLevel[j], j);
                spellInfo.setEffectBasePoints(dbc_spell_entry->EffectBasePoints[j], j);
                spellInfo.setEffectMechanic(dbc_spell_entry->EffectMechanic[j], j);
                spellInfo.setEffectImplicitTargetA(dbc_spell_entry->EffectImplicitTargetA[j], j);
                spellInfo.setEffectImplicitTargetB(dbc_spell_entry->EffectImplicitTargetB[j], j);
                spellInfo.setEffectRadiusIndex(dbc_spell_entry->EffectRadiusIndex[j], j);
                spellInfo.setEffectApplyAuraName(dbc_spell_entry->EffectApplyAuraName[j], j);
                spellInfo.setEffectAmplitude(dbc_spell_entry->EffectAmplitude[j], j);
                spellInfo.setEffectMultipleValue(dbc_spell_entry->EffectMultipleValue[j], j);
                spellInfo.setEffectChainTarget(dbc_spell_entry->EffectChainTarget[j], j);
                spellInfo.setEffectItemType(dbc_spell_entry->EffectItemType[j], j);
                spellInfo.setEffectMiscValue(dbc_spell_entry->EffectMiscValue[j], j);
#if VERSION_STRING >= TBC
                spellInfo.setEffectMiscValueB(dbc_spell_entry->EffectMiscValueB[j], j);
#endif
                spellInfo.setEffectTriggerSpell(dbc_spell_entry->EffectTriggerSpell[j], j);
                spellInfo.setEffectPointsPerComboPoint(dbc_spell_entry->EffectPointsPerComboPoint[j], j);
#if VERSION_STRING == WotLK
                for (uint8_t x = 0; x < 3; ++x)
                    spellInfo.setEffectSpellClassMask(dbc_spell_entry->EffectSpellClassMask[j][x], j, x);
#endif
            }
            spellInfo.setSpellPriority(dbc_spell_entry->spellPriority);
            spellInfo.setName(dbc_spell_entry->Name[0]);
            spellInfo.setRank(dbc_spell_entry->Rank[0]);
            spellInfo.setManaCostPercentage(dbc_spell_entry->ManaCostPercentage);
            spellInfo.setStartRecoveryCategory(dbc_spell_entry->StartRecoveryCategory);
            spellInfo.setStartRecoveryTime(dbc_spell_entry->StartRecoveryTime);
            spellInfo.setMaxTargetLevel(dbc_spell_entry->MaxTargetLevel);
            spellInfo.setSpellFamilyName(dbc_spell_entry->SpellFamilyName);
#if VERSION_STRING != WotLK
            for (uint8_t j = 0; j < 2; ++j)
                spellInfo.setSpellFamilyFlags(dbc_spell_entry->SpellFamilyFlags[j], j);
#else
            for (uint8_t j = 0; j < MAX_SPELL_EFFECTS; ++j)
                spellInfo.setSpellFamilyFlags(dbc_spell_entry->SpellFamilyFlags[j], j);
#endif
            spellInfo.setMaxTargets(dbc_spell_entry->MaxTargets);
            spellInfo.setDmgClass(dbc_spell_entry->DmgClass);
            spellInfo.setPreventionType(dbc_spell_entry->PreventionType);
            for (uint8_t j = 0; j < MAX_SPELL_EFFECTS; ++j)
                spellInfo.setEffectDamageMultiplier(dbc_spell_entry->EffectDamageMultiplier[j], j);
#if VERSION_STRING >= TBC
            for (uint8_t j = 0; j < MAX_SPELL_TOTEM_CATEGORIES; ++j)
                spellInfo.setTotemCategory(dbc_spell_entry->TotemCategory[j], j);
            spellInfo.setRequiresAreaId(dbc_spell_entry->AreaGroupId);
#endif
#if VERSION_STRING == WotLK
            for (uint8_t j = 0; j < MAX_SPELL_EFFECTS; ++j)
                spellInfo.setEffectBonusMultiplier(dbc_spell_entry->EffectBonusMultiplier[j], j);
#endif
        }
    }
#else
            spellInfo.setAttributesExH(dbc_spell_entry->AttributesExH);
            spellInfo.setAttributesExI(dbc_spell_entry->AttributesExI);
            spellInfo.setAttributesExJ(dbc_spell_entry->AttributesExJ);

            spellInfo.setName(dbc_spell_entry->Name);
            spellInfo.setRank(dbc_spell_entry->Rank);

            // Initialize DBC links
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

            // Data from SpellAuraOptions.dbc
            if (dbc_spell_entry->SpellAuraOptionsId && dbc_spell_entry->GetSpellAuraOptions() != nullptr)
            {
                spellInfo.setMaxstack(dbc_spell_entry->GetSpellAuraOptions()->MaxStackAmount);
                spellInfo.setProcChance(dbc_spell_entry->GetSpellAuraOptions()->procChance);
                spellInfo.setProcCharges(dbc_spell_entry->GetSpellAuraOptions()->procCharges);
                spellInfo.setProcFlags(dbc_spell_entry->GetSpellAuraOptions()->procFlags);
            }

            // Data from SpellAuraRestrictions.dbc
            if (dbc_spell_entry->SpellAuraRestrictionsId && dbc_spell_entry->GetSpellAuraRestrictions() != nullptr)
            {
                spellInfo.setCasterAuraState(dbc_spell_entry->GetSpellAuraRestrictions()->CasterAuraState);
                spellInfo.setTargetAuraState(dbc_spell_entry->GetSpellAuraRestrictions()->TargetAuraState);
                spellInfo.setCasterAuraStateNot(dbc_spell_entry->GetSpellAuraRestrictions()->CasterAuraStateNot);
                spellInfo.setTargetAuraStateNot(dbc_spell_entry->GetSpellAuraRestrictions()->TargetAuraStateNot);
                spellInfo.setCasterAuraSpell(dbc_spell_entry->GetSpellAuraRestrictions()->casterAuraSpell);
                spellInfo.setTargetAuraSpell(dbc_spell_entry->GetSpellAuraRestrictions()->targetAuraSpell);
                spellInfo.setCasterAuraSpellNot(dbc_spell_entry->GetSpellAuraRestrictions()->CasterAuraSpellNot);
                spellInfo.setTargetAuraSpellNot(dbc_spell_entry->GetSpellAuraRestrictions()->TargetAuraSpellNot);
            }

            // Data from SpellCastingRequirements.dbc
            if (dbc_spell_entry->SpellCastingRequirementsId && dbc_spell_entry->GetSpellCastingRequirements() != nullptr)
            {
                spellInfo.setFacingCasterFlags(dbc_spell_entry->GetSpellCastingRequirements()->FacingCasterFlags);
                spellInfo.setRequiresAreaId(dbc_spell_entry->GetSpellCastingRequirements()->AreaGroupId);
                spellInfo.setRequiresSpellFocus(dbc_spell_entry->GetSpellCastingRequirements()->RequiresSpellFocus);
            }

            // Data from SpellCategories.dbc
            if (dbc_spell_entry->SpellCategoriesId && dbc_spell_entry->GetSpellCategories() != nullptr)
            {
                spellInfo.setCategory(dbc_spell_entry->GetSpellCategories()->Category);
                spellInfo.setDispelType(dbc_spell_entry->GetSpellCategories()->DispelType);
                spellInfo.setDmgClass(dbc_spell_entry->GetSpellCategories()->DmgClass);
                spellInfo.setMechanicsType(dbc_spell_entry->GetSpellCategories()->MechanicsType);
                spellInfo.setPreventionType(dbc_spell_entry->GetSpellCategories()->PreventionType);
                spellInfo.setStartRecoveryCategory(dbc_spell_entry->GetSpellCategories()->StartRecoveryCategory);
            }

            // Data from SpellClassOptions.dbc
            if (dbc_spell_entry->SpellClassOptionsId && dbc_spell_entry->GetSpellClassOptions() != nullptr)
            {
                spellInfo.setSpellFamilyName(dbc_spell_entry->GetSpellClassOptions()->SpellFamilyName);
                for (uint8_t j = 0; j < MAX_SPELL_EFFECTS; ++j)
                    spellInfo.setSpellFamilyFlags(dbc_spell_entry->GetSpellClassOptions()->SpellFamilyFlags[j], j);
            }

            // Data from SpellCooldowns.dbc
            if (dbc_spell_entry->SpellCooldownsId && dbc_spell_entry->GetSpellCooldowns() != nullptr)
            {
                spellInfo.setCategoryRecoveryTime(dbc_spell_entry->GetSpellCooldowns()->CategoryRecoveryTime);
                spellInfo.setRecoveryTime(dbc_spell_entry->GetSpellCooldowns()->RecoveryTime);
                spellInfo.setStartRecoveryTime(dbc_spell_entry->GetSpellCooldowns()->StartRecoveryTime);
            }

            // Data from SpellEquippedItems.dbc
            if (dbc_spell_entry->SpellEquippedItemsId && dbc_spell_entry->GetSpellEquippedItems() != nullptr)
            {
                spellInfo.setEquippedItemClass(dbc_spell_entry->GetSpellEquippedItems()->EquippedItemClass);
                spellInfo.setEquippedItemInventoryTypeMask(dbc_spell_entry->GetSpellEquippedItems()->EquippedItemInventoryTypeMask);
                spellInfo.setEquippedItemSubClass(dbc_spell_entry->GetSpellEquippedItems()->EquippedItemSubClassMask);
            }

            // Data from SpellInterrupts.dbc
            if (dbc_spell_entry->SpellInterruptsId && dbc_spell_entry->GetSpellInterrupts() != nullptr)
            {
                spellInfo.setAuraInterruptFlags(dbc_spell_entry->GetSpellInterrupts()->AuraInterruptFlags);
                spellInfo.setChannelInterruptFlags(dbc_spell_entry->GetSpellInterrupts()->ChannelInterruptFlags);
                spellInfo.setInterruptFlags(dbc_spell_entry->GetSpellInterrupts()->InterruptFlags);
            }

            // Data from SpellLevels.dbc
            if (dbc_spell_entry->SpellLevelsId && dbc_spell_entry->GetSpellLevels() != nullptr)
            {
                spellInfo.setBaseLevel(dbc_spell_entry->GetSpellLevels()->baseLevel);
                spellInfo.setMaxLevel(dbc_spell_entry->GetSpellLevels()->maxLevel);
                spellInfo.setSpellLevel(dbc_spell_entry->GetSpellLevels()->spellLevel);
            }

            // Data from SpellPower.dbc
            if (dbc_spell_entry->SpellPowerId && dbc_spell_entry->GetSpellPower() != nullptr)
            {
                spellInfo.setManaCost(dbc_spell_entry->GetSpellPower()->manaCost);
                spellInfo.setManaCostPerlevel(dbc_spell_entry->GetSpellPower()->manaCostPerlevel);
                spellInfo.setManaCostPercentage(dbc_spell_entry->GetSpellPower()->ManaCostPercentage);
                spellInfo.setManaPerSecond(dbc_spell_entry->GetSpellPower()->manaPerSecond);
                spellInfo.setManaPerSecondPerLevel(dbc_spell_entry->GetSpellPower()->manaPerSecondPerLevel);
            }

            // Data from SpellReagents.dbc
            if (dbc_spell_entry->SpellReagentsId && dbc_spell_entry->GetSpellReagents() != nullptr)
            {
                for (uint8_t j = 0; j < MAX_SPELL_REAGENTS; ++j)
                {
                    spellInfo.setReagent(dbc_spell_entry->GetSpellReagents()->Reagent[j], j);
                    spellInfo.setReagentCount(dbc_spell_entry->GetSpellReagents()->ReagentCount[j], j);
                }
            }

            // Data from SpellShapeshift.dbc
            if (dbc_spell_entry->SpellShapeshiftId && dbc_spell_entry->GetSpellShapeshift() != nullptr)
            {
                spellInfo.setRequiredShapeShift(dbc_spell_entry->GetSpellShapeshift()->Shapeshifts);
                spellInfo.setShapeshiftExclude(dbc_spell_entry->GetSpellShapeshift()->ShapeshiftsExcluded);
            }

            // Data from SpellTargetRestrictions.dbc
            if (dbc_spell_entry->SpellTargetRestrictionsId && dbc_spell_entry->GetSpellTargetRestrictions() != nullptr)
            {
                spellInfo.setMaxTargets(dbc_spell_entry->GetSpellTargetRestrictions()->MaxAffectedTargets);
                spellInfo.setMaxTargetLevel(dbc_spell_entry->GetSpellTargetRestrictions()->MaxTargetLevel);
                spellInfo.setTargetCreatureType(dbc_spell_entry->GetSpellTargetRestrictions()->TargetCreatureType);
                spellInfo.setTargets(dbc_spell_entry->GetSpellTargetRestrictions()->Targets);
            }

            // Data from SpellTotems.dbc
            if (dbc_spell_entry->SpellTotemsId && dbc_spell_entry->GetSpellTotems() != nullptr)
            {
                for (uint8_t j = 0; j < MAX_SPELL_TOTEMS; ++j)
                {
                    spellInfo.setTotemCategory(dbc_spell_entry->GetSpellTotems()->TotemCategory[j], j);
                    spellInfo.setTotem(dbc_spell_entry->GetSpellTotems()->Totem[j], j);
                }
            }

            // Data from SpellEffect.dbc
            for (uint8_t j = 0; j < MAX_SPELL_EFFECTS; ++j)
            {
                const auto spell_effect_entry = GetSpellEffectEntry(spell_id, j);
                if (spell_effect_entry != nullptr)
                {
                    spellInfo.setEffect(spell_effect_entry->Effect, j);
                    spellInfo.setEffectMultipleValue(spell_effect_entry->EffectMultipleValue, j);
                    spellInfo.setEffectApplyAuraName(spell_effect_entry->EffectApplyAuraName, j);
                    spellInfo.setEffectAmplitude(spell_effect_entry->EffectAmplitude, j);
                    spellInfo.setEffectBasePoints(spell_effect_entry->EffectBasePoints, j);
                    spellInfo.setEffectBonusMultiplier(spell_effect_entry->EffectBonusMultiplier, j);
                    spellInfo.setEffectDamageMultiplier(spell_effect_entry->EffectDamageMultiplier, j);
                    spellInfo.setEffectChainTarget(spell_effect_entry->EffectChainTarget, j);
                    spellInfo.setEffectDieSides(spell_effect_entry->EffectDieSides, j);
                    spellInfo.setEffectItemType(spell_effect_entry->EffectItemType, j);
                    spellInfo.setEffectMechanic(spell_effect_entry->EffectMechanic, j);
                    spellInfo.setEffectMiscValue(spell_effect_entry->EffectMiscValue, j);
                    spellInfo.setEffectMiscValueB(spell_effect_entry->EffectMiscValueB, j);
                    spellInfo.setEffectPointsPerComboPoint(spell_effect_entry->EffectPointsPerComboPoint, j);
                    spellInfo.setEffectRadiusIndex(spell_effect_entry->EffectRadiusIndex, j);
                    spellInfo.setEffectRadiusMaxIndex(spell_effect_entry->EffectRadiusMaxIndex, j);
                    spellInfo.setEffectRealPointsPerLevel(spell_effect_entry->EffectRealPointsPerLevel, j);
                    for (uint8_t x = 0; x < 3; ++x)
                        spellInfo.setEffectSpellClassMask(spell_effect_entry->EffectSpellClassMask[x], j, x);
                    spellInfo.setEffectTriggerSpell(spell_effect_entry->EffectTriggerSpell, j);
                    spellInfo.setEffectImplicitTargetA(spell_effect_entry->EffectImplicitTargetA, j);
                    spellInfo.setEffectImplicitTargetB(spell_effect_entry->EffectImplicitTargetB, j);
                    spellInfo.setEffectSpellId(spell_effect_entry->EffectSpellId, j);
                    spellInfo.setEffectIndex(spell_effect_entry->EffectIndex, j);
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

    LoadSpellCustomOverride();

    loadSpellCoefficientOverride();

    for (auto it = sSpellCustomizations.GetSpellInfoStore()->begin(); it != sSpellCustomizations.GetSpellInfoStore()->end(); ++it)
    {
        auto spellentry = GetSpellInfo(it->first);
        if (spellentry != nullptr)
        {
            // Calculate spell coefficient
            setSpellCoefficient(spellentry);

            //Set spell overwrites (effect based)
            SetEffectAmplitude(spellentry);
            SetAuraFactoryFunc(spellentry);

            // Set custom values (effect based)
            SetMeleeSpellBool(spellentry);
            SetRangedSpellBool(spellentry);

            // Set custom values (spell based)
            SetMissingCIsFlags(spellentry);
            SetOnShapeshiftChange(spellentry);
        }
    }
}

void SpellCustomizations::LoadSpellCustomOverride()
{
    uint32 override_count = 0;
    //                                                        0       1           2                          3                              4                    5
    if (QueryResult* result = WorldDatabase.Query("SELECT spell_id, `rank`, assign_on_target_flag, assign_from_caster_on_self_flag, assign_self_cast_only, assign_c_is_flag, "
    //                                                   6            7            8               9             10              11             12           13
                                                  " coef_flags, coef_Dspell, coef_Otspell, proc_on_namehash, proc_flags, proc_target_selfs, proc_chance, proc_charges, "
    //                                                   14                     15                           16                          17         
                                                  "proc_interval, proc_effect_trigger_spell_0, proc_effect_trigger_spell_1, proc_effect_trigger_spell_2 FROM spell_custom_override"))
    {
        do
        {
            Field* fields = result->Fetch();

            SpellInfo* spell_entry = GetSpellInfo(fields[0].GetUInt32());
            if (spell_entry != nullptr)
            {
                if (fields[1].isSet())
                    spell_entry->custom_RankNumber = fields[1].GetUInt32();

                if (fields[2].isSet())
                    spell_entry->custom_BGR_one_buff_on_target = fields[2].GetUInt32();

                //fields[3] not used todo remove this

                if (fields[4].isSet())
                    spell_entry->custom_self_cast_only = fields[4].GetBool();

                if (fields[5].isSet())
                    spell_entry->custom_c_is_flags = fields[5].GetUInt32();

                // fields[6] not used, todo remove this

                // fields[7] not used, todo remove this

                // fields[8] not used, todo remove this

                //proc

                //fields[9] proc_on_namehash not used todo remove this!

                //proc_flags
                if (fields[10].isSet())
                    spell_entry->setProcFlags(fields[10].GetUInt32());
                //proc_target_selfs
                if (fields[11].isSet() && fields[11].GetBool())
                    spell_entry->addProcFlags(PROC_TARGET_SELF);
                //proc_chance
                if (fields[12].isSet())
                    spell_entry->setProcChance(fields[12].GetUInt32());
                //proc_charges
                if (fields[13].isSet())
                    spell_entry->setProcCharges(fields[13].GetUInt32());
                //proc_interval
                if (fields[14].isSet())
                    spell_entry->custom_proc_interval = fields[14].GetUInt32();

                //proc_effect_trigger_spell_0
                if (fields[15].isSet())
                {
                    spell_entry->setEffectTriggerSpell(fields[15].GetUInt32(), 0);
                    if (spell_entry->getEffectTriggerSpell(0) > 0)
                        spell_entry->setEffectApplyAuraName(SPELL_AURA_PROC_TRIGGER_SPELL, 0);
                }
                //proc_effect_trigger_spell_1
                if (fields[16].isSet())
                {
                    spell_entry->setEffectTriggerSpell(fields[16].GetUInt32(), 1);
                    if (spell_entry->getEffectTriggerSpell(1) > 0)
                        spell_entry->setEffectApplyAuraName(SPELL_AURA_PROC_TRIGGER_SPELL, 1);
                }
                //proc_effect_trigger_spell_2
                if (fields[17].isSet())
                {
                    spell_entry->setEffectTriggerSpell(fields[17].GetUInt32(), 2);
                    if (spell_entry->getEffectTriggerSpell(2) > 0)
                        spell_entry->setEffectApplyAuraName(SPELL_AURA_PROC_TRIGGER_SPELL, 2);
                }

                ++override_count;
            }

        } while (result->NextRow());
        delete result;
    }

    if (override_count > 0)
        LOG_DETAIL("Loaded %u override values from spell_custom_override table", override_count);
    else
        LogDebugFlag(LF_DB_TABLES, "SpellCustomizations::LoadSpellRanks : Your spell_custom_override table is empty!");
}

void SpellCustomizations::loadSpellCoefficientOverride()
{
    auto overridden_coeffs = 0;

    //                                                      0           1                     2
    if (const auto result = WorldDatabase.Query("SELECT spell_id, direct_coefficient, overtime_coefficient "
                                          "FROM spell_coefficient_override WHERE min_build <= %u AND max_build >= %u", VERSION_STRING, VERSION_STRING))
    {
        do
        {
            const auto fields = result->Fetch();
            auto spellInfo = GetSpellInfo(fields[0].GetUInt32());
            if (spellInfo == nullptr)
            {
                LOG_ERROR("Invalid spell entry (%u) in table `spell_coefficient_override`, skipped", fields[0].GetUInt32());
                continue;
            }

            const auto direct_override = fields[1].GetFloat();
            const auto overtime_override = fields[2].GetFloat();
            // Coeff can be overridden to 0 when it won't receive any bonus from spell power (default value is -1)
            if (direct_override >= 0)
                spellInfo->spell_coeff_direct_override = direct_override;
            if (overtime_override >= 0)
                spellInfo->spell_coeff_overtime_override = overtime_override;
            ++overridden_coeffs;

        } while (result->NextRow());
        delete result;
    }

    if (overridden_coeffs > 0)
        LOG_DETAIL("Loaded %u coefficient overrides from spell_coefficient_override table", overridden_coeffs);
    else
        LogDebugFlag(LF_DB_TABLES, "SpellCustomizations::loadSpellCoefficientOverride : Your spell_coefficient_override table is empty!");
}

void SpellCustomizations::setSpellCoefficient(SpellInfo *sp)
{
    // note:
    // dot and hot parts are stored as per tick, like in wotlk spell.dbc
    // channeled spell coeffs are also stored in direct member, as per missile/tick

    // override table notes:
    // overriding a spell which trigger another spell (like channeled spell) won't alter coeff of the triggered spell
    // dots, hots and channeled spells will have coeffs for full duration in table, following code divides it per tick

    const auto baseDuration = float(GetDuration(sSpellDurationStore.LookupEntry(sp->getDurationIndex())));
    const auto isOverTimeSpell = sp->hasEffectApplyAuraName(SPELL_AURA_PERIODIC_DAMAGE) || sp->hasEffectApplyAuraName(SPELL_AURA_PERIODIC_HEAL);

    // Get coefficient override from database if any is set
    if (sp->spell_coeff_direct_override != -1)
    {
        sp->spell_coeff_direct = sp->spell_coeff_direct_override;

        // Store coeff value as per missile for channeled spells
        if (sp->isChanneled())
        {
            for (uint8_t i = 0; i < MAX_SPELL_EFFECTS; ++i)
            {
                if (sp->getEffectApplyAuraName(i) == SPELL_AURA_PERIODIC_TRIGGER_SPELL ||
                    sp->getEffectApplyAuraName(i) == SPELL_AURA_PERIODIC_TRIGGER_SPELL_WITH_VALUE ||
                    sp->getEffectApplyAuraName(i) == SPELL_AURA_PERIODIC_DAMAGE ||
                    sp->getEffectApplyAuraName(i) == SPELL_AURA_PERIODIC_HEAL ||
                    sp->getEffectApplyAuraName(i) == SPELL_AURA_PERIODIC_LEECH)
                {
                    sp->spell_coeff_direct /= baseDuration / sp->getEffectAmplitude(i);
                    break;
                }
            }
        }
    }
    if (sp->spell_coeff_overtime_override != -1)
    {
        sp->spell_coeff_overtime = sp->spell_coeff_overtime_override;

        // Store coeff value as per tick for over-time spells
        if (isOverTimeSpell)
        {
            for (uint8_t i = 0; i < MAX_SPELL_EFFECTS; ++i)
            {
                if (sp->getEffectApplyAuraName(i) == SPELL_AURA_PERIODIC_DAMAGE ||
                    sp->getEffectApplyAuraName(i) == SPELL_AURA_PERIODIC_HEAL ||
                    sp->getEffectApplyAuraName(i) == SPELL_AURA_PERIODIC_LEECH)
                {
                    sp->spell_coeff_overtime /= baseDuration / sp->getEffectAmplitude(i);
                    break;
                }
            }
        }
    }

    // If coefficients are already set, do not alter them
    // this can happen if this spell is triggered by another spell or coefficients are overriden in database
    if (sp->spell_coeff_direct != -1 || sp->spell_coeff_overtime != -1)
        return;

    // Skip non-magic spells
    if (sp->getDmgClass() != SPELL_DMG_TYPE_MAGIC)
        return;

    // Skip calculation if spell family name isn't any of the player classes
    // Note: some player spells may have generic spell family name but these spells can be added into the SQL table
    if (sp->getSpellFamilyName() != SPELLFAMILY_MAGE &&
        sp->getSpellFamilyName() != SPELLFAMILY_WARLOCK &&
        sp->getSpellFamilyName() != SPELLFAMILY_PRIEST &&
        sp->getSpellFamilyName() != SPELLFAMILY_DRUID &&
        sp->getSpellFamilyName() != SPELLFAMILY_PALADIN &&
        sp->getSpellFamilyName() != SPELLFAMILY_SHAMAN)
        return;

#if VERSION_STRING >= WotLK
    // Load spell coefficient values from DBC
    for (uint8_t i = 0; i < MAX_SPELL_EFFECTS; ++i)
    {
        // If effect triggers another spell, ignore the coefficient value
        // The spell to be triggered has the coefficient already and later it will be handled in direct spell check
        if (sp->getEffect(i) == SPELL_EFFECT_TRIGGER_SPELL ||
            sp->getEffect(i) == SPELL_EFFECT_TRIGGER_SPELL_WITH_VALUE ||
            sp->getEffectApplyAuraName(i) == SPELL_AURA_PERIODIC_TRIGGER_SPELL ||
            sp->getEffectApplyAuraName(i) == SPELL_AURA_PERIODIC_TRIGGER_SPELL_WITH_VALUE)
            continue;

        if (sp->getEffectBonusMultiplier(i) > 0)
        {
            const auto coefficientValue = sp->getEffectBonusMultiplier(i);
            // For channeled and over-time spells, the coefficient is already stored as per tick in DBC
            if (sp->isChanneled() &&
               (sp->getEffectApplyAuraName(i) == SPELL_AURA_PERIODIC_TRIGGER_SPELL ||
                sp->getEffectApplyAuraName(i) == SPELL_AURA_PERIODIC_TRIGGER_SPELL_WITH_VALUE ||
                sp->getEffectApplyAuraName(i) == SPELL_AURA_PERIODIC_DAMAGE ||
                sp->getEffectApplyAuraName(i) == SPELL_AURA_PERIODIC_HEAL ||
                sp->getEffectApplyAuraName(i) == SPELL_AURA_PERIODIC_LEECH))
                sp->spell_coeff_direct = coefficientValue;

            else if (sp->isDamagingEffect(i) || sp->isHealingEffect(i) || sp->getEffect(i) == SPELL_EFFECT_HEALTH_LEECH)
                sp->spell_coeff_direct = coefficientValue;

            else if (sp->getEffectApplyAuraName(i) == SPELL_AURA_PERIODIC_DAMAGE ||
                      sp->getEffectApplyAuraName(i) == SPELL_AURA_PERIODIC_HEAL ||
                      sp->getEffectApplyAuraName(i) == SPELL_AURA_PERIODIC_LEECH)
                sp->spell_coeff_overtime = coefficientValue;
        }
    }
#else
    // Calculate base spell coefficient
    auto spellCastTime = float(GetCastTime(sSpellCastTimesStore.LookupEntry(sp->getCastingTimeIndex())));
    if (spellCastTime < 1500)
        spellCastTime = 1500;
#if VERSION_STRING == Classic
    // Classic has 100% spell coefficient cap
    else if (spellCastTime > 3500)
        spellCastTime = 3500;
#else
    else if (spellCastTime > 7000)
        spellCastTime = 7000;
#endif

    // Leech spells (spells that deal damage and heal)
    const auto hasLeechEffect = sp->hasEffect(SPELL_EFFECT_HEALTH_LEECH);
    const auto hasLeechAura = sp->hasEffectApplyAuraName(SPELL_AURA_PERIODIC_LEECH);
    const auto isLeechSpell = hasLeechEffect || hasLeechAura;

    // Channeled spells
    if (hasChanneledEffect(sp))
    {
        auto spellDuration = baseDuration;
#if VERSION_STRING == Classic
        // Classic has 100% spell coefficient cap
        if (spellDuration > 3500)
            spellDuration = 3500;
#endif
        sp->spell_coeff_direct = spellDuration / 3500;

        // Spells with additional effects receive 5% penalty
#if VERSION_STRING == Classic
        if (hasAdditionalEffects(sp))
            sp->spell_coeff_direct *= 0.95f;

        // In Classic all Area of Effect spells receive 33% of the coefficient
        if (isAoESpell(sp))
            sp->spell_coeff_direct /= 3;
#else
        if (hasAdditionalEffects(sp))
        {
            sp->spell_coeff_direct *= 0.95f;
            // In TBC, Area of Effect spells with additional effects receive 33% of the coefficient
            if (isAoESpell(sp))
                sp->spell_coeff_direct /= 3;
        }
        else
        {
            // and normal Area of Effect spells receive 50% of the coefficient
            if (isAoESpell(sp))
                sp->spell_coeff_direct /= 2;
        }
#endif

        // Leech spells receive 50% of the coefficient
        if (isLeechSpell)
            sp->spell_coeff_direct /= 2;

        // Spells below level 20 receive a significant penalty
        if (sp->getBaseLevel() < 20)
        {
            const auto penalty = 1.0f - ((20.0f - float(sp->getBaseLevel())) * 0.0375f);
            sp->spell_coeff_direct *= penalty;
        }

        // Store coeff value as per missile / tick
        for (uint8_t i = 0; i < MAX_SPELL_EFFECTS; ++i)
        {
            if (sp->getEffectApplyAuraName(i) == SPELL_AURA_PERIODIC_TRIGGER_SPELL ||
                sp->getEffectApplyAuraName(i) == SPELL_AURA_PERIODIC_TRIGGER_SPELL_WITH_VALUE ||
                sp->getEffectApplyAuraName(i) == SPELL_AURA_PERIODIC_DAMAGE ||
                sp->getEffectApplyAuraName(i) == SPELL_AURA_PERIODIC_HEAL ||
                sp->getEffectApplyAuraName(i) == SPELL_AURA_PERIODIC_LEECH)
            {
                sp->spell_coeff_direct /= baseDuration / sp->getEffectAmplitude(i);
                // For channeled spells which trigger another spell on each "missile", set triggered spell's coeff to match master spell's coeff
                if (sp->getEffectApplyAuraName(i) == SPELL_AURA_PERIODIC_TRIGGER_SPELL ||
                    sp->getEffectApplyAuraName(i) == SPELL_AURA_PERIODIC_TRIGGER_SPELL_WITH_VALUE)
                {
                    auto triggeredSpell = sSpellCustomizations.GetSpellInfo(sp->getEffectTriggerSpell(i));
                    // But do not alter its coefficient if it's already overridden in database
                    if (triggeredSpell != nullptr && triggeredSpell->spell_coeff_direct_override == -1)
                        triggeredSpell->spell_coeff_direct = sp->spell_coeff_direct;
                }
                break;
            }
        }
    }

    // Hybrid spells (spells with direct and over-time damage or direct and over-time healing effect)
    else if ((sp->hasDamagingEffect() && sp->hasEffectApplyAuraName(SPELL_AURA_PERIODIC_DAMAGE)) ||
             (sp->hasHealingEffect() && sp->hasEffectApplyAuraName(SPELL_AURA_PERIODIC_HEAL)))
    {
        auto spellDuration = baseDuration;
#if VERSION_STRING == Classic
        // Classic has 100% spell coefficient cap
        if (spellDuration > 15000)
            spellDuration = 15000;
#endif
        const auto castTime = spellCastTime / 3500;
        spellDuration /= 15000;

        sp->spell_coeff_direct = (castTime * castTime) / (spellDuration + castTime);
        sp->spell_coeff_overtime = (spellDuration * spellDuration) / (castTime + spellDuration);

        // Spells with additional effects receive 5% penalty
#if VERSION_STRING == Classic
        if (hasAdditionalEffects(sp))
        {
            sp->spell_coeff_direct *= 0.95f;
            sp->spell_coeff_overtime *= 0.95f;
        }

        // In Classic all Area of Effect spells receive 33% of the coefficient
        if (isAoESpell(sp))
        {
            sp->spell_coeff_direct /= 3;
            sp->spell_coeff_overtime /= 3;
        }
#else
        if (hasAdditionalEffects(sp))
        {
            sp->spell_coeff_direct *= 0.95f;
            sp->spell_coeff_overtime *= 0.95f;
            // In TBC, Area of Effect spells with additional effects receive 33% of the coefficient
            if (isAoESpell(sp))
            {
                sp->spell_coeff_direct /= 3;
                sp->spell_coeff_overtime /= 3;
            }
        }
        else
        {
            // and normal Area of Effect spells receive 50% of the coefficient
            if (isAoESpell(sp))
            {
                sp->spell_coeff_direct /= 2;
                sp->spell_coeff_overtime /= 2;
            }
        }
#endif

        // Leech spells receive 50% of the coefficient
        if (isLeechSpell)
        {
            sp->spell_coeff_direct /= 2;
            sp->spell_coeff_overtime /= 2;
        }

        // Spells below level 20 receive a significant penalty
        if (sp->getBaseLevel() < 20)
        {
            const auto penalty = 1.0f - ((20.0f - float(sp->getBaseLevel())) * 0.0375f);
            sp->spell_coeff_direct *= penalty;
            sp->spell_coeff_overtime *= penalty;
        }

        // Store over-time coeff value as per tick
        for (uint8_t i = 0; i < MAX_SPELL_EFFECTS; ++i)
        {
            if (sp->getEffectApplyAuraName(i) == SPELL_AURA_PERIODIC_DAMAGE ||
                sp->getEffectApplyAuraName(i) == SPELL_AURA_PERIODIC_HEAL ||
                sp->getEffectApplyAuraName(i) == SPELL_AURA_PERIODIC_LEECH)
            {
                /*
                    If a spell has less than five ticks, each tick receives one-fifth of the bonus per tick.
                    If an over time spell has 5 or more ticks, the spell receives full benefit, divided equally between the number of ticks.
                */
                const auto ticks = baseDuration / sp->getEffectAmplitude(i);
                if (ticks < 5)
                    sp->spell_coeff_overtime /= 5;
                else
                    sp->spell_coeff_overtime /= ticks;
                break;
            }
        }
    }

    // Direct damage and healing spells
    else if (!isOverTimeSpell && ((sp->hasDamagingEffect() && !sp->hasEffect(SPELL_EFFECT_POWER_BURN)) || (sp->hasHealingEffect() && !sp->hasEffect(SPELL_EFFECT_HEAL_MAX_HEALTH)) || hasLeechEffect))
    {
        sp->spell_coeff_direct = spellCastTime / 3500;

        // Spells with additional effects receive 5% penalty
#if VERSION_STRING == Classic
        if (hasAdditionalEffects(sp))
            sp->spell_coeff_direct *= 0.95f;

        // In Classic all Area of Effect spells receive 33% of the coefficient
        if (isAoESpell(sp))
            sp->spell_coeff_direct /= 3;
#else
        if (hasAdditionalEffects(sp))
        {
            sp->spell_coeff_direct *= 0.95f;
            // In TBC, Area of Effect spells with additional effects receive 33% of the coefficient
            if (isAoESpell(sp))
                sp->spell_coeff_direct /= 3;
        }
        else
        {
            // and normal Area of Effect spells receive 50% of the coefficient
            if (isAoESpell(sp))
                sp->spell_coeff_direct /= 2;
        }
#endif

        // Leech spells receive 50% of the coefficient
        if (isLeechSpell)
            sp->spell_coeff_direct /= 2;

        // Spells below level 20 receive a significant penalty
        if (sp->getBaseLevel() < 20)
        {
            const auto penalty = 1.0f - ((20.0f - float(sp->getBaseLevel())) * 0.0375f);
            sp->spell_coeff_direct *= penalty;
        }
    }

    // Damage and healing over-time spells
    else if ((isOverTimeSpell || hasLeechAura) && !sp->hasDamagingEffect() && !sp->hasHealingEffect() && !hasChanneledEffect(sp))
    {
        auto spellDuration = baseDuration;
#if VERSION_STRING == Classic
        // Classic has 100% spell coefficient cap
        if (spellDuration > 15000)
            spellDuration = 15000;
#endif
        sp->spell_coeff_overtime = spellDuration / 15000;

        // Spells with additional effects receive 5% penalty
#if VERSION_STRING == Classic
        if (hasAdditionalEffects(sp))
            sp->spell_coeff_overtime *= 0.95f;

        // In Classic all Area of Effect spells receive 33% of the coefficient
        if (isAoESpell(sp))
            sp->spell_coeff_overtime /= 3;
#else
        if (hasAdditionalEffects(sp))
        {
            sp->spell_coeff_overtime *= 0.95f;
            // In TBC, Area of Effect spells with additional effects receive 33% of the coefficient
            if (isAoESpell(sp))
                sp->spell_coeff_overtime /= 3;
        }
        else
        {
            // and normal Area of Effect spells receive 50% of the coefficient
            if (isAoESpell(sp))
                sp->spell_coeff_overtime /= 2;
        }
#endif

        // Leech spells receive 50% of the coefficient
        if (isLeechSpell)
            sp->spell_coeff_overtime /= 2;

        // Spells below level 20 receive a significant penalty
        if (sp->getBaseLevel() < 20)
        {
            const auto penalty = 1.0f - ((20.0f - float(sp->getBaseLevel())) * 0.0375f);
            sp->spell_coeff_overtime *= penalty;
        }

        // Store coeff value as per tick
        for (uint8_t i = 0; i < MAX_SPELL_EFFECTS; ++i)
        {
            if (sp->getEffectApplyAuraName(i) == SPELL_AURA_PERIODIC_DAMAGE ||
                sp->getEffectApplyAuraName(i) == SPELL_AURA_PERIODIC_HEAL ||
                sp->getEffectApplyAuraName(i) == SPELL_AURA_PERIODIC_LEECH)
            {
                /*
                    If a spell has less than five ticks, each tick receives one-fifth of the bonus per tick.
                    If an over time spell has 5 or more ticks, the spell receives full benefit, divided equally between the number of ticks.
                */
                const auto ticks = baseDuration / sp->getEffectAmplitude(i);
                if (ticks < 5)
                    sp->spell_coeff_overtime /= 5;
                else
                    sp->spell_coeff_overtime /= ticks;
                break;
            }
        }
    }
#endif
}

// Fix if it is a periodic trigger with amplitude = 0, to avoid division by zero
void SpellCustomizations::SetEffectAmplitude(SpellInfo* spell_entry)
{
    for (uint8 y = 0; y < 3; y++)
    {
        if (spell_entry->getEffect(y) != SPELL_EFFECT_APPLY_AURA)
            continue;

        if ((spell_entry->getEffectApplyAuraName(y) == SPELL_AURA_PERIODIC_TRIGGER_SPELL || spell_entry->getEffectApplyAuraName(y) == SPELL_AURA_PERIODIC_TRIGGER_SPELL_WITH_VALUE) && spell_entry->getEffectApplyAuraName(y) == 0)
        {
            spell_entry->setEffectAmplitude(1000, y);

            LogDebugFlag(LF_DB_TABLES, "SpellCustomizations::SetEffectAmplitude : EffectAmplitude applied Spell - %s (%u)", spell_entry->getName().c_str(), spell_entry->getId());
        }
    }
}

void SpellCustomizations::SetAuraFactoryFunc(SpellInfo* spell_entry)
{
    bool spell_aura_factory_functions_loaded = false;

    for (uint8 y = 0; y < 3; y++)
    {
        if (spell_entry->getEffect(y) != SPELL_EFFECT_APPLY_AURA)
            continue;

        if (spell_entry->getEffectApplyAuraName(y) == SPELL_AURA_SCHOOL_ABSORB && spell_entry->AuraFactoryFunc == nullptr)
        {
            spell_entry->AuraFactoryFunc = (void * (*)) &AbsorbAura::Create;

            spell_aura_factory_functions_loaded = true;
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
        if (spell_entry->getEffect(z) == SPELL_EFFECT_SCHOOL_DAMAGE && spell_entry->getDmgClass() == SPELL_DMG_TYPE_MELEE)
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
        if (spell_entry->getEffect(z) == SPELL_EFFECT_SCHOOL_DAMAGE && spell_entry->getDmgClass() == SPELL_DMG_TYPE_RANGED)
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
        }
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
