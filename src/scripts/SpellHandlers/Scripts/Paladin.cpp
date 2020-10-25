/*
Copyright (c) 2014-2020 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"

enum PaladinSpells
{
    SPELL_BLOOD_CORRUPTION              = 53742,
    SPELL_EYE_FOR_AN_EYE_DAMAGE         = 25997,
    SPELL_EYE_FOR_AN_EYE_DUMMY_R1       = 9799,
    SPELL_EYE_FOR_AN_EYE_DUMMY_R2       = 25988,
    SPELL_HOLY_VENGEANCE                = 31803,
    SPELL_JUDGEMENT_OF_LIGHT_DEBUFF     = 20185,
    SPELL_JUDGEMENT_OF_LIGHT_HEAL       = 20267,
    SPELL_JUDGEMENT_OF_WISDOM_DEBUFF    = 20186,
    SPELL_JUDGEMENT_OF_WISDOM_MANA      = 20268,
    SPELL_SEAL_OF_CORRUPTION_DIRECT     = 53739,
    SPELL_SEAL_OF_CORRUPTION_DUMMY      = 53736,
    SPELL_SEAL_OF_RIGHTEOUSNESS         = 25742,
    SPELL_SEAL_OF_RIGHTOUESNESS_DUMMY   = 21084,
    SPELL_SEAL_OF_VENGEANCE_DIRECT      = 42463,
    SPELL_SEAL_OF_VENGEANCE_DUMMY       = 31801,
};

class EyeForAnEyeDummy : public SpellScript
{
public:
    SpellScriptCheckDummy onAuraDummyEffect(Aura* aur, AuraEffectModifier* aurEff, bool apply) override
    {
        if (!apply)
            return SpellScriptCheckDummy::DUMMY_OK;

        auto spellProc = aur->getOwner()->addProcTriggerSpell(sSpellMgr.getSpellInfo(SPELL_EYE_FOR_AN_EYE_DAMAGE), aur->getSpellInfo(), aur->getCasterGuid(), 0);
        if (spellProc != nullptr)
        {
            spellProc->setExtraProcFlags(EXTRA_PROC_ON_CRIT_ONLY);
            spellProc->setOverrideEffectDamage(EFF_INDEX_0, aurEff->mDamage);
        }

        return SpellScriptCheckDummy::DUMMY_OK;
    }

    void onAuraRemove(Aura* aur, AuraRemoveMode /*mode*/) override
    {
        aur->getOwner()->removeProcTriggerSpell(SPELL_EYE_FOR_AN_EYE_DAMAGE, aur->getCasterGuid());
    }
};

class EyeForAnEye : public SpellScript
{
public:
    SpellScriptExecuteState onDoProcEffect(SpellProc* spellProc, Unit* victim, SpellInfo const* /*castingSpell*/, DamageInfo damageInfo) override
    {
        const auto dmg = damageInfo.realDamage * spellProc->getOverrideEffectDamage(EFF_INDEX_0) / 100;
        spellProc->getProcOwner()->castSpell(victim, spellProc->getSpell(), dmg, true);
        return SpellScriptExecuteState::EXECUTE_PREVENT;
    }

    SpellScriptEffectDamage doCalculateEffect(Spell* spell, uint8_t /*effIndex*/, int32_t* damage) override
    {
        if (spell->getUnitCaster() == nullptr)
            return SpellScriptEffectDamage::DAMAGE_DEFAULT;

        // Damage cannot exceed 50% of paladin's health
        const int32_t maxDmg = spell->getUnitCaster()->getMaxHealth() / 2;
        if (*damage > maxDmg)
            *damage = maxDmg;

        return SpellScriptEffectDamage::DAMAGE_FULL_RECALCULATION;
    }
};

class JudgementOfLightDebuff : public SpellScript
{
public:
    // todo: TBC and classic
#if VERSION_STRING == WotLK
    SpellScriptCheckDummy onAuraDummyEffect(Aura* aur, AuraEffectModifier* aurEff, bool apply) override
    {
        if (!apply)
            return SpellScriptCheckDummy::DUMMY_OK;

        auto spellProc = aur->getOwner()->addProcTriggerSpell(sSpellMgr.getSpellInfo(SPELL_JUDGEMENT_OF_LIGHT_HEAL), aur->getSpellInfo(), aur->getCasterGuid(), 0);
        if (spellProc != nullptr)
            spellProc->setOverrideEffectDamage(EFF_INDEX_0, aurEff->mDamage);

        return SpellScriptCheckDummy::DUMMY_OK;
    }
#endif

    void onAuraRemove(Aura* aur, AuraRemoveMode /*mode*/) override
    {
        aur->getOwner()->removeProcTriggerSpell(SPELL_JUDGEMENT_OF_LIGHT_HEAL, aur->getCasterGuid());
    }
};

class JudgementOfLightHeal : public SpellScript
{
public:
    void onCreateSpellProc(SpellProc* spellProc, Object* /*obj*/) override
    {
        spellProc->setCastedByProcCreator(true);
    }

    uint32_t calcProcChance(SpellProc* /*spellProc*/, Unit* /*victim*/, SpellInfo const* /*castingSpell*/) override
    {
        // TODO: this is a hackfix but almost the correct value
        // should have 15 PPM, but (atm) PPM system works only with melee attacks, and this should proc on spell attacks as well
        // also, if PPM is set, then it calculates proc chance according to aura owner's attack speed, not to attacker's attack speed
        return 50;
    }

#if VERSION_STRING == WotLK
    SpellScriptEffectDamage doCalculateEffect(Spell* spell, uint8_t /*effIndex*/, int32_t* damage) override
    {
        if (spell->GetUnitTarget() == nullptr)
            return SpellScriptEffectDamage::DAMAGE_DEFAULT;

        *damage = spell->GetUnitTarget()->getMaxHealth() * (*damage) / 100;
        return SpellScriptEffectDamage::DAMAGE_FULL_RECALCULATION;
    }
#endif
};

class JudgementOfWisdomDebuff : public SpellScript
{
public:
    // todo: TBC and classic
#if VERSION_STRING == WotLK
    SpellScriptCheckDummy onAuraDummyEffect(Aura* aur, AuraEffectModifier* /*aurEff*/, bool apply) override
    {
        if (!apply)
            return SpellScriptCheckDummy::DUMMY_OK;

        aur->getOwner()->addProcTriggerSpell(sSpellMgr.getSpellInfo(SPELL_JUDGEMENT_OF_WISDOM_MANA), aur->getSpellInfo(), aur->getCasterGuid(), 0);
        return SpellScriptCheckDummy::DUMMY_OK;
    }
#endif

    void onAuraRemove(Aura* aur, AuraRemoveMode /*mode*/) override
    {
        aur->getOwner()->removeProcTriggerSpell(SPELL_JUDGEMENT_OF_WISDOM_MANA, aur->getCasterGuid());
    }
};

class JudgementOfWisdomMana : public SpellScript
{
public:
    void onCreateSpellProc(SpellProc* spellProc, Object* /*obj*/) override
    {
        spellProc->setCastedByProcCreator(true);
    }

    bool canProc(SpellProc* /*spellProc*/, Unit* victim, SpellInfo const* /*castingSpell*/, DamageInfo /*dmg*/) override
    {
        if (victim == nullptr || victim->getPowerType() != POWER_TYPE_MANA)
            return false;

        return true;
    }

    uint32_t calcProcChance(SpellProc* /*spellProc*/, Unit* /*victim*/, SpellInfo const* /*castingSpell*/) override
    {
        // TODO: this is a hackfix but almost the correct value
        // should have 15 PPM, but PPM system works only with melee attacks, and this should proc on spell attacks as well
        // also, if PPM is set, then it calculates proc chance according to aura owner's attack speed, not to attacker's attack speed
        return 50;
    }

#if VERSION_STRING == WotLK
    SpellScriptEffectDamage doCalculateEffect(Spell* spell, uint8_t /*effIndex*/, int32_t* damage) override
    {
        if (spell->GetUnitTarget() == nullptr)
            return SpellScriptEffectDamage::DAMAGE_DEFAULT;

        *damage = spell->GetUnitTarget()->getBaseMana() * (*damage) / 100;
        return SpellScriptEffectDamage::DAMAGE_FULL_RECALCULATION;
    }
#endif
};

#if VERSION_STRING == WotLK
class SealOfCorruptionDummy : public SpellScript
{
public:
    SpellScriptCheckDummy onAuraDummyEffect(Aura* aur, AuraEffectModifier* aurEff, bool apply) override
    {
        if (aurEff->effIndex != EFF_INDEX_0 || !apply)
            return SpellScriptCheckDummy::DUMMY_OK;

        aur->getOwner()->addProcTriggerSpell(sSpellMgr.getSpellInfo(SPELL_BLOOD_CORRUPTION), aur->getSpellInfo(), aur->getCasterGuid(), 0);
        aur->getOwner()->addProcTriggerSpell(sSpellMgr.getSpellInfo(SPELL_SEAL_OF_CORRUPTION_DIRECT), aur->getSpellInfo(), aur->getCasterGuid(), 0);

        return SpellScriptCheckDummy::DUMMY_OK;
    }

    void onAuraRemove(Aura* aur, AuraRemoveMode /*mode*/) override
    {
        aur->getOwner()->removeProcTriggerSpell(SPELL_BLOOD_CORRUPTION, aur->getCasterGuid());
        aur->getOwner()->removeProcTriggerSpell(SPELL_SEAL_OF_CORRUPTION_DIRECT, aur->getCasterGuid());
    }
};
#endif

class SealOfRighteousnessDummy : public SpellScript
{
public:
    SpellScriptCheckDummy onAuraDummyEffect(Aura* aur, AuraEffectModifier* aurEff, bool apply) override
    {
        if (aurEff->effIndex != EFF_INDEX_0 || !apply)
            return SpellScriptCheckDummy::DUMMY_OK;

        aur->getOwner()->addProcTriggerSpell(sSpellMgr.getSpellInfo(SPELL_SEAL_OF_RIGHTEOUSNESS), aur->getSpellInfo(), aur->getCasterGuid(), 0);
        return SpellScriptCheckDummy::DUMMY_OK;
    }

    void onAuraRemove(Aura* aur, AuraRemoveMode /*mode*/) override
    {
        aur->getOwner()->removeProcTriggerSpell(SPELL_SEAL_OF_RIGHTEOUSNESS, aur->getCasterGuid());
    }
};

#if VERSION_STRING >= TBC
// Spell id is replaced by Seal of Truth in cata, but it functions exactly like Seal of Vengeance
class SealOfVengeanceAndCorruptionDirect : public SpellScript
{
public:
    bool canProc(SpellProc* spellProc, Unit* victim, SpellInfo const* /*castingSpell*/, DamageInfo /*damageInfo*/) override
    {
        if (victim == nullptr)
            return false;

        uint32_t auraId = SPELL_HOLY_VENGEANCE;
        if (spellProc->getSpell()->getId() == SPELL_SEAL_OF_CORRUPTION_DIRECT)
            auraId = SPELL_BLOOD_CORRUPTION;

        const auto aur = victim->getAuraWithId(auraId);
        if (aur == nullptr)
            return false;

#if VERSION_STRING == TBC
        // As of Patch 2.2.0, Seal of Vengeance deals a small amount of Holy Damage if it strikes a target with 5 stacks of the debuff already applied.
        if (aur->getStackCount() < 5)
            return false;
#elif VERSION_STRING >= Cata
        // Once Censure is stacked to 5 times, each of the Paladin's attacks also deals 15% weapon damage
        if (aur->getStackCount() < 5)
            return false;
#endif

        return true;
    }

#if VERSION_STRING == WotLK
    SpellScriptEffectDamage doCalculateEffect(Spell* spell, uint8_t /*effIndex*/, int32_t* damage) override
    {
        // According to WoWhead from 3.2.0 patch, all auto attacks and special attacks can proc this
        // Weapon damage starts at 6.6% and goes up to 33% if target has five stacks of debuff
        if (spell->GetUnitTarget() == nullptr)
            return SpellScriptEffectDamage::DAMAGE_DEFAULT;

        uint32_t auraId = SPELL_HOLY_VENGEANCE;
        if (spell->getSpellInfo()->getId() == SPELL_SEAL_OF_CORRUPTION_DIRECT)
            auraId = SPELL_BLOOD_CORRUPTION;

        float_t dmgPercent = 6.6f;
        const auto aur = spell->GetUnitTarget()->getAuraWithId(auraId);
        if (aur != nullptr)
            dmgPercent *= aur->getStackCount();

        *damage = static_cast<uint32_t>(std::round(dmgPercent));
        return SpellScriptEffectDamage::DAMAGE_FULL_RECALCULATION;
    }
#endif
};
#endif

#if VERSION_STRING >= TBC
// Spell id is replaced by Censure in cata, but it functions exactly like Holy Vengeance
class SealOfVengeanceAndCorruptionDot : public SpellScript
{
public:
    bool canProc(SpellProc* /*spellProc*/, Unit* /*victim*/, SpellInfo const* castingSpell, DamageInfo /*damageInfo*/) override
    {
        // Dot portion should only be applied on melee hits
        if (castingSpell == nullptr)
            return true;

#if VERSION_STRING == WotLK
        // and on Hammer of the Righteous
        if (castingSpell->getSpellFamilyFlags(EFF_INDEX_0) == 0 &&
            castingSpell->getSpellFamilyFlags(EFF_INDEX_1) == 0x40000 &&
            castingSpell->getSpellFamilyFlags(EFF_INDEX_2) == 0)
            return true;
#endif

        return false;
    }
};
#endif

#if VERSION_STRING >= TBC
// Spell id is replaced by Seal of Truth in cata, but it functions exactly like Seal of Vengeance
class SealOfVengeanceDummy : public SpellScript
{
public:
    SpellScriptCheckDummy onAuraDummyEffect(Aura* aur, AuraEffectModifier* aurEff, bool apply) override
    {
        if (aurEff->effIndex != EFF_INDEX_0 || !apply)
            return SpellScriptCheckDummy::DUMMY_OK;

#if VERSION_STRING == TBC
        // 20 procs per minute in TBC
        auto dotProc = aur->getOwner()->addProcTriggerSpell(sSpellMgr.getSpellInfo(SPELL_HOLY_VENGEANCE), aur->getSpellInfo(), aur->getCasterGuid(), 0);
        if (dotProc != nullptr)
            dotProc->setProcsPerMinute(20);
#else
        aur->getOwner()->addProcTriggerSpell(sSpellMgr.getSpellInfo(SPELL_HOLY_VENGEANCE), aur->getSpellInfo(), aur->getCasterGuid(), 0);
#endif

        aur->getOwner()->addProcTriggerSpell(sSpellMgr.getSpellInfo(SPELL_SEAL_OF_VENGEANCE_DIRECT), aur->getSpellInfo(), aur->getCasterGuid(), 0);
        return SpellScriptCheckDummy::DUMMY_OK;
    }

    void onAuraRemove(Aura* aur, AuraRemoveMode /*mode*/) override
    {
        aur->getOwner()->removeProcTriggerSpell(SPELL_HOLY_VENGEANCE, aur->getCasterGuid());
        aur->getOwner()->removeProcTriggerSpell(SPELL_SEAL_OF_VENGEANCE_DIRECT, aur->getCasterGuid());
    }
};
#endif

void setupPaladinSpells(ScriptMgr* mgr)
{
    // Call legacy script setup
    SetupLegacyPaladinSpells(mgr);

    uint32_t eyeForEyeIds[] =
    {
        SPELL_EYE_FOR_AN_EYE_DUMMY_R1,
        SPELL_EYE_FOR_AN_EYE_DUMMY_R2,
        0
    };
    mgr->register_spell_script(eyeForEyeIds, new EyeForAnEyeDummy);
    mgr->register_spell_script(SPELL_EYE_FOR_AN_EYE_DAMAGE, new EyeForAnEye);

    mgr->register_spell_script(SPELL_JUDGEMENT_OF_LIGHT_DEBUFF, new JudgementOfLightDebuff);
    mgr->register_spell_script(SPELL_JUDGEMENT_OF_LIGHT_HEAL, new JudgementOfLightHeal);
    mgr->register_spell_script(SPELL_JUDGEMENT_OF_WISDOM_DEBUFF, new JudgementOfWisdomDebuff);
    mgr->register_spell_script(SPELL_JUDGEMENT_OF_WISDOM_MANA, new JudgementOfWisdomMana);

#if VERSION_STRING == WotLK
    mgr->register_spell_script(SPELL_SEAL_OF_CORRUPTION_DIRECT, new SealOfVengeanceAndCorruptionDirect);
    mgr->register_spell_script(SPELL_SEAL_OF_CORRUPTION_DUMMY, new SealOfCorruptionDummy);
    mgr->register_spell_script(SPELL_BLOOD_CORRUPTION, new SealOfVengeanceAndCorruptionDot);
#endif

    mgr->register_spell_script(SPELL_SEAL_OF_RIGHTOUESNESS_DUMMY, new SealOfRighteousnessDummy);

#if VERSION_STRING >= TBC
    mgr->register_spell_script(SPELL_SEAL_OF_VENGEANCE_DIRECT, new SealOfVengeanceAndCorruptionDirect);
    mgr->register_spell_script(SPELL_SEAL_OF_VENGEANCE_DUMMY, new SealOfVengeanceDummy);
    mgr->register_spell_script(SPELL_HOLY_VENGEANCE, new SealOfVengeanceAndCorruptionDot);
#endif
}
