/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "WDBStructures.hpp"
#include "Spell/Definitions/SpellFamily.hpp"
#include "WDBStores.hpp"

WDB::Structures::SpellAuraOptionsEntry const* WDB::Structures::SpellEntry::GetSpellAuraOptions() const
{
    return spellAuraOptionsId ? sSpellAuraOptionsStore.lookupEntry(spellAuraOptionsId) : nullptr;
}

WDB::Structures::SpellAuraRestrictionsEntry const* WDB::Structures::SpellEntry::GetSpellAuraRestrictions() const
{
    return spellAuraRestrictionsId ? sSpellAuraRestrictionsStore.lookupEntry(spellAuraRestrictionsId) : nullptr;
}

WDB::Structures::SpellCastingRequirementsEntry const* WDB::Structures::SpellEntry::GetSpellCastingRequirements() const
{
    return spellCastingRequirementsId ? sSpellCastingRequirementsStore.lookupEntry(spellCastingRequirementsId) : nullptr;
}

WDB::Structures::SpellCategoriesEntry const* WDB::Structures::SpellEntry::GetSpellCategories() const
{
    return spellCategoriesId ? sSpellCategoriesStore.lookupEntry(spellCategoriesId) : nullptr;
}

WDB::Structures::SpellClassOptionsEntry const* WDB::Structures::SpellEntry::GetSpellClassOptions() const
{
    return spellClassOptionsId ? sSpellClassOptionsStore.lookupEntry(spellClassOptionsId) : nullptr;
}

WDB::Structures::SpellCooldownsEntry const* WDB::Structures::SpellEntry::GetSpellCooldowns() const
{
    return spellCooldownsId ? sSpellCooldownsStore.lookupEntry(spellCooldownsId) : nullptr;
}

WDB::Structures::SpellEffectEntry const* WDB::Structures::SpellEntry::GetSpellEffect(uint8_t eff) const
{
    return GetSpellEffectEntry(id, eff);
}

WDB::Structures::SpellEquippedItemsEntry const* WDB::Structures::SpellEntry::GetSpellEquippedItems() const
{
    return spellEquippedItemsId ? sSpellEquippedItemsStore.lookupEntry(spellEquippedItemsId) : nullptr;
}

WDB::Structures::SpellInterruptsEntry const* WDB::Structures::SpellEntry::GetSpellInterrupts() const
{
    return spellInterruptsId ? sSpellInterruptsStore.lookupEntry(spellInterruptsId) : nullptr;
}

WDB::Structures::SpellLevelsEntry const* WDB::Structures::SpellEntry::GetSpellLevels() const
{
    return spellLevelsId ? sSpellLevelsStore.lookupEntry(spellLevelsId) : nullptr;
}

WDB::Structures::SpellPowerEntry const* WDB::Structures::SpellEntry::GetSpellPower() const
{
    if (WoW::isServerExpansionAtLeast(WoW::Expansion::_Mop))
    {
        // Verified against the 5.4.8 data layout: Spell.dbc has no SpellPowerId on Mop,
        // SpellPower.dbc rows carry the spell they belong to in their spellId column. Looking the
        // row up by the spell id returned the power data of an unrelated spell - for login
        // casts that turned out to be a health-cost row, so Spell::takePower()
        // damaged the caster on every login and killed fresh low level characters.
        return getSpellPowerEntry(id);
    }

    if (WoW::getServerExpansion() == WoW::Expansion::_Cata)
        return spellPowerId ? sSpellPowerStore.lookupEntry(spellPowerId) : nullptr;

    return nullptr;
}

WDB::Structures::SpellReagentsEntry const* WDB::Structures::SpellEntry::GetSpellReagents() const
{
    return spellReagentsId ? sSpellReagentsStore.lookupEntry(spellReagentsId) : nullptr;
}


WDB::Structures::SpellScalingEntry const* WDB::Structures::SpellEntry::GetSpellScaling() const
{
    return spellScalingId ? sSpellScalingStore.lookupEntry(spellScalingId) : nullptr;
}

WDB::Structures::SpellShapeshiftEntry const* WDB::Structures::SpellEntry::GetSpellShapeshift() const
{
    return spellShapeshiftId ? sSpellShapeshiftStore.lookupEntry(spellShapeshiftId) : nullptr;
}

WDB::Structures::SpellTargetRestrictionsEntry const* WDB::Structures::SpellEntry::GetSpellTargetRestrictions() const
{
    return spellTargetRestrictionsId ? sSpellTargetRestrictionsStore.lookupEntry(spellTargetRestrictionsId) : nullptr;
}

WDB::Structures::SpellTotemsEntry const* WDB::Structures::SpellEntry::GetSpellTotems() const
{
    return spellTotemsId ? sSpellTotemsStore.lookupEntry(spellTotemsId) : nullptr;
}

WDB::Structures::SpellMiscEntry const* WDB::Structures::SpellEntry::GetSpellMisc() const
{
    return spellMiscId ? sSpellMiscStore.lookupEntry(spellMiscId) : nullptr;
}

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
    return aura ? aura->maxStackAmount : 0;
}

uint32_t WDB::Structures::SpellEntry::GetManaCostPercentage() const
{
    SpellPowerEntry const* power = GetSpellPower();
    if (!power)
        return 0;

    if (WoW::getServerExpansion() == WoW::Expansion::_Cata)
        return power->manaCostPercentage;

    return static_cast<uint32_t>(power->manaCostPercentageFloat);
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
    SpellEffectEntry const* spellEffect = GetSpellEffect(index);
    return spellEffect ? spellEffect->Effect : 0;
}

uint32_t WDB::Structures::SpellEntry::GetAuraInterruptFlags() const
{
    SpellInterruptsEntry const* interrupt = GetSpellInterrupts();
    return interrupt ? interrupt->AuraInterruptFlags : 0;
}

uint32_t WDB::Structures::SpellEntry::GetEffectImplicitTargetAByIndex(uint8_t index) const
{
    SpellEffectEntry const* spellEffect = GetSpellEffect(index);
    return spellEffect ? spellEffect->EffectImplicitTargetA : TARGET_NONE;
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
    SpellEffectEntry const* spellEffect = GetSpellEffect(index);
    return spellEffect ? spellEffect->EffectMiscValue : 0;
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
    return power ? power->manaCostPerLevel : 0;
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
    SpellEffectEntry const* spellEffect = GetSpellEffect(index);
    return spellEffect ? spellEffect->EffectApplyAuraName : 0;
}
