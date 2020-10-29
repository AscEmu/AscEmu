/*
Copyright (c) 2014-2020 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"

enum MageSpells
{
    SPELL_GLYPH_OF_THE_PENGUIN  = 52648,
    SPELL_HOT_STREAK_BUFF       = 48108,
    SPELL_HOT_STREAK_R1         = 44445,
    SPELL_HOT_STREAK_R2         = 44446,
    SPELL_HOT_STREAK_R3         = 44448,
    SPELL_MASTER_OF_ELEMENTS_R1 = 29074,
    SPELL_MASTER_OF_ELEMENTS_R2 = 29075,
    SPELL_MASTER_OF_ELEMENTS_R3 = 29076,
    SPELL_MASTER_OF_ELEMENTS    = 29077,
    SPELL_POLYMORPH_R1          = 118,
    SPELL_POLYMORPH_R2          = 12824,
    SPELL_POLYMORPH_R3          = 12825,
    SPELL_POLYMORPH_R4          = 12826,
    SPELL_POLYMORPH_TURTLE      = 28271,
    SPELL_POLYMORPH_PIG         = 28272,
    SPELL_POLYMORPH_SERPENT     = 61025,
    SPELL_POLYMORPH_BLACK_CAT   = 61305,
    SPELL_POLYMORPH_RABBIT      = 61721,
    SPELL_POLYMORPH_TURKEY      = 61780,

    ICON_POLYMORPH_SHEEP        = 82,
    CREATURE_CHILLY             = 29726, // Penguin NPC for Glyph of the Penguin
};

#if VERSION_STRING >= WotLK
class HotStreakDummy : public SpellScript
{
public:
    SpellScriptCheckDummy onAuraDummyEffect(Aura* aur, AuraEffectModifier* aurEff, bool apply) override
    {
        if (aurEff->effIndex != EFF_INDEX_0 || !apply)
            return SpellScriptCheckDummy::DUMMY_OK;

        // should proc on Living Bomb (direct damage only), Fireball, Fire Blast, Scorch and Frostfire bolt
        uint32_t procFamilyMask[3] = { 0x13, 0x11000, 0x8 };
        auto spellProc = aur->getOwner()->addProcTriggerSpell(sSpellMgr.getSpellInfo(SPELL_HOT_STREAK_BUFF), aur->getSpellInfo(), aur->getCasterGuid(), 0, procFamilyMask);
        if (spellProc != nullptr)
            spellProc->setProcChance(aurEff->mDamage);

        return SpellScriptCheckDummy::DUMMY_OK;
    }

    void onAuraRemove(Aura* aur, AuraRemoveMode /*mode*/) override
    {
        aur->getOwner()->removeProcTriggerSpell(SPELL_HOT_STREAK_BUFF, aur->getCasterGuid());
    }
};

class HotStreak : public SpellScript
{
public:
    bool canProc(SpellProc* /*spellProc*/, Unit* /*victim*/, SpellInfo const* /*castingSpell*/, DamageInfo damageInfo) override
    {
        if (damageInfo.isCritical)
            ++critsInRow;
        else
            critsInRow = 0;

        if (critsInRow < 2)
            return false;

        critsInRow = 0;
        return true;
    }

private:
    uint8_t critsInRow = 0;
};
#endif

#if VERSION_STRING < Mop
class MasterOfElementsDummy : public SpellScript
{
public:
    SpellScriptCheckDummy onAuraDummyEffect(Aura* aur, AuraEffectModifier* aurEff, bool apply) override
    {
        if (!apply)
            return SpellScriptCheckDummy::DUMMY_OK;

        auto spellProc = aur->getOwner()->addProcTriggerSpell(sSpellMgr.getSpellInfo(SPELL_MASTER_OF_ELEMENTS), aur->getSpellInfo(), aur->getCasterGuid(), 0);
        if (spellProc != nullptr)
        {
            spellProc->setOverrideEffectDamage(EFF_INDEX_1, aurEff->mDamage);
            spellProc->setExtraProcFlags(EXTRA_PROC_ON_CRIT_ONLY);
        }

        return SpellScriptCheckDummy::DUMMY_OK;
    }

    void onAuraRemove(Aura* aur, AuraRemoveMode /*mode*/) override
    {
        aur->getOwner()->removeProcTriggerSpell(SPELL_MASTER_OF_ELEMENTS, aur->getCasterGuid());
    }
};

class MasterOfElements : public SpellScript
{
public:
    void onCreateSpellProc(SpellProc* spellProc, Object* /*obj*/) override
    {
        spellProc->setCastedOnProcOwner(true);
    }

    bool canProc(SpellProc* /*spellProc*/, Unit* /*victim*/, SpellInfo const* castingSpell, DamageInfo damageInfo) override
    {
        if (castingSpell == nullptr || damageInfo.weaponType == RANGED)
            return false;
#if VERSION_STRING < WotLK
        // Can proc only from fire or frost school
        if (castingSpell->getSchoolMask() & SCHOOL_MASK_FIRE || castingSpell->getSchoolMask() & SCHOOL_MASK_FROST)
            return true;
        return false;
#else
        // Reset aura before continuing
        originalAura = nullptr;
        return true;
#endif
    }

    bool canProcOnTriggered(SpellProc* /*spellProc*/, Unit* /*victim*/, SpellInfo const* /*castingSpell*/, Aura* triggeredFromAura) override
    {
        if (triggeredFromAura == nullptr)
            return false;

        originalAura = triggeredFromAura;
        return true;
    }

    SpellScriptExecuteState onDoProcEffect(SpellProc* spellProc, Unit* /*victim*/, SpellInfo const* castingSpell, DamageInfo /*damageInfo*/) override
    {
        int32_t manaReturn = 0;
        if (originalAura != nullptr && !originalAura->isDeleted() && originalAura->getSpellInfo()->isChanneled())
        {
            // "For channeled spells that deal damage in ticks such as Arcane Missiles and Blizzard, the mana returned is based on the tick that critically hit, and not the entire spell."
            // i.e. Arcane Missiles deals damage in 5 ticks; it will only calculate mana return for 1/5th of the cost of Arcane Missiles

            uint16_t ticks = 1;

            // Find the aura effect
            for (uint8_t i = 0; i < MAX_SPELL_EFFECTS; ++i)
            {
                const auto aurEff = originalAura->getAuraEffect(i);
                if (aurEff.mAuraEffect == SPELL_AURA_NONE)
                    continue;

                if (aurEff.mAuraEffect == SPELL_AURA_PERIODIC_TRIGGER_SPELL ||
                    aurEff.mAuraEffect == SPELL_AURA_PERIODIC_TRIGGER_SPELL_WITH_VALUE)
                {
                    ticks = originalAura->getPeriodicTickCountForEffect(aurEff.effIndex);
                    break;
                }
            }

            // Divide mana return with tick count
            manaReturn = originalAura->getSpellInfo()->getBasePowerCost(spellProc->getProcOwner()) / ticks;
            originalAura = nullptr;
        }
        else
        {
            manaReturn = castingSpell->getBasePowerCost(spellProc->getProcOwner());
        }

        if (manaReturn <= 0)
            return SpellScriptExecuteState::EXECUTE_PREVENT;

        manaReturn = static_cast<int32_t>(std::round(manaReturn * spellProc->getOverrideEffectDamage(EFF_INDEX_1) / 100.0f));
        spellProc->setOverrideEffectDamage(EFF_INDEX_0, manaReturn);
        return SpellScriptExecuteState::EXECUTE_OK;
    }

private:
    Aura* originalAura = nullptr;
};
#endif

class Polymorph : public SpellScript
{
public:
    SpellScriptExecuteState beforeAuraEffect(Aura* aur, AuraEffectModifier* aurEff, bool apply) override
    {
        if (aurEff->mAuraEffect != SPELL_AURA_TRANSFORM)
            return SpellScriptExecuteState::EXECUTE_OK;

        if (apply)
        {
            // Dismount player on aura apply
            // TODO: should also dismount creatures
            if (aur->getPlayerOwner() != nullptr)
                aur->getPlayerOwner()->Dismount();

            // Add this unitstate only for player polymorph spells
            // Mostly polymorphs casted by creatures won't regenerate health
            // Also, reset health regenerate timer
            aur->getOwner()->addUnitStateFlag(UNIT_STATE_POLYMORPHED);
            aur->getOwner()->setHRegenTimer(1000);

            // Glyph of the Penguin
            const auto caster = aur->GetUnitCaster();
            if (caster != nullptr && caster->HasAura(SPELL_GLYPH_OF_THE_PENGUIN) && aur->getSpellInfo()->getSpellIconID() == ICON_POLYMORPH_SHEEP)
            {
                // Override misc value (Sheep) with Penguin npc
                aurEff->miscValue = CREATURE_CHILLY;
            }
        }
        else
        {
            aur->getOwner()->removeUnitStateFlag(UNIT_STATE_POLYMORPHED);
        }

        return SpellScriptExecuteState::EXECUTE_OK;
    }
};

void setupMageSpells(ScriptMgr* mgr)
{
    // Call legacy script setup
    SetupLegacyMageSpells(mgr);

#if VERSION_STRING >= WotLK
    uint32_t hotStreakIds[] =
    {
        SPELL_HOT_STREAK_R1,
        SPELL_HOT_STREAK_R2,
        SPELL_HOT_STREAK_R3,
        0
    };
    mgr->register_spell_script(hotStreakIds, new HotStreakDummy);
    mgr->register_spell_script(SPELL_HOT_STREAK_BUFF, new HotStreak);
#endif

#if VERSION_STRING < Mop
    uint32_t masterOfElementsId[] =
    {
        SPELL_MASTER_OF_ELEMENTS_R1,
        SPELL_MASTER_OF_ELEMENTS_R2,
        SPELL_MASTER_OF_ELEMENTS_R3,
        0
    };
    mgr->register_spell_script(masterOfElementsId, new MasterOfElementsDummy);
    mgr->register_spell_script(SPELL_MASTER_OF_ELEMENTS, new MasterOfElements);
#endif

    uint32_t polymorphIds[] =
    {
        SPELL_POLYMORPH_R1,
        SPELL_POLYMORPH_R2,
        SPELL_POLYMORPH_R3,
        SPELL_POLYMORPH_R4,
        SPELL_POLYMORPH_TURTLE,
        SPELL_POLYMORPH_PIG,
#if VERSION_STRING >= WotLK
        SPELL_POLYMORPH_SERPENT,
        SPELL_POLYMORPH_BLACK_CAT,
        SPELL_POLYMORPH_RABBIT,
        SPELL_POLYMORPH_TURKEY,
#endif
        0
    };
    mgr->register_spell_script(polymorphIds, new Polymorph);
}
