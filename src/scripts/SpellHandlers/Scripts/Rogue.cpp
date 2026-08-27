/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "Objects/Units/Unit.hpp"
#include "Objects/Units/Players/Player.hpp"
#include "Spell/Spell.hpp"
#include "Spell/SpellAura.hpp"
#include "Spell/SpellInfo.hpp"
#include "Spell/SpellMgr.hpp"
#include "Spell/SpellScript.hpp"
#include "Storage/WDB/WDBStores.hpp"
#include "Storage/WDB/WDBStructures.hpp"
#include "Utilities/Random.hpp"

enum RogueSpells
{
    SPELL_CHEAT_DEATH_ABSORB_R3     = 31230,
    SPELL_CHEAT_DEATH_DUMMY         = 31231,
    SPELL_CHEATED_DEATH             = 45181,
    SPELL_CHEATING_DEATH            = 45182,
    SPELL_CUT_TO_THE_CHASE_R1       = 51664,
    SPELL_CRIPPLING_POISON          = 3409,
    SPELL_DEADLY_BREW_R1            = 51625,
};

#if VERSION_STRING >= TBC
class CheatDeathAbsorb : public SpellScript
{
public:
    SpellScriptExecuteState onAuraAbsorb(Aura* aur, AuraEffectModifier* aurEff, uint32_t* absorbed, uint32_t* dmg, bool initialCheck) override
    {
        auto* const plrOwner = aur->getPlayerOwner();
        if (plrOwner == nullptr)
            return SpellScriptExecuteState::EXECUTE_PREVENT;

        if (initialCheck)
        {
            // Check for cooldown
            if (plrOwner->hasSpellOnCooldown(sSpellMgr.getSpellInfo(SPELL_CHEAT_DEATH_DUMMY)))
                return SpellScriptExecuteState::EXECUTE_PREVENT;

#if VERSION_STRING < Mop
            // Check proc chance
            if (!Util::checkChance(aurEff->getEffectDamage()))
                return SpellScriptExecuteState::EXECUTE_PREVENT;
#endif

            // Check if damage would kill player
            const auto exactHealth = plrOwner->getExactCurrentHealth();
            if (static_cast<int32_t>(*dmg) < exactHealth)
                return SpellScriptExecuteState::EXECUTE_PREVENT;

#if VERSION_STRING >= Mop
            if (aurEff->getEffectExtra2Field() == 0)
            {
                // On first absorb calculate max absorb
                // "Hotfix (2012-09-14): Cheat Death can now only absorb up to twice the rogue's maximum health."
                aurEff->setEffectDamage(static_cast<int32_t>(plrOwner->getMaxHealth() * 2));
                aurEff->setEffectExtra2Field(1);
            }
#endif

            // "If the Rogue is below 10% health, the killing blow is completely absorbed"
            // "If the Rogue is over 10% health, enough damage will be absorbed to reduce the Rogue’s health down to 10%"
            const int32_t minHealth = plrOwner->getMaxHealth() / 10;
            const auto realDmg = exactHealth > minHealth ? exactHealth - minHealth : 0;
            *absorbed = *dmg - realDmg;
            *dmg = realDmg;

#if VERSION_STRING < Mop
            // Set batched absorb so aura will be called on health update
            aurEff->setEffectExtraField(aurEff->getEffectExtraField() + *absorbed);
            return SpellScriptExecuteState::EXECUTE_PREVENT;
#else // Mop+
            return SpellScriptExecuteState::EXECUTE_OK;
#endif
        }
        else
        {
            // Cast dummy spell on health update which sets the cooldown
            plrOwner->castSpell(plrOwner, SPELL_CHEAT_DEATH_DUMMY, true);

            aurEff->setEffectExtraField(0);
#if VERSION_STRING >= Mop
            // Recalculate max hp next time Cheat Death can proc
            aurEff->setEffectDamage(aurEff->getEffectBaseDamage());
            aurEff->setEffectExtra2Field(0);
#endif
            return SpellScriptExecuteState::EXECUTE_PREVENT;
        }
    }
};

class CheatDeathDummy : public SpellScript
{
#if VERSION_STRING < Cata
    static inline constexpr int32_t cooldown = 60000; // 60 sec
#else // Cata+
    static inline constexpr int32_t cooldown = 90000; // 90 sec
#endif
public:
    SpellScriptCheckDummy onDummyOrScriptedEffect(Spell* spell, uint8_t effIndex) override
    {
        auto* const plrCaster = spell->getPlayerCaster();
        if (plrCaster == nullptr || effIndex != EFF_INDEX_0)
            return SpellScriptCheckDummy::DUMMY_OK;

        // Set cooldown
        plrCaster->addSpellCooldown(spell->getSpellInfo(), nullptr, spell, cooldown);

        // Cast dmg reduction aura
#if VERSION_STRING < Cata
        // "It is now reduced by a maximum of 90%, depending on how much resilience the Rogue has"
        // "The damage reduction will be four times the damage reduction resilience causes against critical strikes"
        const int32_t resilienceCrit = plrCaster->getCombatRating(CR_CRIT_TAKEN_MELEE);
        SpellForcedBasePoints forcedBasePoints;
        forcedBasePoints.set(EFF_INDEX_0, -std::min(90, resilienceCrit * 4));
        plrCaster->castSpell(plrCaster, SPELL_CHEATING_DEATH, forcedBasePoints, true);
#else // Cata+
        // No longer modified by resilience
        plrCaster->castSpell(plrCaster, SPELL_CHEATING_DEATH, true);
#endif
#if VERSION_STRING >= Mop
        // Cast new debuff
        plrCaster->castSpell(plrCaster, SPELL_CHEATED_DEATH, true);
#endif
        return SpellScriptCheckDummy::DUMMY_OK;
    }
};

class CheatingDeath : public SpellScript
{
public:
    SpellScriptCheckDummy onAuraDummyEffect(Aura* aur, AuraEffectModifier* aurEff, bool apply) override
    {
        if (aurEff->getEffectIndex() != EFF_INDEX_0)
            return SpellScriptCheckDummy::DUMMY_OK;

        const float value = apply ?
            aurEff->getEffectDamage() / 100.f :
            -aurEff->getEffectDamage() / 100.f;

        for (uint8_t i = SCHOOL_NORMAL; i <= SCHOOL_ARCANE; ++i)
            aur->getOwner()->m_damageTakenPctMod[i] += value;

        return SpellScriptCheckDummy::DUMMY_OK;
    }
};

#if VERSION_STRING >= Mop
class CheatedDeath : public SpellScript
{
public:
    SpellScriptCheckDummy onAuraDummyEffect(Aura* /*aur*/, AuraEffectModifier* /*aurEff*/, bool /*apply*/) override
    {
        return SpellScriptCheckDummy::DUMMY_OK;
    }
};
#endif
#endif

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

#if VERSION_STRING >= TBC
    mgr->register_spell_script(SPELL_CHEAT_DEATH_ABSORB_R3, new CheatDeathAbsorb);
    mgr->register_spell_script(SPELL_CHEAT_DEATH_DUMMY, new CheatDeathDummy);
    mgr->register_spell_script(SPELL_CHEATING_DEATH, new CheatingDeath);
#if VERSION_STRING >= Mop
    mgr->register_spell_script(SPELL_CHEATED_DEATH, new CheatedDeath);
#endif
#endif

#if VERSION_STRING >= WotLK
    mgr->register_spell_script(SPELL_CUT_TO_THE_CHASE_R1, new CutToTheChase);
#endif

#if VERSION_STRING >= WotLK && VERSION_STRING < Mop
    mgr->register_spell_script(SPELL_DEADLY_BREW_R1, new DeadlyBrew);
#endif
}
