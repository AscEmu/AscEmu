/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include <algorithm>

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
#include "Utilities/Narrow.hpp"

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
    SPELL_JUDGEMENT_OF_LIGHT_DUMMY          = 20271, // WotLK
    SPELL_JUDGEMENT_DUMMY                   = 20271, // Cata and later
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
    SPELL_SEAL_OF_COMMAND_DUMMY_R1          = 20375,
    SPELL_SEAL_OF_COMMAND_DAMAGE            = 20424,
    SPELL_SEAL_OF_COMMAND_DUMMY_R1_MOP      = 105361,
    SPELL_SEAL_OF_COMMAND_DAMAGE_MOP        = 118215,
    SPELL_SEAL_OF_CORRUPTION_DIRECT         = 53739,
    SPELL_SEAL_OF_CORRUPTION_DUMMY          = 53736,
    SPELL_SEAL_OF_JUSTICE_DUMMY_R1          = 20164,
    SPELL_SEAL_OF_JUSTICE_EFFECT            = 20170,
    SPELL_SEAL_OF_LIGHT_DUMMY_R1            = 20165,
    SPELL_SEAL_OF_LIGHT_HEAL_R1             = 20167,
    SPELL_SEAL_OF_RIGHTEOUSNESS_DAMAGE_R1   = 25742,
    SPELL_SEAL_OF_RIGHTEOUSNESS_DAMAGE_MOP  = 101423,
    SPELL_SEAL_OF_RIGHTEOUSNESS_DAMAGE_AOE  = 101423, // Cata
    SPELL_SEAL_OF_RIGHTEOUSNESS_DUMMY_R1    = 21084,
    SPELL_SEAL_OF_RIGHTEOUSNESS_DUMMY_CATA  = 20154,
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

// the seal ids that changed with Cata and Mop
static uint32_t sealOfCommandDummyId() { return WoW::isServerExpansionAtLeast(WoW::Expansion::_Mop) ? SPELL_SEAL_OF_COMMAND_DUMMY_R1_MOP : SPELL_SEAL_OF_COMMAND_DUMMY_R1; }
static uint32_t sealOfCommandDamageId() { return WoW::isServerExpansionAtLeast(WoW::Expansion::_Mop) ? SPELL_SEAL_OF_COMMAND_DAMAGE_MOP : SPELL_SEAL_OF_COMMAND_DAMAGE; }
static uint32_t sealOfRighteousnessDamageId() { return WoW::isServerExpansionAtLeast(WoW::Expansion::_Mop) ? SPELL_SEAL_OF_RIGHTEOUSNESS_DAMAGE_MOP : SPELL_SEAL_OF_RIGHTEOUSNESS_DAMAGE_R1; }
static uint32_t sealOfRighteousnessDummyId() { return WoW::isServerExpansionAtLeast(WoW::Expansion::_Cata) ? SPELL_SEAL_OF_RIGHTEOUSNESS_DUMMY_CATA : SPELL_SEAL_OF_RIGHTEOUSNESS_DUMMY_R1; }

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

    SpellScriptEffectDamage doCalculateEffect(Spell* spell, uint8_t /*effIndex*/, int32_t* effectDamage) override
    {
        if (spell->getUnitCaster() == nullptr)
            return SpellScriptEffectDamage::DAMAGE_DEFAULT;

        // Damage cannot exceed 50% of paladin's health
        const int32_t maxDmg = spell->getUnitCaster()->getMaxHealth() / 2;
        *effectDamage = std::min(*effectDamage, maxDmg);

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
        case SPELL_SEAL_OF_RIGHTEOUSNESS_DUMMY_CATA:
        case SPELL_SEAL_OF_COMMAND_DUMMY_R1:
        case SPELL_SEAL_OF_COMMAND_DUMMY_R1_MOP:
        if (!WoW::isServerExpansionAtLeast(WoW::Expansion::_Cata))
        {
            case SPELL_SEAL_OF_WISDOM_DUMMY_R1:
        }
        if (WoW::isServerExpansionAtLeast(WoW::Expansion::_TBC))
        {
            case SPELL_SEAL_OF_VENGEANCE_DUMMY: // Seal of Vengeance / Seal of Truth
        }
        if (WoW::isServerExpansion(WoW::Expansion::_TBC) || WoW::isServerExpansion(WoW::Expansion::_WotLK))
        {
            case SPELL_SEAL_OF_BLOOD_DUMMY:
        }
        if (WoW::isServerExpansion(WoW::Expansion::_WotLK))
        {
            case SPELL_SEAL_OF_CORRUPTION_DUMMY:
            case SPELL_SEAL_OF_THE_MARTYR_DUMMY:
        }
        if (!WoW::isServerExpansionAtLeast(WoW::Expansion::_WotLK))
        {
            case SPELL_SEAL_OF_THE_CRUSADER_R1:
        }
            return true;
        default:
            return false;
    }
}

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

            if (!WoW::isServerExpansionAtLeast(WoW::Expansion::_WotLK))
            {
                // Remove seal
                aurEff->getAura()->removeAura();
            }
            // Cast judgement effect
            unitCaster->castSpell(unitTarget, judgementId, true);
            return true;
        }

        if (!WoW::isServerExpansionAtLeast(WoW::Expansion::_WotLK))
        {
            return false;
        }
        else
        {
            // Since wotlk some seals no longer have judgement in spell effects
            // Because player was able to cast this spell it means they have a valid seal
            // Use generic judgement spell
            unitCaster->castSpell(unitTarget, SPELL_JUDGEMENT_DAMAGE_GENERIC, true);
            return true;
        }
    }

    SpellScriptCheckDummy onDummyOrScriptedEffect(Spell* spell, uint8_t /*effIndex*/) override
    {
        if (WoW::isServerExpansion(WoW::Expansion::_WotLK))
        {
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
        }
        else
        {
            handleJudgement(spell);
            return SpellScriptCheckDummy::DUMMY_OK;
        }
    }
};

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
        forcedBasePoints.set(EFF_INDEX_0, Util::float2int32(damageInfo.realDamage * 0.33f));
        spell->getUnitCaster()->castSpell(spell->getUnitCaster(), backFireSpell, forcedBasePoints, true);
    }
};

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

class JudgementOfLightDummy : public SpellScript
{
public:
    SpellScriptExecuteState beforeAuraEffect(Aura* aur, AuraEffectModifier* aurEff, bool apply) override
    {
        if (WoW::isServerExpansionAtLeast(WoW::Expansion::_WotLK))
            return SpellScript::beforeAuraEffect(aur, aurEff, apply);

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
    SpellScriptCheckDummy onAuraDummyEffect(Aura* aur, AuraEffectModifier* aurEff, bool apply) override
    {
        if (!WoW::isServerExpansionAtLeast(WoW::Expansion::_WotLK))
            return SpellScript::onAuraDummyEffect(aur, aurEff, apply);

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
};

class JudgementOfLightHeal : public SpellScript
{
public:
    void onCreateSpellProc(SpellProc* spellProc, Object* /*obj*/) override
    {
        // Casted by attacker
        spellProc->setCastedByProcInitiator(true);
        if (WoW::isServerExpansion(WoW::Expansion::_WotLK))
        {
            // But in wotlk heal effect is done by paladin
            spellProc->setOriginalCasterGuidForProcSpell(spellProc->getCasterGuid());
        }
    }

    uint32_t calcProcChance(SpellProc* spellProc, Unit* victim, SpellInfo const* castingSpell) override
    {
        if (!WoW::isServerExpansion(WoW::Expansion::_WotLK))
            return SpellScript::calcProcChance(spellProc, victim, castingSpell);

        // TODO: this is a hackfix but almost the correct value
        // should have 15 PPM, but (atm) PPM system works only with melee attacks, and this should proc on spell attacks as well
        // also, if PPM is set, then it calculates proc chance according to aura owner's attack speed, not to attacker's attack speed
        return 50;
    }

    SpellScriptEffectDamage doCalculateEffect(Spell* spell, uint8_t effIndex, int32_t* damage) override
    {
        if (!WoW::isServerExpansion(WoW::Expansion::_WotLK))
            return SpellScript::doCalculateEffect(spell, effIndex, damage);

        if (spell->getUnitTarget() == nullptr)
            return SpellScriptEffectDamage::DAMAGE_DEFAULT;

        *damage = spell->getUnitTarget()->getMaxHealth() * (*damage) / 100;
        return SpellScriptEffectDamage::DAMAGE_NO_BONUSES;
    }
};

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

        if (WoW::isServerExpansion(WoW::Expansion::_TBC))
        {
            // Damage multiplies per stack of dot
            if (auraCount > 1)
                *damage *= auraCount;

            return SpellScriptEffectDamage::DAMAGE_DEFAULT;
        }
        else
        {
            // Calculate bonuses here to increase damage properly
            *damage = Util::float2int32(spell->getUnitCaster()->applySpellDamageBonus(spell->getUnitCaster(), spell->getSpellInfo(), effIndex, *damage, false, spell));
            if (WoW::isServerExpansion(WoW::Expansion::_WotLK))
            {
                // One stack increases damage by 10%
                *damage = *damage * (100 + (10 * auraCount)) / 100;
            }
            else
            {
                // One stack increases damage by 20%
                *damage = *damage * (100 + (20 * auraCount)) / 100;
            }
            return SpellScriptEffectDamage::DAMAGE_NO_BONUSES;
        }
    }
};

class JudgementOfWisdomDummy : public SpellScript
{
public:
    SpellScriptExecuteState beforeAuraEffect(Aura* aur, AuraEffectModifier* aurEff, bool apply) override
    {
        if (WoW::isServerExpansionAtLeast(WoW::Expansion::_WotLK))
            return SpellScript::beforeAuraEffect(aur, aurEff, apply);

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
    SpellScriptCheckDummy onAuraDummyEffect(Aura* aur, AuraEffectModifier* aurEff, bool apply) override
    {
        if (!WoW::isServerExpansionAtLeast(WoW::Expansion::_WotLK))
            return SpellScript::onAuraDummyEffect(aur, aurEff, apply);

        if (apply)
            aur->getOwner()->addProcTriggerSpell(sSpellMgr.getSpellInfo(SPELL_JUDGEMENT_OF_WISDOM_MANA), aur, aur->getCasterGuid());
        else
            aur->getOwner()->removeProcTriggerSpell(SPELL_JUDGEMENT_OF_WISDOM_MANA, aur->getCasterGuid());

        return SpellScriptCheckDummy::DUMMY_OK;
    }
};

class JudgementOfWisdomMana : public SpellScript
{
public:
    void onCreateSpellProc(SpellProc* spellProc, Object* /*obj*/) override
    {
        // Casted by attacker
        spellProc->setCastedByProcInitiator(true);
        if (WoW::isServerExpansion(WoW::Expansion::_WotLK))
        {
            // But in wotlk mana effect is done by paladin
            spellProc->setOriginalCasterGuidForProcSpell(spellProc->getCasterGuid());
        }
    }

    bool canProc(SpellProc* /*spellProc*/, Unit* victim, SpellInfo const* /*castingSpell*/, DamageInfo /*dmg*/) override
    {
        if (victim == nullptr || victim->getPowerType() != POWER_TYPE_MANA)
            return false;

        return true;
    }

    uint32_t calcProcChance(SpellProc* spellProc, Unit* victim, SpellInfo const* castingSpell) override
    {
        if (!WoW::isServerExpansion(WoW::Expansion::_WotLK))
            return SpellScript::calcProcChance(spellProc, victim, castingSpell);

        // TODO: this is a hackfix but almost the correct value
        // should have 15 PPM, but PPM system works only with melee attacks, and this should proc on spell attacks as well
        // also, if PPM is set, then it calculates proc chance according to aura owner's attack speed, not to attacker's attack speed
        return 50;
    }

    SpellScriptEffectDamage doCalculateEffect(Spell* spell, uint8_t effIndex, int32_t* damage) override
    {
        if (!WoW::isServerExpansion(WoW::Expansion::_WotLK))
            return SpellScript::doCalculateEffect(spell, effIndex, damage);

        if (spell->getUnitTarget() == nullptr)
            return SpellScriptEffectDamage::DAMAGE_DEFAULT;

        *damage = spell->getUnitTarget()->getBaseMana() * (*damage) / 100;
        return SpellScriptEffectDamage::DAMAGE_NO_BONUSES;
    }
};

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
        const int32_t backfireDmg = damageInfo.realDamage * spellProc->getOverrideEffectDamage(EFF_INDEX_1) / 100;
        spellProc->setOverrideEffectDamage(EFF_INDEX_0, std::max(backfireDmg, 1));
        return SpellScriptExecuteState::EXECUTE_OK;
    }
};

class SealOfCommandDummy : public SpellScript
{
public:
    SpellScriptCheckDummy onAuraDummyEffect(Aura* aur, AuraEffectModifier* aurEff, bool apply) override
    {
        if (aurEff->getEffectIndex() != EFF_INDEX_0)
            return SpellScriptCheckDummy::DUMMY_OK;

        if (apply)
            aur->getOwner()->addProcTriggerSpell(sSpellMgr.getSpellInfo(sealOfCommandDamageId()), aur, aur->getCasterGuid());
        else
            aur->getOwner()->removeProcTriggerSpell(sealOfCommandDamageId(), aur->getCasterGuid());

        return SpellScriptCheckDummy::DUMMY_OK;
    }
};

class SealOfCommandDamage : public SpellScript
{
public:
    void onCreateSpellProc(SpellProc* spellProc, Object* /*obj*/) override
    {
        if (!WoW::isServerExpansionAtLeast(WoW::Expansion::_WotLK))
        {
            // 7 PPM
            spellProc->setProcsPerMinute(7.f);
        }
        else
        {
            // Procs only from Seal of Righteousness, Seal of Truth and Seal of Justice
            spellProc->setProcClassMask(EFF_INDEX_0, 0x20000200);
            spellProc->setProcClassMask(EFF_INDEX_1, 0x20000800);
            spellProc->setProcClassMask(EFF_INDEX_2, 0x10000);
        }
    }
};

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

class SealOfJusticeEffect : public SpellScript
{
public:
    void onCreateSpellProc(SpellProc* spellProc, Object* obj) override
    {
        if (WoW::isServerExpansionAtLeast(WoW::Expansion::_Cata))
            return SpellScript::onCreateSpellProc(spellProc, obj);

        // 5 PPM
        spellProc->setProcsPerMinute(5.f);
    }

    // Stun effect was replaced by damage effect in Cata
    SpellScriptEffectDamage doCalculateEffect(Spell* spell, uint8_t effIndex, int32_t* damage) override
    {
        if (!WoW::isServerExpansion(WoW::Expansion::_Cata))
            return SpellScript::doCalculateEffect(spell, effIndex, damage);

        auto* const unitCaster = spell->getUnitCaster();
        if (unitCaster == nullptr)
            return SpellScriptEffectDamage::DAMAGE_DEFAULT;

        float_t speed = 2.0f;
        if (const auto* const plrCaster = spell->getPlayerCaster())
        {
            if (const auto* const mainhandWeapon = plrCaster->getItemInterface()->GetInventoryItem(EQUIPMENT_SLOT_MAINHAND))
                speed = mainhandWeapon->getItemProperties()->Delay / 1000.0f;
        }
        // Weapon speed * (0.005 * AP + 0.01 * SPH)
        *damage = Util::float2int32(unitCaster->applySpellDamageBonus(spell->getOriginalCaster(), spell->getSpellInfo(), effIndex, speed, true, spell));
        return SpellScriptEffectDamage::DAMAGE_NO_BONUSES;
    }
};

class SealOfLightAndInsightHeal : public SpellScript
{
public:
    void onCreateSpellProc(SpellProc* spellProc, Object* /*obj*/) override
    {
        if (!WoW::isServerExpansionAtLeast(WoW::Expansion::_Cata))
        {
            // 15 PPM
            spellProc->setProcsPerMinute(15.f);
        }
        else
        {
            // 8.5 PPM according to wowhead comment from patch 4.2.2
            spellProc->setProcsPerMinute(8.5f);
        }
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

        const auto* damageSpell = sSpellMgr.getEquivalentSpellRankFor(aur->getSpellInfo(), sSpellMgr.getSpellInfo(sealOfRighteousnessDamageId()));
        if (WoW::isServerExpansion(WoW::Expansion::_Cata))
        {
            // Seals of Command talent
            if (aur->getOwner()->hasAurasWithId(SPELL_SEALS_OF_COMMAND_DUMMY))
                damageSpell = sSpellMgr.getSpellInfo(SPELL_SEAL_OF_RIGHTEOUSNESS_DAMAGE_AOE);
        }
        if (damageSpell == nullptr)
            return SpellScriptCheckDummy::DUMMY_OK;

        if (apply)
        {
            if (WoW::isServerExpansionAtLeast(WoW::Expansion::_WotLK))
            {
                aur->getOwner()->addProcTriggerSpell(damageSpell, aur, aur->getCasterGuid());
            }
            else
            {
                auto spellProc = aur->getOwner()->addProcTriggerSpell(damageSpell, aur, aur->getCasterGuid());
                if (spellProc != nullptr)
                    spellProc->setOverrideEffectDamage(aurEff->getEffectIndex(), aurEff->getEffectDamage());
            }
        }
        else
        {
            aur->getOwner()->removeProcTriggerSpell(damageSpell->getId(), aur->getCasterGuid());
        }

        return SpellScriptCheckDummy::DUMMY_OK;
    }
};

class SealOfRighteousnessDamage : public SpellScript
{
public:
    SpellScriptEffectDamage doCalculateEffect(Spell* spell, uint8_t effIndex, int32_t* damage) override
    {
        auto* const unitCaster = spell->getUnitCaster();
        if (unitCaster == nullptr)
            return SpellScriptEffectDamage::DAMAGE_DEFAULT;

        float_t speed = 2.0f;
        const auto* const mainHandWeapon = spell->getPlayerCaster() != nullptr ? spell->getPlayerCaster()->getItemInterface()->GetInventoryItem(EQUIPMENT_SLOT_MAINHAND) : nullptr;
        if (mainHandWeapon != nullptr)
            speed = mainHandWeapon->getItemProperties()->Delay / 1000.f;

        if (!WoW::isServerExpansionAtLeast(WoW::Expansion::_WotLK))
        {
            // Due to somewhat complex damage calculation need to apply spell damage manually
            // This is essentially applySpellDamageBonus dumbed down
            float_t baseDamage = 0.0f;
            auto spellCoeff = spell->getSpellInfo()->getEffectSpellPowerCoefficient(effIndex);
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

            auto bonusDmg = static_cast<float_t>(unitCaster->GetDamageDoneMod(SCHOOL_HOLY));
            if (bonusDmg > 0.0f)
            {
                bonusDmg *= spellCoeff;

                if (WoW::isServerExpansion(WoW::Expansion::_TBC))
                {
                    // Apply general downranking penalty
                    if (bonusDmg > 0.0f && unitCaster->isPlayer())
                        bonusDmg *= unitCaster->getSpellDownrankingPenalty(spell->getSpellInfo()->getMaxLevel());
                }
            }

            unitCaster->applySpellModifiers(SPELLMOD_PENALTY, &bonusDmg, spell->getSpellInfo(), spell);

            float_t fullDmg = baseDamage + std::max(0.0f, bonusDmg);
            unitCaster->applySpellModifiers(SPELLMOD_DAMAGE_DONE, &fullDmg, spell->getSpellInfo(), spell);
            fullDmg *= unitCaster->GetDamageDonePctMod(SCHOOL_HOLY);

            *damage = Util::float2int32(fullDmg);
        }
        else
        {
            // Wotlk: Weapon speed * (0.022 * AP + 0.044 * SPH)
            // Cata: Weapon speed * (0.011 * AP + 0.022 * SPH)
            *damage = Util::float2int32(unitCaster->applySpellDamageBonus(spell->getOriginalCaster(), spell->getSpellInfo(), effIndex, speed, true, spell));
        }
        return SpellScriptEffectDamage::DAMAGE_NO_BONUSES;
    }
};

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

        if (WoW::isServerExpansion(WoW::Expansion::_TBC))
        {
            // As of Patch 2.2.0, Seal of Vengeance deals a small amount of Holy Damage if it strikes a target with 5 stacks of the debuff already applied.
            if (aur->getStackCount() < 5)
                return false;
        }

        return true;
    }

    SpellScriptEffectDamage doCalculateEffect(Spell* spell, uint8_t effIndex, int32_t* damage) override
    {
        if (!WoW::isServerExpansionAtLeast(WoW::Expansion::_WotLK))
            return SpellScript::doCalculateEffect(spell, effIndex, damage);

        if (spell->getUnitTarget() == nullptr)
            return SpellScriptEffectDamage::DAMAGE_DEFAULT;

        uint32_t auraId = SPELL_HOLY_VENGEANCE;
        if (spell->getSpellInfo()->getId() == SPELL_SEAL_OF_CORRUPTION_DIRECT)
            auraId = SPELL_BLOOD_CORRUPTION;

        // Weapon damage per debuff stack, up to five stacks: 6.6% in WotLK (from 3.2.0 every attack can proc this), 3% in Cata, 2.4% from Mop on
        float_t dmgPercent = 2.4f;
        if (WoW::isServerExpansion(WoW::Expansion::_WotLK))
            dmgPercent = 6.6f;
        else if (WoW::isServerExpansion(WoW::Expansion::_Cata))
            dmgPercent = 3.f;

        if (const auto* const dotAur = spell->getUnitTarget()->getAuraWithId(auraId))
            dmgPercent *= dotAur->getStackCount();

        *damage = Util::float2int32(dmgPercent);
        return SpellScriptEffectDamage::DAMAGE_NO_BONUSES;
    }
};

class SealOfVengeanceAndCorruptionAndTruthDot : public SpellScript
{
public:
    void onCreateSpellProc(SpellProc* proc, Object* obj) override
    {
        if (!WoW::isServerExpansion(WoW::Expansion::_TBC))
            return SpellScript::onCreateSpellProc(proc, obj);

        // 20 PPM
        proc->setProcsPerMinute(20.f);
    }

    bool canProc(SpellProc* spellProc, Unit* victim, SpellInfo const* castingSpell, DamageInfo damageInfo) override
    {
        if (!WoW::isServerExpansionAtLeast(WoW::Expansion::_WotLK))
            return SpellScript::canProc(spellProc, victim, castingSpell, damageInfo);

        if (castingSpell != nullptr)
        {
            if (!WoW::isServerExpansionAtLeast(WoW::Expansion::_Cata))
            {
                // Dot portion should only be applied on melee hits and on Hammer of the Righteous
                if (castingSpell->getSpellFamilyFlags(EFF_INDEX_0) == 0 &&
                    castingSpell->getSpellFamilyFlags(EFF_INDEX_1) == 0x40000 &&
                    castingSpell->getSpellFamilyFlags(EFF_INDEX_2) == 0)
                    return true;

                return false;
            }
            else
            {
                // Procs only from melee attacks and single target spells
                const auto targetMask = castingSpell->getRequiredTargetMask(true);
                if (targetMask & SPELL_TARGET_AREA_MASK)
                    return false;

                // Cannot proc from Avenger's Shield
                if (castingSpell->getSpellFamilyFlags(EFF_INDEX_0) == 0x4000 &&
                    castingSpell->getSpellFamilyFlags(EFF_INDEX_1) == 0 &&
                    castingSpell->getSpellFamilyFlags(EFF_INDEX_2) == 0)
                    return false;
            }
        }

        return true;
    }
};

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

class Vengeance : public SpellScript
{
public:
    void onCreateSpellProc(SpellProc* spellProc, Object* /*obj*/) override
    {
        spellProc->setExtraProcFlags(EXTRA_PROC_ON_CRIT_ONLY);
    }
};

void setupPaladinSpells(ScriptMgr* mgr)
{
    // Call legacy script setup
    SetupLegacyPaladinSpells(mgr);

    if (WoW::isServerExpansion(WoW::Expansion::_WotLK))
    {
        mgr->register_spell_script(SPELL_ART_OF_WAR_PROC_R1, new ArtOfWar);
    }

    if (!WoW::isServerExpansionAtLeast(WoW::Expansion::_Mop))
    {
        mgr->register_spell_script(SPELL_EYE_FOR_AN_EYE_DUMMY_R1, new EyeForAnEyeDummy);
        mgr->register_spell_script(SPELL_EYE_FOR_AN_EYE_DAMAGE, new EyeForAnEye);
    }

    if (WoW::isServerExpansion(WoW::Expansion::_WotLK))
    {
        uint32_t judgementIds[] =
        {
            SPELL_JUDGEMENT_OF_LIGHT_DUMMY,
            SPELL_JUDGEMENT_OF_WISDOM_DUMMY,
            SPELL_JUDGEMENT_OF_JUSTICE_DUMMY,
            0
        };
        mgr->register_spell_script(judgementIds, new JudgementDummy);
    }
    else if (!WoW::isServerExpansionAtLeast(WoW::Expansion::_Mop))
    {
        mgr->register_spell_script(SPELL_JUDGEMENT_DUMMY, new JudgementDummy);
    }

    if (WoW::isServerExpansion(WoW::Expansion::_TBC) || WoW::isServerExpansion(WoW::Expansion::_WotLK))
    {
        uint32_t judgementOfBloodMartyrDamage[] =
        {
            SPELL_JUDGEMENT_OF_BLOOD_DAMAGE,
            WoW::isServerExpansion(WoW::Expansion::_WotLK) ? static_cast<uint32_t>(SPELL_JUDGEMENT_OF_THE_MARTYR_DAMAGE) : 0u,
            0
        };
        mgr->register_spell_script(judgementOfBloodMartyrDamage, new JudgementOfBloodMartyrDamage);
    }

    if (!WoW::isServerExpansionAtLeast(WoW::Expansion::_Cata))
    {
        mgr->register_spell_script(SPELL_JUDGEMENT_OF_COMMAND_DUMMY_R1, new JudgementOfCommandDummy);
        if (!WoW::isServerExpansionAtLeast(WoW::Expansion::_WotLK))
        {
            mgr->register_spell_script(SPELL_JUDGEMENT_OF_COMMAND_DAMAGE_R1, new JudgementOfCommandDamage);
        }
    }

    if (!WoW::isServerExpansionAtLeast(WoW::Expansion::_Cata))
    {
        mgr->register_spell_script(SPELL_JUDGEMENT_OF_LIGHT_DEBUFF, new JudgementOfLightDummy);
        mgr->register_spell_script(SPELL_JUDGEMENT_OF_LIGHT_HEAL, new JudgementOfLightHeal);
    }

    if (WoW::isServerExpansionAtLeast(WoW::Expansion::_TBC) && !WoW::isServerExpansionAtLeast(WoW::Expansion::_Mop))
    {
        uint32_t judgementOfVengeanceCorruptionTruth[] =
        {
            SPELL_JUDGEMENT_OF_VENGEANCE,
            WoW::isServerExpansion(WoW::Expansion::_WotLK) ? static_cast<uint32_t>(SPELL_JUDGEMENT_OF_CORRUPTION) : 0u,
            0
        };
        mgr->register_spell_script(judgementOfVengeanceCorruptionTruth, new JudgementOfVengeanceCorruptionTruthDamage);
    }

    if (!WoW::isServerExpansionAtLeast(WoW::Expansion::_Cata))
    {
        mgr->register_spell_script(SPELL_JUDGEMENT_OF_WISDOM_DEBUFF, new JudgementOfWisdomDummy);
        mgr->register_spell_script(SPELL_JUDGEMENT_OF_WISDOM_MANA, new JudgementOfWisdomMana);
    }

    if (WoW::isServerExpansion(WoW::Expansion::_TBC) || WoW::isServerExpansion(WoW::Expansion::_WotLK))
    {
        uint32_t sealOfBloodMartyrDummy[] =
        {
            SPELL_SEAL_OF_BLOOD_DUMMY,
            WoW::isServerExpansion(WoW::Expansion::_WotLK) ? static_cast<uint32_t>(SPELL_SEAL_OF_THE_MARTYR_DUMMY) : 0u,
            0
        };
        mgr->register_spell_script(sealOfBloodMartyrDummy, new SealOfBloodAndMartyrDummy);
        uint32_t sealOfBloodMartyrBackfire[] =
        {
            SPELL_SEAL_OF_BLOOD_BACKFIRE,
            WoW::isServerExpansion(WoW::Expansion::_WotLK) ? static_cast<uint32_t>(SPELL_SEAL_OF_THE_MARTYR_BACKFIRE) : 0u,
            0
        };
        mgr->register_spell_script(sealOfBloodMartyrBackfire, new SealOfBloodAndMartyrBackfire);
    }

    if (WoW::isServerExpansionAtLeast(WoW::Expansion::_Mop) && !WoW::isServerExpansionAtLeast(WoW::Expansion::_Legion))
    {
        mgr->register_spell_script(sealOfCommandDummyId(), new SealOfCommandDummy);
    }
    if (!WoW::isServerExpansionAtLeast(WoW::Expansion::_WotLK) || WoW::isServerExpansion(WoW::Expansion::_Cata))
    {
        mgr->register_spell_script(sealOfCommandDamageId(), new SealOfCommandDamage);
    }

    if (WoW::isServerExpansion(WoW::Expansion::_Cata))
    {
        mgr->register_spell_script(SPELL_SEAL_OF_JUSTICE_DUMMY_R1, new SealOfJusticeDummy);
    }
    if (!WoW::isServerExpansionAtLeast(WoW::Expansion::_Mop))
    {
        mgr->register_spell_script(SPELL_SEAL_OF_JUSTICE_EFFECT, new SealOfJusticeEffect);
    }

    if (!WoW::isServerExpansionAtLeast(WoW::Expansion::_Legion))
    {
        mgr->register_spell_script(SPELL_SEAL_OF_LIGHT_HEAL_R1, new SealOfLightAndInsightHeal);
    }

    if (!WoW::isServerExpansionAtLeast(WoW::Expansion::_Legion))
    {
        mgr->register_spell_script(sealOfRighteousnessDummyId(), new SealOfRighteousnessDummy);
    }
    if (!WoW::isServerExpansionAtLeast(WoW::Expansion::_Mop))
    {
        uint32_t sealOfRighteousnessDamage[] =
        {
            sealOfRighteousnessDamageId(),
            WoW::isServerExpansion(WoW::Expansion::_Cata) ? static_cast<uint32_t>(SPELL_SEAL_OF_RIGHTEOUSNESS_DAMAGE_AOE) : 0u,
            0
        };
        mgr->register_spell_script(sealOfRighteousnessDamage, new SealOfRighteousnessDamage);
    }

    if (!WoW::isServerExpansionAtLeast(WoW::Expansion::_WotLK))
    {
        mgr->register_spell_script(SPELL_SEAL_OF_THE_CRUSADER_R1, new SealOfTheCrusader);
    }

    if (WoW::isServerExpansionAtLeast(WoW::Expansion::_TBC) && !WoW::isServerExpansionAtLeast(WoW::Expansion::_Legion))
    {
        uint32_t sealOfVengeanceCorruptionTruthDirect[] =
        {
            SPELL_SEAL_OF_VENGEANCE_DIRECT,
            WoW::isServerExpansion(WoW::Expansion::_WotLK) ? static_cast<uint32_t>(SPELL_SEAL_OF_CORRUPTION_DIRECT) : 0u,
            0
        };
        mgr->register_spell_script(sealOfVengeanceCorruptionTruthDirect, new SealOfVengeanceAndCorruptionAndTruthDirect);
        uint32_t sealOfVengeanceCorruptionTruthDummy[] =
        {
            SPELL_SEAL_OF_VENGEANCE_DUMMY,
            WoW::isServerExpansion(WoW::Expansion::_WotLK) ? static_cast<uint32_t>(SPELL_SEAL_OF_CORRUPTION_DUMMY) : 0u,
            0
        };
        mgr->register_spell_script(sealOfVengeanceCorruptionTruthDummy, new SealOfVengeanceAndCorruptionAndTruthDummy);
        uint32_t sealOfVengeanceCorruptionTruthDot[] =
        {
            SPELL_HOLY_VENGEANCE,
            WoW::isServerExpansion(WoW::Expansion::_WotLK) ? static_cast<uint32_t>(SPELL_BLOOD_CORRUPTION) : 0u,
            0
        };
        mgr->register_spell_script(sealOfVengeanceCorruptionTruthDot, new SealOfVengeanceAndCorruptionAndTruthDot);
    }

    if (!WoW::isServerExpansionAtLeast(WoW::Expansion::_Cata))
    {
        mgr->register_spell_script(SPELL_SEAL_OF_WISDOM_EFFECT_R1, new SealOfWisdomEffect);
    }

    if (!WoW::isServerExpansionAtLeast(WoW::Expansion::_Cata))
    {
        mgr->register_spell_script(SPELL_VENGEANCE_PROC_R1, new Vengeance);
    }
}
