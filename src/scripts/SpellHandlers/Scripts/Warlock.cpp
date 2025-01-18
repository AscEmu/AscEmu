/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "Objects/Units/Unit.hpp"
#include "Spell/SpellAura.hpp"
#include "Spell/SpellMgr.hpp"
#include "Spell/SpellProc.hpp"
#include "Spell/SpellScript.hpp"

enum WarlockSpells
{
    SPELL_BACKLASH_PROC             = 34936,
    SPELL_NIGHTFALL_R1              = 18094,
    SPELL_SHADOW_TRANCE_PROC        = 17941,
};

#if VERSION_STRING >= TBC
class Backlash : public SpellScript
{
public:
    void onAuraCreate(Aura* aur) override
    {
        // SpellInfo is missing charge count
        aur->setCharges(1, false);
    }

    void onCreateSpellProc(SpellProc* proc, Object* /*obj*/) override
    {
        // 8 sec cooldown
        proc->setProcInterval(8000);
        proc->setCastedOnProcOwner(true);
    }
};
#endif

class NightfallDummy : public SpellScript
{
public:
    SpellScriptCheckDummy onAuraDummyEffect(Aura* aur, AuraEffectModifier* /*aurEff*/, bool apply) override
    {
        if (apply)
        {
            // Should proc on Corruption and Drain Life
            const uint32_t procMask[3] = { 0xA, 0, 0 };
            aur->getOwner()->addProcTriggerSpell(sSpellMgr.getSpellInfo(SPELL_SHADOW_TRANCE_PROC), aur, aur->getCasterGuid(), procMask);
        }
        else
        {
            aur->getOwner()->removeProcTriggerSpell(SPELL_SHADOW_TRANCE_PROC, aur->getCasterGuid());
        }

        return SpellScriptCheckDummy::DUMMY_OK;
    }
};

class ShadowTrance : public SpellScript
{
public:
    void onAuraCreate(Aura* aur) override
    {
        // SpellInfo is missing charge count
        aur->setCharges(1, false);
    }

    void onCreateSpellProc(SpellProc* proc, Object* /*obj*/) override
    {
        proc->setCastedOnProcOwner(true);
    }

    bool canProc(SpellProc* proc, Unit* /*victim*/, SpellInfo const* /*castingSpell*/, DamageInfo damageInfo) override
    {
        // Cannot proc if warlock already has the aura
        if (proc->getProcOwner()->hasAurasWithId(SPELL_SHADOW_TRANCE_PROC))
            return false;

        // Should proc only when dealing damage
        if (damageInfo.realDamage == 0)
            return false;

        return true;
    }
};

void setupWarlockSpells(ScriptMgr* mgr)
{
    // Call legacy script setup
    SetupLegacyWarlockSpells(mgr);

#if VERSION_STRING >= TBC
    mgr->register_spell_script(SPELL_BACKLASH_PROC, new Backlash);
#endif

    mgr->register_spell_script(SPELL_NIGHTFALL_R1, new NightfallDummy);
    mgr->register_spell_script(SPELL_SHADOW_TRANCE_PROC, new ShadowTrance);
}
