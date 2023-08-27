/*
Copyright (c) 2014-2023 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "Objects/Units/Unit.hpp"
#include "Spell/Spell.hpp"
#include "Spell/SpellAuras.h"
#include "Spell/SpellMgr.hpp"
#include "Spell/SpellProc.hpp"

enum ShamanSpells
{
    SPELL_EARTH_SHIELD_R1               = 974,
    SPELL_EARTH_SHIELD_R2               = 32593,
    SPELL_EARTH_SHIELD_R3               = 32594,
    SPELL_EARTH_SHIELD_R4               = 49283,
    SPELL_EARTH_SHIELD_R5               = 49284,
    SPELL_EARTH_SHIELD_HEAL             = 379,
    SPELL_LIGHTNING_SHIELD_DUMMY_R1     = 324,
    SPELL_LIGHTNING_SHIELD_DUMMY_R2     = 325,
    SPELL_LIGHTNING_SHIELD_DUMMY_R3     = 905,
    SPELL_LIGHTNING_SHIELD_DUMMY_R4     = 945,
    SPELL_LIGHTNING_SHIELD_DUMMY_R5     = 8134,
    SPELL_LIGHTNING_SHIELD_DUMMY_R6     = 10431,
    SPELL_LIGHTNING_SHIELD_DUMMY_R7     = 10432,
    SPELL_LIGHTNING_SHIELD_DUMMY_R8     = 25469,
    SPELL_LIGHTNING_SHIELD_DUMMY_R9     = 25472,
    SPELL_LIGHTNING_SHIELD_DUMMY_R10    = 49280,
    SPELL_LIGHTNING_SHIELD_DUMMY_R11    = 49281,
    SPELL_LIGHTNING_SHIELD_DMG_R1       = 26364,
    SPELL_LIGHTNING_SHIELD_DMG_R2       = 26365,
    SPELL_LIGHTNING_SHIELD_DMG_R3       = 26366,
    SPELL_LIGHTNING_SHIELD_DMG_R4       = 26367,
    SPELL_LIGHTNING_SHIELD_DMG_R5       = 26369,
    SPELL_LIGHTNING_SHIELD_DMG_R6       = 26370,
    SPELL_LIGHTNING_SHIELD_DMG_R7       = 26363,
    SPELL_LIGHTNING_SHIELD_DMG_R8       = 26371,
    SPELL_LIGHTNING_SHIELD_DMG_R9       = 26372,
    SPELL_LIGHTNING_SHIELD_DMG_R10      = 49278,
    SPELL_LIGHTNING_SHIELD_DMG_R11      = 49279,
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
    // todo: remove this when spell ranking is implemented
    uint32_t getDamageSpellForDummyRank(uint32_t spellId) const
    {
        switch (spellId)
        {
            case SPELL_LIGHTNING_SHIELD_DUMMY_R1:
                return SPELL_LIGHTNING_SHIELD_DMG_R1;
            case SPELL_LIGHTNING_SHIELD_DUMMY_R2:
                return SPELL_LIGHTNING_SHIELD_DMG_R2;
            case SPELL_LIGHTNING_SHIELD_DUMMY_R3:
                return SPELL_LIGHTNING_SHIELD_DMG_R3;
            case SPELL_LIGHTNING_SHIELD_DUMMY_R4:
                return SPELL_LIGHTNING_SHIELD_DMG_R4;
            case SPELL_LIGHTNING_SHIELD_DUMMY_R5:
                return SPELL_LIGHTNING_SHIELD_DMG_R5;
            case SPELL_LIGHTNING_SHIELD_DUMMY_R6:
                return SPELL_LIGHTNING_SHIELD_DMG_R6;
            case SPELL_LIGHTNING_SHIELD_DUMMY_R7:
                return SPELL_LIGHTNING_SHIELD_DMG_R7;
            case SPELL_LIGHTNING_SHIELD_DUMMY_R8:
                return SPELL_LIGHTNING_SHIELD_DMG_R8;
            case SPELL_LIGHTNING_SHIELD_DUMMY_R9:
                return SPELL_LIGHTNING_SHIELD_DMG_R9;
            case SPELL_LIGHTNING_SHIELD_DUMMY_R10:
                return SPELL_LIGHTNING_SHIELD_DMG_R10;
            case SPELL_LIGHTNING_SHIELD_DUMMY_R11:
                return SPELL_LIGHTNING_SHIELD_DMG_R11;
        }

        return 0;
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

        // Override effect with real proc effect
        auto spellProc = aur->getOwner()->addProcTriggerSpell(sSpellMgr.getSpellInfo(getDamageSpellForDummyRank(aur->getSpellId())), aur, aur->getCasterGuid());
        if (spellProc != nullptr)
            spellProc->setOverrideEffectDamage(EFF_INDEX_0, aurEff->getEffectDamage());

        return SpellScriptExecuteState::EXECUTE_PREVENT;
    }

    void onAuraRemove(Aura* aur, AuraRemoveMode /*mode*/) override
    {
        aur->getOwner()->removeProcTriggerSpell(getDamageSpellForDummyRank(aur->getSpellId()), aur->getCasterGuid());
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
    uint32_t earthShieldIds[] =
    {
        SPELL_EARTH_SHIELD_R1,
#if VERSION_STRING < Cata
        SPELL_EARTH_SHIELD_R2,
        SPELL_EARTH_SHIELD_R3,
#if VERSION_STRING == WotLK
        SPELL_EARTH_SHIELD_R4,
        SPELL_EARTH_SHIELD_R5,
#endif
#endif
        0
    };
    mgr->register_spell_script(earthShieldIds, new EarthShieldDummy);
    mgr->register_spell_script(SPELL_EARTH_SHIELD_HEAL, new EarthShield);
#endif

    uint32_t lightningShieldDummyIds[] =
    {
        SPELL_LIGHTNING_SHIELD_DUMMY_R1,
#if VERSION_STRING < Cata
        SPELL_LIGHTNING_SHIELD_DUMMY_R2,
        SPELL_LIGHTNING_SHIELD_DUMMY_R3,
        SPELL_LIGHTNING_SHIELD_DUMMY_R4,
        SPELL_LIGHTNING_SHIELD_DUMMY_R5,
        SPELL_LIGHTNING_SHIELD_DUMMY_R6,
        SPELL_LIGHTNING_SHIELD_DUMMY_R7,
#if VERSION_STRING >= TBC
        SPELL_LIGHTNING_SHIELD_DUMMY_R8,
        SPELL_LIGHTNING_SHIELD_DUMMY_R9,
#if VERSION_STRING == WotLK
        SPELL_LIGHTNING_SHIELD_DUMMY_R10,
        SPELL_LIGHTNING_SHIELD_DUMMY_R11,
#endif
#endif
#endif
        0
    };
    mgr->register_spell_script(lightningShieldDummyIds, new LightningShieldDummy);
    uint32_t lightningShieldDmgIds[] =
    {
        SPELL_LIGHTNING_SHIELD_DMG_R1,
#if VERSION_STRING < Cata
        SPELL_LIGHTNING_SHIELD_DMG_R2,
        SPELL_LIGHTNING_SHIELD_DMG_R3,
        SPELL_LIGHTNING_SHIELD_DMG_R4,
        SPELL_LIGHTNING_SHIELD_DMG_R5,
        SPELL_LIGHTNING_SHIELD_DMG_R6,
        SPELL_LIGHTNING_SHIELD_DMG_R7,
#if VERSION_STRING >= TBC
        SPELL_LIGHTNING_SHIELD_DMG_R8,
        SPELL_LIGHTNING_SHIELD_DMG_R9,
#if VERSION_STRING == WotLK
        SPELL_LIGHTNING_SHIELD_DMG_R10,
        SPELL_LIGHTNING_SHIELD_DMG_R11,
#endif
#endif
#endif
        0
    };
    mgr->register_spell_script(lightningShieldDmgIds, new LightningShield);
}
