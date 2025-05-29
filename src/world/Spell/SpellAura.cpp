/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "SpellAura.hpp"

#include "Spell/Spell.hpp"
#include "Spell/SpellInfo.hpp"
#include "SpellCastTargets.hpp"
#include "Definitions/SpellCastTargetFlags.hpp"
#include "Definitions/SpellIsFlags.hpp"
#include "Definitions/SpellMechanics.hpp"
#include "Definitions/SpellTypes.hpp"
#include "SpellMgr.hpp"
#include "Definitions/SpellEffects.hpp"
#include "Definitions/SpellFamily.hpp"
#include "Logging/Logger.hpp"
#include "Objects/Units/Players/Player.hpp"
#include "Server/EventMgr.h"
#include "Server/Script/HookInterface.hpp"
#include "Server/Script/ScriptMgr.hpp"

extern const char* SpellAuraNames[TOTAL_SPELL_AURAS];

AuraEffectModifier const* Aura::getAuraEffect(uint8_t effIndex) const
{
    return &m_auraEffects[effIndex];
}

AuraEffectModifier* Aura::getModifiableAuraEffect(uint8_t effIndex)
{
    return &m_auraEffects[effIndex];
}

bool Aura::hasAuraEffect(AuraEffect auraEffect) const
{
    for (uint8_t i = 0; i < MAX_SPELL_EFFECTS; ++i)
    {
        if (m_auraEffects[i].getAuraEffectType() == auraEffect)
            return true;
    }

    return false;
}

void Aura::addAuraEffect(AuraEffect auraEffect, int32_t damage, int32_t miscValue, float_t effectPctModifier, bool isStaticDamage, uint8_t effIndex, bool reapplying/* = false*/)
{
    if (effIndex >= MAX_SPELL_EFFECTS)
        return;

    if (auraEffect >= TOTAL_SPELL_AURAS)
    {
        sLogger.failure("Aura::addAuraEffect : Unknown aura effect type {} for spell id {}", auraEffect, getSpellId());
        return;
    }

    if (m_auraEffects[effIndex].getAuraEffectType() != SPELL_AURA_NONE)
    {
        sLogger.failure("Aura::addAuraEffect : Tried to add effect to index {} but effect already exists", effIndex);
        return;
    }

    m_auraEffects[effIndex].setAuraEffectType(auraEffect);
    m_auraEffects[effIndex].setEffectDamage(reapplying ? damage * getStackCount() : damage);
    m_auraEffects[effIndex].setEffectBaseDamage(damage);
    m_auraEffects[effIndex].setEffectMiscValue(miscValue);
    m_auraEffects[effIndex].setEffectPercentModifier(effectPctModifier);
    m_auraEffects[effIndex].setEffectDamageStatic(isStaticDamage);
    m_auraEffects[effIndex].setEffectIndex(effIndex);
    m_auraEffects[effIndex].setAura(this);
    ++m_auraEffectCount;

    // Add aura effect to unit only if aura has a slot
    if (m_auraSlot != 0xFFFF)
        getOwner()->_addAuraEffect(&m_auraEffects[effIndex]);

    if (!reapplying)
    {
        // Calculate effect amplitude
        _calculateEffectAmplitude(effIndex);
    }
}

void Aura::addAuraEffect(AuraEffectModifier const* auraEffect, bool reapplying/* = false*/)
{
    if (auraEffect == nullptr)
        return;

    addAuraEffect(auraEffect->getAuraEffectType(), auraEffect->getEffectBaseDamage(), auraEffect->getEffectMiscValue(),
        auraEffect->getEffectPercentModifier(), auraEffect->isEffectDamageStatic(), auraEffect->getEffectIndex(), reapplying);
}

void Aura::removeAuraEffect(uint8_t effIndex, bool reapplying/* = false*/)
{
    if (effIndex >= MAX_SPELL_EFFECTS)
        return;

    // Remove aura effect from unit
    getOwner()->_removeAuraEffect(&m_auraEffects[effIndex]);

    if (!reapplying)
    {
        // Unapply the modifier
        m_auraEffects[effIndex].applyEffect(false);
    }

    m_auraEffects[effIndex].setAuraEffectType(SPELL_AURA_NONE);
    m_auraEffects[effIndex].setEffectDamage(0.0f);
    m_auraEffects[effIndex].setEffectBaseDamage(0);
    m_auraEffects[effIndex].setEffectFixedDamage(0);
    m_auraEffects[effIndex].setEffectMiscValue(0);
    m_auraEffects[effIndex].setEffectAmplitude(0);
    m_auraEffects[effIndex].setEffectDamageFraction(0.0f);
    m_auraEffects[effIndex].setEffectPercentModifier(1.0f);
    m_auraEffects[effIndex].setEffectDamageStatic(false);
    m_auraEffects[effIndex].setEffectIndex(0);
    m_auraEffects[effIndex].setAura(nullptr);
    --m_auraEffectCount;

    if (!reapplying)
    {
        // Check aura effects on next update
        m_checkAuraEffects = true;
    }
}

void Aura::removeAllAuraEffects()
{
    for (uint8_t i = 0; i < MAX_SPELL_EFFECTS; ++i)
    {
        if (m_auraEffects[i].getAuraEffectType() == SPELL_AURA_NONE)
            continue;

        removeAuraEffect(i);
    }
}

uint8_t Aura::getAppliedEffectCount() const
{
    return m_auraEffectCount;
}

uint16_t Aura::getAuraSlot() const
{
    return m_auraSlot;
}

void Aura::setAuraSlot(uint16_t slot)
{
    m_auraSlot = slot;
}

int32_t Aura::getEffectDamage(uint8_t effIndex) const
{
    if (effIndex >= MAX_SPELL_EFFECTS)
        return 0;

    return m_auraEffects[effIndex].getEffectDamage();
}

int32_t Aura::getEffectDamageByEffect(AuraEffect auraEffect) const
{
    for (uint8_t i = 0; i < MAX_SPELL_EFFECTS; ++i)
    {
        if (m_auraEffects[i].getAuraEffectType() == auraEffect)
            return m_auraEffects[i].getEffectDamage();
    }

    return 0;
}

void Aura::removeAura(AuraRemoveMode mode/* = AURA_REMOVE_BY_SERVER*/)
{
    sEventMgr.RemoveEvents(this);

    // Check if aura has already been removed
    if (m_isGarbage)
        return;

    sScriptMgr.callScriptedAuraOnRemove(this, mode);
    sHookInterface.OnAuraRemove(this);

    sLogger.debug("Removing aura {} from unit {}", getSpellId(), getOwner()->getGuid());

    m_isGarbage = true;

    // Reset diminishing return timer
    getOwner()->removeDiminishingReturnTimer(getSpellInfo());

    const auto caster = GetUnitCaster();
    if (caster != nullptr)
    {
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
            uint8_t j = 0;
            for (; j < MAX_SPELL_EFFECTS; ++j)
                if (getSpellInfo()->getEffect(j) == SPELL_EFFECT_ADD_FARSIGHT)
                    break;

            if (j != MAX_SPELL_EFFECTS)
                static_cast<Player*>(caster)->setFarsightGuid(0);
        }

        // If this aura can affect one target at a time, remove this target from the caster map
        if (getSpellInfo()->getAttributesExE() & ATTRIBUTESEXE_SINGLE_TARGET_AURA && getOwner()->getAuraCountForId(getSpellId()) == 1)
            getOwner()->removeSingleTargetGuidForAura(getSpellInfo()->getId());

        // only remove channel stuff if caster == target, then it's not removed twice, for example, arcane missiles applies a dummy aura to target
        if (caster == getOwner() && m_spellInfo->getChannelInterruptFlags() != 0)
        {
            caster->setChannelObjectGuid(0);
            caster->setChannelSpellId(0);
        }

        if (caster->isPlayer() && getSpellInfo()->hasEffect(SPELL_EFFECT_SUMMON))
        {
            const auto charm = caster->getWorldMapUnit(caster->getCharmGuid());
            if (charm != nullptr && charm->getCreatedBySpellId() == getSpellInfo()->getId())
                caster->unPossess();
        }
    }

    // Clear area aura targets
    if (IsAreaAura() && getCasterGuid() == getOwner()->getGuid())
        ClearAATargets();

    // Send packet before removing modifiers
    // Otherwise shapeshift spells can still appear active on stance bar -Appled
    if (m_visualSlot < AuraSlots::NEGATIVE_VISUAL_SLOT_END)
    {
#if VERSION_STRING < WotLK
        getOwner()->setAura(this, false);
        getOwner()->setAuraFlags(this, false);
        getOwner()->setAuraLevel(this);
        getOwner()->setAuraApplication(this);
#endif

        getOwner()->m_auraVisualList[m_visualSlot] = 0;
        getOwner()->sendAuraUpdate(this, true);
        getOwner()->updateAuraForGroup(m_visualSlot);
    }

    // Remove aura from unit before removing modifiers
    auto auraHolder = getOwner()->_removeAura(this);

    // Remove all modifiers
    removeAllAuraEffects();

    // Remove aurastates
    if (getSpellInfo()->getMechanicsType() == MECHANIC_ENRAGED && !--m_target->m_ascEnraged)
        getOwner()->removeAuraStateAndAuras(AURASTATE_FLAG_ENRAGED);
    else if (getSpellInfo()->getMechanicsType() == MECHANIC_BLEEDING && !--m_target->m_ascBleed)
        getOwner()->removeAuraStateAndAuras(AURASTATE_FLAG_BLEED);
    if (getSpellInfo()->custom_BGR_one_buff_on_target & SPELL_TYPE_SEAL && !--m_target->m_ascSeal)
        getOwner()->removeAuraStateAndAuras(AURASTATE_FLAG_JUDGEMENT);

    getOwner()->addGarbageAura(std::move(auraHolder));
}

bool Aura::isDeleted() const
{
    return m_isGarbage;
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

    for (const auto& aurEff : caster->getAuraEffectList(SPELL_AURA_ALLOW_DOT_TO_CRIT))
    {
        if (aurEff->getAura()->getSpellInfo()->isAuraEffectAffectingSpell(SPELL_AURA_ALLOW_DOT_TO_CRIT, spellInfo))
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

void Aura::applyModifiers(bool apply, AuraEffect applyOnlyFor/* = SPELL_AURA_NONE*/)
{
    for (uint8_t i = 0; i < MAX_SPELL_EFFECTS; ++i)
    {
        if (m_auraEffects[i].getAuraEffectType() == SPELL_AURA_NONE)
            continue;

        if (applyOnlyFor != SPELL_AURA_NONE && m_auraEffects[i].getAuraEffectType() != applyOnlyFor)
            continue;

        m_auraEffects[i].applyEffect(apply);

        sLogger.debugFlag(AscEmu::Logging::LF_AURA, "Aura::applyModifiers : Spell Id {}, Aura Effect {} ({}), Target GUID {}, EffectIndex {}, Duration {}, Damage {}, MiscValue {}",
            getSpellInfo()->getId(), m_auraEffects[i].getAuraEffectType(), SpellAuraNames[m_auraEffects[i].getAuraEffectType()], getOwner()->getGuid(), i, getTimeLeft(), m_auraEffects[i].getEffectDamage(), m_auraEffects[i].getEffectMiscValue());
    }

    // Modifiers are applied => aura can be updated now
    m_updatingModifiers = false;
}

void Aura::updateModifiers()
{
    m_updatingModifiers = true;
    for (uint8_t i = 0; i < MAX_SPELL_EFFECTS; ++i)
    {
        switch (m_auraEffects[i].getAuraEffectType())
        {
            case SPELL_AURA_MOD_DECREASE_SPEED:
                UpdateAuraModDecreaseSpeed(&m_auraEffects[i]);
                break;
#if VERSION_STRING >= TBC
#if VERSION_STRING >= WotLK
            case SPELL_AURA_MOD_ATTACK_POWER_BY_STAT_PCT:
#endif
            case SPELL_AURA_MOD_RANGED_ATTACK_POWER_BY_STAT_PCT:
                m_auraEffects[i].applyEffect(false);
                m_auraEffects[i].applyEffect(true);
                break;
#endif
            default:
                break;
        }

        sLogger.debug("Aura::updateModifiers : Spell Id {}, Aura Effect {} ({}), Target GUID {}, EffectIndex {}, Duration {}, Damage {}, MiscValue {}",
            getSpellInfo()->getId(), m_auraEffects[i].getAuraEffectType(), SpellAuraNames[m_auraEffects[i].getAuraEffectType()], getOwner()->getGuid(), i, getTimeLeft(), m_auraEffects[i].getEffectDamage(), m_auraEffects[i].getEffectMiscValue());
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

int32_t Aura::getOriginalDuration() const
{
    return m_originalDuration;
}

void Aura::setOriginalDuration(int32_t dur)
{
    m_originalDuration = dur;
}

void Aura::setNewMaxDuration(int32_t dur, bool refreshDuration/* = true*/)
{
    m_originalDuration = dur;

    if (refreshDuration)
    {
        refreshOrModifyStack();
    }
    else
    {
        const auto newDur = static_cast<int32_t>(dur * m_spellHaste);
        setMaxDuration(newDur);
        setTimeLeft(newDur);
    }
}

uint16_t Aura::getPeriodicTickCountForEffect(uint8_t effIndex) const
{
    if (m_auraEffects[effIndex].getAuraEffectType() == SPELL_AURA_NONE)
        return 1;

    if (m_auraEffects[effIndex].getEffectAmplitude() == 0 || getMaxDuration() == -1)
        return 1;

    // Never return 0 to avoid division by zero later
    return std::max(static_cast<uint16_t>(1), static_cast<uint16_t>(getMaxDuration() / m_auraEffects[effIndex].getEffectAmplitude()));
}

void Aura::refreshOrModifyStack([[maybe_unused]]bool saveMods/* = false*/, int16_t modifyStackAmount/* = 0*/)
{
    int32_t maxStacks = getSpellInfo()->getMaxstack() == 0 ? 1 : static_cast<int32_t>(getSpellInfo()->getMaxstack());

    // Check for aura stack cheat
    const auto plrHolder = getPlayerOwner();
    if (plrHolder != nullptr && plrHolder->m_cheats.hasAuraStackCheat)
        maxStacks = 255;

    const auto curStackCount = getStackCount();
    int32_t newStackCount = curStackCount + modifyStackAmount;

    if (modifyStackAmount < 0)
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
        // Before cata, aura's crit chance, attack power bonus and damage bonuses (NYI) were saved on aura refresh in special situations
        if (!saveMods)
        {
            _calculateAttackPowerBonus();
            _calculateCritChance();
        }
#else
        _calculateAttackPowerBonus();
        _calculateCritChance();
#endif
        _calculateSpellPowerBonus();
        _calculateSpellHaste();

        // Recalculate aura duration and effect amplitudes based on new haste
        setMaxDuration(static_cast<int32_t>(m_originalDuration * m_spellHaste));
        for (uint8_t i = 0; i < MAX_SPELL_EFFECTS; ++i)
        {
            if (m_auraEffects[i].getAuraEffectType() == SPELL_AURA_NONE)
                continue;

            _calculateEffectAmplitude(i);
        }

        // Reset charges to max count
        if (m_originalCharges > 0)
            setCharges(m_originalCharges, false);

        // Reset duration to max
        setTimeLeft(getMaxDuration());
    }

    m_updatingModifiers = true;
    const auto unsignedStackCount = static_cast<uint8_t>(newStackCount);
    m_stackCount = unsignedStackCount;

    // Reapply modifiers
    for (uint8_t i = 0; i < MAX_SPELL_EFFECTS; ++i)
    {
        if (m_auraEffects[i].getAuraEffectType() != SPELL_AURA_NONE)
        {
            m_auraEffects[i].applyEffect(false, true);
            m_auraEffects[i].setEffectDamage(m_auraEffects[i].getEffectBaseDamage() * m_stackCount);
            m_auraEffects[i].applyEffect(true, true);
        }
    }

    m_updatingModifiers = false;
    takeUsedSpellModifiers();

    // Call script hook
    sScriptMgr.callScriptedAuraOnRefreshOrGainNewStack(this, unsignedStackCount, curStackCount);

#if VERSION_STRING < WotLK
    getOwner()->setAuraApplication(this);
#endif

    // Send aura update
    getOwner()->sendAuraUpdate(this, false);
    getOwner()->updateAuraForGroup(m_visualSlot);
}

uint8_t Aura::getStackCount() const
{
    return m_stackCount;
}

uint16_t Aura::getCharges() const
{
    return m_charges;
}

void Aura::setCharges(uint16_t count, bool sendUpdatePacket/* = true*/)
{
    if (m_charges == count)
        return;

    m_charges = count;
    if (sendUpdatePacket)
    {
#if VERSION_STRING < WotLK
        getOwner()->setAuraApplication(this);
#else
        getOwner()->sendAuraUpdate(this, false);
#endif
    }
}

void Aura::removeCharge()
{
    auto charges = getCharges();
    if (charges == 0)
        return;

    --charges;
    if (charges == 0)
    {
        removeAura();
        return;
    }

    setCharges(charges);
}

uint8_t Aura::getAuraFlags() const
{
    ///\ todo: these seem to be wrong for tbc
    uint8_t auraFlags = isNegative() ? AFLAG_NEGATIVE : AFLAG_CANCELLABLE;

    for (uint8_t i = 0; i < MAX_SPELL_EFFECTS; ++i)
    {
        // Check if aura has effects in all indexes
        if (m_auraEffects[i].getAuraEffectType() != SPELL_AURA_NONE)
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

uint32_t Aura::getAttackPowerBonus() const
{
    return m_attackPowerBonus;
}

void Aura::addUsedSpellModifier(AuraEffectModifier const* aurEff)
{
    m_usedModifiers.insert(std::make_pair(aurEff, false));
}

void Aura::removeUsedSpellModifier(AuraEffectModifier const* aurEff)
{
    if (m_usedModifiers.empty())
        return;

    // Mark the spell modifier as removed to prevent memory corruption
    for (auto& usedMod : m_usedModifiers)
    {
        if (usedMod.first == aurEff)
        {
            usedMod.second = true;
            break;
        }
    }
}

void Aura::takeUsedSpellModifiers()
{
    if (m_usedModifiers.empty())
        return;

    for (auto itr = m_usedModifiers.begin(); itr != m_usedModifiers.end();)
    {
        auto aurEff = (*itr).first;
        // Check for faulty entry
        if (aurEff->getAura() == nullptr || (*itr).second)
        {
            itr = m_usedModifiers.erase(itr);
            continue;
        }

        aurEff->getAura()->removeCharge();
        itr = m_usedModifiers.erase(itr);
    }
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
        return getOwner()->getWorldMapObject(getCasterGuid());

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
        if (m_auraEffects[i].getAuraEffectType() == SPELL_AURA_NONE)
            continue;
        // Effects with 0 amplitude are not periodic
        if (m_auraEffects[i].getEffectAmplitude() <= 0)
            continue;

        m_periodicTimer[i] -= diff;
        if (m_periodicTimer[i] <= 0)
        {
            periodicTick(&m_auraEffects[i]);
            m_periodicTimer[i] += m_auraEffects[i].getEffectAmplitude();
        }
    }

    // Check if aura is about to expire
    if (m_duration > 0 && !skipDurationCheck)
    {
        m_duration -= diff;
        if (m_duration <= 0)
        {
            m_duration = 0;
            removeAura(AURA_REMOVE_ON_EXPIRE);
        }
    }
}

bool Aura::isAbsorbAura() const
{
    return false;
}

SpellInfo const* Aura::getSpellInfo() const
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
        if (getAuraEffect(i)->getAuraEffectType() == SPELL_AURA_NONE)
            continue;

        switch (getAuraEffect(i)->getAuraEffectType())
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
        m_critChance = casterUnit->getCriticalChanceForHealSpell(nullptr, this, getOwner());
    else
        m_critChance = casterUnit->getCriticalChanceForDamageSpell(nullptr, this, getOwner());
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
    m_healPowerBonus = casterUnit->m_healDoneMod[getSpellInfo()->getFirstSchoolFromSchoolMask()];
}

void Aura::_calculateAttackPowerBonus()
{
    const auto casterUnit = GetUnitCaster();
    if (casterUnit == nullptr)
    {
        m_attackPowerBonus = 0;
        return;
    }

    // Get snapshot of caster's current attack power
    m_attackPowerBonus = casterUnit->getAttackPower();
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
    int32_t amplitude = static_cast<int32_t>(getSpellInfo()->getEffectAmplitude(effIndex));

    const auto caster = GetUnitCaster();
    if (caster != nullptr)
    {
        // Apply modifiers
        caster->applySpellModifiers(SPELLMOD_AMPTITUDE, &amplitude, getSpellInfo(), nullptr, this);
    }

    m_auraEffects[effIndex].setEffectAmplitude(static_cast<int32_t>(amplitude * m_spellHaste));
}

bool Aura::_canHasteAffectDuration()
{
    if (getSpellInfo()->getAttributesExE() & ATTRIBUTESEXE_HASTE_AFFECTS_DURATION)
        return true;

#if VERSION_STRING >= WotLK
    const auto caster = GetUnitCaster();
    if (caster == nullptr)
        return false;

    for (const auto& aurEff : caster->getAuraEffectList(SPELL_AURA_ALLOW_HASTE_AFFECT_DURATION))
    {
        // Check if caster has an aura which allows haste to modify duration
        if (aurEff->getAura()->getSpellInfo()->isAuraEffectAffectingSpell(SPELL_AURA_ALLOW_HASTE_AFFECT_DURATION, getSpellInfo()))
            return true;
    }
#endif

    return false;
}

void Aura::periodicTick(AuraEffectModifier* aurEff)
{
    auto effectFloatValue = aurEff->getEffectFloatDamage();
    const auto scriptResult = sScriptMgr.callScriptedAuraOnPeriodicTick(this, aurEff, &effectFloatValue);
    // If script prevents default action, do not continue
    if (scriptResult == SpellScriptExecuteState::EXECUTE_PREVENT)
        return;

    int32_t customDamage = 0;
    auto effectIntValue = static_cast<int32_t>(std::ceil(effectFloatValue));

    switch (aurEff->getAuraEffectType())
    {
        case SPELL_AURA_PERIODIC_DAMAGE:
        {
            const auto casterUnit = GetUnitCaster();

            if (casterUnit != nullptr)
                casterUnit->doSpellDamage(getOwner(), getSpellId(), effectFloatValue, aurEff->getEffectIndex(), pSpellId != 0, true, false, false, nullptr, this, aurEff);
            else
                getOwner()->doSpellDamage(getOwner(), getSpellId(), effectFloatValue, aurEff->getEffectIndex(), pSpellId != 0, true, false, false, nullptr, this, aurEff);
        } break;
        case SPELL_AURA_PERIODIC_HEAL:
        {
            const auto casterUnit = GetUnitCaster();

            if (casterUnit != nullptr)
                casterUnit->doSpellHealing(getOwner(), getSpellId(), effectFloatValue, pSpellId != 0, true, false, false, nullptr, this, aurEff);
            else
                getOwner()->doSpellHealing(getOwner(), getSpellId(), effectFloatValue, pSpellId != 0, true, false, false, nullptr, this, aurEff);

            // Hackfixes from legacy method
            if (casterUnit != nullptr)
            {
                if (aurEff->getEffectDamage() > 0)
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
                            DamageInfo sdmg;

                            sdmg.fullDamage = aurEff->getEffectDamage();
                            sdmg.resistedDamage = 0;
                            sdmg.schoolMask = SchoolMask(getSpellInfo()->getSchoolMask());
                            casterUnit->dealDamage(casterUnit, static_cast<uint32_t>(aurEff->getEffectDamage()), 0);
                            casterUnit->sendAttackerStateUpdate(casterUnit->GetNewGUID(), casterUnit->GetNewGUID(), HITSTATUS_NORMALSWING, static_cast<uint32_t>(aurEff->getEffectDamage()), 0, sdmg, 0, VisualState::ATTACK, 0, 0);
                        } break;
                        default:
                            break;
                    }
                }
            }
        } break;
        case SPELL_AURA_PERIODIC_HEAL_PCT:
        {
            const auto casterUnit = GetUnitCaster();

            if (casterUnit != nullptr)
                casterUnit->doSpellHealing(getOwner(), getSpellId(), effectFloatValue, pSpellId != 0, true, false, false, nullptr, this, aurEff);
            else
                getOwner()->doSpellHealing(getOwner(), getSpellId(), effectFloatValue, pSpellId != 0, true, false, false, nullptr, this, aurEff);
        } break;
        case SPELL_AURA_PERIODIC_POWER_PCT:
        {
            if (!getOwner()->isAlive())
                return;

            const auto powerType = static_cast<PowerType>(aurEff->getEffectMiscValue());
            if (getOwner()->getMaxPower(powerType) == 0)
                return;

            // Hackfix from legacy method
            const auto spellId = getSpellId() == 60069 ? 49766 : getSpellId();
            const auto casterUnit = GetUnitCaster();

            // Send packet first
            const auto unsignedValue = static_cast<uint32_t>(effectIntValue);
            getOwner()->sendPeriodicAuraLog(m_casterGuid, getOwner()->GetNewGUID(), getSpellInfo(), unsignedValue, 0, 0, 0, aurEff->getAuraEffectType(), false, powerType);

            if (casterUnit != nullptr)
                casterUnit->energize(getOwner(), spellId, unsignedValue, powerType, false);
            else
                getOwner()->energize(getOwner(), spellId, unsignedValue, powerType, false);
        } break;
#if VERSION_STRING >= TBC
        case SPELL_AURA_PERIODIC_TRIGGER_SPELL_WITH_VALUE:
        {
            customDamage = effectIntValue;
        } [[fallthrough]];
#endif
        case SPELL_AURA_PERIODIC_TRIGGER_SPELL:
        {
            const auto triggerId = getSpellInfo()->getEffectTriggerSpell(aurEff->getEffectIndex());
            const auto triggerInfo = sSpellMgr.getSpellInfo(triggerId);
            if (triggerInfo == nullptr)
            {
                sLogger.failure("Aura::periodicTick : Periodic trigger aura effect has invalid spell id ({}) in aura id {}", triggerId, getSpellId());
                return;
            }

            auto* const originalCaster = GetUnitCaster();
            Unit* casterUnit = nullptr;
            auto* target = getOwner();

            // Note; some spells might need a spell script here to set correct caster and target

            if (triggerInfo->isTriggerSpellCastedByCaster(getSpellInfo()))
                casterUnit = originalCaster;
            else
                casterUnit = target;

            // If spell is channeled, periodic target should be the channel object
            if (originalCaster != nullptr && getSpellInfo()->isChanneled())
            {
                target = originalCaster->getWorldMapUnit(originalCaster->getChannelObjectGuid());
                if (target == nullptr)
                    target = getOwner();
            }

            if (casterUnit != nullptr)
            {
                Spell* triggerSpell = sSpellMgr.newSpell(casterUnit, triggerInfo, true, this);
#if VERSION_STRING >= TBC
                if (aurEff->getAuraEffectType() == SPELL_AURA_PERIODIC_TRIGGER_SPELL_WITH_VALUE)
                {
                    for (uint8_t i = 0; i < MAX_SPELL_EFFECTS; ++i)
                        triggerSpell->forced_basepoints->set(i, customDamage);
                }
#endif

                SpellCastTargets spellTargets(0);
                spellTargets.addTargetMask(TARGET_FLAG_UNIT);
                spellTargets.setUnitTarget(target->getGuid());

                triggerSpell->prepare(&spellTargets);
            }
        } break;
        case SPELL_AURA_PERIODIC_ENERGIZE:
        {
            if (!getOwner()->isAlive())
                return;

            const auto powerType = static_cast<PowerType>(aurEff->getEffectMiscValue());
            if (getOwner()->getMaxPower(powerType) == 0)
                return;

            const auto casterUnit = GetUnitCaster();

            // Send packet first
            const auto unsignedValue = static_cast<uint32_t>(effectIntValue);
            getOwner()->sendPeriodicAuraLog(m_casterGuid, getOwner()->GetNewGUID(), getSpellInfo(), unsignedValue, 0, 0, 0, aurEff->getAuraEffectType(), false, powerType);

            if (casterUnit != nullptr)
                casterUnit->energize(getOwner(), getSpellId(), unsignedValue, powerType, false);
            else
                getOwner()->energize(getOwner(), getSpellId(), unsignedValue, powerType, false);
        } break;
        case SPELL_AURA_PERIODIC_LEECH:
        case SPELL_AURA_PERIODIC_HEALTH_FUNNEL:
        {
            const auto casterUnit = GetUnitCaster();

            // Deal damage (heal part is called in doSpellDamage)
            if (casterUnit != nullptr)
                casterUnit->doSpellDamage(getOwner(), getSpellId(), effectFloatValue, aurEff->getEffectIndex(), pSpellId != 0, true, true, false, nullptr, this, aurEff);
            else
                getOwner()->doSpellDamage(getOwner(), getSpellId(), effectFloatValue, aurEff->getEffectIndex(), pSpellId != 0, true, true, false, nullptr, this, aurEff);
        } break;
        case SPELL_AURA_PERIODIC_MANA_LEECH:
        {
            if (!getOwner()->isAlive())
                return;

            if (getOwner()->getMaxPower(POWER_TYPE_MANA) == 0)
                return;

            const auto casterUnit = GetUnitCaster();
            if (getOwner()->m_schoolImmunityList[getSpellInfo()->getFirstSchoolFromSchoolMask()] != 0)
            {
                if (casterUnit != nullptr)
                    casterUnit->sendSpellOrDamageImmune(getCasterGuid(), getOwner(), getSpellId());

                return;
            }

            const auto manaLeech = std::min(effectIntValue, static_cast<int32_t>(getOwner()->getPower(POWER_TYPE_MANA)));
            getOwner()->modPower(POWER_TYPE_MANA, -manaLeech);

            // If caster does not exist or is not alive, mana cannot be leeched
            if (casterUnit == nullptr || !casterUnit->isAlive())
                return;

            const auto manaMultiplier = getSpellInfo()->getEffectMultipleValue(aurEff->getEffectIndex());
            const auto manaReturn = static_cast<int32_t>(manaLeech * manaMultiplier);

            // Add leeched mana to caster
            const auto curPower = casterUnit->getPower(POWER_TYPE_MANA);
            const auto maxPower = casterUnit->getMaxPower(POWER_TYPE_MANA);
            if (curPower + manaReturn >= maxPower)
                casterUnit->setPower(POWER_TYPE_MANA, maxPower);
            else
                casterUnit->modPower(POWER_TYPE_MANA, manaReturn);

            getOwner()->sendPeriodicAuraLog(m_casterGuid, getOwner()->GetNewGUID(), getSpellInfo(), static_cast<uint32_t>(manaReturn), 0, 0, 0, aurEff->getAuraEffectType(), false, POWER_TYPE_MANA, manaMultiplier);
        } break;
        case SPELL_AURA_PERIODIC_DAMAGE_PERCENT:
        {
            const auto casterUnit = GetUnitCaster();

            if (casterUnit != nullptr)
                casterUnit->doSpellDamage(getOwner(), getSpellId(), effectFloatValue, aurEff->getEffectIndex(), pSpellId != 0, true, false, false, nullptr, this, aurEff);
            else
                getOwner()->doSpellDamage(getOwner(), getSpellId(), effectFloatValue, aurEff->getEffectIndex(), pSpellId != 0, true, false, false, nullptr, this, aurEff);
        } break;
        case SPELL_AURA_PERIODIC_POWER_BURN:
        {
            if (!getOwner()->isAlive())
                return;

            const auto casterUnit = GetUnitCaster();

            if (getOwner()->m_schoolImmunityList[getSpellInfo()->getFirstSchoolFromSchoolMask()] != 0)
            {
                if (casterUnit != nullptr)
                    casterUnit->sendSpellOrDamageImmune(getCasterGuid(), getOwner(), getSpellId());

                return;
            }

            const auto powerType = static_cast<PowerType>(aurEff->getEffectMiscValue());

            // Calculate power amount
            const auto powerAmount = std::min(effectIntValue, static_cast<int32_t>(getOwner()->getPower(powerType)));
            getOwner()->modPower(powerType, -powerAmount);

            auto damage = powerAmount * getSpellInfo()->getEffectMultipleValue(aurEff->getEffectIndex());
            // Should do at least 1 damage (see Chromaggus and Brood Affliction: Blue)
            ///\ todo: verify this
            damage = std::max(1.0f, damage);

            if (casterUnit != nullptr)
                casterUnit->doSpellDamage(getOwner(), getSpellId(), damage, aurEff->getEffectIndex(), pSpellId != 0, true, false, false, nullptr, this, aurEff);
            else
                getOwner()->doSpellDamage(getOwner(), getSpellId(), damage, aurEff->getEffectIndex(), pSpellId != 0, true, false, false, nullptr, this, aurEff);
        } break;
#if VERSION_STRING >= TBC
        case SPELL_AURA_PERIODIC_TRIGGER_DUMMY:
        {
            // Drink spells use periodic dummy trigger since TBC
            const auto effIndex = aurEff->getEffectIndex();
            if (effIndex > 0 && getAuraEffect(effIndex - 1U)->getAuraEffectType() == SPELL_AURA_MOD_POWER_REGEN)
            {
                if (getPlayerOwner() == nullptr)
                    return;

                // Set this effect's value to the mana regen effect, so it will be removed when aura is removed
                m_auraEffects[effIndex - 1].setEffectDamage(aurEff->getEffectDamage());
                getPlayerOwner()->updateManaRegeneration();

                // Disable this periodic effect
                aurEff->setEffectAmplitude(0);
                return;
            }

            // Check that the dummy effect is handled properly in spell script
            // In case it's not, generate warning to debug log
            const auto dummyResult = sScriptMgr.callScriptedAuraOnDummyEffect(this, aurEff, true);
            if (dummyResult == SpellScriptCheckDummy::DUMMY_OK)
                break;

            if (sScriptMgr.CallScriptedDummyAura(getSpellId(), aurEff->getEffectIndex(), this, true))
                break;

            sLogger.debugFlag(AscEmu::Logging::LF_AURA_EFF, "Spell aura {} has a periodic trigger dummy effect but no handler for it", getSpellId());
        } break;
#endif
        default:
            break;
    }
}

// Absorb aura

AbsorbAura::AbsorbAura(SpellInfo* spellInfo, int32_t duration, Object* caster, Unit* target, bool temporary/* = false*/, Item* i_caster/* = nullptr*/) :
    Aura(spellInfo, duration, caster, target, temporary, i_caster) {}

Aura* AbsorbAura::Create(SpellInfo* spellInfo, int32_t duration, Object* caster, Unit* target, bool temporary/* = false*/, Item* i_caster/* = nullptr*/)
{
    return new AbsorbAura(spellInfo, duration, caster, target, temporary, i_caster);
}

uint32_t AbsorbAura::absorbDamage(SchoolMask schoolMask, uint32_t* dmg, bool checkOnly)
{
    // Check if aura can absorb this school
    if (!(m_absorbSchoolMask & schoolMask))
        return 0;

    // Absorb damage is checked before packets are sent but real absorption happens on health update
    if (checkOnly)
    {
        uint32_t absorbedDamage = 0;
        uint32_t damageToAbsorb = *dmg;

        // Check if aura absorbs only percantage of the damage
        if (m_pctAbsorbValue < 100)
            damageToAbsorb = damageToAbsorb * m_pctAbsorbValue / 100;

        auto absorbValue = getRemainingAbsorbAmount();
        if (damageToAbsorb >= absorbValue)
        {
            *dmg -= absorbValue;
            absorbedDamage = absorbValue;

            m_absorbDamageBatch = m_absorbValue;
        }
        else
        {
            *dmg -= damageToAbsorb;
            absorbedDamage = damageToAbsorb;

            m_absorbDamageBatch += damageToAbsorb;
        }

        return absorbedDamage;
    }
    else
    {
        if (m_absorbValue > m_absorbDamageBatch)
            m_absorbValue -= m_absorbDamageBatch;
        else
            m_absorbValue = 0;

        m_absorbDamageBatch = 0;

        if (m_absorbValue <= 0)
            removeAura();
    }

    return 0;
}

uint32_t AbsorbAura::getRemainingAbsorbAmount() const
{
    if (m_absorbValue == 0 || m_absorbDamageBatch > m_absorbValue)
        return 0;
    else
        return m_absorbValue - m_absorbDamageBatch;
}

uint32_t AbsorbAura::getTotalAbsorbAmount() const
{
    return m_totalAbsorbValue;
}

void AbsorbAura::spellAuraEffectSchoolAbsorb(AuraEffectModifier* aurEff, bool apply)
{
    if (!apply)
        return;

    auto absorbValue = calcAbsorbAmount(aurEff);

    m_totalAbsorbValue = absorbValue;
    m_absorbValue = absorbValue;
    m_pctAbsorbValue = CalcPctDamage();
    m_absorbSchoolMask = SchoolMask(aurEff->getEffectMiscValue());
}

bool AbsorbAura::isAbsorbAura() const
{
    return true;
}

uint32_t AbsorbAura::calcAbsorbAmount(AuraEffectModifier* aurEff)
{
    // Call for legacy script hook
    auto val = CalcAbsorbAmount(aurEff);

    const auto unitCaster = GetUnitCaster();
    if (unitCaster != nullptr && !aurEff->isEffectDamageStatic())
    {
        // Apply spell power coefficient
        val = static_cast<int32_t>(std::ceil(unitCaster->applySpellDamageBonus(getSpellInfo(), val, aurEff->getEffectPercentModifier(), false, nullptr, aurEff->getAura())));
    }

    return static_cast<uint32_t>(val);
}

int32_t AbsorbAura::CalcAbsorbAmount(AuraEffectModifier* aurEff) { return aurEff->getEffectDamage(); }
uint8_t AbsorbAura::CalcPctDamage() { return 100; }

