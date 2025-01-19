/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "Objects/Units/Players/Player.hpp"
#include "Spell/Spell.hpp"
#include "Spell/SpellAura.hpp"
#include "Spell/SpellInfo.hpp"
#include "Spell/SpellMgr.hpp"
#include "Spell/SpellScript.hpp"

enum DruidSpells
{
    SPELL_FUROR_R1                          = 17056,
    SPELL_FUROR_RAGE                        = 17057,
    SPELL_FUROR_ENERGY                      = 17099,
    SPELL_IMPROVED_LEADER_OF_THE_PACK_R1    = 34297,
    SPELL_LEADER_OF_THE_PACK_DUMMY          = 17007,
    SPELL_LEADER_OF_THE_PACK_AURA           = 24932,
    SPELL_LEADER_OF_THE_PACK_HEAL           = 34299,
    SPELL_LEADER_OF_THE_PACK_MANA           = 60889,
};

#if VERSION_STRING < Mop
class FurorDummy : public SpellScript
{
public:
    SpellScriptCheckDummy onAuraDummyEffect(Aura* aur, AuraEffectModifier* aurEff, bool apply) override
    {
        if (apply)
        {
            const auto procChance = aurEff->getEffectDamage();

            // Make Furor rage proc on Bear Form passive aura
            const uint32_t rageProcMask[3] = { 0, 0, 0x2 };
            aur->getOwner()->addProcTriggerSpell(sSpellMgr.getSpellInfo(SPELL_FUROR_RAGE), aur->getSpellInfo(), aur->getCasterGuid(), procChance, PROC_ON_DONE_POSITIVE_SPELL_DAMAGE_CLASS_NONE, rageProcMask, aur);

            // Make Furor energy proc on Cat Form passive aura
            const uint32_t energyProcMask[3] = { 0x8000000, 0, 0 };
#if VERSION_STRING < WotLK
            aur->getOwner()->addProcTriggerSpell(sSpellMgr.getSpellInfo(SPELL_FUROR_ENERGY), aur->getSpellInfo(), aur->getCasterGuid(), procChance, PROC_ON_DONE_POSITIVE_SPELL_DAMAGE_CLASS_NONE, energyProcMask, aur);
#else
            auto spellProc = aur->getOwner()->addProcTriggerSpell(sSpellMgr.getSpellInfo(SPELL_FUROR_ENERGY), aur->getSpellInfo(), aur->getCasterGuid(), 100, PROC_ON_DONE_POSITIVE_SPELL_DAMAGE_CLASS_NONE, energyProcMask, aur);
            // Energy amount equals proc chance
            if (spellProc != nullptr)
                spellProc->setOverrideEffectDamage(EFF_INDEX_0, procChance);
#endif
        }
        else
        {
            aur->getOwner()->removeProcTriggerSpell(SPELL_FUROR_RAGE, aur->getCasterGuid());
            aur->getOwner()->removeProcTriggerSpell(SPELL_FUROR_ENERGY, aur->getCasterGuid());
        }

        return SpellScriptCheckDummy::DUMMY_OK;
    }
};

class Furor : public SpellScript
{
public:
    bool canProcOnTriggered(SpellProc* /*spellProc*/, Unit* /*victim*/, SpellInfo const* /*castingSpell*/, Aura* /*triggeredFromAura*/) override
    {
        // Both forms' passive auras are triggered spells
        return true;
    }
};
#endif

#if VERSION_STRING < Cata
class ImprovedLeaderOfThePackDummy : public SpellScript
{
public:
#if VERSION_STRING == WotLK
    void onAuraApply(Aura* aur) override
    {
        auto spellProc = aur->getOwner()->addProcTriggerSpell(sSpellMgr.getSpellInfo(SPELL_LEADER_OF_THE_PACK_MANA), sSpellMgr.getSpellInfo(SPELL_LEADER_OF_THE_PACK_AURA), aur->getCasterGuid(), 100, PROC_ON_TAKEN_POSITIVE_SPELL_DAMAGE_CLASS_NONE, nullptr, aur);
        if (spellProc != nullptr)
            spellProc->setOverrideEffectDamage(EFF_INDEX_0, aur->getSpellInfo()->calculateEffectValue(EFF_INDEX_1));
    }

    void onAuraRemove(Aura* aur, AuraRemoveMode /*mode*/) override
    {
        aur->getOwner()->removeProcTriggerSpell(SPELL_LEADER_OF_THE_PACK_MANA, aur->getCasterGuid());
    }
#endif
};
#endif

class LeaderOfThePack : public SpellScript
{
public:
    SpellScriptCheckDummy onAuraDummyEffect(Aura* aur, AuraEffectModifier* aurEff, bool apply) override
    {
        if (aurEff->getEffectIndex() != EFF_INDEX_1)
            return SpellScriptCheckDummy::DUMMY_OK;

        if (apply)
        {
            // Check if caster has Improved Leader of the Pack
            if (aurEff->getEffectDamage() > 0)
            {
                auto spellProc = aur->getOwner()->addProcTriggerSpell(sSpellMgr.getSpellInfo(SPELL_LEADER_OF_THE_PACK_HEAL), aur, aur->getCasterGuid());
                if (spellProc != nullptr)
                    spellProc->setOverrideEffectDamage(EFF_INDEX_0, aurEff->getEffectDamage());
            }
        }
        else
        {
            aur->getOwner()->removeProcTriggerSpell(SPELL_LEADER_OF_THE_PACK_HEAL, aur->getCasterGuid());
        }

        return SpellScriptCheckDummy::DUMMY_OK;
    }
};

class LeaderOfThePackDummy : public SpellScript
{
public:
    SpellScriptCheckDummy onAuraDummyEffect(Aura* aur, AuraEffectModifier* /*aurEff*/, bool apply) override
    {
        if (apply)
        {
            if (aur->getPlayerOwner() != nullptr)
                aur->getPlayerOwner()->addShapeShiftSpell(SPELL_LEADER_OF_THE_PACK_AURA);
        }
        else
        {
            if (aur->getPlayerOwner() != nullptr)
                aur->getPlayerOwner()->removeShapeShiftSpell(SPELL_LEADER_OF_THE_PACK_AURA);
        }

        return SpellScriptCheckDummy::DUMMY_OK;
    }
};

class LeaderOfThePackHeal : public SpellScript
{
public:
    void onCreateSpellProc(SpellProc* spellProc, Object* /*obj*/) override
    {
        // should have 6 seconds cooldown
        spellProc->setProcInterval(6000);
        // should proc only on crits
        spellProc->setExtraProcFlags(EXTRA_PROC_ON_CRIT_ONLY);

        spellProc->setCastedByProcCreator(true);
        spellProc->setCastedOnProcOwner(true);
    }

    SpellScriptEffectDamage doCalculateEffect(Spell* spell, uint8_t /*effIndex*/, int32_t* damage) override
    {
        if (spell->getUnitTarget() != nullptr)
            *damage = static_cast<int32_t>(std::round(spell->getUnitTarget()->getMaxHealth() * static_cast<float_t>(*damage / 100.0f)));

        // no healing bonuses
        return SpellScriptEffectDamage::DAMAGE_NO_BONUSES;
    }
};

#if VERSION_STRING == WotLK
class LeaderOfThePackMana : public SpellScript
{
public:
    bool canProc(SpellProc* /*spellProc*/, Unit* /*victim*/, SpellInfo const* castingSpell, DamageInfo /*damageInfo*/) override
    {
        if (castingSpell == nullptr || castingSpell->getId() != SPELL_LEADER_OF_THE_PACK_HEAL)
            return false;

        // Mana spell should proc when heal spell has proced
        // this proc is only registered to talent owner so no need to do further checks
        return true;
    }

    bool canProcOnTriggered(SpellProc* /*spellProc*/, Unit* /*vicitm*/, SpellInfo const* /*castingSpell*/, Aura* /*triggeredFromAura*/) override
    {
        // Heal spell is triggered so this must proc on triggered
        return true;
    }
};
#endif

void setupDruidSpells(ScriptMgr* mgr)
{
    // Call legacy script setup
    SetupLegacyDruidSpells(mgr);

#if VERSION_STRING < Mop
    mgr->register_spell_script(SPELL_FUROR_R1, new FurorDummy);
    mgr->register_spell_script(SPELL_FUROR_RAGE, new Furor);
    mgr->register_spell_script(SPELL_FUROR_ENERGY, new Furor);
#endif

#if VERSION_STRING < Cata
    mgr->register_spell_script(SPELL_IMPROVED_LEADER_OF_THE_PACK_R1, new ImprovedLeaderOfThePackDummy);
#endif

    mgr->register_spell_script(SPELL_LEADER_OF_THE_PACK_DUMMY, new LeaderOfThePackDummy);
    mgr->register_spell_script(SPELL_LEADER_OF_THE_PACK_AURA, new LeaderOfThePack);
    mgr->register_spell_script(SPELL_LEADER_OF_THE_PACK_HEAL, new LeaderOfThePackHeal);
#if VERSION_STRING == WotLK
    mgr->register_spell_script(SPELL_LEADER_OF_THE_PACK_MANA, new LeaderOfThePackMana);
#endif
}
