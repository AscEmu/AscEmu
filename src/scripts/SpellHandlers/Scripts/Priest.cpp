/*
Copyright (c) 2014-2020 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"

#include "Spell/Definitions/SpellEffectTarget.h"

enum PriestSpells
{
    SPELL_ABOLISH_DISEASE                   = 552,
    SPELL_BODY_AND_SOUL_R1                  = 64127,
    SPELL_BODY_AND_SOUL_R2                  = 64129,
    SPELL_BODY_AND_SOUL_SPEED_R1            = 64128,
    SPELL_BODY_AND_SOUL_SPEED_R2            = 65081,
    SPELL_BODY_AND_SOUL_POISON              = 64134,
    SPELL_BODY_AND_SOUL_POISON_SINGLE       = 64136,
    SPELL_DIVINE_AEGIS_R1                   = 47509,
    SPELL_DIVINE_AEGIS_R2                   = 47511,
    SPELL_DIVINE_AEGIS_R3                   = 47515,
    SPELL_DIVINE_AEGIS                      = 47753,
    SPELL_EMPOWERED_RENEW_R1                = 63534,
    SPELL_EMPOWERED_RENEW_R2                = 63542,
    SPELL_EMPOWERED_RENEW_R3                = 63543,
    SPELL_EMPOWERED_RENEW_HEAL              = 63544,
    SPELL_HOLY_CONCENTRATION_R1             = 34754,
    SPELL_HOLY_CONCENTRATION_R2             = 63724,
    SPELL_HOLY_CONCENTRATION_R3             = 63725,
    SPELL_IMPROVED_DEVOURING_PLAGUE_R1      = 63625,
    SPELL_IMPROVED_DEVOURING_PLAGUE_R2      = 63626,
    SPELL_IMPROVED_DEVOURING_PLAGUE_R3      = 63627,
    SPELL_IMPROVED_DEVOURING_PLAGUE_DMG     = 63675,
    SPELL_IMPROVED_DEVOURING_PLAGUE_HEAL    = 75999,
    SPELL_IMPROVED_MIND_BLAST_R1            = 15273,
    SPELL_IMPROVED_MIND_BLAST_R2            = 15312,
    SPELL_IMPROVED_MIND_BLAST_R3            = 15313,
    SPELL_IMPROVED_MIND_BLAST_R4            = 15314,
    SPELL_IMPROVED_MIND_BLAST_R5            = 15316,
    SPELL_IMPROVED_SPIRIT_TAP_R1            = 49694,
    SPELL_IMPROVED_SPIRIT_TAP_R2            = 59000,
    SPELL_MIND_TRAUMA                       = 48301,
    SPELL_VAMPIRIC_EMBRACE_DUMMY            = 15286,
    SPELL_VAMPIRIC_EMBRACE_HEAL             = 15290,
    SPELL_VAMPIRIC_TOUCH_R1                 = 34914,
    SPELL_VAMPIRIC_TOUCH_R2                 = 34916,
    SPELL_VAMPIRIC_TOUCH_R3                 = 34917,
    SPELL_VAMPIRIC_TOUCH_R4                 = 48159,
    SPELL_VAMPIRIC_TOUCH_R5                 = 48160,
    SPELL_VAMPIRIC_TOUCH_DISPEL             = 64085,
    SPELL_VAMPIRIC_TOUCH_MANA               = 34919,
    SPELL_REPLENISHMENT                     = 57669,

    SPELL_ICON_ABOLISH_DISEASE              = 264,
};

#if VERSION_STRING >= WotLK
#if VERSION_STRING == WotLK
class AbolishDisease : public SpellScript
{
public:
    void onAuraRemove(Aura* aur, AuraRemoveMode /*mode*/) override
    {
        // Remove Body and Soul poison proc
        aur->getOwner()->RemoveAura(SPELL_BODY_AND_SOUL_POISON);
    }
};
#endif

#if VERSION_STRING < Mop
// Handles only the poison dispel proc (speed buff is handled by spellAuraEffectProcTriggerSpell)
class BodyAndSoulDummy : public SpellScript
{
    SpellScriptCheckDummy onAuraDummyEffect(Aura* aur, AuraEffectModifier* aurEff, bool apply) override
    {
        if (!apply)
            return SpellScriptCheckDummy::DUMMY_OK;

        // Dispel poison proc on Abolish Disease in wotlk and on Cure Disease in cata
        uint32_t procMask[3] = { 0, 0x1, 0x400 };
#if VERSION_STRING == WotLK
        uint32_t spellId = SPELL_BODY_AND_SOUL_POISON;
#elif VERSION_STRING == Cata
        uint32_t spellId = SPELL_BODY_AND_SOUL_POISON_SINGLE;
#endif
        auto spellProc = aur->getOwner()->addProcTriggerSpell(sSpellMgr.getSpellInfo(spellId), aur->getSpellInfo(), aur->getCasterGuid(), 0, procMask);
        if (spellProc != nullptr)
        {
            const auto procChance = aurEff->mDamage;
            spellProc->setProcChance(procChance);
        }
        return SpellScriptCheckDummy::DUMMY_OK;
    }

    void onAuraRemove(Aura* aur, AuraRemoveMode /*mode*/) override
    {
#if VERSION_STRING == Cata
        aur->getOwner()->removeProcTriggerSpell(SPELL_BODY_AND_SOUL_POISON_SINGLE, aur->getCasterGuid());
#elif VERSION_STRING == WotLK
        aur->getOwner()->removeProcTriggerSpell(SPELL_BODY_AND_SOUL_POISON, aur->getCasterGuid());
#endif
    }
};

#if VERSION_STRING == WotLK
class BodyAndSoulPoison : public SpellScript
{
    bool canProc(SpellProc* /*spellProc*/, Unit* /*victim*/, SpellInfo const* castingSpell, DamageInfo /*damageInfo*/) override
    {
        // Cure Disease and Abolish Disease contain same spell mask
        // Check that the spell is Abolish Disease
        if (castingSpell == nullptr || castingSpell->getSpellIconID() != SPELL_ICON_ABOLISH_DISEASE)
            return false;

        return true;
    }
};
#endif
#endif

class BodyAndSoulSpeed : public SpellScript
{
public:
    void onCreateSpellProc(SpellProc* spellProc, Object* /*obj*/) override
    {
        // Should proc only on Power Word: Shield
        spellProc->setProcClassMask(0, 0x1);
        spellProc->setProcClassMask(1, 0);
        spellProc->setProcClassMask(2, 0x400);
    }
};

class DivineAegisDummy : public SpellScript
{
public:
    SpellScriptCheckDummy onAuraDummyEffect(Aura* aur, AuraEffectModifier* aurEff, bool apply) override
    {
        if (!apply)
            return SpellScriptCheckDummy::DUMMY_OK;

        auto spellProc = aur->getOwner()->addProcTriggerSpell(sSpellMgr.getSpellInfo(SPELL_DIVINE_AEGIS), aur->getSpellInfo(), aur->getCasterGuid(), 0);
        if (spellProc != nullptr)
        {
            spellProc->setExtraProcFlags(EXTRA_PROC_ON_CRIT_ONLY);
            spellProc->setOverrideEffectDamage(EFF_INDEX_2, aurEff->mDamage);
        }

        return SpellScriptCheckDummy::DUMMY_OK;
    }

    void onAuraRemove(Aura* aur, AuraRemoveMode /*mode*/) override
    {
        aur->getOwner()->removeProcTriggerSpell(SPELL_DIVINE_AEGIS, aur->getCasterGuid());
    }
};

class DivineAegis : public SpellScript
{
public:
    bool canProc(SpellProc* /*spellProc*/, Unit* /*victim*/, SpellInfo const* castingSpell, DamageInfo /*damageInfo*/) override
    {
        // Should proc only on heal spells
        if (castingSpell == nullptr || !castingSpell->hasEffect(SPELL_EFFECT_HEAL))
            return false;
        return true;
    }

    SpellScriptExecuteState onDoProcEffect(SpellProc* spellProc, Unit* victim, SpellInfo const* /*castingSpell*/, DamageInfo damageInfo) override
    {
        if (victim == nullptr)
            return SpellScriptExecuteState::EXECUTE_PREVENT;

        auto absorb = static_cast<uint32_t>(std::ceil(damageInfo.realDamage * spellProc->getOverrideEffectDamage(EFF_INDEX_2) / 100.0f));
        // Patch 3.1.0: Divine Aegis effects will now stack, however the amount absorbed cannot exceed 125 * level(of the target).
        if (absorb > (victim->getLevel() * 125))
            absorb = victim->getLevel() * 125;

        spellProc->setOverrideEffectDamage(EFF_INDEX_0, absorb);
        return SpellScriptExecuteState::EXECUTE_OK;
    }
};

#if VERSION_STRING == WotLK
class EmpoweredRenewDummy : public SpellScript
{
public:
    SpellScriptCheckDummy onAuraDummyEffect(Aura* /*aur*/, AuraEffectModifier* /*aurEff*/, bool /*apply*/) override
    {
        // TODO: heal amount
        return SpellScriptCheckDummy::DUMMY_OK;
    }
};
#endif
#endif

#if VERSION_STRING >= TBC
#if VERSION_STRING < Cata
class HolyConcentration : public SpellScript
{
public:
    void onCreateSpellProc(SpellProc* spellProc, Object* /*obj*/) override
    {
        // Should proc on critical Flash Heal, Greater Heal or Binding Heal
        spellProc->setProcClassMask(EFF_INDEX_0, 0x1800);
        spellProc->setProcClassMask(EFF_INDEX_1, 0x4);
#if VERSION_STRING == WotLK
        // or on critical Empowered Renew
        spellProc->setProcClassMask(EFF_INDEX_2, 0x1000);
#endif
        spellProc->setExtraProcFlags(EXTRA_PROC_ON_CRIT_ONLY);
    }

    bool canProcOnTriggered(SpellProc* /*spellProc*/, Unit* /*victim*/, SpellInfo const* /*castingSpell*/, Aura* /*triggeredFromAura*/) override
    {
        // Empowered Renew is a triggered spell
        return true;
    }
};
#endif
#endif

#if VERSION_STRING >= WotLK
#if VERSION_STRING < Mop
class ImprovedDevouringPlagueDummy : public SpellScript
{
public:
    SpellScriptCheckDummy onAuraDummyEffect(Aura* aur, AuraEffectModifier* /*aurEff*/, bool apply) override
    {
        if (!apply)
            return SpellScriptCheckDummy::DUMMY_OK;

        // Cast on devouring plague
        // TODO: missing damage calculations and the heal spell
        uint32_t procMask[3] = { 0x2000000, 0x1000, 0x400 };
        aur->getOwner()->addProcTriggerSpell(sSpellMgr.getSpellInfo(SPELL_IMPROVED_DEVOURING_PLAGUE_DMG), aur->getSpellInfo(), aur->getCasterGuid(), 100, PROC_ON_DONE_NEGATIVE_SPELL_DAMAGE_CLASS_MAGIC, EXTRA_PROC_NULL, 0, 0, procMask);
        return SpellScriptCheckDummy::DUMMY_OK;
    }

    void onAuraRemove(Aura* aur, AuraRemoveMode /*mode*/) override
    {
        aur->getOwner()->removeProcTriggerSpell(SPELL_IMPROVED_DEVOURING_PLAGUE_DMG, aur->getCasterGuid());
    }
};

class ImprovedMindBlastDummy : public SpellScript
{
public:
    void onAuraApply(Aura* aur) override
    {
        const auto procChance = aur->getSpellInfo()->getEffectBasePoints(EFF_INDEX_1) + 1;
        const uint32_t procMask[3] = { 0x2000, 0, 0 };
        aur->getOwner()->addProcTriggerSpell(sSpellMgr.getSpellInfo(SPELL_MIND_TRAUMA), aur->getSpellInfo(), aur->getCasterGuid(), procChance, PROC_ON_DONE_NEGATIVE_SPELL_DAMAGE_CLASS_MAGIC, 0, 0, procMask);
    }

    void onAuraRemove(Aura* aur, AuraRemoveMode /*mode*/) override
    {
        aur->getOwner()->removeProcTriggerSpell(SPELL_MIND_TRAUMA, aur->getCasterGuid());
    }
};

class MindTrauma : public SpellScript
{
public:
    bool canProc(SpellProc* proc, Unit* /*victim*/, SpellInfo const* /*castingSpell*/, DamageInfo /*damageInfo*/) override
    {
        // Requires Shadowform
        if (proc->getProcOwner()->getShapeShiftForm() != FORM_SHADOW)
            return false;

        return true;
    }
};
#endif

#if VERSION_STRING == WotLK
class ImprovedSpiritTap : public SpellScript
{
public:
    void onCreateSpellProc(SpellProc* spellProc, Object* /*obj*/) override
    {
        // Should proc on Mind Blast, Shadow Word: Death and Mind Flay critical hits
        spellProc->setProcClassMask(EFF_INDEX_0, 0x802000);
        spellProc->setProcClassMask(EFF_INDEX_1, 0x2);
        spellProc->setProcClassMask(EFF_INDEX_2, 0);
        spellProc->setExtraProcFlags(EXTRA_PROC_ON_CRIT_ONLY);
    }

    bool canProcOnTriggered(SpellProc* /*spellProc*/, Unit* /*victim*/, SpellInfo const* /*castingSpell*/, Aura* /*triggeredFromAura*/) override
    {
        // Mind Flay "ticks" are triggered from the original spell, so this must proc on triggered spells
        return true;
    }

    uint32_t calcProcChance(SpellProc* spellProc, Unit* /*victim*/, SpellInfo const* castingSpell) override
    {
        if (castingSpell == nullptr)
            return 0;

        // Mind Flay critical hits have only 50% chance to proc
        if (castingSpell->getSpellFamilyFlags(EFF_INDEX_0) == 0x800000)
            return 50;

        // Mind Blast and Shadow Word: Death have 100% chance
        return spellProc->getProcChance();
    }
};
#endif
#endif

class VampiricEmbraceDummy : public SpellScript
{
public:
    SpellScriptCheckDummy onAuraDummyEffect(Aura* aur, AuraEffectModifier* aurEff, bool apply) override
    {
        if (!apply)
            return SpellScriptCheckDummy::DUMMY_OK;

#if VERSION_STRING >= Mop
        // TODO: Mop
#else
        auto spellProc = aur->getOwner()->addProcTriggerSpell(sSpellMgr.getSpellInfo(SPELL_VAMPIRIC_EMBRACE_HEAL), aur->getSpellInfo(), aur->getCasterGuid(), 0);
        if (spellProc != nullptr)
            spellProc->setOverrideEffectDamage(EFF_INDEX_2, aurEff->mDamage);
#endif
        return SpellScriptCheckDummy::DUMMY_OK;
    }

    void onAuraRemove(Aura* aur, AuraRemoveMode /*mode*/) override
    {
        aur->getOwner()->removeProcTriggerSpell(SPELL_VAMPIRIC_EMBRACE_HEAL, aur->getCasterGuid());
    }
};

class VampiricEmbraceHeal : public SpellScript
{
public:
    void onCreateSpellProc(SpellProc* spellProc, Object* /*obj*/) override
    {
#if VERSION_STRING < WotLK
        spellProc->setCastedByProcCreator(true);
#else
        spellProc->setCastedOnProcOwner(true);
#endif
    }

    bool canProc([[maybe_unused]]SpellProc* spellProc, [[maybe_unused]]Unit* victim, SpellInfo const* castingSpell, DamageInfo damageInfo) override
    {
        if (castingSpell == nullptr)
            return false;

#if VERSION_STRING < WotLK
        // Should proc only for the priest who casted Vampiric Embrace
        if (victim == nullptr || victim->getGuid() != spellProc->getCasterGuid())
            return false;
#endif

        // Only on shadow damage
        // Check also that it's not procing from wand
        if (!(damageInfo.schoolMask & SCHOOL_MASK_SHADOW) || damageInfo.isHeal || damageInfo.weaponType == RANGED)
            return false;

        // Only on single target spells
        //\ todo: not sure on this
        if (!(castingSpell->hasTargetType(EFF_TARGET_SINGLE_ENEMY) || castingSpell->hasTargetType(EFF_TARGET_SELECTED_ENEMY_CHANNELED)))
            return false;

        // Check spell's effects
        for (uint8_t i = 0; i < MAX_SPELL_EFFECTS; ++i)
        {
            if (castingSpell->getEffect(i) == SPELL_EFFECT_NULL)
                continue;

            // Procs on normal shadow damage
            if (castingSpell->getEffect(i) == SPELL_EFFECT_SCHOOL_DAMAGE)
                return true;

            // Procs on periodic shadow damage
            if (damageInfo.isPeriodic &&
               (castingSpell->getEffectApplyAuraName(i) == SPELL_AURA_PERIODIC_DAMAGE || castingSpell->getEffectApplyAuraName(i) == SPELL_AURA_PERIODIC_LEECH))
                return true;
        }

        return false;
    }

    SpellScriptExecuteState onDoProcEffect(SpellProc* spellProc, Unit* /*victim*/, SpellInfo const* /*castingSpell*/, DamageInfo damageInfo) override
    {
        const auto healPct = spellProc->getOverrideEffectDamage(EFF_INDEX_2);
        const auto heal = static_cast<uint32_t>(std::ceil(damageInfo.realDamage * healPct / 100.0f));

#if VERSION_STRING >= Mop
        // TODO: Mop
#else
#if VERSION_STRING >= WotLK
#if VERSION_STRING == Cata
        const auto partyHealPct = std::round(healPct / 2.0f);
#elif VERSION_STRING == WotLK
        const auto partyHealPct = std::round(healPct / 5.0f);
#endif
        const auto partyHeal = static_cast<uint32_t>(std::ceil(damageInfo.realDamage * partyHealPct / 100.0f));

        spellProc->setOverrideEffectDamage(EFF_INDEX_0, partyHeal);
        spellProc->setOverrideEffectDamage(EFF_INDEX_1, heal);
#else
        // Same amount for party and self in Classic and TBC
        spellProc->setOverrideEffectDamage(EFF_INDEX_0, heal);
#endif
#endif
        if (heal == 0)
            return SpellScriptExecuteState::EXECUTE_PREVENT;

        return SpellScriptExecuteState::EXECUTE_OK;
    }

#if VERSION_STRING >= WotLK
#if VERSION_STRING < Mop
    void filterEffectTargets(Spell* spell, uint8_t effIndex, std::vector<uint64_t>* effTargets) override
    {
        if (effIndex != EFF_INDEX_0)
            return;

        // Remove caster from party effect
        effTargets->erase(std::remove(effTargets->begin(), effTargets->end(), spell->getCaster()->getGuid()), effTargets->end());
    }
#endif
#endif
};

#if VERSION_STRING >= TBC
class VampiricTouch : public SpellScript
{
public:
    SpellScriptCheckDummy onAuraDummyEffect(Aura* aur, AuraEffectModifier* aurEff, bool apply) override
    {
        if (!apply || aurEff->effIndex != EFF_INDEX_0)
            return SpellScriptCheckDummy::DUMMY_OK;

#if VERSION_STRING == TBC
        auto spellProc = aur->getOwner()->addProcTriggerSpell(sSpellMgr.getSpellInfo(SPELL_VAMPIRIC_TOUCH_MANA), aur->getSpellInfo(), aur->getCasterGuid(), 0);
        if (spellProc != nullptr)
            spellProc->setOverrideEffectDamage(EFF_INDEX_2, aurEff->mDamage);
#elif VERSION_STRING == Mop
        // Should proc only when this aura deals damage
        auto procMask = aur->getSpellInfo()->getSpellFamilyFlags();
        aur->getOwner()->addProcTriggerSpell(sSpellMgr.getSpellInfo(SPELL_VAMPIRIC_TOUCH_MANA), aur->getSpellInfo(), aur->getCasterGuid(), 0, procMask);
#else
        // Should proc only on Mind Blast
        uint32_t procMask[3] = { 0x2000, 0, 0 };
        aur->getOwner()->addProcTriggerSpell(sSpellMgr.getSpellInfo(SPELL_VAMPIRIC_TOUCH_MANA), aur->getSpellInfo(), aur->getCasterGuid(), 0, procMask);
#endif
        return SpellScriptCheckDummy::DUMMY_OK;
    }

    void onAuraRemove(Aura* aur, [[maybe_unused]]AuraRemoveMode mode) override
    {
        aur->getOwner()->removeProcTriggerSpell(SPELL_VAMPIRIC_TOUCH_MANA, aur->getCasterGuid());

#if VERSION_STRING == WotLK
        if (mode != AURA_REMOVE_ON_DISPEL)
            return;

        // Create backfire damage on dispel
        const auto caster = aur->GetUnitCaster();
        const auto backfireDmg = aur->getAuraEffect(EFF_INDEX_1).mDamage * 8;
        if (caster != nullptr)
            caster->castSpell(aur->getOwner(), SPELL_VAMPIRIC_TOUCH_DISPEL, backfireDmg, true);
#endif
    }
};

class VampiricTouchMana : public SpellScript
{
public:
    void onCreateSpellProc(SpellProc* spellProc, Object* /*obj*/) override
    {
        spellProc->setCastedByProcCreator(true);
    }

    bool canProc(SpellProc* spellProc, Unit* victim, SpellInfo const* /*castingSpell*/, DamageInfo damageInfo) override
    {
        // Should proc only for the priest who casted Vampiric Touch
        if (victim == nullptr || victim->getGuid() != spellProc->getCasterGuid())
            return false;

        // Only on shadow damage
        // Check also that it's not procing from wand
        if (!(damageInfo.schoolMask & SCHOOL_MASK_SHADOW) || damageInfo.weaponType == RANGED)
            return false;

        return true;
    }

    SpellScriptExecuteState onDoProcEffect(SpellProc* spellProc, Unit* /*victim*/, SpellInfo const* /*castingSpell*/, [[maybe_unused]]DamageInfo damageInfo) override
    {
#if VERSION_STRING == TBC
        auto mana = static_cast<uint32_t>(std::ceil(damageInfo.realDamage * spellProc->getOverrideEffectDamage(EFF_INDEX_2) / 100.0f));
        spellProc->setOverrideEffectDamage(EFF_INDEX_0, mana);
        return SpellScriptExecuteState::EXECUTE_OK;
#elif VERSION_STRING < Mop
        const auto caster = spellProc->getProcOwner()->GetMapMgrUnit(spellProc->getCasterGuid());
        if (caster == nullptr)
            return SpellScriptExecuteState::EXECUTE_PREVENT;

        caster->castSpell(caster, SPELL_REPLENISHMENT, true);
        return SpellScriptExecuteState::EXECUTE_PREVENT;
#else
        return SpellScriptExecuteState::EXECUTE_OK;
#endif
    }
};
#endif

void setupPriestSpells(ScriptMgr* mgr)
{
    // Call legacy script setup
    SetupLegacyPriestSpells(mgr);

#if VERSION_STRING >= WotLK
#if VERSION_STRING < Mop
    uint32_t bodyAndSoulIds[] =
    {
        SPELL_BODY_AND_SOUL_R1,
        SPELL_BODY_AND_SOUL_R2,
        0
    };
    mgr->register_spell_script(bodyAndSoulIds, new BodyAndSoulDummy);
#endif
#if VERSION_STRING == WotLK
    mgr->register_spell_script(SPELL_ABOLISH_DISEASE, new AbolishDisease);
    mgr->register_spell_script(SPELL_BODY_AND_SOUL_POISON, new BodyAndSoulPoison);
#endif
    uint32_t bodyAndSoulSpeedIds[] =
    {
        SPELL_BODY_AND_SOUL_SPEED_R1,
        SPELL_BODY_AND_SOUL_SPEED_R2,
        0
    };
    mgr->register_spell_script(bodyAndSoulSpeedIds, new BodyAndSoulSpeed);

    uint32_t divineAegisIds[] =
    {
        SPELL_DIVINE_AEGIS_R1,
        SPELL_DIVINE_AEGIS_R2,
        SPELL_DIVINE_AEGIS_R3,
        0
    };
    mgr->register_spell_script(divineAegisIds, new DivineAegisDummy);
    mgr->register_spell_script(SPELL_DIVINE_AEGIS, new DivineAegis);

#if VERSION_STRING == WotLK
    uint32_t empoweredRenewIds[] =
    {
        SPELL_EMPOWERED_RENEW_R1,
        SPELL_EMPOWERED_RENEW_R2,
        SPELL_EMPOWERED_RENEW_R3,
        0
    };
    mgr->register_spell_script(empoweredRenewIds, new EmpoweredRenewDummy);
#endif

#if VERSION_STRING >= TBC
#if VERSION_STRING < Cata
    uint32_t holyConcentrationIds[] =
    {
        SPELL_HOLY_CONCENTRATION_R1,
#if VERSION_STRING == WotLK
        SPELL_HOLY_CONCENTRATION_R2,
        SPELL_HOLY_CONCENTRATION_R3,
#endif
        0
    };
    mgr->register_spell_script(holyConcentrationIds, new HolyConcentration);
#endif
#endif

#if VERSION_STRING < Mop
    uint32_t improvedDevouringPlagueIds[] =
    {
        SPELL_IMPROVED_DEVOURING_PLAGUE_R1,
        SPELL_IMPROVED_DEVOURING_PLAGUE_R2,
        SPELL_IMPROVED_DEVOURING_PLAGUE_R3,
        0
    };
    mgr->register_spell_script(improvedDevouringPlagueIds, new ImprovedDevouringPlagueDummy);

    uint32_t improvedMindBlastIds[] =
    {
        SPELL_IMPROVED_MIND_BLAST_R1,
        SPELL_IMPROVED_MIND_BLAST_R2,
        SPELL_IMPROVED_MIND_BLAST_R3,
        SPELL_IMPROVED_MIND_BLAST_R4,
        SPELL_IMPROVED_MIND_BLAST_R5,
        0
    };
    mgr->register_spell_script(improvedMindBlastIds, new ImprovedMindBlastDummy);
    mgr->register_spell_script(SPELL_MIND_TRAUMA, new MindTrauma);
#endif

#if VERSION_STRING == WotLK
    uint32_t improvedSpiritTapIds[] =
    {
        SPELL_IMPROVED_SPIRIT_TAP_R1,
        SPELL_IMPROVED_SPIRIT_TAP_R2,
        0
    };
    mgr->register_spell_script(improvedSpiritTapIds, new ImprovedSpiritTap);
#endif
#endif

    mgr->register_spell_script(SPELL_VAMPIRIC_EMBRACE_DUMMY, new VampiricEmbraceDummy);
    mgr->register_spell_script(SPELL_VAMPIRIC_EMBRACE_HEAL, new VampiricEmbraceHeal);

#if VERSION_STRING >= TBC
    uint32_t vampiricTouchIds[] =
    {
        SPELL_VAMPIRIC_TOUCH_R1,
        SPELL_VAMPIRIC_TOUCH_R2,
        SPELL_VAMPIRIC_TOUCH_R3,
        SPELL_VAMPIRIC_TOUCH_R4,
        SPELL_VAMPIRIC_TOUCH_R5,
        0
    };
    mgr->register_spell_script(vampiricTouchIds, new VampiricTouch);
    mgr->register_spell_script(SPELL_VAMPIRIC_TOUCH_MANA, new VampiricTouchMana);
#endif
}
