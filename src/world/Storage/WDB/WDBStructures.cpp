/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "WDBStructures.hpp"
#include "Spell/Definitions/SpellFamily.hpp"
#include "WDBStores.hpp"

#if VERSION_STRING >= Cata

WDB::Structures::SpellAuraOptionsEntry const* WDB::Structures::SpellEntry::GetSpellAuraOptions() const
{
    return SpellAuraOptionsId ? sSpellAuraOptionsStore.lookupEntry(SpellAuraOptionsId) : nullptr;
}

WDB::Structures::SpellAuraRestrictionsEntry const* WDB::Structures::SpellEntry::GetSpellAuraRestrictions() const
{
    return SpellAuraRestrictionsId ? sSpellAuraRestrictionsStore.lookupEntry(SpellAuraRestrictionsId) : nullptr;
}

WDB::Structures::SpellCastingRequirementsEntry const* WDB::Structures::SpellEntry::GetSpellCastingRequirements() const
{
    return SpellCastingRequirementsId ? sSpellCastingRequirementsStore.lookupEntry(SpellCastingRequirementsId) : nullptr;
}

WDB::Structures::SpellCategoriesEntry const* WDB::Structures::SpellEntry::GetSpellCategories() const
{
    return SpellCategoriesId ? sSpellCategoriesStore.lookupEntry(SpellCategoriesId) : nullptr;
}

WDB::Structures::SpellClassOptionsEntry const* WDB::Structures::SpellEntry::GetSpellClassOptions() const
{
    return SpellClassOptionsId ? sSpellClassOptionsStore.lookupEntry(SpellClassOptionsId) : nullptr;
}

WDB::Structures::SpellCooldownsEntry const* WDB::Structures::SpellEntry::GetSpellCooldowns() const
{
    return SpellCooldownsId ? sSpellCooldownsStore.lookupEntry(SpellCooldownsId) : nullptr;
}

WDB::Structures::SpellEffectEntry const* WDB::Structures::SpellEntry::GetSpellEffect(uint8_t eff) const
{
    return GetSpellEffectEntry(Id, eff);
}

WDB::Structures::SpellEquippedItemsEntry const* WDB::Structures::SpellEntry::GetSpellEquippedItems() const
{
    return SpellEquippedItemsId ? sSpellEquippedItemsStore.lookupEntry(SpellEquippedItemsId) : nullptr;
}

WDB::Structures::SpellInterruptsEntry const* WDB::Structures::SpellEntry::GetSpellInterrupts() const
{
    return SpellInterruptsId ? sSpellInterruptsStore.lookupEntry(SpellInterruptsId) : nullptr;
}

WDB::Structures::SpellLevelsEntry const* WDB::Structures::SpellEntry::GetSpellLevels() const
{
    return SpellLevelsId ? sSpellLevelsStore.lookupEntry(SpellLevelsId) : nullptr;
}

WDB::Structures::SpellPowerEntry const* WDB::Structures::SpellEntry::GetSpellPower() const
{
#if VERSION_STRING == Cata
    return SpellPowerId ? sSpellPowerStore.lookupEntry(SpellPowerId) : nullptr;
#else
    return sSpellPowerStore.lookupEntry(Id);
#endif
}

#if VERSION_STRING == Cata
WDB::Structures::SpellReagentsEntry const* WDB::Structures::SpellEntry::GetSpellReagents() const
{
    return SpellReagentsId ? sSpellReagentsStore.lookupEntry(SpellReagentsId) : nullptr;
}
#endif

WDB::Structures::SpellScalingEntry const* WDB::Structures::SpellEntry::GetSpellScaling() const
{
    return SpellScalingId ? sSpellScalingStore.lookupEntry(SpellScalingId) : nullptr;
}

WDB::Structures::SpellShapeshiftEntry const* WDB::Structures::SpellEntry::GetSpellShapeshift() const
{
    return SpellShapeshiftId ? sSpellShapeshiftStore.lookupEntry(SpellShapeshiftId) : nullptr;
}

WDB::Structures::SpellTargetRestrictionsEntry const* WDB::Structures::SpellEntry::GetSpellTargetRestrictions() const
{
    return SpellTargetRestrictionsId ? sSpellTargetRestrictionsStore.lookupEntry(SpellTargetRestrictionsId) : nullptr;
}

WDB::Structures::SpellTotemsEntry const* WDB::Structures::SpellEntry::GetSpellTotems() const
{
    return SpellTotemsId ? sSpellTotemsStore.lookupEntry(SpellTotemsId) : nullptr;
}

#if VERSION_STRING == Mop
WDB::Structures::SpellMiscEntry const* WDB::Structures::SpellEntry::GetSpellMisc() const
{
    return SpellTotemsId ? sSpellMiscStore.lookupEntry(SpellMiscId) : nullptr;
}
#endif

uint32_t WDB::Structures::SpellEntry::GetManaCost() const
{
    SpellPowerEntry const* power = GetSpellPower();
    return power ? power->manaCost : 0;
}

uint32_t WDB::Structures::SpellEntry::GetPreventionType() const
{
    SpellCategoriesEntry const* cat = GetSpellCategories();
    return cat ? cat->PreventionType : 0;
}

uint32_t WDB::Structures::SpellEntry::GetCategory() const
{
    SpellCategoriesEntry const* cat = GetSpellCategories();
    return cat ? cat->Category : 0;
}

uint32_t WDB::Structures::SpellEntry::GetStartRecoveryTime() const
{
    SpellCooldownsEntry const* cd = GetSpellCooldowns();
    return cd ? cd->StartRecoveryTime : 0;
}

uint32_t WDB::Structures::SpellEntry::GetMechanic() const
{
    SpellCategoriesEntry const* cat = GetSpellCategories();
    return cat ? cat->MechanicsType : 0;
}

uint32_t WDB::Structures::SpellEntry::GetRecoveryTime() const
{
    SpellCooldownsEntry const* cd = GetSpellCooldowns();
    return cd ? cd->RecoveryTime : 0;
}

uint32_t WDB::Structures::SpellEntry::GetCategoryRecoveryTime() const
{
    SpellCooldownsEntry const* cd = GetSpellCooldowns();
    return cd ? cd->CategoryRecoveryTime : 0;
}

uint32_t WDB::Structures::SpellEntry::GetStartRecoveryCategory() const
{
    SpellCategoriesEntry const* cat = GetSpellCategories();
    return cat ? cat->StartRecoveryCategory : 0;
}

uint32_t WDB::Structures::SpellEntry::GetSpellLevel() const
{
    SpellLevelsEntry const* levels = GetSpellLevels();
    return levels ? levels->spellLevel : 0;
}

int32_t WDB::Structures::SpellEntry::GetEquippedItemClass() const
{
    SpellEquippedItemsEntry const* items = GetSpellEquippedItems();
    return items ? items->EquippedItemClass : -1;
}

uint32_t WDB::Structures::SpellEntry::GetSpellFamilyName() const
{
    SpellClassOptionsEntry const* classOpt = GetSpellClassOptions();
    return classOpt ? classOpt->SpellFamilyName : SPELLFAMILY_GENERIC;
}

uint32_t WDB::Structures::SpellEntry::GetDmgClass() const
{
    SpellCategoriesEntry const* cat = GetSpellCategories();
    return cat ? cat->DmgClass : 0;
}

uint32_t WDB::Structures::SpellEntry::GetDispel() const
{
    SpellCategoriesEntry const* cat = GetSpellCategories();
    return cat ? cat->DispelType : 0;
}

uint32_t WDB::Structures::SpellEntry::GetMaxAffectedTargets() const
{
    SpellTargetRestrictionsEntry const* target = GetSpellTargetRestrictions();
    return target ? target->MaxAffectedTargets : 0;
}

uint32_t WDB::Structures::SpellEntry::GetStackAmount() const
{
    SpellAuraOptionsEntry const* aura = GetSpellAuraOptions();
    return aura ? aura->MaxStackAmount : 0;
}

uint32_t WDB::Structures::SpellEntry::GetManaCostPercentage() const
{
    SpellPowerEntry const* power = GetSpellPower();
#if VERSION_STRING == Cata
    return power ? power->ManaCostPercentage : 0;
#else
    return power ? power->ManaCostPercentageFloat : 0;
#endif
}

uint32_t WDB::Structures::SpellEntry::GetProcCharges() const
{
    SpellAuraOptionsEntry const* aura = GetSpellAuraOptions();
    return aura ? aura->procCharges : 0;
}

uint32_t WDB::Structures::SpellEntry::GetProcChance() const
{
    SpellAuraOptionsEntry const* aura = GetSpellAuraOptions();
    return aura ? aura->procChance : 0;
}

uint32_t WDB::Structures::SpellEntry::GetMaxLevel() const
{
    SpellLevelsEntry const* levels = GetSpellLevels();
    return levels ? levels->maxLevel : 0;
}

uint32_t WDB::Structures::SpellEntry::GetTargetAuraState() const
{
    SpellAuraRestrictionsEntry const* aura = GetSpellAuraRestrictions();
    return aura ? aura->TargetAuraState : 0;
}

uint32_t WDB::Structures::SpellEntry::GetManaPerSecond() const
{
    SpellPowerEntry const* power = GetSpellPower();
    return power ? power->manaPerSecond : 0;
}

uint32_t WDB::Structures::SpellEntry::GetRequiresSpellFocus() const
{
    SpellCastingRequirementsEntry const* castReq = GetSpellCastingRequirements();
    return castReq ? castReq->RequiresSpellFocus : 0;
}

uint32_t WDB::Structures::SpellEntry::GetSpellEffectIdByIndex(uint8_t index) const
{
    SpellEffectEntry const* effect = GetSpellEffect(index);
    return effect ? effect->Effect : 0;
}

uint32_t WDB::Structures::SpellEntry::GetAuraInterruptFlags() const
{
    SpellInterruptsEntry const* interrupt = GetSpellInterrupts();
    return interrupt ? interrupt->AuraInterruptFlags : 0;
}

uint32_t WDB::Structures::SpellEntry::GetEffectImplicitTargetAByIndex(uint8_t index) const
{
    SpellEffectEntry const* effect = GetSpellEffect(index);
    return effect ? effect->EffectImplicitTargetA : TARGET_NONE;
}

int32_t WDB::Structures::SpellEntry::GetAreaGroupId() const
{
    SpellCastingRequirementsEntry const* castReq = GetSpellCastingRequirements();
    return castReq ? castReq->AreaGroupId : 0;
}

uint32_t WDB::Structures::SpellEntry::GetFacingCasterFlags() const
{
    SpellCastingRequirementsEntry const* castReq = GetSpellCastingRequirements();
    return castReq ? castReq->FacingCasterFlags : 0;
}

uint32_t WDB::Structures::SpellEntry::GetBaseLevel() const
{
    SpellLevelsEntry const* levels = GetSpellLevels();
    return levels ? levels->baseLevel : 0;
}

uint32_t WDB::Structures::SpellEntry::GetInterruptFlags() const
{
    SpellInterruptsEntry const* interrupt = GetSpellInterrupts();
    return interrupt ? interrupt->InterruptFlags : 0;
}

uint32_t WDB::Structures::SpellEntry::GetTargetCreatureType() const
{
    SpellTargetRestrictionsEntry const* target = GetSpellTargetRestrictions();
    return target ? target->TargetCreatureType : 0;
}

int32_t WDB::Structures::SpellEntry::GetEffectMiscValue(uint8_t index) const
{
    SpellEffectEntry const* effect = GetSpellEffect(index);
    return effect ? effect->EffectMiscValue : 0;
}

uint32_t WDB::Structures::SpellEntry::GetStances() const
{
    SpellShapeshiftEntry const* ss = GetSpellShapeshift();
    return ss ? ss->Shapeshifts : 0;
}

uint32_t WDB::Structures::SpellEntry::GetStancesNot() const
{
    SpellShapeshiftEntry const* ss = GetSpellShapeshift();
    return ss ? ss->ShapeshiftsExcluded : 0;
}

uint32_t WDB::Structures::SpellEntry::GetProcFlags() const
{
    SpellAuraOptionsEntry const* aura = GetSpellAuraOptions();
    return aura ? aura->procFlags : 0;
}

uint32_t WDB::Structures::SpellEntry::GetChannelInterruptFlags() const
{
    SpellInterruptsEntry const* interrupt = GetSpellInterrupts();
    return interrupt ? interrupt->ChannelInterruptFlags : 0;
}

uint32_t WDB::Structures::SpellEntry::GetManaCostPerLevel() const
{
    SpellPowerEntry const* power = GetSpellPower();
    return power ? power->manaCostPerlevel : 0;
}

uint32_t WDB::Structures::SpellEntry::GetCasterAuraState() const
{
    SpellAuraRestrictionsEntry const* aura = GetSpellAuraRestrictions();
    return aura ? aura->CasterAuraState : 0;
}

uint32_t WDB::Structures::SpellEntry::GetTargets() const
{
    SpellTargetRestrictionsEntry const* target = GetSpellTargetRestrictions();
    return target ? target->Targets : 0;
}

uint32_t WDB::Structures::SpellEntry::GetEffectApplyAuraNameByIndex(uint8_t index) const
{
    SpellEffectEntry const* effect = GetSpellEffect(index);
    return effect ? effect->EffectApplyAuraName : 0;
}
#endif
