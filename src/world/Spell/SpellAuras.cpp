/*
Copyright (c) 2014-2020 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "SpellAuras.h"

#include "Definitions/AuraInterruptFlags.h"
#include "Definitions/SpellFamily.h"
#include "Definitions/SpellIsFlags.h"
#include "Definitions/SpellMechanics.h"
#include "Definitions/SpellTypes.h"
#include "SpellHelpers.h"
#include "SpellMgr.h"

#include "Server/Script/ScriptMgr.h"

using AscEmu::World::Spell::Helpers::spellModFlatFloatValue;
using AscEmu::World::Spell::Helpers::spellModFlatIntValue;
using AscEmu::World::Spell::Helpers::spellModPercentageFloatValue;
using AscEmu::World::Spell::Helpers::spellModPercentageIntValue;

extern pSpellAura SpellAuraHandler[TOTAL_SPELL_AURAS];
extern const char* SpellAuraNames[TOTAL_SPELL_AURAS];

AuraEffectModifier Aura::getAuraEffect(uint8_t effIndex) const
{
    return m_auraEffects[effIndex];
}

bool Aura::hasAuraEffect(AuraEffect auraEffect) const
{
    for (uint8_t i = 0; i < MAX_SPELL_EFFECTS; ++i)
    {
        if (m_auraEffects[i].mAuraEffect == auraEffect)
            return true;
    }

    return false;
}

void Aura::addAuraEffect(AuraEffect auraEffect, int32_t damage, int32_t miscValue, uint8_t effIndex)
{
    if (effIndex >= MAX_SPELL_EFFECTS)
        return;

    if (auraEffect >= TOTAL_SPELL_AURAS)
    {
        LogError("Aura::addAuraEffect : Unknown aura effect type %u for spell id %u", auraEffect, getSpellId());
        return;
    }

    m_auraEffects[effIndex].mAuraEffect = auraEffect;
    m_auraEffects[effIndex].mDamage = damage;
    m_auraEffects[effIndex].miscValue = miscValue;
    m_auraEffects[effIndex].effIndex = effIndex;
    ++m_auraEffectCount;

    // Calculate effect amplitude
    _calculateEffectAmplitude(effIndex);
}

void Aura::removeAuraEffect(uint8_t effIndex)
{
    if (effIndex >= MAX_SPELL_EFFECTS)
        return;

    // Unapply the modifier
    (*this.*SpellAuraHandler[m_auraEffects[effIndex].mAuraEffect])(&m_auraEffects[effIndex], false);

    m_auraEffects[effIndex].mAuraEffect = SPELL_AURA_NONE;
    m_auraEffects[effIndex].mDamage = 0;
    m_auraEffects[effIndex].mFixedDamage = 0;
    m_auraEffects[effIndex].miscValue = 0;
    m_auraEffects[effIndex].mAmplitude = 0;
    m_auraEffects[effIndex].mDamageFraction = 0.0f;
    m_auraEffects[effIndex].effIndex = 0;
    --m_auraEffectCount;

    // Check aura effects on next update
    m_checkAuraEffects = true;
}

uint8_t Aura::getAppliedEffectCount() const
{
    return m_auraEffectCount;
}

int32_t Aura::getEffectDamage(uint8_t effIndex) const
{
    if (effIndex >= MAX_SPELL_EFFECTS)
        return 0;

    return m_auraEffects[effIndex].mDamage;
}

int32_t Aura::getEffectDamageByEffect(AuraEffect auraEffect) const
{
    for (uint8_t i = 0; i < MAX_SPELL_EFFECTS; ++i)
    {
        if (m_auraEffects[i].mAuraEffect == auraEffect)
            return m_auraEffects[i].mDamage;
    }

    return 0;
}

void Aura::removeAura()
{
    sEventMgr.RemoveEvents(this);

    // Check if aura has already been removed
    if (m_isGarbage)
        return;

    sHookInterface.OnAuraRemove(this);

    LogDebugFlag(LF_AURA, "Removing aura %u from unit %u", getSpellId(), getOwner()->getGuid());

    m_isGarbage = true;

    // Remove all modifiers
    applyModifiers(false);

    for (uint8_t i = 0; i < MAX_SPELL_EFFECTS; ++i)
    {
        if (getSpellInfo()->getEffect(i) == 0)
            continue;

        ///\ todo: verify this
        if (getSpellInfo()->getEffect(i) == SPELL_EFFECT_TRIGGER_SPELL)
        {
            const auto triggerSpell = sSpellMgr.getSpellInfo(getSpellInfo()->getEffectTriggerSpell(i));
            if (triggerSpell != nullptr)
            {
                if (triggerSpell->getDurationIndex() < getSpellInfo()->getDurationIndex())
                    getOwner()->RemoveAura(getSpellInfo()->getEffectTriggerSpell(i));
            }
        }
        else if (IsAreaAura() && getCasterGuid() == getOwner()->getGuid())
        {
            ClearAATargets();
        }
    }

    if (getSpellInfo()->getProcCharges() > 0 && getSpellInfo()->custom_proc_interval == 0)
    {
        if (getOwner()->m_chargeSpellsInUse)
        {
            getOwner()->m_chargeSpellRemoveQueue.push_back(getSpellId());
        }
        else
        {
            auto iter = getOwner()->m_chargeSpells.find(getSpellId());
            if (iter != getOwner()->m_chargeSpells.end())
            {
                if (iter->second.count > 1)
                    --iter->second.count;
                else
                    getOwner()->m_chargeSpells.erase(iter);
            }
        }
    }

    // Reset diminishing return timer
    getOwner()->removeDiminishingReturnTimer(getSpellInfo());

    const auto caster = GetUnitCaster();
    if (caster != nullptr)
    {
        // Remove attacker
        if (caster != getOwner())
        {
            caster->CombatStatus.RemoveAttackTarget(getOwner());
            getOwner()->CombatStatus.RemoveAttacker(caster, caster->getGuid());
        }

        /**********************Cooldown**************************
        * this is only needed for some spells
        * for now only spells that have:
        * (m_spellInfo->Attributes == 0x2050000) && !(m_spellInfo->AttributesEx) ||
        * m_spellProto->Attributes == 0x2040100
        * are handled. Its possible there are more spells like this
        *************************************************************/
        if (caster->isPlayer() && caster->IsInWorld() && getSpellInfo()->custom_c_is_flags & SPELL_FLAG_IS_REQUIRECOOLDOWNUPDATE)
        {
            const auto p = static_cast<Player*>(caster);
            p->sendSpellCooldownEventPacket(getSpellInfo()->getId());
        }

        // Far Sight effect
        ///\ todo: handle this in effects
        if (caster->isPlayer() && caster->IsInWorld() && static_cast<Player*>(caster)->getFarsightGuid() != 0)
        {
            uint8_t j = 0;;
            for (; j < MAX_SPELL_EFFECTS; ++j)
                if (getSpellInfo()->getEffect(j) == SPELL_EFFECT_ADD_FARSIGHT)
                    break;

            if (j != MAX_SPELL_EFFECTS)
                static_cast<Player*>(caster)->setFarsightGuid(0);
        }

        // If this aura can affect one target at a time, remove this target from the caster map
        if (getSpellInfo()->getAttributesExE() & ATTRIBUTESEXE_SINGLE_TARGET_AURA && getOwner()->GetAuraStackCount(getSpellId()) == 1)
            getOwner()->removeSingleTargetGuidForAura(getSpellInfo()->getId());

        // only remove channel stuff if caster == target, then it's not removed twice, for example, arcane missiles applies a dummy aura to target
        if (caster == getOwner() && m_spellInfo->getChannelInterruptFlags() != 0)
        {
            caster->setChannelObjectGuid(0);
            caster->setChannelSpellId(0);
        }

        if (caster->isPlayer() && getSpellInfo()->hasEffect(SPELL_EFFECT_SUMMON))
        {
            const auto charm = caster->GetMapMgrUnit(caster->getCharmGuid());
            if (charm != nullptr && charm->getCreatedBySpellId() == getSpellInfo()->getId())
                static_cast<Player*>(caster)->UnPossess();
        }
    }
    else
    {
        // Remove attacker
        getOwner()->CombatStatus.RemoveAttacker(nullptr, m_casterGuid);
    }

    // Remove aurastates
    if (getSpellInfo()->getMechanicsType() == MECHANIC_ENRAGED && !--m_target->asc_enraged)
        getOwner()->removeAuraStateAndAuras(AURASTATE_FLAG_ENRAGED);
    else if (getSpellInfo()->getMechanicsType() == MECHANIC_BLEEDING && !--m_target->asc_bleed)
        getOwner()->removeAuraStateAndAuras(AURASTATE_FLAG_BLEED);
    if (getSpellInfo()->custom_BGR_one_buff_on_target & SPELL_TYPE_SEAL && !--m_target->asc_seal)
        getOwner()->removeAuraStateAndAuras(AURASTATE_FLAG_JUDGEMENT);

    // Send packet
    if (m_visualSlot < MAX_NEGATIVE_VISUAL_AURAS_END)
    {
#if VERSION_STRING < WotLK
        getOwner()->setAura(m_visualSlot, 0);
        getOwner()->setAuraFlags(m_visualSlot, 0);
        getOwner()->setAuraLevel(m_visualSlot, GetUnitCaster() != nullptr ? GetUnitCaster()->getLevel() : worldConfig.player.playerLevelCap);
        getOwner()->setAuraApplication(m_visualSlot, 1);
#endif

        getOwner()->m_auravisuals[m_visualSlot] = 0;
        getOwner()->sendAuraUpdate(this, true);
        getOwner()->UpdateAuraForGroup(m_visualSlot);
    }

    getOwner()->m_auras[m_auraSlot] = nullptr;
    getOwner()->AddGarbageAura(this);
}

bool Aura::canPeriodicEffectCrit()
{
#if VERSION_STRING < WotLK
    // Periodic effects never crit in Classic or TBC
    return false;
#elif VERSION_STRING == WotLK
    const auto caster = GetUnitCaster();
    if (caster == nullptr)
        return false;

    const auto spellInfo = getSpellInfo();

    if (caster->isPlayer())
    {
        // Patch 3.3.3 - The damage-over-time component of Rupture can now produce critical strikes
        if (spellInfo->getSpellFamilyName() == SPELLFAMILY_ROGUE && spellInfo->getSpellFamilyFlags(0) == 0x100000)
            return true;
    }

    for (auto i = MAX_TOTAL_AURAS_START; i < MAX_TOTAL_AURAS_END; ++i)
    {
        Aura* aur = caster->m_auras[i];
        if (aur == nullptr || !aur->hasAuraEffect(SPELL_AURA_ALLOW_DOT_TO_CRIT))
            continue;

        if (aur->getSpellInfo()->isAuraEffectAffectingSpell(SPELL_AURA_ALLOW_DOT_TO_CRIT, spellInfo))
            return true;
    }

    return false;
#else
    ///\ todo: confirm can ALL periodic effects crit after cata
    const auto caster = GetUnitCaster();
    if (caster != nullptr && caster->isPlayer())
        return true;

    return false;
#endif
}

void Aura::applyModifiers(bool apply)
{
    for (uint8_t i = 0; i < MAX_SPELL_EFFECTS; ++i)
    {
        if (m_auraEffects[i].mAuraEffect == SPELL_AURA_NONE)
            continue;

        if (apply)
            (*this.*SpellAuraHandler[m_auraEffects[i].mAuraEffect])(&m_auraEffects[i], true);
        else
            removeAuraEffect(i);

        LogDebugFlag(LF_AURA, "Aura::applyModifiers : Spell Id %u, Aura Effect %u (%s), Target GUID %u, EffectIndex %u, Duration %u, Damage %d, MiscValue %d",
            getSpellInfo()->getId(), m_auraEffects[i].mAuraEffect, SpellAuraNames[m_auraEffects[i].mAuraEffect], getOwner()->getGuid(), i, getTimeLeft(), m_auraEffects[i].mDamage, m_auraEffects[i].miscValue);
    }

    // Modifiers are applied => aura can be updated now
    m_updatingModifiers = false;
}

void Aura::updateModifiers()
{
    m_updatingModifiers = true;
    for (uint8_t i = 0; i < MAX_SPELL_EFFECTS; ++i)
    {
        switch (m_auraEffects[i].mAuraEffect)
        {
            case SPELL_AURA_MOD_DECREASE_SPEED:
                UpdateAuraModDecreaseSpeed(&m_auraEffects[i]);
                break;
            case SPELL_AURA_MOD_ATTACK_POWER_BY_STAT_PCT:
            case SPELL_AURA_MOD_RANGED_ATTACK_POWER_BY_STAT_PCT:
                (*this.*SpellAuraHandler[m_auraEffects[i].mAuraEffect])(&m_auraEffects[i], false);
                (*this.*SpellAuraHandler[m_auraEffects[i].mAuraEffect])(&m_auraEffects[i], true);
                break;
            default:
                break;
        }

        LogDebugFlag(LF_AURA, "Aura::updateModifiers : Spell Id %u, Aura Effect %u (%s), Target GUID %u, EffectIndex %u, Duration %u, Damage %d, MiscValue %d",
            getSpellInfo()->getId(), m_auraEffects[i].mAuraEffect, SpellAuraNames[m_auraEffects[i].mAuraEffect], getOwner()->getGuid(), i, getTimeLeft(), m_auraEffects[i].mDamage, m_auraEffects[i].miscValue);
    }
    m_updatingModifiers = false;
}

int32_t Aura::getTimeLeft() const
{
    return m_duration;
}

void Aura::setTimeLeft(int32_t dur)
{
    if (dur > getMaxDuration())
        dur = getMaxDuration();

    m_duration = dur;
}

int32_t Aura::getMaxDuration() const
{
    return m_maxDuration;
}

void Aura::setMaxDuration(int32_t dur)
{
    m_maxDuration = dur;
}

void Aura::refresh([[maybe_unused]]bool saveMods/* = false*/, int16_t modifyStacks/* = 0*/)
{
    int32_t maxStacks = getSpellInfo()->getMaxstack() == 0 ? 1 : getSpellInfo()->getMaxstack();

    // If aura uses charges, use charges instead
    if (getSpellInfo()->getProcCharges() > 0)
    {
        auto spellCharges = static_cast<int32_t>(getSpellInfo()->getProcCharges());

        // Apply modifiers to charge count
        const auto caster = GetUnitCaster();
        if (caster != nullptr)
        {
            spellModFlatIntValue(caster->SM_FCharges, &spellCharges, getSpellInfo()->getSpellFamilyFlags());
            spellModPercentageIntValue(caster->SM_PCharges, &spellCharges, getSpellInfo()->getSpellFamilyFlags());
        }

        maxStacks = spellCharges;
    }

    // Check for aura stack cheat
    const auto plrHolder = getPlayerOwner();
    if (plrHolder != nullptr && plrHolder->m_cheats.AuraStackCheat)
        maxStacks = 255;

    const auto curStackCount = getStackCount();
    int32_t newStackCount = curStackCount + modifyStacks;

    if (modifyStacks < 0)
    {
        // If stack count reaches zero, remove aura
        if (newStackCount <= 0)
        {
            removeAura();
            return;
        }
    }
    else
    {
        if (newStackCount >= maxStacks)
            newStackCount = maxStacks;

        // Recalculate aura modifiers on reapply or when stack count increases
#if VERSION_STRING < Cata
        // Before cata, aura's crit chance and damage bonuses (NYI) were saved on aura refresh
        if (!saveMods)
        {
            _calculateCritChance();
        }
#else
        _calculateCritChance();
#endif
        _calculateSpellPowerBonus();
        _calculateSpellHaste();

        // Recalculate aura duration and effect amplitudes based on new haste
        setMaxDuration(static_cast<int32_t>(m_originalDuration * m_spellHaste));
        for (uint8_t i = 0; i < MAX_SPELL_EFFECTS; ++i)
        {
            if (m_auraEffects[i].mAuraEffect == SPELL_AURA_NONE)
                continue;

            _calculateEffectAmplitude(i);
        }

        // Reset duration to max
        setTimeLeft(getMaxDuration());
    }

    m_updatingModifiers = true;
    m_stackCount = static_cast<uint8_t>(newStackCount);

    // Reapply modifiers
    for (uint8_t i = 0; i < MAX_SPELL_EFFECTS; ++i)
    {
        if (m_auraEffects[i].mAuraEffect != SPELL_AURA_NONE)
        {
            (*this.*SpellAuraHandler[m_auraEffects[i].mAuraEffect])(&m_auraEffects[i], false);

            if (curStackCount != newStackCount)
            {
                // Divide damage by old stack amount to get amount per stack
                m_auraEffects[i].mDamage /= curStackCount;
                // And multiply it with new stack amount
                m_auraEffects[i].mDamage *= m_stackCount;
            }

            (*this.*SpellAuraHandler[m_auraEffects[i].mAuraEffect])(&m_auraEffects[i], true);
        }
    }

    m_updatingModifiers = false;

#if VERSION_STRING < WotLK
    if (curStackCount != newStackCount)
        getOwner()->setAuraApplication(m_visualSlot, getStackCount());
#endif

    // Send aura update
    getOwner()->sendAuraUpdate(this, false);
    getOwner()->UpdateAuraForGroup(m_visualSlot);
}

uint8_t Aura::getStackCount() const
{
    return m_stackCount;
}

uint8_t Aura::getAuraFlags() const
{
    ///\ todo: these seem to be wrong for tbc
    uint8_t auraFlags = isNegative() ? AFLAG_NEGATIVE : AFLAG_CANCELLABLE;

    for (uint8_t i = 0; i < MAX_SPELL_EFFECTS; ++i)
    {
        // Check if aura has effects in all indexes
        if (m_auraEffects[i].mAuraEffect != SPELL_AURA_NONE)
            auraFlags |= 1 << i;
    }

    if (getCasterGuid() == getOwner()->getGuid())
        auraFlags |= AFLAG_IS_CASTER;

    if (getMaxDuration() > 0 && !(getSpellInfo()->getAttributesExE() & ATTRIBUTESEXE_HIDE_DURATION))
        auraFlags |= AFLAG_DURATION;

#if VERSION_STRING >= Cata
    if (getSpellInfo()->getAttributesExH() & ATTRIBUTESEXH_SEND_AURA_EFFECT_AMOUNT)
        auraFlags |= AFLAG_SEND_EFFECT_AMOUNT;
#endif

    return auraFlags;
}

void Aura::setNegative(bool negative)
{
    mPositive = !negative;
}

bool Aura::isNegative() const
{
    return !mPositive;
}

float_t Aura::getCritChance() const
{
    return m_critChance;
}

int32_t Aura::getSpellPowerBonus() const
{
    return m_spellPowerBonus;
}

int32_t Aura::getHealPowerBonus() const
{
    return m_healPowerBonus;
}

Unit* Aura::getOwner() const
{
    return m_target;
}

Player* Aura::getPlayerOwner() const
{
    return p_target;
}

Object* Aura::getCaster() const
{
    if (getOwner() == nullptr)
        return nullptr;
    if (getCasterGuid() == getOwner()->getGuid())
        return getOwner();
    if (getOwner()->IsInWorld())
        return getOwner()->GetMapMgrObject(getCasterGuid());

    return nullptr;
}

uint64_t Aura::getCasterGuid() const
{
    return m_casterGuid;
}

void Aura::update(unsigned long diff, bool skipDurationCheck/* = false*/)
{
    // Do not update trash
    if (m_isGarbage)
        return;

    // Do not update aura while updating modifiers
    if (m_updatingModifiers)
        return;

    // Check aura effects if aura has been flagged for check
    if (m_checkAuraEffects)
    {
        if (m_auraEffectCount == 0)
        {
            // Aura is active but has no effects -> remove aura
            removeAura();
            return;
        }

        m_checkAuraEffects = false;
    }

    // Periodic auras
    for (uint8_t i = 0; i < MAX_SPELL_EFFECTS; ++i)
    {
        if (m_periodicTimer[i] == 0)
            continue;
        if (m_auraEffects[i].mAuraEffect == SPELL_AURA_NONE)
            continue;
        // Effects with 0 amplitude are not periodic
        if (m_auraEffects[i].mAmplitude <= 0)
            continue;

        m_periodicTimer[i] -= diff;
        if (m_periodicTimer[i] <= 0)
        {
            periodicTick(&m_auraEffects[i]);
            m_periodicTimer[i] += m_auraEffects[i].mAmplitude;
        }
    }

    // Check if aura is about to expire
    if (m_duration > 0 && !skipDurationCheck)
    {
        m_duration -= diff;
        if (m_duration < 0)
        {
            m_duration = 0;
            removeAura();
        }
    }
}

SpellInfo* Aura::getSpellInfo() const
{
    return m_spellInfo;
}

uint32_t Aura::getSpellId() const
{
    return getSpellInfo()->getId();
}

void Aura::_calculateCritChance()
{
    if (!canPeriodicEffectCrit())
    {
        m_critChance = 0.0f;
        return;
    }

    const auto casterUnit = GetUnitCaster();
    if (casterUnit == nullptr)
    {
        m_critChance = 0.0f;
        return;
    }

    auto usesHealing = false;
    for (uint8_t i = 0; i < MAX_SPELL_EFFECTS; ++i)
    {
        if (getAuraEffect(i).mAuraEffect == SPELL_AURA_NONE)
            continue;

        switch (getAuraEffect(i).mAuraEffect)
        {
            case SPELL_AURA_PERIODIC_HEAL:
            case SPELL_AURA_PERIODIC_HEAL_PCT:
            case SPELL_AURA_PERIODIC_HEALTH_FUNNEL:
                usesHealing = true;
                break;
            default:
                break;
        }

        if (usesHealing)
            break;
    }

    // Get snapshot of caster's current crit chance
    if (usesHealing)
        m_critChance = casterUnit->getCriticalChanceForHealSpell(getSpellInfo());
    else
        m_critChance = casterUnit->getCriticalChanceForDamageSpell(getSpellInfo(), getOwner());
}

void Aura::_calculateSpellPowerBonus()
{
    const auto casterUnit = GetUnitCaster();
    if (casterUnit == nullptr)
    {
        m_spellPowerBonus = 0;
        m_healPowerBonus = 0;
        return;
    }

    // Get snapshot of caster's current spell power
    m_spellPowerBonus = casterUnit->GetDamageDoneMod(getSpellInfo()->getFirstSchoolFromSchoolMask());
    m_healPowerBonus = casterUnit->HealDoneMod[getSpellInfo()->getFirstSchoolFromSchoolMask()];
}

void Aura::_calculateSpellHaste()
{
    // Reset modifier
    m_spellHaste = 1.0f;

    // Get snapshot of caster's current haste
    const auto caster = GetUnitCaster();
    if (_canHasteAffectDuration() && caster != nullptr)
        m_spellHaste = caster->getModCastSpeed();
}

void Aura::_calculateEffectAmplitude(uint8_t effIndex)
{
    int32_t amplitude = getSpellInfo()->getEffectAmplitude(effIndex);

    const auto caster = GetUnitCaster();
    if (caster != nullptr)
    {
        // Apply modifiers
        spellModFlatIntValue(caster->SM_FAmptitude, &amplitude, getSpellInfo()->getSpellFamilyFlags());
        spellModPercentageIntValue(caster->SM_PAmptitude, &amplitude, getSpellInfo()->getSpellFamilyFlags());
    }

    m_auraEffects[effIndex].mAmplitude = static_cast<int32_t>(amplitude * m_spellHaste);
}

bool Aura::_canHasteAffectDuration()
{
    if (getSpellInfo()->getAttributesExE() & ATTRIBUTESEXE_HASTE_AFFECTS_DURATION)
        return true;

    const auto caster = GetUnitCaster();
    if (caster == nullptr)
        return false;

    for (auto i = MAX_TOTAL_AURAS_START; i < MAX_TOTAL_AURAS_END; ++i)
    {
        Aura* aur = caster->m_auras[i];
        if (aur == nullptr || !aur->hasAuraEffect(SPELL_AURA_ALLOW_HASTE_AFFECT_DURATION))
            continue;

        // Check if caster has an aura which allows haste to modify duration
        if (aur->getSpellInfo()->isAuraEffectAffectingSpell(SPELL_AURA_ALLOW_HASTE_AFFECT_DURATION, getSpellInfo()))
            return true;
    }

    return false;
}

bool Aura::_checkNegative() const
{
    if (m_spellInfo->custom_c_is_flags & SPELL_FLAG_IS_FORCEDDEBUFF)
        return true;

    if (m_spellInfo->custom_c_is_flags & SPELL_FLAG_IS_FORCEDBUFF)
        return false;

    if (m_spellInfo->getAttributes() & ATTRIBUTES_NEGATIVE)
        return true;

    // Check each effect
    // If any effect contain one of the following aura effects, the aura is negative
    for (uint8_t i = 0; i < MAX_SPELL_EFFECTS; ++i)
    {
        if (getSpellInfo()->getEffectApplyAuraName(i) == SPELL_AURA_NONE)
            continue;

        switch (getSpellInfo()->getEffectApplyAuraName(i))
        {
            //\ todo: add more checks later
            case SPELL_AURA_PERIODIC_DAMAGE:
            case SPELL_AURA_PERIODIC_LEECH:
            case SPELL_AURA_PERIODIC_MANA_LEECH:
            case SPELL_AURA_PERIODIC_DAMAGE_PERCENT:
            case SPELL_AURA_PERIODIC_POWER_BURN:
                // No need to do other checks, definitely negative
                return true;
            default:
                break;
        }
    }

    return false;
}

void Aura::periodicTick(AuraEffectModifier* aurEff)
{
    switch (aurEff->mAuraEffect)
    {
        case SPELL_AURA_PERIODIC_DAMAGE:
        {
            const auto staticDamage = false;
            const auto casterUnit = GetUnitCaster();

            if (casterUnit != nullptr)
                casterUnit->doSpellDamage(getOwner(), getSpellId(), aurEff->mDamage, aurEff->effIndex, true, staticDamage, true, false, false, this, aurEff);
            else
                getOwner()->doSpellDamage(getOwner(), getSpellId(), aurEff->mDamage, aurEff->effIndex, true, staticDamage, true, false, false, this, aurEff);
        } break;
        case SPELL_AURA_PERIODIC_HEAL:
        {
            if (!getOwner()->isAlive())
                return;

            const auto staticDamage = false;
            const auto casterUnit = GetUnitCaster();

            if (casterUnit != nullptr)
                casterUnit->doSpellHealing(getOwner(), getSpellId(), aurEff->mDamage, true, staticDamage, true, false, false, this, aurEff);
            else
                getOwner()->doSpellHealing(getOwner(), getSpellId(), aurEff->mDamage, true, staticDamage, true, false, false, this, aurEff);

            if (getSpellInfo()->getAuraInterruptFlags() & AURA_INTERRUPT_ON_STAND_UP)
                getOwner()->Emote(EMOTE_ONESHOT_EAT);

            // Hackfixes from legacy method
            if (casterUnit != nullptr)
            {
                if (aurEff->mDamage > 0)
                {
                    switch (getSpellInfo()->getId())
                    {
                        //SPELL_HASH_HEALTH_FUNNEL
                        case 755:
                        case 3698:
                        case 3699:
                        case 3700:
                        case 11693:
                        case 11694:
                        case 11695:
                        case 16569:
                        case 27259:
                        case 40671:
                        case 46467:
                        case 47856:
                        case 60829:
                        {
                            dealdamage sdmg;

                            sdmg.full_damage = aurEff->mDamage;
                            sdmg.resisted_damage = 0;
                            sdmg.school_type = 0;
                            casterUnit->DealDamage(casterUnit, aurEff->mDamage, 0, 0, 0);
                            casterUnit->SendAttackerStateUpdate(casterUnit, casterUnit, &sdmg, aurEff->mDamage, 0, 0, 0, ATTACK);
                        } break;
                        default:
                            break;
                    }
                }
            }
        } break;
        case SPELL_AURA_PERIODIC_HEAL_PCT:
        {
            if (!getOwner()->isAlive())
                return;

            const auto staticDamage = true;
            const auto casterUnit = GetUnitCaster();
            const auto healAmt = static_cast<int32_t>(getOwner()->getMaxHealth() * (aurEff->mDamage / 100.0f));

            if (casterUnit != nullptr)
                casterUnit->doSpellHealing(getOwner(), getSpellId(), healAmt, true, staticDamage, true, false, false, this, aurEff);
            else
                getOwner()->doSpellHealing(getOwner(), getSpellId(), healAmt, true, staticDamage, true, false, false, this, aurEff);

            if (getSpellInfo()->getAuraInterruptFlags() & AURA_INTERRUPT_ON_STAND_UP)
                getOwner()->Emote(EMOTE_ONESHOT_EAT);
        } break;
        case SPELL_AURA_PERIODIC_POWER_PCT:
        {
            if (!getOwner()->isAlive())
                return;

            // Hackfix from legacy method
            const auto spellId = getSpellId() == 60069 ? 49766 : getSpellId();

            const auto casterUnit = GetUnitCaster();
            const auto powerType = static_cast<PowerType>(aurEff->miscValue);
            const auto powerAmt = static_cast<int32_t>(getOwner()->getMaxPower(powerType) * (aurEff->mDamage / 100.0f));

            if (casterUnit != nullptr)
                casterUnit->energize(getOwner(), spellId, powerAmt, powerType);
            else
                getOwner()->energize(getOwner(), spellId, powerAmt, powerType);

            getOwner()->sendPeriodicAuraLog(m_casterGuid, getOwner()->GetNewGUID(), getSpellInfo(), powerAmt, 0, 0, 0, aurEff->mAuraEffect, false, powerType);

            if (getSpellInfo()->getAuraInterruptFlags() & AURA_INTERRUPT_ON_STAND_UP && aurEff->miscValue == POWER_TYPE_MANA)
                getOwner()->Emote(EMOTE_ONESHOT_EAT);
        } break;
        case SPELL_AURA_PERIODIC_TRIGGER_SPELL:
        {
            const auto triggerSpell = sSpellMgr.getSpellInfo(getSpellInfo()->getEffectTriggerSpell(aurEff->effIndex));
            const auto casterUnit = GetUnitCaster();
            // This is bad, we are using GenerateTargets to get correct target for periodic target
            // but it is very very inaccurate and should never be used
            // This should be fixed soon
            if (casterUnit != nullptr)
                casterUnit->castSpell(nullptr, triggerSpell, true);
        } break;
        case SPELL_AURA_PERIODIC_ENERGIZE:
        {
            if (!getOwner()->isAlive())
                return;

            const auto casterUnit = GetUnitCaster();
            const auto powerType = static_cast<PowerType>(aurEff->miscValue);

            // Send packet first
            getOwner()->sendPeriodicAuraLog(m_casterGuid, getOwner()->GetNewGUID(), getSpellInfo(), aurEff->mDamage, 0, 0, 0, aurEff->mAuraEffect, false, powerType);

            if (casterUnit != nullptr)
                casterUnit->energize(getOwner(), getSpellId(), aurEff->mDamage, powerType, false);
            else
                getOwner()->energize(getOwner(), getSpellId(), aurEff->mDamage, powerType, false);

            if (getSpellInfo()->getAuraInterruptFlags() & AURA_INTERRUPT_ON_STAND_UP && aurEff->miscValue == POWER_TYPE_MANA)
                getOwner()->Emote(EMOTE_ONESHOT_EAT);
        } break;
        case SPELL_AURA_PERIODIC_LEECH:
        {
            const auto staticDamage = false;
            const auto casterUnit = GetUnitCaster();

            // Deal damage (heal part is called in doSpellDamage)
            if (casterUnit != nullptr)
                casterUnit->doSpellDamage(getOwner(), getSpellId(), aurEff->mDamage, aurEff->effIndex, true, staticDamage, true, true, false, this, aurEff);
            else
                getOwner()->doSpellDamage(getOwner(), getSpellId(), aurEff->mDamage, aurEff->effIndex, true, staticDamage, true, true, false, this, aurEff);
        } break;
        case SPELL_AURA_PERIODIC_HEALTH_FUNNEL:
        {
            if (!getOwner()->isAlive())
                return;

            const auto staticDamage = true;
            const auto casterUnit = GetUnitCaster();

            // Deal damage
            int32_t damageInflicted = 0;
            if (casterUnit != nullptr)
                damageInflicted = casterUnit->doSpellDamage(getOwner(), getSpellId(), aurEff->mDamage, aurEff->effIndex, true, staticDamage, true, false, false, this, aurEff);
            else
                damageInflicted = getOwner()->doSpellDamage(getOwner(), getSpellId(), aurEff->mDamage, aurEff->effIndex, true, staticDamage, true, false, false, this, aurEff);

            // If caster does not exist or is not alive, health cannot be leeched
            if (casterUnit == nullptr || !casterUnit->isAlive())
                return;

            // Heal caster
            const auto healAmount = static_cast<uint32_t>(damageInflicted * getSpellInfo()->getEffectMultipleValue(aurEff->effIndex));
            casterUnit->doSpellHealing(casterUnit, getSpellId(), healAmount, true, false, true, false, false, this, aurEff);
        } break;
        case SPELL_AURA_PERIODIC_MANA_LEECH:
        {
            if (!getOwner()->isAlive())
                return;

            const auto casterUnit = GetUnitCaster();
            if (getOwner()->getMaxPower(POWER_TYPE_MANA) == 0)
                return;

            if (getOwner()->SchoolImmunityList[getSpellInfo()->getFirstSchoolFromSchoolMask()] != 0)
            {
                if (casterUnit != nullptr)
                    casterUnit->sendSpellOrDamageImmune(getCasterGuid(), getOwner(), getSpellId());

                return;
            }

            const auto manaLeech = std::min(aurEff->mDamage, static_cast<int32_t>(getOwner()->getPower(POWER_TYPE_MANA)));
            getOwner()->modPower(POWER_TYPE_MANA, -manaLeech);

            // If caster does not exist or is not alive, mana cannot be leeched
            if (casterUnit == nullptr || !casterUnit->isAlive())
                return;

            const auto manaMultiplier = getSpellInfo()->getEffectMultipleValue(aurEff->effIndex);
            const auto manaReturn = static_cast<int32_t>(manaLeech * manaMultiplier);

            // Add leeched mana to caster
            const auto curPower = casterUnit->getPower(POWER_TYPE_MANA);
            const auto maxPower = casterUnit->getMaxPower(POWER_TYPE_MANA);
            if (curPower + manaReturn >= maxPower)
                casterUnit->setPower(POWER_TYPE_MANA, maxPower);
            else
                casterUnit->modPower(POWER_TYPE_MANA, manaReturn);

            getOwner()->sendPeriodicAuraLog(m_casterGuid, getOwner()->GetNewGUID(), getSpellInfo(), manaReturn, 0, 0, 0, aurEff->mAuraEffect, false, POWER_TYPE_MANA, manaMultiplier);
        } break;
        case SPELL_AURA_PERIODIC_DAMAGE_PERCENT:
        {
            const auto staticDamage = true;
            const auto casterUnit = GetUnitCaster();

            const auto dmg = static_cast<int32_t>(aurEff->mDamage / 100.0f * getOwner()->getMaxHealth());

            if (casterUnit != nullptr)
                casterUnit->doSpellDamage(getOwner(), getSpellId(), dmg, aurEff->effIndex, pSpellId == 0, staticDamage, true, false, false, this, aurEff);
            else
                getOwner()->doSpellDamage(getOwner(), getSpellId(), dmg, aurEff->effIndex, pSpellId == 0, staticDamage, true, false, false, this, aurEff);
        } break;
        case SPELL_AURA_PERIODIC_POWER_BURN:
        {
            if (!getOwner()->isAlive())
                return;

            const auto staticDamage = true;
            const auto casterUnit = GetUnitCaster();

            if (getOwner()->SchoolImmunityList[getSpellInfo()->getFirstSchoolFromSchoolMask()] != 0)
            {
                if (casterUnit != nullptr)
                    casterUnit->sendSpellOrDamageImmune(getCasterGuid(), getOwner(), getSpellId());

                return;
            }

            const auto powerType = static_cast<PowerType>(aurEff->miscValue);
            if (getOwner()->getMaxPower(powerType) == 0)
                return;

            // Calculate power amount
            const auto powerAmount = std::min(static_cast<uint32_t>(aurEff->mDamage), getOwner()->getPower(powerType));
            getOwner()->modPower(powerType, -static_cast<int32_t>(powerAmount));

            auto damage = static_cast<uint32_t>(powerAmount * getSpellInfo()->getEffectMultipleValue(aurEff->effIndex));
            // Should do at least 1 damage (see Chromaggus and Brood Affliction: Blue)
            ///\ todo: verify this
            damage = std::max(1u, damage);

            if (casterUnit != nullptr)
                casterUnit->doSpellDamage(getOwner(), getSpellId(), damage, aurEff->effIndex, false, staticDamage, true, false, false, this, aurEff);
            else
                getOwner()->doSpellDamage(getOwner(), getSpellId(), damage, aurEff->effIndex, false, staticDamage, true, false, false, this, aurEff);
        } break;
        case SPELL_AURA_PERIODIC_TRIGGER_DUMMY:
        {
            if (!sScriptMgr.CallScriptedDummyAura(getSpellId(), aurEff->effIndex, this, true))
                LogDebugFlag(LF_AURA, "Spell aura %u has a periodic trigger dummy effect but no handler for it", getSpellId());
        } break;
        case SPELL_AURA_PERIODIC_TRIGGER_SPELL_WITH_VALUE:
        {
            const auto triggeredInfo = sSpellMgr.getSpellInfo(getSpellInfo()->getEffectTriggerSpell(aurEff->effIndex));
            const auto casterUnit = GetUnitCaster();
            // This is bad, we are using GenerateTargets to get correct target for periodic target
            // but it is very very inaccurate and should never be used
            // This should be fixed soon
            if (casterUnit != nullptr)
                casterUnit->castSpell(nullptr, triggeredInfo, aurEff->mDamage, true);
        } break;
        default:
            break;
    }
}
