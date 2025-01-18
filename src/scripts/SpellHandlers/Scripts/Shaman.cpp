/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "Objects/Units/Unit.hpp"
#include "Spell/Spell.hpp"
#include "Spell/SpellAura.hpp"
#include "Spell/SpellInfo.hpp"
#include "Spell/SpellMgr.hpp"
#include "Spell/SpellProc.hpp"
#include "Spell/SpellScript.hpp"

enum ShamanSpells
{
    SPELL_EARTH_SHIELD_R1               = 974,
    SPELL_EARTH_SHIELD_HEAL             = 379,
    SPELL_LIGHTNING_SHIELD_DUMMY_R1     = 324,
    SPELL_LIGHTNING_SHIELD_DMG_R1       = 26364,
};

// This is a common script to setup proc cooldown for Earth Shield, Water Shield and Lightning Shield
class ElementalShield : public SpellScript
{
public:
    void onCreateSpellProc(SpellProc* spellProc, Object* /*obj*/) override
    {
        spellProc->setProcInterval(3000);
    }
};

#if VERSION_STRING >= TBC
class EarthShieldDummy : public SpellScript
{
public:
    SpellScriptEffectDamage doCalculateEffect(Spell* spell, uint8_t effIndex, int32_t* dmg) override
    {
        if (effIndex != EFF_INDEX_0 || spell->getUnitCaster() == nullptr)
            return SpellScriptEffectDamage::DAMAGE_DEFAULT;

        // Calculate healing done here so correct percent modifiers are applied
        *dmg = static_cast<int32_t>(std::ceil(spell->getUnitCaster()->applySpellHealingBonus(spell->getSpellInfo(), *dmg, 1.0f, false, spell)));
        return SpellScriptEffectDamage::DAMAGE_NO_BONUSES;
    }

    SpellScriptCheckDummy onAuraDummyEffect(Aura* aur, AuraEffectModifier* aurEff, bool apply) override
    {
        if (apply)
        {
            auto spellProc = aur->getOwner()->addProcTriggerSpell(sSpellMgr.getSpellInfo(SPELL_EARTH_SHIELD_HEAL), aur, aur->getCasterGuid());
            if (spellProc != nullptr)
                spellProc->setOverrideEffectDamage(EFF_INDEX_0, aurEff->getEffectDamage());
        }
        else
        {
            aur->getOwner()->removeProcTriggerSpell(SPELL_EARTH_SHIELD_HEAL, aur->getCasterGuid());
        }

        return SpellScriptCheckDummy::DUMMY_OK;
    }
};

class EarthShield : public ElementalShield
{
public:
    void onCreateSpellProc(SpellProc* spellProc, Object* obj) override
    {
        spellProc->setCastedByProcCreator(true);
        spellProc->setCastedOnProcOwner(true);

        ElementalShield::onCreateSpellProc(spellProc, obj);
    }

    bool canProc(SpellProc* /*spellProc*/, Unit* /*victim*/, SpellInfo const* castingSpell, DamageInfo damageInfo) override
    {
        // should proc only on damage spells
        if (castingSpell != nullptr && !castingSpell->isDamagingSpell())
            return false;

        // should not proc on absorb
        if (damageInfo.realDamage <= 0)
            return false;

        return true;
    }

    SpellScriptEffectDamage doCalculateEffect(Spell* /*spell*/, uint8_t /*effIndex*/, int32_t* /*dmg*/) override
    {
        // Fully calculated in main aura
        return SpellScriptEffectDamage::DAMAGE_FULL_RECALCULATION;
    }
};
#endif

class LightningShieldDummy : public SpellScript
{
public:
    SpellInfo const* getDamageSpellForDummyRank(SpellInfo const* dummyInfo) const
    {
        const auto* dmgInfo = sSpellMgr.getSpellInfo(SPELL_LIGHTNING_SHIELD_DMG_R1);
        if (!dummyInfo->hasSpellRanks() || dmgInfo == nullptr || !dmgInfo->hasSpellRanks())
            return dmgInfo;

        const auto dummyRank = dummyInfo->getRankInfo()->getRank();
        return dmgInfo->getRankInfo()->getSpellWithRank(dummyRank);
    }

    SpellScriptEffectDamage doCalculateEffect(Spell* spell, uint8_t effIndex, int32_t* dmg) override
    {
        if (effIndex != EFF_INDEX_0 || spell->getUnitCaster() == nullptr)
            return SpellScriptEffectDamage::DAMAGE_DEFAULT;

        // Calculate damage done here so correct percent modifiers are applied
        *dmg = static_cast<int32_t>(std::ceil(spell->getUnitCaster()->applySpellDamageBonus(spell->getSpellInfo(), *dmg, 1.0f, false, spell)));
        return SpellScriptEffectDamage::DAMAGE_NO_BONUSES;
    }

    SpellScriptExecuteState beforeAuraEffect(Aura* aur, AuraEffectModifier* aurEff, bool apply) override
    {
        if (!apply)
            return SpellScriptExecuteState::EXECUTE_OK;

        const auto damageSpell = getDamageSpellForDummyRank(aur->getSpellInfo());
        if (damageSpell == nullptr)
            return SpellScriptExecuteState::EXECUTE_PREVENT;

        // TODO: classic uses another dummy spell before damage

        // Override effect with real proc effect
        auto spellProc = aur->getOwner()->addProcTriggerSpell(damageSpell, aur, aur->getCasterGuid());
        if (spellProc != nullptr)
            spellProc->setOverrideEffectDamage(EFF_INDEX_0, aurEff->getEffectDamage());

        return SpellScriptExecuteState::EXECUTE_PREVENT;
    }

    void onAuraRemove(Aura* aur, AuraRemoveMode /*mode*/) override
    {
        if (const auto damageSpell = getDamageSpellForDummyRank(aur->getSpellInfo()))
            aur->getOwner()->removeProcTriggerSpell(damageSpell->getId(), aur->getCasterGuid());
    }
};

class LightningShield : public ElementalShield
{
public:
    bool canProc(SpellProc* /*spellProc*/, Unit* /*victim*/, SpellInfo const* /*castingSpell*/, DamageInfo damageInfo) override
    {
        // should not proc on absorb
        if (damageInfo.realDamage <= 0)
            return false;

        return true;
    }

    SpellScriptEffectDamage doCalculateEffect(Spell* /*spell*/, uint8_t /*effIndex*/, int32_t* /*dmg*/) override
    {
        // Fully calculated in main aura
        return SpellScriptEffectDamage::DAMAGE_FULL_RECALCULATION;
    }
};

void setupShamanSpells(ScriptMgr* mgr)
{
    // Call legacy script setup
    SetupLegacyShamanSpells(mgr);

#if VERSION_STRING >= TBC
    mgr->register_spell_script(SPELL_EARTH_SHIELD_R1, new EarthShieldDummy);
    mgr->register_spell_script(SPELL_EARTH_SHIELD_HEAL, new EarthShield);
#endif

    mgr->register_spell_script(SPELL_LIGHTNING_SHIELD_DUMMY_R1, new LightningShieldDummy);
    mgr->register_spell_script(SPELL_LIGHTNING_SHIELD_DMG_R1, new LightningShield);
}
