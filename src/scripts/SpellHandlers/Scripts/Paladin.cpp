/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "Management/ItemInterface.h"
#include "Objects/Item.hpp"
#include "Objects/Units/Unit.hpp"
#include "Objects/Units/Players/Player.hpp"
#include "Spell/Spell.hpp"
#include "Spell/SpellAura.hpp"
#include "Spell/SpellInfo.hpp"
#include "Spell/SpellMgr.hpp"
#include "Spell/SpellScript.hpp"
#include "Spell/SpellTarget.h"
#include "Spell/Definitions/SpellDamageType.hpp"

enum PaladinSpells
{
    SPELL_ART_OF_WAR_PROC_R1                = 53489,
    SPELL_BLOOD_CORRUPTION                  = 53742,
    SPELL_EYE_FOR_AN_EYE_DAMAGE             = 25997,
    SPELL_EYE_FOR_AN_EYE_DUMMY_R1           = 9799,
    SPELL_HOLY_VENGEANCE                    = 31803,
    SPELL_JUDGEMENT_OF_BLOOD_DAMAGE         = 31898,
    SPELL_JUDGEMENT_OF_BLOOD_BACKFIRE       = 32220,
    SPELL_JUDGEMENT_OF_COMMAND_DUMMY_R1     = 20425,
    SPELL_JUDGEMENT_OF_COMMAND_DAMAGE_R1    = 20467,
    SPELL_JUDGEMENT_OF_CORRUPTION           = 53733,
    SPELL_JUDGEMENT_OF_JUSTICE_DUMMY        = 53407,
    SPELL_JUDGEMENT_OF_JUSTICE_DEBUFF       = 20184,
#if VERSION_STRING == WotLK
    SPELL_JUDGEMENT_OF_LIGHT_DUMMY          = 20271,
#else
    SPELL_JUDGEMENT_DUMMY                   = 20271,
#endif
    SPELL_JUDGEMENT_OF_LIGHT_DEBUFF         = 20185,
    SPELL_JUDGEMENT_OF_LIGHT_HEAL           = 20267,
    SPELL_JUDGEMENT_OF_THE_MARTYR_DAMAGE    = 53726,
    SPELL_JUDGEMENT_OF_THE_MARTYR_BACKFIRE  = 53725,
    SPELL_JUDGEMENT_OF_VENGEANCE            = 31804,
    SPELL_JUDGEMENT_OF_WISDOM_DUMMY         = 53408,
    SPELL_JUDGEMENT_OF_WISDOM_DEBUFF        = 20186,
    SPELL_JUDGEMENT_OF_WISDOM_MANA          = 20268,
    SPELL_JUDGEMENT_DAMAGE_GENERIC          = 54158,
    SPELL_SEAL_OF_BLOOD_DUMMY               = 31892,
    SPELL_SEAL_OF_BLOOD_DAMAGE              = 31893,
    SPELL_SEAL_OF_BLOOD_BACKFIRE            = 32221,
#if VERSION_STRING < Mop
    SPELL_SEAL_OF_COMMAND_DUMMY_R1          = 20375,
    SPELL_SEAL_OF_COMMAND_DAMAGE            = 20424,
#else
    SPELL_SEAL_OF_COMMAND_DUMMY_R1          = 105361,
    SPELL_SEAL_OF_COMMAND_DAMAGE            = 118215,
#endif
    SPELL_SEAL_OF_CORRUPTION_DIRECT         = 53739,
    SPELL_SEAL_OF_CORRUPTION_DUMMY          = 53736,
    SPELL_SEAL_OF_JUSTICE_DUMMY_R1          = 20164,
    SPELL_SEAL_OF_JUSTICE_EFFECT            = 20170,
    SPELL_SEAL_OF_LIGHT_DUMMY_R1            = 20165,
    SPELL_SEAL_OF_LIGHT_HEAL_R1             = 20167,
#if VERSION_STRING < Mop
    SPELL_SEAL_OF_RIGHTEOUSNESS_DAMAGE_R1   = 25742,
#else
    SPELL_SEAL_OF_RIGHTEOUSNESS_DAMAGE_R1   = 101423,
#endif
#if VERSION_STRING == Cata
    SPELL_SEAL_OF_RIGHTEOUSNESS_DAMAGE_AOE  = 101423,
#endif
#if VERSION_STRING < Cata
    SPELL_SEAL_OF_RIGHTEOUSNESS_DUMMY_R1    = 21084,
#else
    SPELL_SEAL_OF_RIGHTEOUSNESS_DUMMY_R1    = 20154,
#endif
    SPELL_SEAL_OF_THE_CRUSADER_R1           = 21082,
    SPELL_SEAL_OF_THE_MARTYR_DUMMY          = 53720,
    SPELL_SEAL_OF_THE_MARTYR_DAMAGE         = 53719,
    SPELL_SEAL_OF_THE_MARTYR_BACKFIRE       = 53718,
    SPELL_SEAL_OF_VENGEANCE_DIRECT          = 42463,
    SPELL_SEAL_OF_VENGEANCE_DUMMY           = 31801,
    SPELL_SEAL_OF_WISDOM_DUMMY_R1           = 20166,
    SPELL_SEAL_OF_WISDOM_EFFECT_R1          = 20168,
    SPELL_SEALS_OF_COMMAND_DUMMY            = 85126,
    SPELL_VENGEANCE_PROC_R1                 = 20050,
};

#if VERSION_STRING == WotLK
class ArtOfWar : public SpellScript
{
public:
    void onCreateSpellProc(SpellProc* spellProc, Object* /*obj*/) override
    {
        spellProc->setExtraProcFlags(EXTRA_PROC_ON_CRIT_ONLY);
    }

    bool canProc(SpellProc* /*spellProc*/, Unit* /*victim*/, SpellInfo const* castingspell, DamageInfo /*damageInfo*/) override
    {
        // Should proc only from melee attacks or melee spells
        if (castingspell != nullptr && castingspell->getDmgClass() != SPELL_DMG_TYPE_MELEE)
            return false;

        return true;
    }
};
#endif

class EyeForAnEyeDummy : public SpellScript
{
public:
    SpellScriptCheckDummy onAuraDummyEffect(Aura* aur, AuraEffectModifier* aurEff, bool apply) override
    {
        if (apply)
        {
            auto spellProc = aur->getOwner()->addProcTriggerSpell(sSpellMgr.getSpellInfo(SPELL_EYE_FOR_AN_EYE_DAMAGE), aur, aur->getCasterGuid());
            if (spellProc != nullptr)
            {
                spellProc->setExtraProcFlags(EXTRA_PROC_ON_CRIT_ONLY);
                spellProc->setOverrideEffectDamage(EFF_INDEX_0, aurEff->getEffectDamage());
            }
        }
        else
        {
            aur->getOwner()->removeProcTriggerSpell(SPELL_EYE_FOR_AN_EYE_DAMAGE, aur->getCasterGuid());
        }

        return SpellScriptCheckDummy::DUMMY_OK;
    }
};

class EyeForAnEye : public SpellScript
{
public:
    SpellScriptExecuteState onDoProcEffect(SpellProc* spellProc, Unit* /*victim*/, SpellInfo const* /*castingSpell*/, DamageInfo damageInfo) override
    {
        damage = damageInfo.realDamage * spellProc->getOverrideEffectDamage(EFF_INDEX_0) / 100;
        if (damage == 0)
            return SpellScriptExecuteState::EXECUTE_PREVENT;

        return SpellScriptExecuteState::EXECUTE_OK;
    }

    SpellScriptExecuteState onCastProcSpell(SpellProc* /*spellProc*/, Unit* /*caster*/, Unit* /*victim*/, Spell* spell) override
    {
        spell->forced_basepoints->set(EFF_INDEX_0, damage);
        damage = 0;
        return SpellScriptExecuteState::EXECUTE_OK;
    }

    SpellScriptEffectDamage doCalculateEffect(Spell* spell, uint8_t /*effIndex*/, int32_t* damage) override
    {
        if (spell->getUnitCaster() == nullptr)
            return SpellScriptEffectDamage::DAMAGE_DEFAULT;

        // Damage cannot exceed 50% of paladin's health
        const int32_t maxDmg = spell->getUnitCaster()->getMaxHealth() / 2;
        if (*damage > maxDmg)
            *damage = maxDmg;

        return SpellScriptEffectDamage::DAMAGE_NO_BONUSES;
    }

private:
    uint32_t damage = 0;
};

// TODO: move this to SpellInfo when working with single target auras
static constexpr bool isSealSpell(SpellInfo const* spellInfo)
{
    const auto spellId = spellInfo->hasSpellRanks() ?
        spellInfo->getRankInfo()->getFirstSpell()->getId() :
        spellInfo->getId();

    switch (spellId)
    {
        case SPELL_SEAL_OF_JUSTICE_DUMMY_R1:
        case SPELL_SEAL_OF_LIGHT_DUMMY_R1: // Seal of Light / Seal of Insight
        case SPELL_SEAL_OF_RIGHTEOUSNESS_DUMMY_R1:
        case SPELL_SEAL_OF_COMMAND_DUMMY_R1:
#if VERSION_STRING < Cata
        case SPELL_SEAL_OF_WISDOM_DUMMY_R1:
#endif
#if VERSION_STRING >= TBC
        case SPELL_SEAL_OF_VENGEANCE_DUMMY: // Seal of Vengeance / Seal of Truth
#endif
#if VERSION_STRING == TBC || VERSION_STRING == WotLK
        case SPELL_SEAL_OF_BLOOD_DUMMY:
#endif
#if VERSION_STRING == WotLK
        case SPELL_SEAL_OF_CORRUPTION_DUMMY:
        case SPELL_SEAL_OF_THE_MARTYR_DUMMY:
#endif
#if VERSION_STRING < WotLK
        case SPELL_SEAL_OF_THE_CRUSADER_R1:
#endif
            return true;
        default:
            return false;
    }
}

#if VERSION_STRING < Mop
class JudgementDummy : public SpellScript
{
public:
    bool handleJudgement(Spell const* spell) const
    {
        auto* const unitCaster = spell->getUnitCaster();
        if (unitCaster == nullptr)
            return false;

        auto* const unitTarget = spell->getUnitTarget();
        if (unitTarget == nullptr)
            return false;

        // All seals apply a dummy aura effect that contains spell id for judgement effect
        for (const auto& aurEff : unitCaster->getAuraEffectList(SPELL_AURA_DUMMY))
        {
            if (!isSealSpell(aurEff->getAura()->getSpellInfo()))
                continue;

            // All judgement ids are in the 3rd effect
            if (aurEff->getEffectIndex() != EFF_INDEX_2)
                continue;

            if (aurEff->getEffectDamage() <= 0)
                continue;

            const auto judgementId = static_cast<uint32_t>(aurEff->getEffectDamage());

#if VERSION_STRING < WotLK
            // Remove seal
            aurEff->getAura()->removeAura();
#endif
            // Cast judgement effect
            unitCaster->castSpell(unitTarget, judgementId, true);
            return true;
        }

#if VERSION_STRING < WotLK
        return false;
#else
        // Since wotlk some seals no longer have judgement in spell effects
        // Because player was able to cast this spell it means they have a valid seal
        // Use generic judgement spell
        unitCaster->castSpell(unitTarget, SPELL_JUDGEMENT_DAMAGE_GENERIC, true);
        return true;
#endif
    }

    SpellScriptCheckDummy onDummyOrScriptedEffect(Spell* spell, uint8_t /*effIndex*/) override
    {
#if VERSION_STRING == WotLK
        if (!handleJudgement(spell))
            return SpellScriptCheckDummy::DUMMY_OK;

        // Cast additional judgement effect on target
        uint32_t additionalJudgementId = 0;
        switch (spell->getSpellInfo()->getId())
        {
            case SPELL_JUDGEMENT_OF_LIGHT_DUMMY:
                additionalJudgementId = SPELL_JUDGEMENT_OF_LIGHT_DEBUFF;
                break;
            case SPELL_JUDGEMENT_OF_WISDOM_DUMMY:
                additionalJudgementId = SPELL_JUDGEMENT_OF_WISDOM_DEBUFF;
                break;
            default: // SPELL_JUDGEMENT_OF_JUSTICE_DUMMY
                additionalJudgementId = SPELL_JUDGEMENT_OF_JUSTICE_DEBUFF;
                break;
        }

        // Handle single target check temporarily here
        // There can be only one judgement active at any one time
        if (const auto prevTargetGuid = spell->getUnitCaster()->getSingleTargetGuidForAura(additionalJudgementId))
        {
            if (const auto* const prevTarget = spell->getUnitCaster()->getWorldMapUnit(prevTargetGuid))
                prevTarget->removeAllAurasById(additionalJudgementId);

            spell->getUnitCaster()->removeSingleTargetGuidForAura(additionalJudgementId);
        }
        spell->getUnitCaster()->castSpell(spell->getUnitTarget(), additionalJudgementId, true);
        spell->getUnitCaster()->setSingleTargetGuidForAura(additionalJudgementId, spell->getUnitTarget()->getGuid());
        return SpellScriptCheckDummy::DUMMY_OK;
#else
        handleJudgement(spell);
        return SpellScriptCheckDummy::DUMMY_OK;
#endif
    }
};
#endif

#if VERSION_STRING == TBC || VERSION_STRING == WotLK
class JudgementOfBloodMartyrDamage : public SpellScript
{
public:
    void afterSpellEffect(Spell* spell, uint8_t /*effIndex*/, DamageInfo const& damageInfo) override
    {
        if (spell->getUnitCaster() == nullptr)
            return;
        // todo: should backfire happen on absorbed damage?
        if (damageInfo.realDamage == 0)
            return;
        const auto backFireSpell = spell->getSpellInfo()->getId() == SPELL_JUDGEMENT_OF_THE_MARTYR_DAMAGE ?
            SPELL_JUDGEMENT_OF_THE_MARTYR_BACKFIRE :
            SPELL_JUDGEMENT_OF_BLOOD_BACKFIRE;
        SpellForcedBasePoints forcedBasePoints;
        forcedBasePoints.set(EFF_INDEX_0, static_cast<int32_t>(std::round(damageInfo.realDamage * 0.33f)));
        spell->getUnitCaster()->castSpell(spell->getUnitCaster(), backFireSpell, forcedBasePoints, true);
    }
};
#endif

#if VERSION_STRING < Cata
class JudgementOfCommandDummy : public SpellScript
{
public:
    SpellScriptCheckDummy onDummyOrScriptedEffect(Spell* spell, uint8_t /*effIndex*/) override
    {
        auto* const unitCaster = spell->getUnitCaster();
        if (unitCaster == nullptr)
            return SpellScriptCheckDummy::DUMMY_OK;

        auto* const unitTarget = spell->getUnitTarget();
        if (unitTarget == nullptr)
            return SpellScriptCheckDummy::DUMMY_OK;

        unitCaster->castSpell(unitTarget, static_cast<uint32_t>(spell->damage), true);
        return SpellScriptCheckDummy::DUMMY_OK;
    }
};

#if VERSION_STRING < WotLK
class JudgementOfCommandDamage : public SpellScript
{
public:
    SpellScriptEffectDamage doCalculateEffect(Spell* spell, uint8_t /*effIndex*/, int32_t* damage) override
    {
        if (const auto* const unitTarget = spell->getUnitTarget())
        {
            // Effect has damage for if target is stunned
            if (!unitTarget->hasUnitStateFlag(UNIT_STATE_STUNNED))
                *damage /= 2;
        }
        return SpellScriptEffectDamage::DAMAGE_DEFAULT;
    }
};
#endif
#endif

#if VERSION_STRING < Cata
class JudgementOfLightDummy : public SpellScript
{
public:
#if VERSION_STRING < WotLK
    SpellScriptExecuteState beforeAuraEffect(Aura* aur, AuraEffectModifier* /*aurEff*/, bool apply) override
    {
        // Classic has the proc spell in dbc but in tbc it is a serverside spell
        // Since we do not support serverside spells yet, overwrite default effect here
        const auto* const healSpell = sSpellMgr.getEquivalentSpellRankFor(aur->getSpellInfo(), sSpellMgr.getSpellInfo(SPELL_JUDGEMENT_OF_LIGHT_HEAL));
        if (healSpell == nullptr)
            return SpellScriptExecuteState::EXECUTE_PREVENT;

        if (apply)
            aur->getOwner()->addProcTriggerSpell(healSpell, aur, aur->getCasterGuid());
        else
            aur->getOwner()->removeProcTriggerSpell(healSpell->getId(), aur->getCasterGuid());

        return SpellScriptExecuteState::EXECUTE_PREVENT;
    }
#else // Wotlk
    SpellScriptCheckDummy onAuraDummyEffect(Aura* aur, AuraEffectModifier* aurEff, bool apply) override
    {
        if (apply)
        {
            auto spellProc = aur->getOwner()->addProcTriggerSpell(sSpellMgr.getSpellInfo(SPELL_JUDGEMENT_OF_LIGHT_HEAL), aur, aur->getCasterGuid());
            if (spellProc != nullptr)
                spellProc->setOverrideEffectDamage(EFF_INDEX_0, aurEff->getEffectDamage());
        }
        else
        {
            aur->getOwner()->removeProcTriggerSpell(SPELL_JUDGEMENT_OF_LIGHT_HEAL, aur->getCasterGuid());
        }

        return SpellScriptCheckDummy::DUMMY_OK;
    }
#endif
};

class JudgementOfLightHeal : public SpellScript
{
public:
    void onCreateSpellProc(SpellProc* spellProc, Object* /*obj*/) override
    {
#if VERSION_STRING < WotLK
        // Casted by attacker
        spellProc->setCastedByProcInitiator(true);
#else // Wotlk
        // Casted by paladin
        spellProc->setCastedByProcCreator(true);
#endif
    }

#if VERSION_STRING == WotLK
    uint32_t calcProcChance(SpellProc* /*spellProc*/, Unit* /*victim*/, SpellInfo const* /*castingSpell*/) override
    {
        // TODO: this is a hackfix but almost the correct value
        // should have 15 PPM, but (atm) PPM system works only with melee attacks, and this should proc on spell attacks as well
        // also, if PPM is set, then it calculates proc chance according to aura owner's attack speed, not to attacker's attack speed
        return 50;
    }

    SpellScriptEffectDamage doCalculateEffect(Spell* spell, uint8_t /*effIndex*/, int32_t* damage) override
    {
        if (spell->getUnitTarget() == nullptr)
            return SpellScriptEffectDamage::DAMAGE_DEFAULT;

        *damage = spell->getUnitTarget()->getMaxHealth() * (*damage) / 100;
        return SpellScriptEffectDamage::DAMAGE_NO_BONUSES;
    }
#endif
};
#endif

#if VERSION_STRING >= TBC && VERSION_STRING < Mop
class JudgementOfVengeanceCorruptionTruthDamage : public SpellScript
{
public:
    SpellScriptEffectDamage doCalculateEffect(Spell* spell, [[maybe_unused]]uint8_t effIndex, int32_t* damage) override
    {
        const auto dotSpell = spell->getSpellInfo()->getId() == SPELL_JUDGEMENT_OF_CORRUPTION ?
            SPELL_BLOOD_CORRUPTION :
            SPELL_HOLY_VENGEANCE;

        if (spell->getUnitCaster() == nullptr || spell->getUnitTarget() == nullptr)
            return SpellScriptEffectDamage::DAMAGE_DEFAULT;

        // todo: confirm if can be judged from other paladins dots
        auto auraCount = spell->getUnitTarget()->getAuraCountForId(dotSpell);
        if (auraCount > 5)
            auraCount = 5;

#if VERSION_STRING == TBC
        // Damage multiplies per stack of dot
        if (auraCount > 1)
            *damage *= auraCount;

        return SpellScriptEffectDamage::DAMAGE_DEFAULT;
#else // Wotlk and cata
        // Calculate bonuses here to increase damage properly
        *damage = static_cast<int32_t>(std::round(spell->getUnitCaster()->applySpellDamageBonus(spell->getUnitCaster(), spell->getSpellInfo(), effIndex, *damage, 1.0f, false, spell)));
#if VERSION_STRING == WotLK
        // One stack increases damage by 10%
        *damage = *damage * (100 + (10 * auraCount)) / 100;
#else // Cata
        // One stack increases damage by 20%
        *damage = *damage * (100 + (20 * auraCount)) / 100;
#endif
        return SpellScriptEffectDamage::DAMAGE_NO_BONUSES;
#endif
    }
};
#endif

#if VERSION_STRING < Cata
class JudgementOfWisdomDummy : public SpellScript
{
public:
#if VERSION_STRING < WotLK
    SpellScriptExecuteState beforeAuraEffect(Aura* aur, AuraEffectModifier* /*aurEff*/, bool apply) override
    {
        // Classic has the proc spell in dbc but in tbc it is a serverside spell
        // Since we do not support serverside spells yet, overwrite default effect here
        const auto* const manaSpell = sSpellMgr.getEquivalentSpellRankFor(aur->getSpellInfo(), sSpellMgr.getSpellInfo(SPELL_JUDGEMENT_OF_WISDOM_MANA));
        if (manaSpell == nullptr)
            return SpellScriptExecuteState::EXECUTE_PREVENT;

        if (apply)
            aur->getOwner()->addProcTriggerSpell(manaSpell, aur, aur->getCasterGuid());
        else
            aur->getOwner()->removeProcTriggerSpell(manaSpell->getId(), aur->getCasterGuid());

        return SpellScriptExecuteState::EXECUTE_PREVENT;
    }
#else // Wotlk
    SpellScriptCheckDummy onAuraDummyEffect(Aura* aur, AuraEffectModifier* /*aurEff*/, bool apply) override
    {
        if (apply)
            aur->getOwner()->addProcTriggerSpell(sSpellMgr.getSpellInfo(SPELL_JUDGEMENT_OF_WISDOM_MANA), aur, aur->getCasterGuid());
        else
            aur->getOwner()->removeProcTriggerSpell(SPELL_JUDGEMENT_OF_WISDOM_MANA, aur->getCasterGuid());

        return SpellScriptCheckDummy::DUMMY_OK;
    }
#endif
};

class JudgementOfWisdomMana : public SpellScript
{
public:
    void onCreateSpellProc(SpellProc* spellProc, Object* /*obj*/) override
    {
#if VERSION_STRING < WotLK
        // Casted by attacker
        spellProc->setCastedByProcInitiator(true);
#else // Wotlk
        // Casted by paladin
        spellProc->setCastedByProcCreator(true);
#endif
    }

    bool canProc(SpellProc* /*spellProc*/, Unit* victim, SpellInfo const* /*castingSpell*/, DamageInfo /*dmg*/) override
    {
        if (victim == nullptr || victim->getPowerType() != POWER_TYPE_MANA)
            return false;

        return true;
    }

#if VERSION_STRING == WotLK
    uint32_t calcProcChance(SpellProc* /*spellProc*/, Unit* /*victim*/, SpellInfo const* /*castingSpell*/) override
    {
        // TODO: this is a hackfix but almost the correct value
        // should have 15 PPM, but PPM system works only with melee attacks, and this should proc on spell attacks as well
        // also, if PPM is set, then it calculates proc chance according to aura owner's attack speed, not to attacker's attack speed
        return 50;
    }

    SpellScriptEffectDamage doCalculateEffect(Spell* spell, uint8_t /*effIndex*/, int32_t* damage) override
    {
        if (spell->getUnitTarget() == nullptr)
            return SpellScriptEffectDamage::DAMAGE_DEFAULT;

        *damage = spell->getUnitTarget()->getBaseMana() * (*damage) / 100;
        return SpellScriptEffectDamage::DAMAGE_NO_BONUSES;
    }
#endif
};
#endif

#if VERSION_STRING == TBC || VERSION_STRING == WotLK
class SealOfBloodAndMartyrDummy : public SpellScript
{
public:
    SpellScriptCheckDummy onAuraDummyEffect(Aura* aur, AuraEffectModifier* aurEff, bool apply) override
    {
        if (aurEff->getEffectIndex() == EFF_INDEX_0)
        {
            const auto damageSpellId = aur->getSpellId() == SPELL_SEAL_OF_THE_MARTYR_DUMMY ?
                SPELL_SEAL_OF_THE_MARTYR_DAMAGE :
                SPELL_SEAL_OF_BLOOD_DAMAGE;

            if (apply)
                aur->getOwner()->addProcTriggerSpell(sSpellMgr.getSpellInfo(damageSpellId), aur, aur->getCasterGuid());
            else
                aur->getOwner()->removeProcTriggerSpell(damageSpellId, aur->getCasterGuid());
        }
        else if (aurEff->getEffectIndex() == EFF_INDEX_1)
        {
            const auto backfireSpellId = aur->getSpellId() == SPELL_SEAL_OF_THE_MARTYR_DUMMY ?
                SPELL_SEAL_OF_THE_MARTYR_BACKFIRE :
                SPELL_SEAL_OF_BLOOD_BACKFIRE;

            if (apply)
            {
                // Create a custom proc for the backfire effect that procs on damage spell
                auto* const backfireProc = aur->getOwner()->addProcTriggerSpell(sSpellMgr.getSpellInfo(backfireSpellId), aur->getSpellInfo(), aur->getCasterGuid(), 100, PROC_ON_DONE_MELEE_SPELL_HIT, nullptr, aur);
                backfireProc->setProcClassMask(EFF_INDEX_0, 0x0);
                backfireProc->setProcClassMask(EFF_INDEX_1, 0x400);
                backfireProc->setProcClassMask(EFF_INDEX_2, 0x0);
                // Add to 2nd index so calculated damage can be put in 1st index
                backfireProc->setOverrideEffectDamage(EFF_INDEX_1, aurEff->getEffectDamage());
                backfireProc->setCastedOnProcOwner(true);
            }
            else
            {
                aur->getOwner()->removeProcTriggerSpell(backfireSpellId, aur->getCasterGuid());
            }
        }

        return SpellScriptCheckDummy::DUMMY_OK;
    }
};

class SealOfBloodAndMartyrBackfire : public SpellScript
{
public:
    bool canProc(SpellProc* /*spellProc*/, Unit* /*victim*/, SpellInfo const* castingSpell, DamageInfo damageInfo) override
    {
        if (castingSpell == nullptr)
            return false;
        // Dummy spell has same family flags as damage spell
        if (castingSpell->getId() != SPELL_SEAL_OF_THE_MARTYR_DAMAGE && castingSpell->getId() != SPELL_SEAL_OF_BLOOD_DAMAGE)
            return false;
        // TODO: should backfire happen on absorb as well?
        return damageInfo.realDamage > 0;
    }

    bool canProcOnTriggered(SpellProc* /*spellProc*/, Unit* /*victim*/, SpellInfo const* /*castingSpell*/, Aura* /*triggeredFromAura*/) override
    {
        // The spell that is triggering this proc is also a triggered spell
        return true;
    }

    SpellScriptExecuteState onDoProcEffect(SpellProc* spellProc, Unit* /*victim*/, SpellInfo const* /*castingSpell*/, DamageInfo damageInfo) override
    {
        const auto backfireDmg = static_cast<int32_t>(std::round(damageInfo.realDamage * spellProc->getOverrideEffectDamage(EFF_INDEX_1) / 100));
        spellProc->setOverrideEffectDamage(EFF_INDEX_0, std::max(backfireDmg, 1));
        return SpellScriptExecuteState::EXECUTE_OK;
    }
};
#endif

#if VERSION_STRING >= Mop
class SealOfCommandDummy : public SpellScript
{
public:
    SpellScriptCheckDummy onAuraDummyEffect(Aura* aur, AuraEffectModifier* aurEff, bool apply) override
    {
        if (aurEff->getEffectIndex() != EFF_INDEX_0)
            return SpellScriptCheckDummy::DUMMY_OK;

        if (apply)
        {
            aur->getOwner()->addProcTriggerSpell(sSpellMgr.getSpellInfo(SPELL_SEAL_OF_COMMAND_DAMAGE), aur, aur->getCasterGuid());
        }
        else
        {
            aur->getOwner()->removeProcTriggerSpell(SPELL_SEAL_OF_COMMAND_DAMAGE, aur->getCasterGuid());
        }

        return SpellScriptCheckDummy::DUMMY_OK;
    }
};
#endif

#if VERSION_STRING < WotLK || VERSION_STRING == Cata
class SealOfCommandDamage : public SpellScript
{
public:
    void onCreateSpellProc(SpellProc* spellProc, Object* /*obj*/) override
    {
#if VERSION_STRING < WotLK
        // 7 PPM
        spellProc->setProcsPerMinute(7.f);
#else // Cata
        // Procs only from Seal of Righteousness, Seal of Truth and Seal of Justice
        spellProc->setProcClassMask(EFF_INDEX_0, 0x20000200);
        spellProc->setProcClassMask(EFF_INDEX_1, 0x20000800);
        spellProc->setProcClassMask(EFF_INDEX_2, 0x10000);
#endif
    }
};
#endif

#if VERSION_STRING == Cata
class SealOfJusticeDummy : public SpellScript
{
public:
    SpellScriptCheckDummy onAuraDummyEffect(Aura* aur, AuraEffectModifier* aurEff, bool apply) override
    {
        if (aurEff->getEffectIndex() != EFF_INDEX_0)
            return SpellScriptCheckDummy::DUMMY_OK;

        if (apply)
            aur->getOwner()->addProcTriggerSpell(sSpellMgr.getSpellInfo(SPELL_SEAL_OF_JUSTICE_EFFECT), aur, aur->getCasterGuid());
        else
            aur->getOwner()->removeProcTriggerSpell(SPELL_SEAL_OF_JUSTICE_EFFECT, aur->getCasterGuid());

        return SpellScriptCheckDummy::DUMMY_OK;
    }
};
#endif

#if VERSION_STRING < Mop
class SealOfJusticeEffect : public SpellScript
{
public:
#if VERSION_STRING < Cata
    void onCreateSpellProc(SpellProc* spellProc, Object* /*obj*/) override
    {
        // 5 PPM
        spellProc->setProcsPerMinute(5.f);
    }
#endif

#if VERSION_STRING == Cata
    // Stun effect was replaced by damage effect in Cata
    SpellScriptEffectDamage doCalculateEffect(Spell* spell, uint8_t effIndex, int32_t* damage) override
    {
        const auto* const unitCaster = spell->getUnitCaster();
        if (unitCaster == nullptr)
            return SpellScriptEffectDamage::DAMAGE_DEFAULT;

        float_t speed = 2.0f;
        if (const auto* const plrCaster = spell->getPlayerCaster())
        {
            if (const auto* const mainhandWeapon = plrCaster->getItemInterface()->GetInventoryItem(EQUIPMENT_SLOT_MAINHAND))
                speed = mainhandWeapon->getItemProperties()->Delay / 1000.0f;
        }
        // Weapon speed * (0.005 * AP + 0.01 * SPH)
        const auto spCoeff = spell->getSpellInfo()->getEffectSpellPowerCoefficient(effIndex);
        const auto apCoeff = spell->getSpellInfo()->getAttackPowerCoefficient();
        *damage = static_cast<int32_t>(std::round(speed * (apCoeff * unitCaster->getCalculatedAttackPower() + spCoeff * unitCaster->GetDamageDoneMod(SCHOOL_HOLY))));
        return SpellScriptEffectDamage::DAMAGE_NO_BONUSES;
    }
#endif
};
#endif

class SealOfLightAndInsightHeal : public SpellScript
{
public:
    void onCreateSpellProc(SpellProc* spellProc, Object* /*obj*/) override
    {
#if VERSION_STRING < Cata
        // 15 PPM
        spellProc->setProcsPerMinute(15.f);
#else
        // 8.5 PPM according to wowhead comment from patch 4.2.2
        spellProc->setProcsPerMinute(8.5f);
#endif
        spellProc->setCastedOnProcOwner(true);
    }
};

class SealOfRighteousnessDummy : public SpellScript
{
public:
    SpellScriptCheckDummy onAuraDummyEffect(Aura* aur, AuraEffectModifier* aurEff, bool apply) override
    {
        if (aurEff->getEffectIndex() != EFF_INDEX_0)
            return SpellScriptCheckDummy::DUMMY_OK;

        const auto* damageSpell = sSpellMgr.getEquivalentSpellRankFor(aur->getSpellInfo(), sSpellMgr.getSpellInfo(SPELL_SEAL_OF_RIGHTEOUSNESS_DAMAGE_R1));
#if VERSION_STRING == Cata
        // Seals of Command talent
        if (aur->getOwner()->hasAurasWithId(SPELL_SEALS_OF_COMMAND_DUMMY))
            damageSpell = sSpellMgr.getSpellInfo(SPELL_SEAL_OF_RIGHTEOUSNESS_DAMAGE_AOE);
#endif
        if (damageSpell == nullptr)
            return SpellScriptCheckDummy::DUMMY_OK;

        if (apply)
        {
#if VERSION_STRING >= WotLK
            aur->getOwner()->addProcTriggerSpell(damageSpell, aur, aur->getCasterGuid());
#else
            auto spellProc = aur->getOwner()->addProcTriggerSpell(damageSpell, aur, aur->getCasterGuid());
            if (spellProc != nullptr)
                spellProc->setOverrideEffectDamage(aurEff->getEffectIndex(), aurEff->getEffectDamage());
#endif
        }
        else
        {
            aur->getOwner()->removeProcTriggerSpell(damageSpell->getId(), aur->getCasterGuid());
        }

        return SpellScriptCheckDummy::DUMMY_OK;
    }
};

#if VERSION_STRING < Mop
class SealOfRighteousnessDamage : public SpellScript
{
public:
    SpellScriptEffectDamage doCalculateEffect(Spell* spell, uint8_t effIndex, int32_t* damage) override
    {
        const auto* const unitCaster = spell->getUnitCaster();
        if (unitCaster == nullptr)
            return SpellScriptEffectDamage::DAMAGE_DEFAULT;

        float_t speed = 2.0f;
        const auto* const mainHandWeapon = spell->getPlayerCaster() != nullptr ? spell->getPlayerCaster()->getItemInterface()->GetInventoryItem(EQUIPMENT_SLOT_MAINHAND) : nullptr;
        if (mainHandWeapon != nullptr)
            speed = mainHandWeapon->getItemProperties()->Delay / 1000.f;

        auto spellCoeff = spell->getSpellInfo()->getEffectSpellPowerCoefficient(effIndex);
#if VERSION_STRING < WotLK
        float_t baseDamage = 0.0f;
        if (mainHandWeapon != nullptr && mainHandWeapon->getItemProperties()->InventoryType == INVTYPE_2HWEAPON)
        {
            // Two handed weapon
            baseDamage = 1.2f * (*damage * speed / 100.f) + 0.03f * (unitCaster->getMaxDamage() + unitCaster->getMinDamage()) / 2 + 1;
        }
        else
        {
            // One handed weapon
            baseDamage = 0.852f * (*damage * speed / 100.f) + 0.03f * (unitCaster->getMaxDamage() + unitCaster->getMinDamage()) / 2 - 1;
            spellCoeff *= 0.852f;
        }

        *damage = static_cast<int32_t>(std::round(baseDamage + (spellCoeff * unitCaster->GetDamageDoneMod(SCHOOL_HOLY))));
#else
        // Wotlk: Weapon speed * (0.022 * AP + 0.044 * SPH)
        // Cata: Weapon speed * (0.011 * AP + 0.022 * SPH)
        const auto apCoeff = spell->getSpellInfo()->getAttackPowerCoefficient();
        *damage = static_cast<int32_t>(std::round(speed * (apCoeff * unitCaster->getCalculatedAttackPower() + spellCoeff * unitCaster->GetDamageDoneMod(SCHOOL_HOLY))));
#endif
        return SpellScriptEffectDamage::DAMAGE_NO_BONUSES;
    }
};
#endif

#if VERSION_STRING < WotLK
class SealOfTheCrusader : public SpellScript
{
public:
    SpellScriptExecuteState beforeAuraEffect(Aura* aur, AuraEffectModifier* aurEff, bool apply) override
    {
        if (aurEff->getEffectIndex() != EFF_INDEX_1)
            return SpellScriptExecuteState::EXECUTE_NOT_HANDLED;

        // Aura effect makes attack speed 40% faster
        // but aura should also make paladin deal less damage per swing
        // However DPS should remain same before the AP bonus
        // so my calculations give ~28.57% for dmg reduction

        constexpr float_t dmgReductionPct = 0.2857143f;
        if (auto* const playerOwner = aur->getPlayerOwner())
        {
            if (apply)
                playerOwner->m_damageDone.try_emplace(aur->getSpellId(), -1, -1, -dmgReductionPct);
            else
                playerOwner->m_damageDone.erase(aur->getSpellId());

            // Display to client
            playerOwner->modModDamageDonePct(apply ? -dmgReductionPct : dmgReductionPct, SCHOOL_NORMAL);
        }
        else if (auto* const creatureOwner = aur->getOwner()->ToCreature())
        {
            creatureOwner->ModDamageDonePct[SCHOOL_NORMAL] += apply ? -dmgReductionPct : dmgReductionPct;
        }

        return SpellScriptExecuteState::EXECUTE_OK;
    }
};
#endif

#if VERSION_STRING >= TBC
class SealOfVengeanceAndCorruptionAndTruthDirect : public SpellScript
{
public:
    bool canProc(SpellProc* spellProc, Unit* victim, SpellInfo const* /*castingSpell*/, DamageInfo /*damageInfo*/) override
    {
        if (victim == nullptr)
            return false;

        uint32_t auraId = SPELL_HOLY_VENGEANCE;
        if (spellProc->getSpell()->getId() == SPELL_SEAL_OF_CORRUPTION_DIRECT)
            auraId = SPELL_BLOOD_CORRUPTION;

        const auto* const aur = victim->getAuraWithId(auraId);
        if (aur == nullptr)
            return false;

#if VERSION_STRING == TBC
        // As of Patch 2.2.0, Seal of Vengeance deals a small amount of Holy Damage if it strikes a target with 5 stacks of the debuff already applied.
        if (aur->getStackCount() < 5)
            return false;
#endif

        return true;
    }

#if VERSION_STRING >= WotLK
    SpellScriptEffectDamage doCalculateEffect(Spell* spell, uint8_t /*effIndex*/, int32_t* damage) override
    {
        if (spell->getUnitTarget() == nullptr)
            return SpellScriptEffectDamage::DAMAGE_DEFAULT;

        uint32_t auraId = SPELL_HOLY_VENGEANCE;
        if (spell->getSpellInfo()->getId() == SPELL_SEAL_OF_CORRUPTION_DIRECT)
            auraId = SPELL_BLOOD_CORRUPTION;

#if VERSION_STRING == WotLK
        // According to WoWhead from 3.2.0 patch, all auto attacks and special attacks can proc this
        // Weapon damage starts at 6.6% and goes up to 33% if target has five stacks of debuff
        float_t dmgPercent = 6.6f;
#elif VERSION_STRING == Cata
        // Patch 4.3.3 comment
        // Weapon damage starts at 3% and goes up to 15% if target has five stacks of debuff
        float_t dmgPercent = 3.f;
#elif VERSION_STRING == Mop
        // Weapon damage starts at 2.4% and goes up to 12% if target has five stacks of debuff
        float_t dmgPercent = 2.4f;
#endif

        if (const auto* const dotAur = spell->getUnitTarget()->getAuraWithId(auraId))
            dmgPercent *= dotAur->getStackCount();

        *damage = static_cast<int32_t>(std::round(dmgPercent));
        return SpellScriptEffectDamage::DAMAGE_NO_BONUSES;
    }
#endif
};
#endif

#if VERSION_STRING >= TBC
class SealOfVengeanceAndCorruptionAndTruthDot : public SpellScript
{
public:
#if VERSION_STRING == TBC
    void onCreateSpellProc(SpellProc* proc, Object* /*obj*/) override
    {
        // 20 PPM
        proc->setProcsPerMinute(20.f);
    }
#endif

#if VERSION_STRING >= WotLK
    bool canProc(SpellProc* /*spellProc*/, Unit* /*victim*/, SpellInfo const* castingSpell, DamageInfo /*damageInfo*/) override
    {
        if (castingSpell != nullptr)
        {
#if VERSION_STRING < Cata
            // Dot portion should only be applied on melee hits and on Hammer of the Righteous
            if (castingSpell->getSpellFamilyFlags(EFF_INDEX_0) == 0 &&
                castingSpell->getSpellFamilyFlags(EFF_INDEX_1) == 0x40000 &&
                castingSpell->getSpellFamilyFlags(EFF_INDEX_2) == 0)
                return true;

            return false;
#else
            // Procs only from melee attacks and single target spells
            const auto targetMask = castingSpell->getRequiredTargetMask(true);
            if (targetMask & SPELL_TARGET_AREA_MASK)
                return false;

            // Cannot proc from Avenger's Shield
            if (castingSpell->getSpellFamilyFlags(EFF_INDEX_0) == 0x4000 &&
                castingSpell->getSpellFamilyFlags(EFF_INDEX_1) == 0 &&
                castingSpell->getSpellFamilyFlags(EFF_INDEX_2) == 0)
                return false;
#endif
        }

        return true;
    }
#endif
};
#endif

#if VERSION_STRING >= TBC
class SealOfVengeanceAndCorruptionAndTruthDummy : public SpellScript
{
public:
    SpellScriptCheckDummy onAuraDummyEffect(Aura* aur, AuraEffectModifier* aurEff, bool apply) override
    {
        if (aurEff->getEffectIndex() != EFF_INDEX_0)
            return SpellScriptCheckDummy::DUMMY_OK;

        const auto dotSpellId = aur->getSpellId() == SPELL_SEAL_OF_CORRUPTION_DUMMY ?
            SPELL_BLOOD_CORRUPTION :
            SPELL_HOLY_VENGEANCE;
        const auto directSpellId = aur->getSpellId() == SPELL_SEAL_OF_CORRUPTION_DUMMY ?
            SPELL_SEAL_OF_CORRUPTION_DIRECT :
            SPELL_SEAL_OF_VENGEANCE_DIRECT;

        if (apply)
        {
            aur->getOwner()->addProcTriggerSpell(sSpellMgr.getSpellInfo(dotSpellId), aur, aur->getCasterGuid());
            aur->getOwner()->addProcTriggerSpell(sSpellMgr.getSpellInfo(directSpellId), aur, aur->getCasterGuid());
        }
        else
        {
            aur->getOwner()->removeProcTriggerSpell(dotSpellId, aur->getCasterGuid());
            aur->getOwner()->removeProcTriggerSpell(directSpellId, aur->getCasterGuid());
        }

        return SpellScriptCheckDummy::DUMMY_OK;
    }
};
#endif

#if VERSION_STRING < Cata
class SealOfWisdomEffect : public SpellScript
{
public:
    void onCreateSpellProc(SpellProc* proc, Object* /*obj*/) override
    {
        // 15 PPM
        proc->setProcsPerMinute(15.f);
        proc->setCastedOnProcOwner(true);
    }
};
#endif

#if VERSION_STRING < Cata
class Vengeance : public SpellScript
{
public:
    void onCreateSpellProc(SpellProc* spellProc, Object* /*obj*/) override
    {
        spellProc->setExtraProcFlags(EXTRA_PROC_ON_CRIT_ONLY);
    }
};
#endif

void setupPaladinSpells(ScriptMgr* mgr)
{
    // Call legacy script setup
    SetupLegacyPaladinSpells(mgr);

#if VERSION_STRING == WotLK
    mgr->register_spell_script(SPELL_ART_OF_WAR_PROC_R1, new ArtOfWar);
#endif

    mgr->register_spell_script(SPELL_EYE_FOR_AN_EYE_DUMMY_R1, new EyeForAnEyeDummy);
    mgr->register_spell_script(SPELL_EYE_FOR_AN_EYE_DAMAGE, new EyeForAnEye);

#if VERSION_STRING == WotLK
    uint32_t judgementIds[] =
    {
        SPELL_JUDGEMENT_OF_LIGHT_DUMMY,
        SPELL_JUDGEMENT_OF_WISDOM_DUMMY,
        SPELL_JUDGEMENT_OF_JUSTICE_DUMMY,
        0
    };
    mgr->register_spell_script(judgementIds, new JudgementDummy);
#elif VERSION_STRING < Mop
    mgr->register_spell_script(SPELL_JUDGEMENT_DUMMY, new JudgementDummy);
#endif

#if VERSION_STRING == TBC || VERSION_STRING == WotLK
    uint32_t judgementOfBloodMartyrDamage[] =
    {
        SPELL_JUDGEMENT_OF_BLOOD_DAMAGE,
#if VERSION_STRING == WotLK
        SPELL_JUDGEMENT_OF_THE_MARTYR_DAMAGE,
#endif
        0
    };
    mgr->register_spell_script(judgementOfBloodMartyrDamage, new JudgementOfBloodMartyrDamage);
#endif

#if VERSION_STRING < Cata
    mgr->register_spell_script(SPELL_JUDGEMENT_OF_COMMAND_DUMMY_R1, new JudgementOfCommandDummy);
#if VERSION_STRING < WotLK
    mgr->register_spell_script(SPELL_JUDGEMENT_OF_COMMAND_DAMAGE_R1, new JudgementOfCommandDamage);
#endif
#endif

#if VERSION_STRING < Cata
    mgr->register_spell_script(SPELL_JUDGEMENT_OF_LIGHT_DEBUFF, new JudgementOfLightDummy);
    mgr->register_spell_script(SPELL_JUDGEMENT_OF_LIGHT_HEAL, new JudgementOfLightHeal);
#endif

#if VERSION_STRING >= TBC && VERSION_STRING < Mop
    uint32_t judgementOfVengeanceCorruptionTruth[] =
    {
        SPELL_JUDGEMENT_OF_VENGEANCE,
#if VERSION_STRING == WotLK
        SPELL_JUDGEMENT_OF_CORRUPTION,
#endif
        0
    };
    mgr->register_spell_script(judgementOfVengeanceCorruptionTruth, new JudgementOfVengeanceCorruptionTruthDamage);
#endif

#if VERSION_STRING < Cata
    mgr->register_spell_script(SPELL_JUDGEMENT_OF_WISDOM_DEBUFF, new JudgementOfWisdomDummy);
    mgr->register_spell_script(SPELL_JUDGEMENT_OF_WISDOM_MANA, new JudgementOfWisdomMana);
#endif

#if VERSION_STRING == TBC || VERSION_STRING == WotLK
    uint32_t sealOfBloodMartyrDummy[] =
    {
        SPELL_SEAL_OF_BLOOD_DUMMY,
#if VERSION_STRING == WotLK
        SPELL_SEAL_OF_THE_MARTYR_DUMMY,
#endif
        0
    };
    mgr->register_spell_script(sealOfBloodMartyrDummy, new SealOfBloodAndMartyrDummy);
    uint32_t sealOfBloodMartyrBackfire[] =
    {
        SPELL_SEAL_OF_BLOOD_BACKFIRE,
#if VERSION_STRING == WotLK
        SPELL_SEAL_OF_THE_MARTYR_BACKFIRE,
#endif
        0
    };
    mgr->register_spell_script(sealOfBloodMartyrBackfire, new SealOfBloodAndMartyrBackfire);
#endif

#if VERSION_STRING >= Mop
    mgr->register_spell_script(SPELL_SEAL_OF_COMMAND_DUMMY_R1, new SealOfCommandDummy);
#endif
#if VERSION_STRING < WotLK || VERSION_STRING == Cata
    mgr->register_spell_script(SPELL_SEAL_OF_COMMAND_DAMAGE, new SealOfCommandDamage);
#endif

#if VERSION_STRING == Cata
    mgr->register_spell_script(SPELL_SEAL_OF_JUSTICE_DUMMY_R1, new SealOfJusticeDummy);
#endif
#if VERSION_STRING < Mop
    mgr->register_spell_script(SPELL_SEAL_OF_JUSTICE_EFFECT, new SealOfJusticeEffect);
#endif

    mgr->register_spell_script(SPELL_SEAL_OF_LIGHT_HEAL_R1, new SealOfLightAndInsightHeal);

    mgr->register_spell_script(SPELL_SEAL_OF_RIGHTEOUSNESS_DUMMY_R1, new SealOfRighteousnessDummy);
#if VERSION_STRING < Mop
    uint32_t sealOfRighteousnessDamage[] =
    {
        SPELL_SEAL_OF_RIGHTEOUSNESS_DAMAGE_R1,
#if VERSION_STRING == Cata
        SPELL_SEAL_OF_RIGHTEOUSNESS_DAMAGE_AOE,
#endif
        0
    };
    mgr->register_spell_script(sealOfRighteousnessDamage, new SealOfRighteousnessDamage);
#endif

#if VERSION_STRING < WotLK
    mgr->register_spell_script(SPELL_SEAL_OF_THE_CRUSADER_R1, new SealOfTheCrusader);
#endif

#if VERSION_STRING >= TBC
    uint32_t sealOfVengeanceCorruptionTruthDirect[] =
    {
        SPELL_SEAL_OF_VENGEANCE_DIRECT,
#if VERSION_STRING == WotLK
        SPELL_SEAL_OF_CORRUPTION_DIRECT,
#endif
        0
    };
    mgr->register_spell_script(sealOfVengeanceCorruptionTruthDirect, new SealOfVengeanceAndCorruptionAndTruthDirect);
    uint32_t sealOfVengeanceCorruptionTruthDummy[] =
    {
        SPELL_SEAL_OF_VENGEANCE_DUMMY,
#if VERSION_STRING == WotLK
        SPELL_SEAL_OF_CORRUPTION_DUMMY,
#endif
        0
    };
    mgr->register_spell_script(sealOfVengeanceCorruptionTruthDummy, new SealOfVengeanceAndCorruptionAndTruthDummy);
    uint32_t sealOfVengeanceCorruptionTruthDot[] =
    {
        SPELL_HOLY_VENGEANCE,
#if VERSION_STRING == WotLK
        SPELL_BLOOD_CORRUPTION,
#endif
        0
    };
    mgr->register_spell_script(sealOfVengeanceCorruptionTruthDot, new SealOfVengeanceAndCorruptionAndTruthDot);
#endif

#if VERSION_STRING < Cata
    mgr->register_spell_script(SPELL_SEAL_OF_WISDOM_EFFECT_R1, new SealOfWisdomEffect);
#endif

#if VERSION_STRING < Cata
    mgr->register_spell_script(SPELL_VENGEANCE_PROC_R1, new Vengeance);
#endif
}
