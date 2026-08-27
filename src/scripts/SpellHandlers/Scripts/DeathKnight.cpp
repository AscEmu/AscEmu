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
#include "Utilities/Random.hpp"

#if VERSION_STRING >= WotLK
enum DeathknightSpells
{
    SPELL_ANTIMAGIC_SHELL                   = 48707,
    SPELL_ANTIMAGIC_SHELL_RUNIC_POWER       = 49088,
#if VERSION_STRING == WotLK
    SPELL_BLOOD_PRESENCE                    = 48266,
#else
    SPELL_BLOOD_PRESENCE                    = 48263,
#endif
    SPELL_BLOOD_PRESENCE_HEAL               = 50475,
    SPELL_BUTCHERY_ENERGIZE                 = 50163,
    SPELL_BUTCHERY_R1                       = 48979,
    SPELL_DEATH_AND_DECAY_DAMAGE            = 52212,
    SPELL_DEATH_AND_DECAY_R1                = 43265,
    SPELL_DEATH_RUNE_MASTERY_BLOOD          = 50806,
    SPELL_DEATH_RUNE_MASTERY_R1             = 49467,
#if VERSION_STRING == WotLK
    SPELL_FROST_PRESENCE                    = 48263,
#else
    SPELL_FROST_PRESENCE                    = 48266,
#endif
#if VERSION_STRING == WotLK
    SPELL_IMPROVED_BLOOD_PRESENCE_DUMMY     = 63611,
#else
    SPELL_IMPROVED_BLOOD_PRESENCE_DUMMY     = 61261,
#endif
    SPELL_IMPROVED_BLOOD_PRESENCE_R1        = 50365,
    SPELL_IMPROVED_BLOOD_PRESENCE_R2        = 50371,
#if VERSION_STRING == WotLK
    SPELL_IMPROVED_FROST_PRESENCE_DUMMY     = 61261,
#else
    SPELL_IMPROVED_FROST_PRESENCE_DUMMY     = 63621,
#endif
    SPELL_IMPROVED_FROST_PRESENCE_R1        = 50384,
    SPELL_IMPROVED_FROST_PRESENCE_R2        = 50385,
    SPELL_IMPROVED_UNHOLY_PRESENCE_DUMMY    = 63622,
    SPELL_IMPROVED_UNHOLY_PRESENCE_R1       = 50391,
    SPELL_IMPROVED_UNHOLY_PRESENCE_R2       = 50392,
    SPELL_MAGIC_SUPPRESSION_R1              = 49224,
    SPELL_MARK_OF_BLOOD_DUMMY               = 49005,
    SPELL_MARK_OF_BLOOD_HEAL                = 61607,
    SPELL_RUNE_TAP                          = 48982,
    SPELL_SPELL_DEFLECTION_R1               = 49145,
    SPELL_UNHOLY_PRESENCE                   = 48265,
    SPELL_UNHOLY_PRESENCE_MOVE_SPEED        = 49772,
#if VERSION_STRING == WotLK
    SPELL_WILL_OF_THE_NECROPOLIS_DUMMY_R1   = 49189,
    SPELL_WILL_OF_THE_NECROPOLIS_ABSORB_R1  = 52284,
#else
    SPELL_WILL_OF_THE_NECROPOLIS_PROC       = 81162,
    SPELL_WILL_OF_THE_NECROPOLIS_RUNETAP    = 96171,
#endif
};

#if VERSION_STRING < Mop
// This is a common script to handle Death Knight Presences
class Presences : public SpellScript
{
public:
    virtual bool isBloodPresence() const { return false; }
    virtual bool isFrostPresence() const { return false; }
    virtual bool isUnholyPresence() const { return false; }

    void onAuraApply(Aura* aur) override
    {
        // Handle improved presences here
        Aura const* improvedBloodPresence = nullptr;
        Aura const* improvedFrostPresence = nullptr;
        Aura const* improvedUnholyPresence = nullptr;

        // Get all auras in single loop
        for (const auto& unitAura : aur->getOwner()->getPassiveAuraRange())
        {
            if (unitAura == nullptr)
                continue;

            if (unitAura->getSpellId() == SPELL_IMPROVED_BLOOD_PRESENCE_R2)
                improvedBloodPresence = unitAura.get();
            if (improvedBloodPresence == nullptr && unitAura->getSpellId() == SPELL_IMPROVED_BLOOD_PRESENCE_R1)
                improvedBloodPresence = unitAura.get();

            if (unitAura->getSpellId() == SPELL_IMPROVED_FROST_PRESENCE_R2)
                improvedFrostPresence = unitAura.get();
            if (improvedFrostPresence == nullptr && unitAura->getSpellId() == SPELL_IMPROVED_FROST_PRESENCE_R1)
                improvedFrostPresence = unitAura.get();

            if (unitAura->getSpellId() == SPELL_IMPROVED_UNHOLY_PRESENCE_R2)
                improvedUnholyPresence = unitAura.get();
            if (improvedUnholyPresence == nullptr && unitAura->getSpellId() == SPELL_IMPROVED_UNHOLY_PRESENCE_R1)
                improvedUnholyPresence = unitAura.get();
        }

        // Cast Improved Blood Presence in Frost and Unholy presences
        if (!isBloodPresence() && improvedBloodPresence != nullptr)
        {
            SpellForcedBasePoints forcedBasePoints;
            forcedBasePoints.set(EFF_INDEX_0, improvedBloodPresence->getAuraEffect(EFF_INDEX_0)->getEffectDamage());
            aur->getOwner()->castSpell(aur->getOwner(), sSpellMgr.getSpellInfo(SPELL_IMPROVED_BLOOD_PRESENCE_DUMMY), forcedBasePoints, true);
        }

        // Cast Improved Frost Presence in Blood and Unholy presences
        if (!isFrostPresence() && improvedFrostPresence != nullptr)
        {
            SpellForcedBasePoints forcedBasePoints;
            forcedBasePoints.set(EFF_INDEX_0, improvedFrostPresence->getAuraEffect(EFF_INDEX_0)->getEffectDamage());
            aur->getOwner()->castSpell(aur->getOwner(), sSpellMgr.getSpellInfo(SPELL_IMPROVED_FROST_PRESENCE_DUMMY), forcedBasePoints, true);
        }

        if (improvedUnholyPresence != nullptr)
        {
#if VERSION_STRING == WotLK
            // Cast Unholy Presence speed aura in Blood and Frost presences
            if (!isUnholyPresence())
            {
                SpellForcedBasePoints forcedBasePoints;
                forcedBasePoints.set(EFF_INDEX_0, improvedUnholyPresence->getAuraEffect(EFF_INDEX_0)->getEffectDamage());
                aur->getOwner()->castSpell(aur->getOwner(), sSpellMgr.getSpellInfo(SPELL_UNHOLY_PRESENCE_MOVE_SPEED), forcedBasePoints, true);
            }
            // Cast Improved Unholy Presence in Unholy Presence
            else
            {
                SpellForcedBasePoints forcedBasePoints;
                const auto basePoints = improvedUnholyPresence->getSpellInfo()->calculateEffectValue(EFF_INDEX_1);
                for (uint8_t i = 0; i < MAX_SPELL_EFFECTS; ++i)
                    forcedBasePoints.set(i, basePoints);

                aur->getOwner()->castSpell(aur->getOwner(), sSpellMgr.getSpellInfo(SPELL_IMPROVED_UNHOLY_PRESENCE_DUMMY), forcedBasePoints, true);
            }
#else
            // Cast Improved Unholy Presence in Blood and Frost presences
            if (!isUnholyPresence())
            {
                SpellForcedBasePoints forcedBasePoints;
                forcedBasePoints.set(EFF_INDEX_0, improvedUnholyPresence->getAuraEffect(EFF_INDEX_0)->getEffectDamage());
                aur->getOwner()->castSpell(aur->getOwner(), sSpellMgr.getSpellInfo(SPELL_IMPROVED_UNHOLY_PRESENCE_DUMMY), forcedBasePoints, true);
            }
#endif
        }
    }

    void onAuraRemove(Aura* aur, AuraRemoveMode /*mode*/) override
    {
        // Remove auras in single loop
        uint32_t spellIds[] =
        {
            SPELL_IMPROVED_BLOOD_PRESENCE_DUMMY,
            SPELL_IMPROVED_FROST_PRESENCE_DUMMY,
            SPELL_UNHOLY_PRESENCE_MOVE_SPEED,
            SPELL_IMPROVED_UNHOLY_PRESENCE_DUMMY,
            0
        };
        aur->getOwner()->removeAllAurasById(spellIds);
    }
};
#endif

class AntiMagicShell : public SpellScript
{
public:
    SpellScriptExecuteState beforeAuraEffect([[maybe_unused]]Aura* aur, AuraEffectModifier* aurEff, bool apply) override
    {
        if (!apply || aurEff->getEffectIndex() != EFF_INDEX_0)
            return SpellScriptExecuteState::EXECUTE_OK;

        // Add the refund pct into effect
#if VERSION_STRING == WotLK
        // 2% of absorbed damage is turned into Runic Power
        aurEff->setEffectExtra2Field(20);
#elif VERSION_STRING == Cata
        // Find Magic Suppression talent which enables Runic Power return
        if (const auto* const magicSuppressionAur = aur->getOwner()->getAuraEffectWithAnyRankOfAndByEffect(SPELL_AURA_ADD_FLAT_MODIFIER, SPELL_MAGIC_SUPPRESSION_R1))
        {
            // "Patch 4.0.6: The ratio of runic power return from Magic Suppression has been reduced by roughly one third"
            // Assume 1.3% -Appled
            aurEff->setEffectExtra2Field(13);
        }
#elif VERSION_STRING == Mop
        // Assume same as cata
        aurEff->setEffectExtra2Field(13);
#endif

        return SpellScriptExecuteState::EXECUTE_OK;
    }

    SpellScriptExecuteState onAuraAbsorb(Aura* aur, AuraEffectModifier* aurEff, uint32_t* absorbed, uint32_t* dmg, bool initialCheck) override
    {
        if (initialCheck)
        {
            *absorbed = *dmg * aurEff->getEffectDamage() / 100;
            *dmg -= *absorbed;
        }
        else if (const auto refundPct = aurEff->getEffectExtra2Field(); refundPct > 0)
        {
            SpellForcedBasePoints forcedBasePoints;
            forcedBasePoints.set(EFF_INDEX_0, *absorbed * refundPct / 100);
            aur->getOwner()->castSpell(aur->getOwner(), SPELL_ANTIMAGIC_SHELL_RUNIC_POWER, forcedBasePoints, true);
        }

        return SpellScriptExecuteState::EXECUTE_OK;
    }
};

#if VERSION_STRING < Mop
class BloodPresence : public Presences
{
public:
    bool isBloodPresence() const override { return true; }

    void onAuraApply(Aura* aur) override
    {
        Presences::onAuraApply(aur);

        aur->getOwner()->castSpell(aur->getOwner(), sSpellMgr.getSpellInfo(SPELL_IMPROVED_BLOOD_PRESENCE_DUMMY), true);

        // TODO: for cata we are missing the spell for rune regeneration and for reduced chance to be critically hit
    }
};
#endif

#if VERSION_STRING == WotLK
class BloodPresenceHeal : public SpellScript
{
public:
    bool canProc(SpellProc* /*spellProc*/, Unit* /*victim*/, SpellInfo const* /*castingSpell*/, DamageInfo damageInfo) override
    {
        return damageInfo.realDamage > 0;
    }

    SpellScriptExecuteState onDoProcEffect(SpellProc* spellProc, Unit* /*victim*/, SpellInfo const* /*castingSpell*/, DamageInfo damageInfo) override
    {
        heal = damageInfo.realDamage * spellProc->getOverrideEffectDamage(EFF_INDEX_0) / 100;
        if (heal == 0)
            return SpellScriptExecuteState::EXECUTE_PREVENT;

        return SpellScriptExecuteState::EXECUTE_OK;
    }

    SpellScriptExecuteState onCastProcSpell(SpellProc* /*spellProc*/, Unit* /*caster*/, Unit* /*victim*/, Spell* spell) override
    {
        spell->forced_basepoints->set(EFF_INDEX_0, heal);
        heal = 0;
        return SpellScriptExecuteState::EXECUTE_OK;
    }

private:
    uint32_t heal = 0;
};
#endif

#if VERSION_STRING < Mop
class Butchery : public SpellScript
{
public:
    SpellScriptCheckDummy onAuraDummyEffect(Aura* aur, AuraEffectModifier* aurEff, bool apply) override
    {
        if (apply)
        {
            auto spellProc = aur->getOwner()->addProcTriggerSpell(sSpellMgr.getSpellInfo(SPELL_BUTCHERY_ENERGIZE), aur, aur->getCasterGuid());
            if (spellProc != nullptr)
                spellProc->setOverrideEffectDamage(EFF_INDEX_0, aurEff->getEffectDamage());
        }
        else
        {
            aur->getOwner()->removeProcTriggerSpell(SPELL_BUTCHERY_ENERGIZE, aur->getCasterGuid());
        }

        return SpellScriptCheckDummy::DUMMY_OK;
    }
};
#endif

class DeathAndDecay : public SpellScript
{
public:
    SpellScriptCheckDummy onAuraDummyEffect(Aura* aur, AuraEffectModifier* aurEff, bool /*apply*/) override
    {
        auto* const caster = aur->GetUnitCaster();
        if (caster == nullptr)
            return SpellScriptCheckDummy::DUMMY_OK;

        SpellForcedBasePoints forcedBasePoints;
        forcedBasePoints.set(EFF_INDEX_0, aurEff->getEffectDamage());
        caster->castSpell(aur->getOwner(), SPELL_DEATH_AND_DECAY_DAMAGE, forcedBasePoints, true);
        return SpellScriptCheckDummy::DUMMY_OK;
    }
};

#if VERSION_STRING == WotLK
class DeathRuneMastery : public SpellScript
{
 public:
     void onAuraApply(Aura* aur) override
     {
         // Should proc only on Obliterate and Death Strike
         const uint32_t procFamilyMask[3] = { 0x10, 0x20000, 0 };
         aur->getOwner()->addProcTriggerSpell(sSpellMgr.getSpellInfo(SPELL_DEATH_RUNE_MASTERY_BLOOD), aur, aur->getCasterGuid(), procFamilyMask);
     }

     void onAuraRemove(Aura* aur, AuraRemoveMode /*mode*/) override
     {
         aur->getOwner()->removeProcTriggerSpell(SPELL_DEATH_RUNE_MASTERY_BLOOD, aur->getCasterGuid());
     }
};
#endif

#if VERSION_STRING < Mop
class FrostPresence : public Presences
{
public:
    bool isFrostPresence() const override { return true; }

    void onAuraApply(Aura* aur) override
    {
        Presences::onAuraApply(aur);

#if VERSION_STRING == WotLK
        aur->getOwner()->castSpell(aur->getOwner(), sSpellMgr.getSpellInfo(SPELL_IMPROVED_FROST_PRESENCE_DUMMY), true);
#endif
    }
};
#endif

#if VERSION_STRING == WotLK
class ImprovedBloodPresenceDummy : public SpellScript
{
public:
    SpellScriptExecuteState beforeAuraEffect(Aura* aur, AuraEffectModifier* aurEff, bool apply) override
    {
        if (apply)
        {
            auto spellProc = aur->getOwner()->addProcTriggerSpell(sSpellMgr.getSpellInfo(SPELL_BLOOD_PRESENCE_HEAL), aur, aur->getCasterGuid());
            if (spellProc != nullptr)
                spellProc->setOverrideEffectDamage(EFF_INDEX_0, aurEff->getEffectDamage());
        }
        else
        {
            aur->getOwner()->removeProcTriggerSpell(SPELL_BLOOD_PRESENCE_HEAL, aur->getCasterGuid());
        }

        // Prevent default action
        return SpellScriptExecuteState::EXECUTE_PREVENT;
    }
};
#endif

#if VERSION_STRING == WotLK
class MarkOfBlood : public SpellScript
{
public:
    SpellScriptCheckDummy onAuraDummyEffect(Aura* aur, AuraEffectModifier* /*aurEff*/, bool apply) override
    {
        if (apply)
            aur->getOwner()->addProcTriggerSpell(sSpellMgr.getSpellInfo(SPELL_MARK_OF_BLOOD_HEAL), aur, aur->getCasterGuid());
        else
            aur->getOwner()->removeProcTriggerSpell(SPELL_MARK_OF_BLOOD_HEAL, aur->getCasterGuid());

        return SpellScriptCheckDummy::DUMMY_OK;
    }
};
#endif

#if VERSION_STRING == WotLK
class SpellDeflection : public SpellScript
{
public:
    SpellScriptExecuteState onAuraAbsorb(Aura* aur, AuraEffectModifier* aurEff, uint32_t* absorbed, uint32_t* dmg, bool initialCheck) override
    {
        if (!initialCheck)
            return SpellScriptExecuteState::EXECUTE_PREVENT;

        auto* const plrOwner = aur->getPlayerOwner();
        if (plrOwner == nullptr)
            return SpellScriptExecuteState::EXECUTE_PREVENT;

        // "You have a chance equal to your Parry chance of taking X % less damage"
        if (!Util::checkChance(plrOwner->getParryChance()))
            return SpellScriptExecuteState::EXECUTE_PREVENT;

        *absorbed = *dmg * aurEff->getEffectDamage() / 100;
        *dmg -= *absorbed;
        return SpellScriptExecuteState::EXECUTE_PREVENT;
    }
};
#endif

#if VERSION_STRING < Mop
class UnholyPresence : public Presences
{
public:
    bool isUnholyPresence() const override { return true; }

    void onAuraApply(Aura* aur) override
    {
        Presences::onAuraApply(aur);

#if VERSION_STRING == WotLK
        aur->getOwner()->castSpell(aur->getOwner(), sSpellMgr.getSpellInfo(SPELL_UNHOLY_PRESENCE_MOVE_SPEED), true);
#endif

        // TODO: in cata we are missing the spell for rune regeneration
    }
};
#endif

#if VERSION_STRING == WotLK
class WillOfTheNecropolisAbsorb : public SpellScript
{
public:
    SpellScriptExecuteState beforeAuraEffect(Aura* aur, AuraEffectModifier* aurEff, bool apply) override
    {
        if (!apply || aurEff->getEffectIndex() != EFF_INDEX_0)
            return SpellScriptExecuteState::EXECUTE_OK;

        // Get health threshold only once instead of on every absorb event
        const auto* const dummyInfo = sSpellMgr.getEquivalentSpellRankFor(aur->getSpellInfo(), sSpellMgr.getSpellInfo(SPELL_WILL_OF_THE_NECROPOLIS_DUMMY_R1));
        if (dummyInfo == nullptr)
            return SpellScriptExecuteState::EXECUTE_PREVENT;

        aurEff->setEffectExtraField(dummyInfo->calculateEffectValue(EFF_INDEX_0, aur->getOwner()));
        return SpellScriptExecuteState::EXECUTE_OK;
    }

    SpellScriptExecuteState onAuraAbsorb(Aura* aur, AuraEffectModifier* aurEff, uint32_t* absorbed, uint32_t* dmg, bool initialCheck) override
    {
        if (!initialCheck)
            return SpellScriptExecuteState::EXECUTE_PREVENT;

        const auto* const owner = aur->getOwner();
        // Get new health pct after damage taken and use exact health
        const int32_t tmpHpPct = (owner->getExactCurrentHealth() - *dmg) * 100;
        const int32_t newHealthPct = std::max(0, tmpHpPct) / owner->getMaxHealth();

        // "Damage that would take you below 35% health or taken while you are at 35% health is reduced"
        // However all damage taken under 35% should also be absorbed
        if (newHealthPct > aurEff->getEffectExtraField())
            return SpellScriptExecuteState::EXECUTE_PREVENT;

        *absorbed = *dmg * aurEff->getEffectDamage() / 100;
        *dmg -= *absorbed;
        return SpellScriptExecuteState::EXECUTE_PREVENT;
    }
};

class WillOfTheNecropolisDummy : public SpellScript
{
public:
    SpellScriptCheckDummy onDummyOrScriptedEffect(Spell* /*spell*/, uint8_t /*effIndex*/) override
    {
        // Contains only the health pct value
        return SpellScriptCheckDummy::DUMMY_OK;
    }
};
#else // Cata+
class WillOfTheNecropolisProc : public SpellScript
{
public:
    void onCreateSpellProc(SpellProc* spellProc, Object* /*obj*/) override
    {
        // 45sec cooldown
        spellProc->setProcInterval(45000);
    }

    bool canProc(SpellProc* spellProc, Unit* /*victim*/, SpellInfo const* /*castingSpell*/, DamageInfo damageInfo) override
    {
        if (damageInfo.realDamage == 0)
            return false;

        // Get new health pct after damage taken and use exact health
        const int32_t newHealthPct = (spellProc->getProcOwner()->getExactCurrentHealth() - damageInfo.realDamage) * 100 / spellProc->getProcOwner()->getMaxHealth();
        return newHealthPct <= 30;
    }

    void afterSpellEffect(Spell* spell, uint8_t effIndex, DamageInfo const& /*damageInfo*/) override
    {
        if (effIndex != EFF_INDEX_0)
            return;

        auto* const caster = spell->getPlayerCaster();
        if (caster == nullptr)
            return;

        // Clear cooldown for Rune Tap and cast second proc spell
        caster->clearCooldownForSpell(SPELL_RUNE_TAP);
        caster->castSpell(caster, SPELL_WILL_OF_THE_NECROPOLIS_RUNETAP, true);
    }
};
#endif

void setupDeathKnightSpells(ScriptMgr* mgr)
{
    // Call legacy script setup
    SetupLegacyDeathKnightSpells(mgr);

    mgr->register_spell_script(SPELL_ANTIMAGIC_SHELL, new AntiMagicShell);

#if VERSION_STRING < Mop
    mgr->register_spell_script(SPELL_BLOOD_PRESENCE, new BloodPresence);
#endif
#if VERSION_STRING == WotLK
    mgr->register_spell_script(SPELL_BLOOD_PRESENCE_HEAL, new BloodPresenceHeal);
#endif

#if VERSION_STRING < Mop
    mgr->register_spell_script(SPELL_BUTCHERY_R1, new Butchery);
#endif

    mgr->register_spell_script(SPELL_DEATH_AND_DECAY_R1, new DeathAndDecay);

#if VERSION_STRING == WotLK
    mgr->register_spell_script(SPELL_DEATH_RUNE_MASTERY_R1, new DeathRuneMastery);
#endif

#if VERSION_STRING < Mop
    mgr->register_spell_script(SPELL_FROST_PRESENCE, new FrostPresence);
#endif

#if VERSION_STRING == WotLK
    mgr->register_spell_script(SPELL_IMPROVED_BLOOD_PRESENCE_DUMMY, new ImprovedBloodPresenceDummy);
#endif

#if VERSION_STRING == WotLK
    mgr->register_spell_script(SPELL_MARK_OF_BLOOD_DUMMY, new MarkOfBlood);
#endif

#if VERSION_STRING == WotLK
    mgr->register_spell_script(SPELL_SPELL_DEFLECTION_R1, new SpellDeflection);
#endif

#if VERSION_STRING < Mop
    mgr->register_spell_script(SPELL_UNHOLY_PRESENCE, new UnholyPresence);
#endif

#if VERSION_STRING == WotLK
    mgr->register_spell_script(SPELL_WILL_OF_THE_NECROPOLIS_ABSORB_R1, new WillOfTheNecropolisAbsorb);
    mgr->register_spell_script(SPELL_WILL_OF_THE_NECROPOLIS_DUMMY_R1, new WillOfTheNecropolisDummy);
#else
    mgr->register_spell_script(SPELL_WILL_OF_THE_NECROPOLIS_PROC, new WillOfTheNecropolisProc);
#endif
}
#endif
