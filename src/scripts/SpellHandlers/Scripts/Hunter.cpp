/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"

enum HunterSpells
{
    SPELL_FEROCIOUS_INSPIRATION_DUMMY_R3    = 34460,
    SPELL_FEROCIOUS_INSPIRATION_DUMMY_PROC  = 34457,
    SPELL_FEROCIOUS_INSPIRATION_PROC_TBC    = 34456,
    SPELL_FEROCIOUS_INSPIRATION_PROC_R3     = 75447,
    SPELL_FOCUSED_FIRE_DUMMY_R1             = 35029,
    SPELL_FOCUSED_FIRE_PROC_R1              = 35060,
    SPELL_SPIRIT_BOND_DUMMY_R1              = 20895,
    SPELL_SPIRIT_BOND_R1                    = 19579,
    SPELL_SPIRIT_BOND_DUMMY_R1_MOP          = 109212,
    SPELL_SPIRIT_BOND_R1_MOP                = 118694,
    SPELL_SPIRIT_BOND_PERIODIC              = 149254,
};

// Spirit Bond got new ids with Mop
static uint32_t spiritBondDummyId() { return WoW::isServerExpansionAtLeast(WoW::Expansion::_Mop) ? SPELL_SPIRIT_BOND_DUMMY_R1_MOP : SPELL_SPIRIT_BOND_DUMMY_R1; }
static uint32_t spiritBondId() { return WoW::isServerExpansionAtLeast(WoW::Expansion::_Mop) ? SPELL_SPIRIT_BOND_R1_MOP : SPELL_SPIRIT_BOND_R1; }

class FerociousInspirationDummy : public SpellScript
{
    void onAuraApply(Aura* aur) override
    {
        if (!WoW::isServerExpansion(WoW::Expansion::_TBC))
            return SpellScript::onAuraApply(aur);

        if (auto* const plrOwner = aur->getPlayerOwner())
            plrOwner->addPetAuraSpell(SPELL_FEROCIOUS_INSPIRATION_DUMMY_PROC, true);
    }

    void onAuraRemove(Aura* aur, AuraRemoveMode mode) override
    {
        if (!WoW::isServerExpansion(WoW::Expansion::_TBC))
            return SpellScript::onAuraRemove(aur, mode);

        if (auto* const plrOwner = aur->getPlayerOwner())
            plrOwner->removePetAuraSpell(SPELL_FEROCIOUS_INSPIRATION_DUMMY_PROC);
    }
    SpellScriptCheckDummy onAuraDummyEffect(Aura* aur, AuraEffectModifier* aurEff, bool apply) override
    {
        if (WoW::isServerExpansion(WoW::Expansion::_TBC))
            return SpellScript::onAuraDummyEffect(aur, aurEff, apply);

        if (aurEff->getEffectIndex() != EFF_INDEX_0)
            return SpellScriptCheckDummy::DUMMY_OK;

        const auto* const areaAuraInfo = sSpellMgr.getEquivalentSpellRankFor(aur->getSpellInfo(), sSpellMgr.getSpellInfo(SPELL_FEROCIOUS_INSPIRATION_PROC_R3));
        if (areaAuraInfo == nullptr)
            return SpellScriptCheckDummy::DUMMY_OK;

        auto* const plrOwner = aur->getPlayerOwner();
        if (plrOwner == nullptr)
            return SpellScriptCheckDummy::DUMMY_OK;

        if (apply)
            plrOwner->addPetAuraSpell(areaAuraInfo->getId(), true);
        else
            plrOwner->removePetAuraSpell(areaAuraInfo->getId());

        return SpellScriptCheckDummy::DUMMY_OK;
    }
};

class FerociousInspirationProc : public SpellScript
{
    void onCreateSpellProc(SpellProc* spellProc, Object* /*obj*/) override
    {
        spellProc->setExtraProcFlags(EXTRA_PROC_ON_CRIT_ONLY);
    }
};

class FocusedFireDummy : public SpellScript
{
    // Spell has dummy effect and not dummy aura effect so handle on aura apply/remove
    void onAuraApply(Aura* aur) override
    {
        const auto* const areaAuraInfo = sSpellMgr.getEquivalentSpellRankFor(aur->getSpellInfo(), sSpellMgr.getSpellInfo(SPELL_FOCUSED_FIRE_PROC_R1));
        if (areaAuraInfo == nullptr)
            return;

        if (auto* const plrOwner = aur->getPlayerOwner())
            plrOwner->addPetAuraSpell(areaAuraInfo->getId());
    }

    void onAuraRemove(Aura* aur, AuraRemoveMode /*mode*/) override
    {
        const auto* const areaAuraInfo = sSpellMgr.getEquivalentSpellRankFor(aur->getSpellInfo(), sSpellMgr.getSpellInfo(SPELL_FOCUSED_FIRE_PROC_R1));
        if (areaAuraInfo == nullptr)
            return;

        if (auto* const plrOwner = aur->getPlayerOwner())
            plrOwner->removePetAuraSpell(areaAuraInfo->getId());
    }
};

class SpiritBondDummy : public SpellScript
{
    SpellScriptCheckDummy onAuraDummyEffect(Aura* aur, AuraEffectModifier* aurEff, bool apply) override
    {
        if (!WoW::isServerExpansionAtLeast(WoW::Expansion::_Mop))
        {
            if (aurEff->getEffectIndex() != EFF_INDEX_0)
                return SpellScriptCheckDummy::DUMMY_OK;
        }
        else
        {
            if (aurEff->getEffectIndex() != EFF_INDEX_1)
                return SpellScriptCheckDummy::DUMMY_OK;
        }

        const auto* const areaAuraInfo = sSpellMgr.getEquivalentSpellRankFor(aur->getSpellInfo(), sSpellMgr.getSpellInfo(spiritBondId()));
        if (areaAuraInfo == nullptr)
            return SpellScriptCheckDummy::DUMMY_OK;

        auto* const plrOwner = aur->getPlayerOwner();
        if (plrOwner == nullptr)
            return SpellScriptCheckDummy::DUMMY_OK;

        if (apply)
            plrOwner->addPetAuraSpell(areaAuraInfo->getId());
        else
            plrOwner->removePetAuraSpell(areaAuraInfo->getId());

        return SpellScriptCheckDummy::DUMMY_OK;
    }
};

class SpiritBond : public SpellScript
{
    SpellScriptCheckDummy onAuraDummyEffect(Aura* aur, AuraEffectModifier* /*aurEff*/, bool apply) override
    {
        // Periodic dummy since mop
        if (!apply)
            return SpellScriptCheckDummy::DUMMY_OK;

        aur->getOwner()->castSpell(aur->getOwner(), SPELL_SPIRIT_BOND_PERIODIC, true);
        return SpellScriptCheckDummy::DUMMY_OK;
    }
};

void setupHunterSpells(ScriptMgr* mgr)
{
    // Call legacy script setup
    SetupLegacyHunterSpells(mgr);

    if (WoW::isServerExpansionAtLeast(WoW::Expansion::_TBC) && !WoW::isServerExpansionAtLeast(WoW::Expansion::_Mop))
    {
        mgr->register_spell_script(SPELL_FEROCIOUS_INSPIRATION_DUMMY_R3, new FerociousInspirationDummy);
        if (WoW::isServerExpansion(WoW::Expansion::_TBC))
        {
            mgr->register_spell_script(SPELL_FEROCIOUS_INSPIRATION_PROC_TBC, new FerociousInspirationProc);
        }
    }

    if (WoW::isServerExpansion(WoW::Expansion::_TBC) || WoW::isServerExpansion(WoW::Expansion::_WotLK))
    {
        mgr->register_spell_script(SPELL_FOCUSED_FIRE_DUMMY_R1, new FocusedFireDummy);
    }

    if (!WoW::isServerExpansionAtLeast(WoW::Expansion::_Legion))
    {
        mgr->register_spell_script(spiritBondDummyId(), new SpiritBondDummy);
        if (WoW::isServerExpansionAtLeast(WoW::Expansion::_Mop))
        {
            mgr->register_spell_script(spiritBondId(), new SpiritBond);
        }
    }
}
