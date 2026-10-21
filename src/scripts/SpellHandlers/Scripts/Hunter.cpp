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
#if VERSION_STRING < Mop
    SPELL_SPIRIT_BOND_DUMMY_R1              = 20895,
    SPELL_SPIRIT_BOND_R1                    = 19579,
#else
    SPELL_SPIRIT_BOND_DUMMY_R1              = 109212,
    SPELL_SPIRIT_BOND_R1                    = 118694,
    SPELL_SPIRIT_BOND_PERIODIC              = 149254,
#endif
};

#if VERSION_STRING >= TBC && VERSION_STRING < Mop
class FerociousInspirationDummy : public SpellScript
{
#if VERSION_STRING == TBC
    void onAuraApply(Aura* aur) override
    {
        if (auto* const plrOwner = aur->getPlayerOwner())
            plrOwner->addPetAuraSpell(SPELL_FEROCIOUS_INSPIRATION_DUMMY_PROC, true);
    }

    void onAuraRemove(Aura* aur, AuraRemoveMode /*mode*/) override
    {
        if (auto* const plrOwner = aur->getPlayerOwner())
            plrOwner->removePetAuraSpell(SPELL_FEROCIOUS_INSPIRATION_DUMMY_PROC);
    }
#else
    SpellScriptCheckDummy onAuraDummyEffect(Aura* aur, AuraEffectModifier* aurEff, bool apply) override
    {
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
#endif
};

#if VERSION_STRING == TBC
class FerociousInspirationProc : public SpellScript
{
    void onCreateSpellProc(SpellProc* spellProc, Object* /*obj*/) override
    {
        spellProc->setExtraProcFlags(EXTRA_PROC_ON_CRIT_ONLY);
    }
};
#endif
#endif

#if VERSION_STRING == TBC || VERSION_STRING == WotLK
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
#endif

#if VERSION_STRING < Legion
class SpiritBondDummy : public SpellScript
{
    SpellScriptCheckDummy onAuraDummyEffect(Aura* aur, AuraEffectModifier* aurEff, bool apply) override
    {
#if VERSION_STRING < Mop
        if (aurEff->getEffectIndex() != EFF_INDEX_0)
            return SpellScriptCheckDummy::DUMMY_OK;
#else
        if (aurEff->getEffectIndex() != EFF_INDEX_1)
            return SpellScriptCheckDummy::DUMMY_OK;
#endif

        const auto* const areaAuraInfo = sSpellMgr.getEquivalentSpellRankFor(aur->getSpellInfo(), sSpellMgr.getSpellInfo(SPELL_SPIRIT_BOND_R1));
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

#if VERSION_STRING >= Mop
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
#endif
#endif

void setupHunterSpells(ScriptMgr* mgr)
{
    // Call legacy script setup
    SetupLegacyHunterSpells(mgr);

#if VERSION_STRING >= TBC && VERSION_STRING < Mop
    mgr->register_spell_script(SPELL_FEROCIOUS_INSPIRATION_DUMMY_R3, new FerociousInspirationDummy);
#if VERSION_STRING == TBC
    mgr->register_spell_script(SPELL_FEROCIOUS_INSPIRATION_PROC_TBC, new FerociousInspirationProc);
#endif
#endif

#if VERSION_STRING == TBC || VERSION_STRING == WotLK
    mgr->register_spell_script(SPELL_FOCUSED_FIRE_DUMMY_R1, new FocusedFireDummy);
#endif

#if VERSION_STRING < Legion
    mgr->register_spell_script(SPELL_SPIRIT_BOND_DUMMY_R1, new SpiritBondDummy);
#if VERSION_STRING >= Mop
    mgr->register_spell_script(SPELL_SPIRIT_BOND_R1, new SpiritBond);
#endif
#endif
}
