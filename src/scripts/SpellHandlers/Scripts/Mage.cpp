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

enum MageSpells
{
    SPELL_ARCANE_MISSILES_PROC  = 79683,
    SPELL_DEEP_FREEZE_DAMAGE    = 71757,
    SPELL_GLYPH_OF_THE_PENGUIN  = 52648,
    SPELL_HOT_STREAK_BUFF       = 48108,
    SPELL_HOT_STREAK_R1         = 44445,
    SPELL_IMPACT_DUMMY          = 64343,
    SPELL_IMPACT_STUN           = 12355,
    SPELL_INVISIBILITY          = 66,
    SPELL_INVISIBILITY_REAL     = 32612,
    SPELL_MASTER_OF_ELEMENTS_R1 = 29074,
    SPELL_MASTER_OF_ELEMENTS    = 29077,
    SPELL_POLYMORPH_R1          = 118,
    SPELL_POLYMORPH_TURTLE      = 28271,
    SPELL_POLYMORPH_PIG         = 28272,
    SPELL_POLYMORPH_SERPENT     = 61025,
    SPELL_POLYMORPH_BLACK_CAT   = 61305,
    SPELL_POLYMORPH_RABBIT      = 61721,
    SPELL_POLYMORPH_TURKEY      = 61780,

    ICON_POLYMORPH_SHEEP        = 82,
    CREATURE_CHILLY             = 29726, // Penguin NPC for Glyph of the Penguin
};

#if VERSION_STRING >= Cata
class ArcaneMissilesProc : public SpellScript
{
public:
    uint32_t calcProcChance(SpellProc* /*proc*/, Unit* /*victim*/, SpellInfo const* /*castingSpell*/) override
    {
        // DBC data says 100% but ingame tooltip says 40%
        return 40;
    }
};
#endif

#if VERSION_STRING >= WotLK
class DeepFreezeDamage : public SpellScript
{
public:
    bool canProc(SpellProc* /*spellProc*/, Unit* /*victim*/, SpellInfo const* /*castingSpell*/, DamageInfo /*damageInfo*/) override
    {
        // TODO: prevent proc for now, fix this later
        return false;
    }
};
#endif

#if VERSION_STRING >= WotLK
class HotStreakDummy : public SpellScript
{
public:
    SpellScriptCheckDummy onAuraDummyEffect(Aura* aur, AuraEffectModifier* aurEff, bool apply) override
    {
        if (aurEff->getEffectIndex() != EFF_INDEX_0)
            return SpellScriptCheckDummy::DUMMY_OK;

        if (apply)
        {
            // should proc on Living Bomb (direct damage only), Fireball, Fire Blast, Scorch and Frostfire bolt
            const uint32_t procFamilyMask[3] = { 0x13, 0x11000, 0x8 };
            auto spellProc = aur->getOwner()->addProcTriggerSpell(sSpellMgr.getSpellInfo(SPELL_HOT_STREAK_BUFF), aur, aur->getCasterGuid(), procFamilyMask);
            if (spellProc != nullptr)
                spellProc->setProcChance(aurEff->getEffectDamage());
        }
        else
        {
            aur->getOwner()->removeProcTriggerSpell(SPELL_HOT_STREAK_BUFF, aur->getCasterGuid());
        }

        return SpellScriptCheckDummy::DUMMY_OK;
    }
};

class HotStreak : public SpellScript
{
public:
    void onAuraCreate(Aura* aur) override
    {
        // SpellInfo is missing charge count
        aur->setCharges(1, false);
    }

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
#if VERSION_STRING >= WotLK
class ImpactDummy : public SpellScript
{
public:
    bool canProc(SpellProc* /*spellProc*/, Unit* /*victim*/, SpellInfo const* /*castingSpell*/, DamageInfo damageInfo) override
    {
        if (damageInfo.weaponType == RANGED)
            return false;

        return damageInfo.fullDamage > 0;
    }

    SpellScriptExecuteState beforeAuraEffect(Aura* aur, AuraEffectModifier* /*aurEff*/, bool apply) override
    {
        // Override default action
        if (apply)
        {
            auto spellProc = aur->getOwner()->addProcTriggerSpell(sSpellMgr.getSpellInfo(SPELL_IMPACT_STUN), aur, aur->getCasterGuid());
            // If this proc is not skipped in next ::handleProc event and it was procced by Fire Blast,
            // the same Fire Blast, that created this aura, will consume this aura
            if (spellProc != nullptr)
                spellProc->skipOnNextHandleProc(true);
        }
        else
        {
            aur->getOwner()->removeProcTriggerSpell(SPELL_IMPACT_STUN, aur->getCasterGuid());
        }

        return SpellScriptExecuteState::EXECUTE_PREVENT;
    }
};
#endif

class Impact : public SpellScript
{
public:
    void onCreateSpellProc(SpellProc* proc, Object* /*obj*/) override
    {
        // TODO: classic and tbc masks
#if VERSION_STRING >= WotLK
        // Should proc only from Fire Blast
        proc->setProcClassMask(EFF_INDEX_0, 0x2);
#endif
    }

#if VERSION_STRING < WotLK
    bool canProc(SpellProc* /*spellProc*/, Unit* /*victim*/, SpellInfo const* /*castingSpell*/, DamageInfo damageInfo) override
    {
        if (damageInfo.weaponType == RANGED)
            return false;

        return damageInfo.fullDamage > 0;
    }
#endif
};
#endif

class Invisibility : public SpellScript
{
public:
    // TODO: missing periodic threat reduction

    SpellScriptExecuteState beforeSpellEffect(Spell* spell, uint8_t /*effIndex*/) override
    {
        if (spell->getUnitCaster() == nullptr)
            return SpellScriptExecuteState::EXECUTE_PREVENT;

        const auto spellDuration = spell->getDuration();
        if (spellDuration == 0)
        {
            // Prismatic Cloak 3/3 reduces duration to 0 and invisibility should be applied instantly
            spell->getUnitCaster()->castSpell(spell->getUnitCaster(), sSpellMgr.getSpellInfo(SPELL_INVISIBILITY_REAL), true);
            return SpellScriptExecuteState::EXECUTE_PREVENT;
        }

        return SpellScriptExecuteState::EXECUTE_OK;
    }

    void onAuraRemove(Aura* aur, AuraRemoveMode mode) override
    {
        if (mode != AURA_REMOVE_ON_EXPIRE)
            return;

        aur->getOwner()->castSpell(aur->getOwner(), sSpellMgr.getSpellInfo(SPELL_INVISIBILITY_REAL), true);
    }
};

#if VERSION_STRING < Mop
class MasterOfElementsDummy : public SpellScript
{
public:
    SpellScriptCheckDummy onAuraDummyEffect(Aura* aur, AuraEffectModifier* aurEff, bool apply) override
    {
        if (apply)
        {
            auto spellProc = aur->getOwner()->addProcTriggerSpell(sSpellMgr.getSpellInfo(SPELL_MASTER_OF_ELEMENTS), aur, aur->getCasterGuid());
            if (spellProc != nullptr)
            {
                spellProc->setOverrideEffectDamage(EFF_INDEX_0, aurEff->getEffectDamage());
                spellProc->setExtraProcFlags(EXTRA_PROC_ON_CRIT_ONLY);
            }
        }
        else
        {
            aur->getOwner()->removeProcTriggerSpell(SPELL_MASTER_OF_ELEMENTS, aur->getCasterGuid());
        }

        return SpellScriptCheckDummy::DUMMY_OK;
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
        if (originalAura != nullptr && !originalAura->isDeleted() && originalAura->getSpellInfo()->isChanneled())
        {
            // "For channeled spells that deal damage in ticks such as Arcane Missiles and Blizzard, the mana returned is based on the tick that critically hit, and not the entire spell."
            // i.e. Arcane Missiles deals damage in 5 ticks; it will only calculate mana return for 1/5th of the cost of Arcane Missiles

            uint16_t ticks = 1;

            // Find the aura effect
            for (uint8_t i = 0; i < MAX_SPELL_EFFECTS; ++i)
            {
                const auto aurEff = originalAura->getAuraEffect(i);
                if (aurEff->getAuraEffectType() == SPELL_AURA_NONE)
                    continue;

#if VERSION_STRING == Classic
                if (aurEff->getAuraEffectType() == SPELL_AURA_PERIODIC_TRIGGER_SPELL)
#else
                if (aurEff->getAuraEffectType() == SPELL_AURA_PERIODIC_TRIGGER_SPELL ||
                    aurEff->getAuraEffectType() == SPELL_AURA_PERIODIC_TRIGGER_SPELL_WITH_VALUE)
#endif
                {
                    ticks = originalAura->getPeriodicTickCountForEffect(aurEff->getEffectIndex());
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

        manaReturn = static_cast<int32_t>(std::round(manaReturn * spellProc->getOverrideEffectDamage(EFF_INDEX_0) / 100.0f));
        return SpellScriptExecuteState::EXECUTE_OK;
    }

    SpellScriptExecuteState onCastProcSpell(SpellProc* /*spellProc*/, Unit* /*caster*/, Unit* /*victim*/, Spell* spell) override
    {
        spell->forced_basepoints->set(EFF_INDEX_0, manaReturn);
        manaReturn = 0;
        return SpellScriptExecuteState::EXECUTE_OK;
    }

private:
    Aura* originalAura = nullptr;
    int32_t manaReturn = 0;
};
#endif

class Polymorph : public SpellScript
{
public:
    SpellScriptExecuteState beforeAuraEffect(Aura* aur, AuraEffectModifier* aurEff, bool apply) override
    {
        if (aurEff->getAuraEffectType() != SPELL_AURA_TRANSFORM)
            return SpellScriptExecuteState::EXECUTE_OK;

        if (apply)
        {
            // Dismount player on aura apply
            // TODO: should also dismount creatures
            if (aur->getPlayerOwner() != nullptr)
                aur->getPlayerOwner()->dismount();

            // Add this unitstate only for player polymorph spells
            // Mostly polymorphs casted by creatures won't regenerate health
            aur->getOwner()->addUnitStateFlag(UNIT_STATE_POLYMORPHED);

            // Glyph of the Penguin
            const auto caster = aur->GetUnitCaster();
            if (caster != nullptr && caster->hasAurasWithId(SPELL_GLYPH_OF_THE_PENGUIN) && aur->getSpellInfo()->getSpellIconID() == ICON_POLYMORPH_SHEEP)
            {
                // Override misc value (Sheep) with Penguin npc
                aurEff->setEffectMiscValue(CREATURE_CHILLY);
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

#if VERSION_STRING >= Cata
    mgr->register_spell_script(SPELL_ARCANE_MISSILES_PROC, new ArcaneMissilesProc);
#endif

#if VERSION_STRING >= WotLK
    mgr->register_spell_script(SPELL_DEEP_FREEZE_DAMAGE, new DeepFreezeDamage);
#endif

#if VERSION_STRING >= WotLK
    mgr->register_spell_script(SPELL_HOT_STREAK_R1, new HotStreakDummy);
    mgr->register_spell_script(SPELL_HOT_STREAK_BUFF, new HotStreak);
#endif

#if VERSION_STRING < Mop
#if VERSION_STRING >= WotLK
    mgr->register_spell_script(SPELL_IMPACT_DUMMY, new ImpactDummy);
#endif
    mgr->register_spell_script(SPELL_IMPACT_STUN, new Impact);
#endif

    mgr->register_spell_script(SPELL_INVISIBILITY, new Invisibility);

#if VERSION_STRING < Mop
    mgr->register_spell_script(SPELL_MASTER_OF_ELEMENTS_R1, new MasterOfElementsDummy);
    mgr->register_spell_script(SPELL_MASTER_OF_ELEMENTS, new MasterOfElements);
#endif

    uint32_t polymorphIds[] =
    {
        SPELL_POLYMORPH_R1,
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
