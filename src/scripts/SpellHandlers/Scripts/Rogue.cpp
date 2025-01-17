/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "Objects/Units/Unit.hpp"
#include "Spell/SpellAura.hpp"
#include "Spell/SpellInfo.hpp"
#include "Spell/SpellScript.hpp"
#include "Spell/Definitions/SpellFamily.hpp"
#include "Storage/WDB/WDBStores.hpp"
#include "Storage/WDB/WDBStructures.hpp"

enum RogueSpells
{
    SPELL_CUT_TO_THE_CHASE_R1       = 51664,
    SPELL_CRIPPLING_POISON          = 3409,
    SPELL_DEADLY_BREW_R1            = 51625,
};

#if VERSION_STRING >= WotLK
class CutToTheChase : public SpellScript
{
public:
    SpellScriptCheckDummy onAuraDummyEffect(Aura* aur, AuraEffectModifier* /*aurEff*/, bool apply) override
    {
        if (apply)
        {
            // Make it proc self on Eviscerate and Envenom
            const uint32_t procFlags[3] = { 0x800000, 0x8, 0 };
            aur->getOwner()->addProcTriggerSpell(aur->getSpellInfo(), aur->getCasterGuid(), aur, procFlags);
        }
        else
        {
            aur->getOwner()->removeProcTriggerSpell(aur->getSpellId(), aur->getCasterGuid());
        }

        return SpellScriptCheckDummy::DUMMY_OK;
    }

    bool canProc(SpellProc* spellProc, Unit* /*victim*/, SpellInfo const* /*castingSpell*/, DamageInfo /*damageInfo*/) override
    {
        // Find Slice and Dice aura
        for (const auto& aurEff : spellProc->getProcOwner()->getAuraEffectList(SPELL_AURA_MOD_HASTE))
        {
            auto* const aur = aurEff->getAura();
            if (aur->getCasterGuid() != spellProc->getCasterGuid())
                continue;

            const auto spinfo = aur->getSpellInfo();
            if (spinfo->getSpellFamilyName() != SPELLFAMILY_ROGUE)
                continue;

            // Slice and Dice
            if (spinfo->getSpellFamilyFlags(0) == 0x40000 &&
                spinfo->getSpellFamilyFlags(1) == 0 &&
                spinfo->getSpellFamilyFlags(2) == 0)
            {
                sliceAura = aur;
                break;
            }
        }

        return sliceAura != nullptr;
    }

    SpellScriptExecuteState onDoProcEffect(SpellProc* /*spellProc*/, Unit* victim, SpellInfo const* /*castingSpell*/, DamageInfo /*damageInfo*/) override
    {
        if (victim == nullptr)
            return SpellScriptExecuteState::EXECUTE_PREVENT;

        // Recalculate duration for aura per 5 combo points
        uint32_t maxDuration = 0;
        const auto durEntry = sSpellDurationStore.lookupEntry(sliceAura->getSpellInfo()->getDurationIndex());
        if (durEntry != nullptr)
            maxDuration = durEntry->Duration3;

        // Override the original duration and refresh aura
        sliceAura->setNewMaxDuration(maxDuration);

        sliceAura = nullptr;
        return SpellScriptExecuteState::EXECUTE_PREVENT;
    }

private:
    Aura* sliceAura = nullptr;
};
#endif

#if VERSION_STRING >= WotLK && VERSION_STRING < Mop
class DeadlyBrew : public SpellScript
{
public:
    SpellScriptCheckDummy onAuraDummyEffect(Aura* aur, AuraEffectModifier* /*aurEff*/, bool apply) override
    {
        if (apply)
        {
            // Make it proc self on Instant Poison, Wound Poison or Mind Numbing Poison
            const uint32_t procFlags[3] = { 0x1000A000, 0x80000, 0 };
            aur->getOwner()->addProcTriggerSpell(aur->getSpellInfo(), aur->getCasterGuid(), aur, procFlags);
        }
        else
        {
            aur->getOwner()->removeProcTriggerSpell(aur->getSpellId(), aur->getCasterGuid());
        }

        return SpellScriptCheckDummy::DUMMY_OK;
    }

    SpellScriptExecuteState onDoProcEffect(SpellProc* spellProc, Unit* victim, SpellInfo const* /*castingSpell*/, DamageInfo /*damageInfo*/) override
    {
        if (victim == nullptr)
            return SpellScriptExecuteState::EXECUTE_PREVENT;

        spellProc->getProcOwner()->castSpell(victim, SPELL_CRIPPLING_POISON, true);
        return SpellScriptExecuteState::EXECUTE_PREVENT;
    }
};
#endif

void setupRogueSpells(ScriptMgr* mgr)
{
    // Call legacy script setup
    SetupLegacyRogueSpells(mgr);

#if VERSION_STRING >= WotLK
    mgr->register_spell_script(SPELL_CUT_TO_THE_CHASE_R1, new CutToTheChase);
#endif

#if VERSION_STRING >= WotLK && VERSION_STRING < Mop
    mgr->register_spell_script(SPELL_DEADLY_BREW_R1, new DeadlyBrew);
#endif
}
