/*
Copyright (c) 2014-2018 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Common.hpp"
#include "DBCStructures.h"
#include "../world/Storage/DBC/DBCStores.h"

int32_t DBC::Structures::SpellEntry::CalculateSimpleValue(SpellEffectIndex eff) const
{
    if (SpellEffectEntry const* effectEntry = GetSpellEffectEntry(Id, eff))
        return effectEntry->CalculateSimpleValue();
    return 0;
}

DBC::Structures::ClassFamilyMask const& DBC::Structures::SpellEntry::GetEffectSpellClassMask(SpellEffectIndex /*eff*/) const
{
    /*if (SpellEffectEntry const* effectEntry = GetSpellEffectEntry(Id, eff))
        return effectEntry->EffectSpellClassMask;*/

    static ClassFamilyMask const emptyCFM;

    return emptyCFM;
}

DBC::Structures::SpellAuraOptionsEntry const* DBC::Structures::SpellEntry::GetSpellAuraOptions() const
{
    return SpellAuraOptionsId ? sSpellAuraOptionsStore.LookupEntry(SpellAuraOptionsId) : nullptr;
}

DBC::Structures::SpellAuraRestrictionsEntry const* DBC::Structures::SpellEntry::GetSpellAuraRestrictions() const
{
    return SpellAuraRestrictionsId ? sSpellAuraRestrictionsStore.LookupEntry(SpellAuraRestrictionsId) : nullptr;
}

DBC::Structures::SpellCastingRequirementsEntry const* DBC::Structures::SpellEntry::GetSpellCastingRequirements() const
{
    return SpellCastingRequirementsId ? sSpellCastingRequirementsStore.LookupEntry(SpellCastingRequirementsId) : nullptr;
}

DBC::Structures::SpellCategoriesEntry const* DBC::Structures::SpellEntry::GetSpellCategories() const
{
    return SpellCategoriesId ? sSpellCategoriesStore.LookupEntry(SpellCategoriesId) : nullptr;
}

DBC::Structures::SpellClassOptionsEntry const* DBC::Structures::SpellEntry::GetSpellClassOptions() const
{
    return SpellClassOptionsId ? sSpellClassOptionsStore.LookupEntry(SpellClassOptionsId) : nullptr;
}

DBC::Structures::SpellCooldownsEntry const* DBC::Structures::SpellEntry::GetSpellCooldowns() const
{
    return SpellCooldownsId ? sSpellCooldownsStore.LookupEntry(SpellCooldownsId) : nullptr;
}

DBC::Structures::SpellEffectEntry const* DBC::Structures::SpellEntry::GetSpellEffect(SpellEffectIndex eff) const
{
    return GetSpellEffectEntry(Id, eff);
}

DBC::Structures::SpellEquippedItemsEntry const* DBC::Structures::SpellEntry::GetSpellEquippedItems() const
{
    return SpellEquippedItemsId ? sSpellEquippedItemsStore.LookupEntry(SpellEquippedItemsId) : nullptr;
}

DBC::Structures::SpellInterruptsEntry const* DBC::Structures::SpellEntry::GetSpellInterrupts() const
{
    return SpellInterruptsId ? sSpellInterruptsStore.LookupEntry(SpellInterruptsId) : nullptr;
}

DBC::Structures::SpellLevelsEntry const* DBC::Structures::SpellEntry::GetSpellLevels() const
{
    return SpellLevelsId ? sSpellLevelsStore.LookupEntry(SpellLevelsId) : nullptr;
}

DBC::Structures::SpellPowerEntry const* DBC::Structures::SpellEntry::GetSpellPower() const
{
    return SpellPowerId ? sSpellPowerStore.LookupEntry(SpellPowerId) : nullptr;
}

DBC::Structures::SpellReagentsEntry const* DBC::Structures::SpellEntry::GetSpellReagents() const
{
    return SpellReagentsId ? sSpellReagentsStore.LookupEntry(SpellReagentsId) : nullptr;
}

DBC::Structures::SpellScalingEntry const* DBC::Structures::SpellEntry::GetSpellScaling() const
{
    return SpellScalingId ? sSpellScalingStore.LookupEntry(SpellScalingId) : nullptr;
}

DBC::Structures::SpellShapeshiftEntry const* DBC::Structures::SpellEntry::GetSpellShapeshift() const
{
    return SpellShapeshiftId ? sSpellShapeshiftStore.LookupEntry(SpellShapeshiftId) : nullptr;
}

DBC::Structures::SpellTargetRestrictionsEntry const* DBC::Structures::SpellEntry::GetSpellTargetRestrictions() const
{
    return SpellTargetRestrictionsId ? sSpellTargetRestrictionsStore.LookupEntry(SpellTargetRestrictionsId) : nullptr;
}

DBC::Structures::SpellTotemsEntry const* DBC::Structures::SpellEntry::GetSpellTotems() const
{
    return SpellTotemsId ? sSpellTotemsStore.LookupEntry(SpellTotemsId) : nullptr;
}

uint32_t DBC::Structures::SpellEntry::GetManaCost() const
{
    SpellPowerEntry const* power = GetSpellPower();
    return power ? power->manaCost : 0;
}

uint32_t DBC::Structures::SpellEntry::GetPreventionType() const
{
    SpellCategoriesEntry const* cat = GetSpellCategories();
    return cat ? cat->PreventionType : 0;
}

uint32_t DBC::Structures::SpellEntry::GetCategory() const
{
    SpellCategoriesEntry const* cat = GetSpellCategories();
    return cat ? cat->Category : 0;
}

uint32_t DBC::Structures::SpellEntry::GetStartRecoveryTime() const
{
    SpellCooldownsEntry const* cd = GetSpellCooldowns();
    return cd ? cd->StartRecoveryTime : 0;
}

uint32_t DBC::Structures::SpellEntry::GetMechanic() const
{
    SpellCategoriesEntry const* cat = GetSpellCategories();
    return cat ? cat->Mechanic : 0;
}

uint32_t DBC::Structures::SpellEntry::GetRecoveryTime() const
{
    SpellCooldownsEntry const* cd = GetSpellCooldowns();
    return cd ? cd->RecoveryTime : 0;
}

uint32_t DBC::Structures::SpellEntry::GetCategoryRecoveryTime() const
{
    SpellCooldownsEntry const* cd = GetSpellCooldowns();
    return cd ? cd->CategoryRecoveryTime : 0;
}

uint32_t DBC::Structures::SpellEntry::GetStartRecoveryCategory() const
{
    SpellCategoriesEntry const* cat = GetSpellCategories();
    return cat ? cat->StartRecoveryCategory : 0;
}

uint32_t DBC::Structures::SpellEntry::GetSpellLevel() const
{
    SpellLevelsEntry const* levels = GetSpellLevels();
    return levels ? levels->spellLevel : 0;
}

int32_t DBC::Structures::SpellEntry::GetEquippedItemClass() const
{
    SpellEquippedItemsEntry const* items = GetSpellEquippedItems();
    return items ? items->EquippedItemClass : -1;
}

SpellFamily DBC::Structures::SpellEntry::GetSpellFamilyName() const
{
    SpellClassOptionsEntry const* classOpt = GetSpellClassOptions();
    return classOpt ? SpellFamily(classOpt->SpellFamilyName) : SPELLFAMILY_GENERIC;
}

uint32_t DBC::Structures::SpellEntry::GetDmgClass() const
{
    SpellCategoriesEntry const* cat = GetSpellCategories();
    return cat ? cat->DmgClass : 0;
}

uint32_t DBC::Structures::SpellEntry::GetDispel() const
{
    SpellCategoriesEntry const* cat = GetSpellCategories();
    return cat ? cat->Dispel : 0;
}

uint32_t DBC::Structures::SpellEntry::GetMaxAffectedTargets() const
{
    SpellTargetRestrictionsEntry const* target = GetSpellTargetRestrictions();
    return target ? target->MaxAffectedTargets : 0;
}

uint32_t DBC::Structures::SpellEntry::GetStackAmount() const
{
    SpellAuraOptionsEntry const* aura = GetSpellAuraOptions();
    return aura ? aura->StackAmount : 0;
}

uint32_t DBC::Structures::SpellEntry::GetManaCostPercentage() const
{
    SpellPowerEntry const* power = GetSpellPower();
    return power ? power->ManaCostPercentage : 0;
}

uint32_t DBC::Structures::SpellEntry::GetProcCharges() const
{
    SpellAuraOptionsEntry const* aura = GetSpellAuraOptions();
    return aura ? aura->procCharges : 0;
}

uint32_t DBC::Structures::SpellEntry::GetProcChance() const
{
    SpellAuraOptionsEntry const* aura = GetSpellAuraOptions();
    return aura ? aura->procChance : 0;
}

uint32_t DBC::Structures::SpellEntry::GetMaxLevel() const
{
    SpellLevelsEntry const* levels = GetSpellLevels();
    return levels ? levels->maxLevel : 0;
}

uint32_t DBC::Structures::SpellEntry::GetTargetAuraState() const
{
    SpellAuraRestrictionsEntry const* aura = GetSpellAuraRestrictions();
    return aura ? aura->TargetAuraState : 0;
}

uint32_t DBC::Structures::SpellEntry::GetManaPerSecond() const
{
    SpellPowerEntry const* power = GetSpellPower();
    return power ? power->manaPerSecond : 0;
}

uint32_t DBC::Structures::SpellEntry::GetRequiresSpellFocus() const
{
    SpellCastingRequirementsEntry const* castReq = GetSpellCastingRequirements();
    return castReq ? castReq->RequiresSpellFocus : 0;
}

uint32_t DBC::Structures::SpellEntry::GetSpellEffectIdByIndex(SpellEffectIndex index) const
{
    SpellEffectEntry const* effect = GetSpellEffect(index);
    return effect ? effect->Effect : 0; // SPELL_EFFECT_NULL;
}

uint32_t DBC::Structures::SpellEntry::GetAuraInterruptFlags() const
{
    SpellInterruptsEntry const* interrupt = GetSpellInterrupts();
    return interrupt ? interrupt->AuraInterruptFlags : 0;
}

uint32_t DBC::Structures::SpellEntry::GetEffectImplicitTargetAByIndex(SpellEffectIndex index) const
{
    SpellEffectEntry const* effect = GetSpellEffect(index);
    return effect ? effect->EffectImplicitTargetA : TARGET_NONE;
}

int32_t DBC::Structures::SpellEntry::GetAreaGroupId() const
{
    SpellCastingRequirementsEntry const* castReq = GetSpellCastingRequirements();
    return castReq ? castReq->AreaGroupId : 0;
}

uint32_t DBC::Structures::SpellEntry::GetFacingCasterFlags() const
{
    SpellCastingRequirementsEntry const* castReq = GetSpellCastingRequirements();
    return castReq ? castReq->FacingCasterFlags : 0;
}

uint32_t DBC::Structures::SpellEntry::GetBaseLevel() const
{
    SpellLevelsEntry const* levels = GetSpellLevels();
    return levels ? levels->baseLevel : 0;
}

uint32_t DBC::Structures::SpellEntry::GetInterruptFlags() const
{
    SpellInterruptsEntry const* interrupt = GetSpellInterrupts();
    return interrupt ? interrupt->InterruptFlags : 0;
}

uint32_t DBC::Structures::SpellEntry::GetTargetCreatureType() const
{
    SpellTargetRestrictionsEntry const* target = GetSpellTargetRestrictions();
    return target ? target->TargetCreatureType : 0;
}

int32_t DBC::Structures::SpellEntry::GetEffectMiscValue(SpellEffectIndex index) const
{
    SpellEffectEntry const* effect = GetSpellEffect(index);
    return effect ? effect->EffectMiscValue : 0;
}

uint32_t DBC::Structures::SpellEntry::GetStances() const
{
    SpellShapeshiftEntry const* ss = GetSpellShapeshift();
    return ss ? ss->Stances : 0;
}

uint32_t DBC::Structures::SpellEntry::GetStancesNot() const
{
    SpellShapeshiftEntry const* ss = GetSpellShapeshift();
    return ss ? ss->StancesNot : 0;
}

uint32_t DBC::Structures::SpellEntry::GetProcFlags() const
{
    SpellAuraOptionsEntry const* aura = GetSpellAuraOptions();
    return aura ? aura->procFlags : 0;
}

uint32_t DBC::Structures::SpellEntry::GetChannelInterruptFlags() const
{
    SpellInterruptsEntry const* interrupt = GetSpellInterrupts();
    return interrupt ? interrupt->ChannelInterruptFlags : 0;
}

uint32_t DBC::Structures::SpellEntry::GetManaCostPerLevel() const
{
    SpellPowerEntry const* power = GetSpellPower();
    return power ? power->manaCostPerlevel : 0;
}

uint32_t DBC::Structures::SpellEntry::GetCasterAuraState() const
{
    SpellAuraRestrictionsEntry const* aura = GetSpellAuraRestrictions();
    return aura ? aura->CasterAuraState : 0;
}

uint32_t DBC::Structures::SpellEntry::GetTargets() const
{
    SpellTargetRestrictionsEntry const* target = GetSpellTargetRestrictions();
    return target ? target->Targets : 0;
}

uint32_t DBC::Structures::SpellEntry::GetEffectApplyAuraNameByIndex(SpellEffectIndex index) const
{
    SpellEffectEntry const* effect = GetSpellEffect(index);
    return effect ? effect->EffectApplyAuraName : 0;
}
