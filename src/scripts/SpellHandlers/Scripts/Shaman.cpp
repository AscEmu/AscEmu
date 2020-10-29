/*
Copyright (c) 2014-2020 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"

enum ShamanSpells
{
    SPELL_EARTH_SHIELD_R1       = 974,
    SPELL_EARTH_SHIELD_R2       = 32593,
    SPELL_EARTH_SHIELD_R3       = 32594,
    SPELL_EARTH_SHIELD_R4       = 49283,
    SPELL_EARTH_SHIELD_R5       = 49284,
    SPELL_EARTH_SHIELD_HEAL     = 379,
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

class EarthShieldDummy : public SpellScript
{
public:
    SpellScriptCheckDummy onAuraDummyEffect(Aura* aur, AuraEffectModifier* aurEff, bool apply) override
    {
        if (!apply)
            return SpellScriptCheckDummy::DUMMY_OK;

        auto spellProc = aur->getOwner()->addProcTriggerSpell(sSpellMgr.getSpellInfo(SPELL_EARTH_SHIELD_HEAL), aur->getSpellInfo(), aur->getCasterGuid(), 0);
        if (spellProc != nullptr)
            spellProc->setOverrideEffectDamage(EFF_INDEX_0, aurEff->mDamage);

        return SpellScriptCheckDummy::DUMMY_OK;
    }

    void onAuraRemove(Aura* aur, AuraRemoveMode /*mode*/) override
    {
        aur->getOwner()->removeProcTriggerSpell(SPELL_EARTH_SHIELD_HEAL, aur->getCasterGuid());
    }
};

class EarthShieldHeal : public ElementalShield
{
public:
    void onCreateSpellProc(SpellProc* spellProc, Object* /*obj*/) override
    {
        spellProc->setCastedByProcCreator(true);
        spellProc->setCastedOnProcOwner(true);
    }
};

void setupShamanSpells(ScriptMgr* mgr)
{
    // Call legacy script setup
    SetupLegacyShamanSpells(mgr);

    uint32_t earthShieldIds[] =
    {
        SPELL_EARTH_SHIELD_R1,
        SPELL_EARTH_SHIELD_R2,
        SPELL_EARTH_SHIELD_R3,
        SPELL_EARTH_SHIELD_R4,
        SPELL_EARTH_SHIELD_R5,
        0
    };
    mgr->register_spell_script(earthShieldIds, new EarthShieldDummy);
    mgr->register_spell_script(SPELL_EARTH_SHIELD_HEAL, new EarthShieldHeal);
}
